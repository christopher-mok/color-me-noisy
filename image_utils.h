#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include <QImage>

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
};


class ImageUtils
{
public:
    ImageUtils();

    Image readImage();

    void writeImage();
};

#endif // IMAGE_UTILS_H
