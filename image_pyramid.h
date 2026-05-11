#ifndef IMAGE_PYRAMID_H
#define IMAGE_PYRAMID_H

#include "image_utils.h"


class ImagePyramid
{
public:
    //run image pyramid
    static std::vector<Image> make_gaussian_pyramid(Image image, float filterStrength);

    //How many levels to the pyramid based on filter strength and image size
    static int num_levels(const Image& image, float f);

    //Interpolates filter strength across levels
    static float filter_strength_at_level(float f, int level, int totalLevels);

    static Image upsample(const Image& image);
    static Image blur(const Image& image, float fStrenth);
    static Image downsample(const Image& image, float fStrength);

private:
    static void set_level(std::vector<Image>& pyramid, int level, const Image& image);



    static float pixWeight(int xdiff, int ydiff, float sigma); // sigma = blur strength


};

#endif // IMAGE_PYRAMID_H
