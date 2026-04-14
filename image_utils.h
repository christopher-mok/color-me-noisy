#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include <QImage>
#include <vector>

struct RGB{
    float r;
    float b;
    float g;
    float a;
};

struct Image{
    std::vector<RGB> pixels;
    int width;
    int height;
    bool isFrame;
    int frameNumber;
};


class ImageUtils
{
public:
    static Image readImage(const QString& path, bool isFrame);

    static void writeImage(Image& image, const QString& path);

    static RGB rgbAt(const Image& img, int x, int y);
};

#endif // IMAGE_UTILS_H
