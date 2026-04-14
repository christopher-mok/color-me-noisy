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

private:
    //Set pyramid level for coarse consistency
    static void set_level(std::vector<Image>& pyramid, int level, const Image& image);

    static Image upsample(const Image& image);
    static Image blur(const Image& image);
    static float pixWeight(int xdiff, int ydiff, float sigma); // sigma = blur strength
    static Image downsample(const Image& image);

    //Downsample respecting filter strength
    static Image downsample_to_level(const Image& image, float f, int level);

};

#endif // IMAGE_PYRAMID_H
