#include <QImage>
#include <QtCore/QDebug>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fftw3.h>
#define PI 3.14159265359

cv::Mat computeFFT(const cv::Mat& input) {
    CV_Assert(input.type() == CV_8UC1); // 入力画像がグレースケールであることを確認

    // 入力を浮動小数点型に変換
    cv::Mat floatingInput;
    input.convertTo(floatingInput, CV_32F);

    // コンプレックス数のプレーンを準備
    cv::Mat planes[] = {floatingInput, cv::Mat::zeros(input.size(), CV_32F)};
    cv::Mat complexImg;
    cv::merge(planes, 2, complexImg);

    // 離散フーリエ変換 (DFT)
    cv::dft(complexImg, complexImg);

    // 実部と虚部に分割
    cv::split(complexImg, planes);
    cv::Mat mag;
    cv::magnitude(planes[0], planes[1], mag);

    // 値に1を足して対数を取る
    mag += 1;
    cv::log(mag, mag);

    // スペクトルの中心を移動
    int cx = mag.cols / 2;
    int cy = mag.rows / 2;
    cv::Mat q0(mag, cv::Rect(0, 0, cx, cy));
    cv::Mat q1(mag, cv::Rect(cx, 0, cx, cy));
    cv::Mat q2(mag, cv::Rect(0, cy, cx, cy));
    cv::Mat q3(mag, cv::Rect(cx, cy, cx, cy));
    cv::Mat tmp;
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);

    // 最大値最小値でノーマライズ
    cv::normalize(mag, mag, 0, 255, cv::NORM_MINMAX, CV_8U);
    // floatingInput.convertTo(mag, CV_8UC1);

    return mag;
}


cv::Mat QImageToCvMat(const QImage &inImage, bool inCloneImageData = true) {
    switch (inImage.format()) {
        // 8-bit, 4 channel
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied: {
            cv::Mat mat(inImage.height(), inImage.width(), CV_8UC4, const_cast<uchar*>(inImage.bits()), static_cast<size_t>(inImage.bytesPerLine()));

            return (inCloneImageData ? mat.clone() : mat);
        }

        // 8-bit, 3 channel
        case QImage::Format_RGB32: {
            if (!inCloneImageData) {
                qWarning() << "QImageToCvMat() - Conversion requires cloning so we don't modify the original QImage data";
            }

            cv::Mat mat(inImage.height(), inImage.width(), CV_8UC4, const_cast<uchar*>(inImage.bits()), static_cast<size_t>(inImage.bytesPerLine()));
            cv::Mat matNoAlpha;

            cv::cvtColor(mat, matNoAlpha, cv::COLOR_BGRA2BGR); // Remove alpha channel

            return matNoAlpha;
        }

        // 8-bit, 1 channel (grayscale)
        case QImage::Format_Grayscale8: {
            cv::Mat mat(inImage.height(), inImage.width(), CV_8UC1, const_cast<uchar*>(inImage.bits()), static_cast<size_t>(inImage.bytesPerLine()));

            return (inCloneImageData ? mat.clone() : mat);
        }

        default:
            qWarning() << "QImageToCvMat() - QImage format not handled in switch:" << inImage.format();
            break;
    }

    return cv::Mat();
}

QImage cvMatToQImage(const cv::Mat &inMat) {
    switch (inMat.type()) {
        // 8ビット、単一チャンネル（グレースケール）
        case CV_8UC1: {
            QImage image(inMat.data, inMat.cols, inMat.rows, static_cast<int>(inMat.step), QImage::Format_Grayscale8);
            return image.copy(); // QImageは入力画像のデータを共有するので、安全に扱うためにコピーする
        }
        
        // 8ビット、3チャンネル（カラー）
        case CV_8UC3: {
            QImage image(inMat.data, inMat.cols, inMat.rows, static_cast<int>(inMat.step), QImage::Format_RGB888);
            return image.rgbSwapped(); // OpenCVはBGRフォーマットを使用するため、RGBに変換する
        }

        default:
            qWarning() << "cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
            break;
    }

    return QImage();
}

void getSqrPart(float *sqrPart, int datLen, float pitch, float wavelength) {
    for (int y=0; y<datLen; y++) {
        for (int x=0; x<datLen; x++) {
            sqrPart[y*datLen + x] = 2.0*PI/wavelength* sqrtf(1.0f - (wavelength*wavelength/pitch/pitch/(float)datLen/(float)datLen) * ((float)((x - datLen/2.0) * (x - datLen/2.0) + (y - datLen/2.0) * (y - datLen/2.0))));
            // std::cout << sqrPart[y*datLen + x] << std::endl;
        }
    }
}

void getTransFunc(fftw_complex *transDz, float z_0, const float *sqrPart, int datLen){
    for (int y=0; y<datLen; y++) {
        for (int x=0; x<datLen; x++) {
            transDz[y*datLen + x][0] = cosf(z_0 * sqrPart[y*datLen + x]);
            transDz[y*datLen + x][1] = sinf(z_0 * sqrPart[y*datLen + x]);
            // std::cout << transDz[y*datLen + x][0] << std::endl;
        }
    }
}

void fftwmultipleArrays(fftw_complex *dst, const fftw_complex *src1, const fftw_complex *src2, int datLen) {
    for (int i = 0; i < datLen*datLen; i++) {
        fftw_complex tmp;
        tmp[0] = src1[i][0] * src2[i][0] - src1[i][1] * src2[i][1];
        tmp[1] = src1[i][0] * src2[i][1] + src1[i][1] * src2[i][0];
        dst[i][0] = tmp[0];
        dst[i][1] = tmp[1];
    }
}

void defineObj(bool *obj, int datLen, int dx, int dy, float rad, float pitch) {
    for (int y=0; y<datLen; y++) {
        for (int x=0; x<datLen; x++) {
            if ((float)((x - dx/2.0 - datLen/2.0) * (x - dx/2.0 - datLen/2.0) + (y - dy/2.0 - datLen/2.0) * (y - dy/2.0 - datLen/2.0)) <= rad*rad/pitch/pitch) {
                obj[y*datLen + x] = (bool)false;
            } else {
                obj[y*datLen + x] = (bool)true;
            }
        }
    }
}

void fftshift2d(fftw_complex* data, int datLen) {
    int halflen = datLen/2;

    // 一時的なバッファを確保
    fftw_complex temp;

    // 象限を入れ替える
    for (int y = 0; y < halflen; y++) {
        for (int x = 0; x < halflen; x++) {
            int srcX = x;
            int srcY = y;
            int dstIndex = (y+halflen) * datLen + (x+halflen);
            int srcIndex = srcY * datLen + srcX;

            temp[0] = data[srcIndex][0]; // 実部
            temp[1] = data[srcIndex][1]; // 虚部
            data[srcIndex][0] = data[dstIndex][0];
            data[srcIndex][1] = data[dstIndex][1];
            data[dstIndex][0] = temp[0];
            data[dstIndex][1] = temp[1];

            srcX = x + halflen;
            srcY = y;
            dstIndex = (y+halflen) * datLen + x;
            srcIndex = srcY * datLen + srcX;

            temp[0] = data[srcIndex][0]; // 実部
            temp[1] = data[srcIndex][1]; // 虚部
            data[srcIndex][0] = data[dstIndex][0];
            data[srcIndex][1] = data[dstIndex][1];
            data[dstIndex][0] = temp[0];
            data[dstIndex][1] = temp[1];
        }
    }
}

void fftDiv(fftw_complex *data, int datLen) {
    for (int idx=0; idx<datLen*datLen; idx++) {
        data[idx][0] /= (float)(datLen*datLen);
        data[idx][1] /= (float)(datLen*datLen);
    }
}

void makeholo(fftw_complex *holo, const bool *obj1, const bool *obj2, const fftw_complex *transDz, const fftw_complex *transZ0, int datLen, fftw_complex *holofft) {
    for (int y=0; y<datLen; y++) {
        for (int x=0; x<datLen; x++) {
            holo[y*datLen + x][0] = (float)(obj1[y*datLen + x]);
            holo[y*datLen + x][1] = 0.0;
        }
    }

    fftw_plan plan = fftw_plan_dft_2d(datLen, datLen, holo, holofft, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_plan plan2 = fftw_plan_dft_2d(datLen, datLen, holofft, holo, FFTW_BACKWARD, FFTW_ESTIMATE);

    fftw_execute(plan);
    
    fftshift2d(holofft, datLen);

    fftwmultipleArrays(holofft, holofft, transDz, datLen);

    fftshift2d(holofft, datLen);

    fftw_execute(plan2);

    fftDiv(holo, datLen);

    for (int y=0; y<datLen; y++) {
        for (int x=0; x<datLen; x++) {
            if (!obj2[y*datLen + x]) {
                holo[y*datLen + x][0] = (float)(obj2[y*datLen + x]);
                holo[y*datLen + x][1] = 0.0;
            }            
        }
    }

    fftw_execute(plan);

    fftshift2d(holofft, datLen);

    fftwmultipleArrays(holofft, holofft, transZ0, datLen);

    fftshift2d(holofft, datLen);

    fftw_execute(plan2);

    fftDiv(holo, datLen);

    fftw_destroy_plan(plan);
    fftw_destroy_plan(plan2);
    fftw_cleanup();
}