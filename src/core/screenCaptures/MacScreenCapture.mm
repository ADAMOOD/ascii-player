#import "core/screenCaptures/MacScreenCapture.h"
#import <CoreGraphics/CoreGraphics.h>
#include <cstddef>

cv::Mat MacScreenCapture::grabScreen() {
  auto monitorId = CGMainDisplayID();

// ignor warnings
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

  CGImageRef image = CGDisplayCreateImage(monitorId);

#pragma clang diagnostic pop

  if (!image)
    return cv::Mat();
  size_t width = CGImageGetWidth(image);
  size_t height = CGImageGetHeight(image);
  size_t bytesPerRow = CGImageGetBytesPerRow(image);
  auto provider = CGImageGetDataProvider(image);
  CFDataRef data = CGDataProviderCopyData(provider);

  const UInt8 *rawPixels = CFDataGetBytePtr(data);
  cv::Mat tempMat(height, width, CV_8UC4, (void *)rawPixels, bytesPerRow);
  cv::Mat finalMat = tempMat.clone();
  cv::cvtColor(finalMat, finalMat, cv::COLOR_BGRA2BGR);

  CFRelease(data);
  CGImageRelease(image);

  return finalMat;
}