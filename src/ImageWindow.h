#pragma once
#include <QApplication>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <QPushButton>
#include <string>
#include <iomanip>
#include <complex>
#include <fftw3.h>

// Defined in imageUtilities.h
QImage cvMatToQImage(const cv::Mat &inMat);
cv::Mat QImageToCvMat(const QImage &inImage, bool inCloneImageData);
cv::Mat computeFFT(const cv::Mat& input);
void getSqrPart(float *sqrPart, int datLen, float pitch, float wavelength);
void getTransFunc(fftw_complex *transDz, float z_0, const float *sqrPart, int datLen);
void fftwmultipleArrays(fftw_complex *dst, const fftw_complex *src1, const fftw_complex *src2, int datLen);
void defineObj(bool *obj, int datLen, int dx, int dy, float rad, float pitch);
void makeholo(fftw_complex *holo, const bool *obj1, const bool *obj2, const fftw_complex *transDz, const fftw_complex *transZ0, int datLen, fftw_complex *holofft) ;

// struct InitParams {
//     int dx = 100;
//     int dy = 0;
//     float pitch = 10.0;
//     float wavelength = 0.6328;
//     int datLen = 512;
//     float dz = 0.0;
//     float z_0 = 220000.0;
//     float rad1 = 40.0;
//     float rad2 = 40.0;
// };

// struct InitParams;

// InitParams params;

class ImageWindow : public QWidget {
    Q_OBJECT
    public:
        ImageWindow(QWidget* parent = nullptr)
            : QWidget(parent), m_imageLabel(new QLabel), m_fftLabel(new QLabel), m_slider(new QSlider(Qt::Horizontal)), m_sliderLabel(new QLabel), m_saveButton(new QPushButton("Save Images")) {
            
            // Create layouts

            // Define main layout
            QVBoxLayout* mainlayout = new QVBoxLayout(this);

            // Define image layout
            QHBoxLayout* imglayout = new QHBoxLayout(this);
            imglayout->addWidget(m_imageLabel);
            imglayout->addWidget(m_fftLabel);

            // Define manipulation layout
            QVBoxLayout* maniplayout = new QVBoxLayout(this);

            // Define slider layout
            QVBoxLayout* sliderLayout = new QVBoxLayout(this);
            m_slider->setOrientation(Qt::Horizontal);
            m_slider->setRange(0,100);
            m_slider->setValue(0);

            updateBrightSliderLabel();
            connect(m_slider, &QSlider::valueChanged, this, [this]() {
                m_sliderLabel->setText("Brightness: " + QString::number(m_slider->value()));
            });

            sliderLayout->addWidget(m_sliderLabel);
            sliderLayout->addWidget(m_slider);

            // Define holo_dx layout
            holo_dx_slider = new QSlider(Qt::Horizontal);
            holo_dx_slider->setOrientation(Qt::Horizontal);
            holo_dx_slider->setRange(0,200);
            holo_dx_slider->setValue(params.dx);
            holo_dx_label = new QLabel("dx: "+QString::number(params.dy));
            connect(holo_dx_slider, &QSlider::valueChanged, this, [this]() {
                holo_dx_label->setText("dx: " + QString::number(holo_dx_slider->value()));
                params.dx = (float)holo_dx_slider->value();
                holoUpdate();
            });

            sliderLayout->addWidget(holo_dx_label);
            sliderLayout->addWidget(holo_dx_slider);

            // Define holo_dy layout
            holo_dy_slider = new QSlider(Qt::Horizontal);
            holo_dy_slider->setOrientation(Qt::Horizontal);
            holo_dy_slider->setRange(0,200);
            holo_dy_slider->setValue(params.dy);
            holo_dy_label = new QLabel("dy: "+QString::number(params.dy));
            connect(holo_dy_slider, &QSlider::valueChanged, this, [this]() {
                holo_dy_label->setText("dy: " + QString::number(holo_dy_slider->value()));
                params.dy = (float)holo_dy_slider->value();
                holoUpdate();
            });

            sliderLayout->addWidget(holo_dy_label);
            sliderLayout->addWidget(holo_dy_slider);

            // Define holo_rad1 layout
            holo_rad1_slider = new QSlider(Qt::Horizontal);
            holo_rad1_slider->setOrientation(Qt::Horizontal);
            holo_rad1_slider->setRange(0,200);
            holo_rad1_slider->setValue(params.rad1);
            holo_rad1_label = new QLabel("rad1: "+QString::number(params.rad1));
            connect(holo_rad1_slider, &QSlider::valueChanged, this, [this]() {
                holo_rad1_label->setText("rad1: " + QString::number(holo_rad1_slider->value()));
                params.rad1 = (float)holo_rad1_slider->value();
                holoUpdate();
            });

            sliderLayout->addWidget(holo_rad1_label);
            sliderLayout->addWidget(holo_rad1_slider);

            // Define holo_rad2 layout
            holo_rad2_slider = new QSlider(Qt::Horizontal);
            holo_rad2_slider->setOrientation(Qt::Horizontal);
            holo_rad2_slider->setRange(0,200);
            holo_rad2_slider->setValue(params.rad2);
            holo_rad2_label = new QLabel("rad2: "+QString::number(params.rad2));
            connect(holo_rad2_slider, &QSlider::valueChanged, this, [this]() {
                holo_rad2_label->setText("rad2: " + QString::number(holo_rad2_slider->value()));
                params.rad2 = (float)holo_rad2_slider->value();
                holoUpdate();
            });

            sliderLayout->addWidget(holo_rad2_label);
            sliderLayout->addWidget(holo_rad2_slider);

            // Define holo_z0 layout
            holo_z0_slider = new QSlider(Qt::Horizontal);
            holo_z0_slider->setOrientation(Qt::Horizontal);
            holo_z0_slider->setRange(0,2000000);
            holo_z0_slider->setValue(params.z_0);
            holo_z0_label = new QLabel("z_0: "+QString::number(params.z_0));
            connect(holo_z0_slider, &QSlider::valueChanged, this, [this]() {
                holo_z0_label->setText("z_0: " + QString::number(holo_z0_slider->value()));
                params.z_0 = (float)holo_z0_slider->value();
                holoUpdate();
            });

            sliderLayout->addWidget(holo_z0_label);
            sliderLayout->addWidget(holo_z0_slider);

            // Define holo_dz layout
            holo_dz_slider = new QSlider(Qt::Horizontal);
            holo_dz_slider->setOrientation(Qt::Horizontal);
            holo_dz_slider->setRange(0,10000);
            holo_dz_slider->setValue(params.dz);
            holo_dz_label = new QLabel("dz: "+QString::number(params.dz));
            connect(holo_dz_slider, &QSlider::valueChanged, this, [this]() {
                holo_dz_label->setText("dz: " + QString::number(holo_dz_slider->value()));
                params.dz = (float)holo_dz_slider->value();
                holoUpdate();
            });

            sliderLayout->addWidget(holo_dz_label);
            sliderLayout->addWidget(holo_dz_slider);

            // Commit manipulation layout
            maniplayout->addLayout(sliderLayout);
            


            // Commit layouts
            mainlayout->addLayout(imglayout);
            mainlayout->addLayout(maniplayout);

            // Commit save button
            mainlayout->addWidget(m_saveButton);
            connect(m_saveButton, &QPushButton::clicked, this, [this]() {
                time_t t = time(nullptr);
                const tm* lt = localtime(&t);
                std::stringstream ss;
                ss << std::put_time(lt, "%Y%m%d_%H%M%S");
                std::string date = ss.str();
                std::string filename = "./out/holo/" + date + ".png";
                std::string fftFilename = "./out/fft/" + date + ".png";
                m_image.save(filename.c_str());
                m_fftImage.save(fftFilename.c_str());
            });

            // Calculate the initial hologram
            sqrPart = new float[params.datLen * params.datLen];
            obj1 = new bool[params.datLen * params.datLen];
            obj2 = new bool[params.datLen * params.datLen];
            transDz = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * params.datLen * params.datLen);
            transZ0 = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * params.datLen * params.datLen);
            holo = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * params.datLen * params.datLen);
            holofft = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * params.datLen * params.datLen);

            getSqrPart(sqrPart, params.datLen, params.pitch, params.wavelength);
            getTransFunc(transDz, params.dz, sqrPart, params.datLen);
            getTransFunc(transZ0, params.z_0, sqrPart, params.datLen);

            defineObj(obj1, params.datLen, params.dx/2.0, params.dy/2.0, params.rad1, params.pitch);
            defineObj(obj2, params.datLen, -1.0*params.dx/2.0, -1.0*params.dy/2.0, params.rad2, params.pitch);

            makeholo(holo, obj1, obj2, transDz, transZ0, params.datLen, holofft);

            // Load the image
            holo8 = new unsigned char[params.datLen * params.datLen];
            for (int i = 0; i < params.datLen * params.datLen; i++) {
                holo8[i] = (unsigned char) (128*(holo[i][0] * holo[i][0] + holo[i][1] * holo[i][1]));
            }

            // m_image = QImage("./mono.png");
            m_image = QImage(holo8, params.datLen, params.datLen, QImage::Format_Grayscale8);
            if (m_image.isNull()) {
               qFatal("Failed to load image.png");
            }
            m_imageLabel->setPixmap(QPixmap::fromImage(m_image));
            changeBrightness(0);

            // Connect the slider valueChanged signal to our slot
            connect(m_slider, &QSlider::valueChanged, this, &ImageWindow::changeBrightness);
        }

        ~ImageWindow() {
            delete m_imageLabel;
            delete m_fftLabel;
            delete m_slider;
            delete m_sliderLabel;
            delete m_saveButton;
            delete sqrPart;
            delete obj1;
            delete obj2;
            delete holo8;
            fftw_free(transDz);
            fftw_free(transZ0);
            fftw_free(holo);
            fftw_free(holofft);

            delete holo_dx_slider;
            delete holo_dx_label;
            delete holo_dy_slider;
            delete holo_dy_label;
            delete holo_rad1_slider;
            delete holo_rad1_label;
            delete holo_rad2_slider;
            delete holo_rad2_label;
            delete holo_z0_slider;
            delete holo_z0_label;
            delete holo_dz_slider;
            delete holo_dz_label;
        }

    public slots:
        void changeBrightness(int value) {
            QImage newImage = m_image;  // Copy the original image
            // Apply brightness change
            for (int y = 0; y < m_image.height(); ++y) {
                for (int x = 0; x < m_image.width(); ++x) {
                    QColor color = m_image.pixelColor(x, y);
                    int newRed = std::clamp(color.red() + value, 0, 255);
                    int newGreen = std::clamp(color.green() + value, 0, 255);
                    int newBlue = std::clamp(color.blue() + value, 0, 255);
                    newImage.setPixelColor(x, y, QColor(newRed, newGreen, newBlue));
                }
            }
            m_imageLabel->setPixmap(QPixmap::fromImage(newImage));
    
            m_fftImage = cvMatToQImage(computeFFT(QImageToCvMat(newImage, true)));
            m_fftLabel->setPixmap(QPixmap::fromImage(m_fftImage));
        }

        void updateBrightSliderLabel() {
            QString sliderText = "Brightness: ";
            m_sliderLabel->setText(sliderText + QString::number(m_slider->value()));
        }

        void holoUpdate() {
            getSqrPart(sqrPart, params.datLen, params.pitch, params.wavelength);
            getTransFunc(transDz, params.dz, sqrPart, params.datLen);
            getTransFunc(transZ0, params.z_0, sqrPart, params.datLen);

            defineObj(obj1, params.datLen, params.dx/2.0, params.dy/2.0, params.rad1, params.pitch);
            defineObj(obj2, params.datLen, -1.0*params.dx/2.0, -1.0*params.dy/2.0, params.rad2, params.pitch);

            makeholo(holo, obj1, obj2, transDz, transZ0, params.datLen, holofft);

            // Load the image
            for (int i = 0; i < params.datLen * params.datLen; i++) {
                holo8[i] = (unsigned char) (128*(holo[i][0] * holo[i][0] + holo[i][1] * holo[i][1]));
            }

            // m_image = QImage("./mono.png");
            QImage new_image = QImage(holo8, params.datLen, params.datLen, QImage::Format_Grayscale8);
            if (new_image.isNull()) {
               qFatal("Failed to load holo");
            }
            m_imageLabel->setPixmap(QPixmap::fromImage(new_image));
            m_fftImage = cvMatToQImage(computeFFT(QImageToCvMat(new_image, true)));
            m_fftLabel->setPixmap(QPixmap::fromImage(m_fftImage));
        }

    private:
        QLabel* m_imageLabel;
        QLabel* m_fftLabel;
        QSlider* m_slider;
        QLabel* m_sliderLabel;
        QImage m_image;
        QImage m_fftImage;
        QPushButton* m_saveButton;
        QSlider* holo_dx_slider;
        QLabel* holo_dx_label;
        QSlider* holo_dy_slider;
        QLabel* holo_dy_label;
        QSlider* holo_rad1_slider;
        QLabel* holo_rad1_label;
        QSlider* holo_rad2_slider;
        QLabel* holo_rad2_label;
        QSlider* holo_z0_slider;
        QLabel* holo_z0_label;
        QSlider* holo_dz_slider;
        QLabel* holo_dz_label;

        // For holo calculation
        float *sqrPart;
        fftw_complex *transDz;
        fftw_complex *transZ0;
        fftw_complex *holo;
        fftw_complex *holofft;
        bool *obj1;
        bool *obj2;
        unsigned char *holo8;

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
        InitParams params;
};