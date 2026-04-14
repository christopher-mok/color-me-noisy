#include "image_pyramid.h"

//run image pyramid
std::vector<Image> ImagePyramid::make_gaussian_pyramid(Image image, float filterStrength){
    std::vector<Image> pyramid; // initialize pyramid with image
    pyramid.push_back(image);

    //Do we determine num levels in here or explicitly in cult?
    int numLevels = num_levels(image, filterStrength);
    
    for(int i = 0; i < numLevels; i++){
        //get filter strength at level by calling that func? then can pass a filter strenhgth
        //into downsample, which then can pass it into blur
        image = downsample(image);
        pyramid.push_back(image);
    }

    return pyramid;
}

//How many levels to the pyramid based on filter strength and image size
int ImagePyramid::num_levels(const Image& image, float f){
    int levels = 0;
    float width = image.width;
    float height = image.height;

    while(width > 1 && height > 1){
        width  *= 0.85f;
        height *= 0.85f;
        levels++;
    }

    int filterLevels = int fLevels = static_cast<int>(std::log(f) / std::log(1.0f / 0.85f)) + 1;
    return std::max(1, std::min(levels, filterLevels));
}

//Interpolates filter strength across levels
float ImagePyramid::filter_strength_at_level(float f, int level, int totalLevels){

}

//Set pyramid level for coarse consistency and upsampling
void ImagePyramid::set_level(std::vector<Image>& pyramid, int level, const Image& image){
    pyramid[level] = image;
}

Image ImagePyramid::downsample(const Image& image){
    // TODO:
    //Image blurred_image == blur(image)
    //return blurred[::2, ::2] (every other pixel basically, resulting image size will be /2)
    //maybe also take in a filter strength
    return image;
}

Image ImagePyramid::upsample(const Image& image){
    
}

float pixWeight(int xdiff, int ydiff, float sigma){
    float x = float(xdiff);
    float y = float(ydiff);
    float w = std::exp(-(x*x + y*y)) / (2*sigma*sigma);
    return w;
}

std::vector<float> makeKernel(int radius, float sigma){
    int rad = 3;
    int diam = rad * 2 + 1;
    std::vector<int> mask(diam*diam);

    for (int kx=0; kx < diam; kx++){
        for (int ky=0; ky < diam; ky++){
            int xdiff = abs(rad - kx);
            int ydiff = abs(rad - ky);
            float w = pixWeight(xdiff, ydiff, sigma);
            int ind = ky * diam + kx;
            mask[ind] = w;
        }
    }
    return mask;

}

Image ImagePyramid::blur(const Image& image){
    float sigma = 1.0f;
    int radius = 3.0f * sigma;
    int diam = radius*2+1;

    // Initiate
    std::vector<RGB> newPixels;
    Image newImage;

    // make kernel
    std::vector<float> kern = makeKernel(radius, sigma);


    // Loop over every image pixel
    for (int r = 0; r < image.width; r++){
        for (int c = 0; c < image.width; c++){
            // initiate accumulation variable
            RGB newRGB;
            newRGB.r = 0;
            newRGB.g = 0;
            newRGB.b = 0;
            // loop through each index in the kernel and get weight
            for (int ki=0; ki < diam*diam; ki ++){
                // find corresponding coord in image
                int kx = ki % radius;
                int ky = std::floor(ki/radius);
                int imgY = std::clamp(r-radius+ky, 0, image.height-1);
                int imgX = std::clamp(c-radius+kx, 0, image.width-1);
                
                RGB targRGB = Image::rgbAt(imgX, imgY);

                // multiply weight and accumulate
                float w = kern[ki];
                newRGB.r += kern[ki] * targRGB.r;
                newRGB.g += kern[ki] * targRGB.g;
                newRGB.b += kern[ki] * targRGB.b;       
            }

            // add to newPixels
            int imgInd = r * image.width + c;
            newPixels[imageInd] = newRGB;

        }
        // make new image
        newImage = image;
        newImage.pixels = newPixels;
        return newImage;
    }
    
    
    


}



//Downsample respecting filter strength
Image ImagePyramid::downsample_to_level(const Image& image, float f, int level){

}
