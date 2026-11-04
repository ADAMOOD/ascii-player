#import "core/overlay/screenCaptures/MacScreenCapture.h"
#import <CoreGraphics/CoreGraphics.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <ScreenCaptureKit/ScreenCaptureKit.h>
#include <iostream>
#include <mutex>
#include <opencv2/imgproc.hpp>
#include <unistd.h>

@interface SCKStreamDelegate : NSObject <SCStreamOutput, SCStreamDelegate>
@property(atomic, assign) MacScreenCapture* cppInstance;
@end

@implementation SCKStreamDelegate

- (void)stream:(SCStream*)stream
    didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
                   ofType:(SCStreamOutputType)type {
    MacScreenCapture* instance = self.cppInstance;
    if (!instance || type != SCStreamOutputTypeScreen)
        return;
    if (!CMSampleBufferIsValid(sampleBuffer))
        return;

    // Only frames with status active are processed
    CFArrayRef attachments = CMSampleBufferGetSampleAttachmentsArray(sampleBuffer, false);
    if (!attachments || CFArrayGetCount(attachments) == 0)
        return;
    NSDictionary* info = (__bridge NSDictionary*) CFArrayGetValueAtIndex(attachments, 0);
    NSNumber* status = info[SCStreamFrameInfoStatus];
    if (!status || status.integerValue != SCFrameStatusComplete)
        return;

    CVPixelBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    if (pixelBuffer == NULL)
        return;

    if (CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly) != kCVReturnSuccess)
        return;

    void* dataPointer = CVPixelBufferGetBaseAddress(pixelBuffer);
    if (dataPointer == NULL) {
        CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
        return;
    }

    const size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);
    const int height = (int) CVPixelBufferGetHeight(pixelBuffer);
    const int width = (int) CVPixelBufferGetWidth(pixelBuffer);

    cv::Mat wrapped(height, width, CV_8UC4, dataPointer, bytesPerRow);
    cv::Mat bgr;
    cv::cvtColor(wrapped, bgr, cv::COLOR_BGRA2BGR);

    CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);

    instance->enqueueFrame(bgr);
}

- (void)stream:(SCStream*)stream didStopWithError:(NSError*)error {
    std::cerr << "[MacScreenCapture] stream stopped with error: "
              << error.localizedDescription.UTF8String << std::endl;
}
@end

MacScreenCapture::~MacScreenCapture() {
    stop();
}

void MacScreenCapture::enqueueFrame(cv::Mat frame) {
    {
        std::unique_lock<std::mutex> uniqueLock(m_queueMutex);
        if (m_frames.size() >= MAX_QUEUE_SIZE) { // only 30 most recent frames
            m_frames.pop();
        }
        m_frames.push(std::move(frame));
    }
    m_frameReady.notify_one();
}

void MacScreenCapture::start() {
    if (m_isCaptureRunning)
        return;

    // permissions
    if (!CGPreflightScreenCaptureAccess()) {
        std::cerr << "[MacScreenCapture] Screen Recording permission missing. "
                     "Enable it in System Settings > Privacy & Security > Screen Recording "
                     "and restart the app.\n";
        CGRequestScreenCaptureAccess();
    }

    m_isCaptureRunning = true;
    const pid_t myPid = getpid();

    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
      SCShareableContent* content = nil;
      SCRunningApplication* myApp = nil;

      for (int attempt = 0; attempt < 50 && m_isCaptureRunning; ++attempt) {
          dispatch_semaphore_t sem = dispatch_semaphore_create(0);
          __block SCShareableContent* fetched = nil;
          __block NSError* fetchError = nil;

          [SCShareableContent
              getShareableContentExcludingDesktopWindows:NO
                                     onScreenWindowsOnly:NO
                                       completionHandler:^(
                                           SCShareableContent* _Nullable c, NSError* _Nullable e
                                       ) {
                                         fetched = c;
                                         fetchError = e;
                                         dispatch_semaphore_signal(sem);
                                       }];
          dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC));

          if (fetchError || !fetched) {
              std::cerr << "[MacScreenCapture] error when loading content: "
                        << (fetchError ? fetchError.localizedDescription.UTF8String : "timeout")
                        << std::endl;
              m_isCaptureRunning = false;
              m_frameReady.notify_all();
              return;
          }

          content = fetched;
          for (SCRunningApplication* app in content.applications) {
              if (app.processID == myPid) {
                  myApp = app;
                  break;
              }
          }
          if (myApp)
              break;
          usleep(100 * 1000);
      }

      if (!m_isCaptureRunning || !content)
          return;

      SCDisplay* display = nil;
      for (SCDisplay* d in content.displays) {
          if (d.displayID == CGMainDisplayID()) {
              display = d;
              break;
          }
      }
      if (!display)
          display = content.displays.firstObject;
      if (!display) {
          std::cerr << "[MacScreenCapture] no display found\n";
          m_isCaptureRunning = false;
          m_frameReady.notify_all();
          return;
      }

      SCContentFilter* filter = nil;
      if (myApp) {
          // exclude whole app and all other processes that are created afterwards DEBUG including
          filter = [[SCContentFilter alloc] initWithDisplay:display
                                      excludingApplications:@[ myApp ]
                                           exceptingWindows:@[]];
          std::cout << "[MacScreenCapture] excluding application PID " << myPid << "\n";
      } else {
          std::cerr << "[MacScreenCapture] own app not found in shareable content, "
                       "falling back to window exclusion\n";
          NSMutableArray<SCWindow*>* excludedWindows = [NSMutableArray array];
          for (SCWindow* win in content.windows) {
              if (win.owningApplication.processID == myPid)
                  [excludedWindows addObject:win];
          }
          filter = [[SCContentFilter alloc] initWithDisplay:display
                                           excludingWindows:excludedWindows];
      }

      SCStreamConfiguration* config = [[SCStreamConfiguration alloc] init];
      config.width = display.width;
      config.height = display.height;
      config.pixelFormat = kCVPixelFormatType_32BGRA;
      config.showsCursor = NO;
      config.queueDepth = 3;
      config.minimumFrameInterval = CMTimeMake(1, 30); // max 30 fps

      SCKStreamDelegate* delegate = [[SCKStreamDelegate alloc] init];
      delegate.cppInstance = this;

      SCStream* stream = [[SCStream alloc] initWithFilter:filter
                                            configuration:config
                                                 delegate:delegate];

      dispatch_queue_t queue =
          dispatch_queue_create("com.ascii-player.stream", DISPATCH_QUEUE_SERIAL);

      NSError* addError = nil;
      BOOL added = [stream addStreamOutput:delegate
                                      type:SCStreamOutputTypeScreen
                        sampleHandlerQueue:queue
                                     error:&addError];
      if (!added) {
          std::cerr << "[MacScreenCapture] addStreamOutput failed: "
                    << addError.localizedDescription.UTF8String << std::endl;
          m_isCaptureRunning = false;
          m_frameReady.notify_all();
          return;
      }

      {
          std::lock_guard<std::mutex> lock(m_streamMutex);
          if (!m_isCaptureRunning) // stop() occured when init
              return;
          m_stream = (__bridge_retained void*) stream;
          m_delegate = (__bridge_retained void*) delegate;
      }

      [stream startCaptureWithCompletionHandler:^(NSError* _Nullable error) {
        if (error) {
            std::cerr << "[MacScreenCapture] capture start failed: "
                      << error.localizedDescription.UTF8String << std::endl;
            m_isCaptureRunning = false;
            m_frameReady.notify_all();
        }
      }];
    });
}

void MacScreenCapture::stop() {
    m_isCaptureRunning = false;
    m_frameReady.notify_all();

    void* rawStream = nullptr;
    void* rawDelegate = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_streamMutex);
        rawStream = m_stream;
        rawDelegate = m_delegate;
        m_stream = nullptr;
        m_delegate = nullptr;
    }
    if (!rawStream)
        return;

    // __bridge_transfer = vrátí vlastnictví ARC, objekty se uvolní na konci scope.
    SCStream* stream = (__bridge_transfer SCStream*) rawStream;
    SCKStreamDelegate* delegate = (__bridge_transfer SCKStreamDelegate*) rawDelegate;

    // Počkáme (max 2 s), až se capture opravdu zastaví, ať žádný callback nepřijde
    // do už zničené C++ instance.
    dispatch_semaphore_t sem = dispatch_semaphore_create(0);
    [stream stopCaptureWithCompletionHandler:^(NSError* _Nullable error) {
      dispatch_semaphore_signal(sem);
    }];
    dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC));

    [stream removeStreamOutput:delegate type:SCStreamOutputTypeScreen error:nil];
    delegate.cppInstance = nullptr;
}