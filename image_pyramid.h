#ifndef IMAGE_PYRAMID_H
#define IMAGE_PYRAMID_H

#include "image_utils.h"


class ImagePyramid
{
public:
    ImagePyramid();
    std::vector<Image> make_gaussian_pyramid(Image im);

private:
    Image upsample(const Image img);
    RGB blur(const Image& img, int x, int y);
    Image downsample(const Image img);
};

#endif // IMAGE_PYRAMID_H
