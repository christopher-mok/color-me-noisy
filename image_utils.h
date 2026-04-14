#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include <QImage>

struct Image{
    QImage pixels;
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
