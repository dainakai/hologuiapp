#include "ImageWindow.h"
#include "imageUtilities.h"
#include <thread>
#include <queue>
#include <mutex>
#include <complex>

std::queue<cv::Mat> imageQueue;
std::mutex imageQueueMutex;

const int MAX_QUEUE_SIZE = 3;

struct InitParams {
    int dx = 100;
    int dy = 0;
    float pitch = 10.0;
    float wavelength = 0.6328;
    int datLen = 512;
    float dz = 0.0;
    float z_0 = 220000.0;
    float rad1 = 40.0;
    float rad2 = 40.0;
};

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    ImageWindow window(nullptr);
    window.show();

    return app.exec();
}
