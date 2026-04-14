#include "image_pyramid.h"

//run image pyramid
std::vector<Image> ImagePyramid::make_gaussian_pyramid(Image image, float filterStrength, int startLevel){

}

//How many levels to the pyramid based on filter strength and image size
int ImagePyramid::num_levels(const Image& image, float f){

}

//Interpolates filter strength across levels
float ImagePyramid::filter_strength_at_level(float f, int level, int totalLevels){

}


//Set pyramid level for coarse consistency
void ImagePyramid::set_level(std::vector<Image>& pyramid, int level, const Image& image){

}

Image ImagePyramid::upsample(const Image& image){

}
RGB ImagePyramid::blur(const Image& image, int x, int y){

}
Image ImagePyramid::downsample(const Image& image){

}

//Downsample respecting filter strength
Image ImagePyramid::downsample_to_level(const Image& image, float f, int level){

}
