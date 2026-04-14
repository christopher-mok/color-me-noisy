#include "image_utils.h"
#include <QImage>
#include <vector>

Image ImageUtils::readImage(const QString& path, bool isFrame) {
    QImage qimg(path);
    if (qimg.isNull()) {
        return Image();  // Return empty image on failure
    }

    Image img;
    img.width = qimg.width();
    img.height = qimg.height();
    img.isFrame = isFrame;
    img.pixels.resize(img.width * img.height);

    // Convert QImage pixels to RGB floats (0.0-1.0 range)
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            QRgb qrbg = qimg.pixel(x, y);
            int index = y * img.width + x;
            img.pixels[index].r = qRed(qrbg) / 255.0f;
            img.pixels[index].g = qGreen(qrbg) / 255.0f;
            img.pixels[index].b = qBlue(qrbg) / 255.0f;
            img.pixels[index].a = qAlpha(qrbg) / 255.0f;
        }
    }

    return img;
}

void ImageUtils::writeImage(Image& img, const QString& path) {
    QImage qimg(img.width, img.height, QImage::Format_ARGB32);
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            int index = y * img.width + x;
            RGB pixel = img.pixels[index];
            QRgb qrbg = qRgba(
                static_cast<int>(pixel.r * 255),
                static_cast<int>(pixel.g * 255),
                static_cast<int>(pixel.b * 255),
                static_cast<int>(pixel.a * 255)
            );
            qimg.setPixel(x, y, qrbg);
        }
    }
    qimg.save(path);
}

RGB ImageUtils::rgbAt(const Image& img, int x, int y){
    int ind = img.width * y + x;
    RGB rgbVal = img.pixels[ind];
    return rgbVal;
}
