#include "core/AsciiEngine.h"
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "core/ConfigManager.h"
#include "strategies/StrategiesFactory.h"

void enableRawMode()
{
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);

    term.c_lflag &= ~(ICANON | ECHO);

    // Nové nastavení pro absolutně neblokující čtení (funguje 100% i ve WSL)
    term.c_cc[VMIN] = 0;  // Nečekej na žádný minimální počet znaků
    term.c_cc[VTIME] = 0; // Časový limit čekání je 0

    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void disableRawMode()
{
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

bool AsciiEngine::init(const std::string &videoPath)
{
    m_cap.open(videoPath);
    if (!m_cap.isOpened())
    {
        std::cerr << "Chyba: Nelze otevrit video: " << videoPath << std::endl;
        return false;
    }
    auto strategy = ConfigManager::getValFromSettings("render_strategy");
    this->setStrategy(strategy);
    m_menuStartIndex = 0;
    m_selectedPropertyIndex = 0;
    double origWidth = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double origHeight = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    m_aspectRatio = origWidth / origHeight;
    m_width = 0;
    updateTerminalSize();
    return true;
}
void AsciiEngine::updateTerminalSize()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    int termW = w.ws_col;
    int termH = w.ws_row - 1; // prevent the last line from scrolling when we print the buffer and to add menu

    int newWidth = termW;
    int newHeight = static_cast<int>((newWidth / m_aspectRatio) * 0.5);

    // if the video is too tall for the terminal, we need to limit the height and recalculate the width
    if (newHeight > termH)
    {
        newHeight = termH;
        newWidth = static_cast<int>((newHeight / 0.5) * m_aspectRatio);
    }

    // if the size has changed, we need to update the buffer
    if (newWidth != m_width || newHeight != m_height)
    {
        m_width = newWidth;
        m_height = newHeight;

        int totalBufferSize = (m_width + 1) * m_height;
        m_frameBuffer.assign(totalBufferSize, ' ');

        for (int y = 0; y < m_height; ++y)
        {
            int newlineIndex = y * (m_width + 1) + m_width;
            m_frameBuffer[newlineIndex] = '\n';
        }
    }
}
void AsciiEngine::frameProducerTask()
{
    while (this->m_isRunning)
    {
        cv::Mat tmp;
        m_cap.read(tmp);
        if (tmp.empty())
        {
            m_isRunning = false;
            m_frameReady.notify_one();
            break;
        }
        std::unique_lock<std::mutex> uniqueLock(m_queueMutex);
        m_queueNotFull.wait(uniqueLock, [&]
                            { return m_frames.size() < MAX_QUEUE_SIZE || !m_isRunning; });
        m_frames.push(tmp);
        uniqueLock.unlock();
        m_frameReady.notify_one();
    }
}
void AsciiEngine::play()
{
    enableRawMode();
    m_isRunning = true;
    // Start the background thread
    m_videoProcessingThread = std::thread(&AsciiEngine::frameProducerTask, this);
    cv::Mat grayFrame;
    cv::Mat resizedFrame;
    std::cout << "\x1b[2J\x1b[?25l";
    while (m_isRunning)
    {
        updateTerminalSize();

        cv::Mat frame = fetchFrameFromQueue();
        if (frame.empty())
            break;

        processFrameToBuffer(frame);

        renderBuffer();
        renderHUD();

        syncFramerate();

        checkUserInput();
    }
    disableRawMode();
    if (m_videoProcessingThread.joinable())
    {
        m_videoProcessingThread.join();
    }

    // clear the frame queue to free memory
    std::unique_lock<std::mutex> lock(m_queueMutex);
    while (!m_frames.empty())
    {
        m_frames.pop();
    }
    lock.unlock();

    std::cout << "\x1b[?25h";
}
void AsciiEngine::renderHUD()
{
    if (m_activeProperties.empty() || !m_currentStrategy)
        return;

    std::cout << "\x1b[2K"; // clear the properties line

    if (m_selectedPropertyIndex < m_menuStartIndex)
    {
        m_menuStartIndex = m_selectedPropertyIndex;
    }

    int totalWidthToCursor = 0;
    for (int i = m_menuStartIndex; i <= m_selectedPropertyIndex; ++i)
    {
        std::string plainText = m_activeProperties[i].toString();
        totalWidthToCursor += plainText.length();

        if (totalWidthToCursor > m_width)
        {
            m_menuStartIndex++;
            i = m_menuStartIndex - 1; // Restart cyklu s novým startem
            totalWidthToCursor = 0;
        }
    }

    // FÁZE 3: VYKRESLENÍ (Nyní víme, že od m_menuStartIndex můžeme bezpečně kreslit)
    int visibleChars = 0;

    for (size_t i = m_menuStartIndex; i < m_activeProperties.size(); ++i)
    {
        std::string plainText = m_activeProperties[i].toString();
        int propLength = plainText.length();

        if (visibleChars + propLength > m_width)
        {
            break;
        }
        visibleChars += propLength;
        // Tisk vybrané položky (Invertované barvy) vs Tisk normální položky
        if (i == static_cast<size_t>(m_selectedPropertyIndex))
        {
            std::cout << "\x1b[7m" << plainText << "\x1b[0m";
        }
        else
        {
            std::cout << plainText;
        }
    }
    std::cout << std::flush;
}

cv::Mat AsciiEngine::fetchFrameFromQueue()
{
    cv::Mat frame;
    std::unique_lock<std::mutex> uniqueLock(m_queueMutex);

    m_frameReady.wait(uniqueLock, [&]
                      { return !m_frames.empty() || !m_isRunning; });

    // Pokud se vypina a fronta je prazdna, vratime prazdny frame
    if (!m_isRunning && m_frames.empty())
        return cv::Mat();

    frame = m_frames.front();
    m_frames.pop();
    uniqueLock.unlock();
    m_queueNotFull.notify_one();

    return frame;
}
void AsciiEngine::setStrategy(std::string newStrategy)
{
    // unique_ptr cannot be copied (to prevent double-free crashes).
    // std::move explicitly transfers ownership from 'newStrategy' to 'm_currentStrategy'.
    m_currentStrategy = std::move(StrategiesFactory::createStrategy(newStrategy));
    m_activeProperties = m_currentStrategy->getProperties();
}
void AsciiEngine::processFrameToBuffer(const cv::Mat &frame)
{
    if (m_currentStrategy)
    {
        m_currentStrategy->render(frame, m_frameBuffer, m_width, m_height);
    }
}
void AsciiEngine::renderBuffer()
{
    std::cout << "\x1b[H" << m_frameBuffer << std::flush; // flush is important to force the output to be printed immediately, otherwise it might be buffered and cause lag
}

void AsciiEngine::syncFramerate()
{
    // TODO
    std::this_thread::sleep_for(std::chrono::milliseconds(33));
}
void AsciiEngine::checkUserInput()
{
    char c;
    // Funkce read zkusí přečíst 1 bajt ze standardního vstupu (0).
    // Protože jsme v O_NONBLOCK režimu, pokud není klávesa stisknutá,
    // read okamžitě vrátí -1 a program jede plynule dál.
    // Pokud je klávesa stisknutá, vrátí 1 a znak se uloží do proměnné 'c'.
    if (read(STDIN_FILENO, &c, 1) == 1)
    {
        if (c == 'q' || c == 'Q')
        {
            m_isRunning = false;
            m_frameReady.notify_one();
            m_queueNotFull.notify_one();
        }
        switch (c)
        {
        case 'a':
        {
            if (m_selectedPropertyIndex > 0)
                m_selectedPropertyIndex--;
            break;
        }
        case 'd':
        {
            if (!m_activeProperties.empty() && m_selectedPropertyIndex < static_cast<int>(m_activeProperties.size()) - 1)
                m_selectedPropertyIndex++;
            break;
        }
        case 'w':
        case 's':
        {
            Property prop = m_activeProperties[m_selectedPropertyIndex];
            prop.ShiftedValue(c == 'w');
            m_currentStrategy->setProperty(prop);
            m_activeProperties = m_currentStrategy->getProperties();

            // BEZPEČNOSTNÍ POJISTKA (Kriticky důležité!)
            // Pokud jsi právě vypnul Hysterezi, zmizely ti dvě vlastnosti (Low a High).
            // Pokud jsi zrovna stál na konci menu, tvůj m_selectedPropertyIndex teď ukazuje mimo vektor!
            if (m_activeProperties.empty())
            {
                m_selectedPropertyIndex = 0;
            }
            else if (m_selectedPropertyIndex >= static_cast<int>(m_activeProperties.size()))
            {
                m_selectedPropertyIndex = m_activeProperties.size() - 1;
            }

            break;
        }
        default:
            break;
        }
    }
}