#include "image_pyramid.h"

//just a downsample
std::vector<Image> ImagePyramid::make_gaussian_pyramid(Image image, float filterStrength){
    std::vector<Image> pyramid; // initialize pyramid with image
    pyramid.push_back(image);

    //Do we determine num levels in here or explicitly in cult?
//    int numLevels = num_levels(image, filterStrength);

    int numLevels = 3;
    
    for(int i = 0; i < numLevels; i++){
        //get filter strength at level by calling that func? then can pass a filter strenhgth
        //into downsample, which then can pass it into blur
        float strength = filter_strength_at_level(filterStrength, i, numLevels);
        image = downsample(image, strength);
        pyramid.push_back(image);
    }

    return pyramid;
}

Image ImagePyramid::downsample(const Image& image, float fStrength){
    Image blurred_image = blur(image);
//    Image blurred_image = image;

    //basically blurred[::2, ::2], or take every other pixel
    int w = image.width / 2;
    int h = image.height / 2;

    Image out;
    out.width = w;
    out.height = h;

    out.pixels.resize(w * h);

    for (int r = 0; r < h; r++) {
        for (int c = 0; c < w; c++) {
            out.pixels[r * w + c] = blurred_image.pixels[(r*2) * image.width + (c*2)];
        }
    }

    //maybe also take in a filter strength
    return out;
}

Image ImagePyramid::upsample(const Image& image){
    // biliear filtering to to double pix count
    return image;
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

    int filterLevels = static_cast<int>(std::log(f) / std::log(1.0f / 0.85f)) + 1;
    return std::max(1, std::min(levels, filterLevels));
}

//Interpolates filter strength across levels
float ImagePyramid::filter_strength_at_level(float f, int level, int totalLevels){
    return f * std::pow(1.f/f, level / (totalLevels-1));
}

//Set pyramid level for coarse consistency and upsampling
void ImagePyramid::set_level(std::vector<Image>& pyramid, int level, const Image& image){
    pyramid[level] = image;
}


float pixWeight(int xdiff, int ydiff, float sigma){
    float x = float(xdiff);
    float y = float(ydiff);
    float w = std::exp(-(x*x + y*y) / (2*sigma*sigma));
    return w;
}

std::vector<float> makeKernel(int radius, float sigma){
    int diam = radius * 2 + 1;
    std::vector<float> mask(diam*diam);

    for (int kx=0; kx < diam; kx++){
        for (int ky=0; ky < diam; ky++){
            int xdiff = abs(radius - kx);
            int ydiff = abs(radius - ky);
            float w = pixWeight(xdiff, ydiff, sigma);
            int ind = ky * diam + kx;
            mask[ind] = w;
        }
    }

    // Normalize
    float sum = 0.f;
    for (float w : mask){
        sum += w;
    }
    for (float& w : mask) {
        w /= sum;
    }

    return mask;

}

Image ImagePyramid::blur(const Image& image){
    float sigma = 3.0f;
    int radius = 3.0f * sigma;
    int diam = radius*2+1;

    // Initiate
    std::vector<RGB> newPixels(image.width * image.height);
    Image newImage;

    // make kernel
    std::vector<float> kern = makeKernel(radius, sigma);

    // Loop over every image pixel
    for (int r = 0; r < image.height; r++){
        for (int c = 0; c < image.width; c++){
            // initiate accumulation variable
            RGB newRGB;
            newRGB.r = 0;
            newRGB.g = 0;
            newRGB.b = 0;
            newRGB.a = 1.0f;
            // loop through each index in the kernel and get weight
            for (int ki=0; ki < diam*diam; ki ++){
                // find corresponding coord in image
                int kx = ki % diam;
                int ky = std::floor(ki/diam);
                int imgY = std::clamp(r-radius+ky, 0, image.height-1);
                int imgX = std::clamp(c-radius+kx, 0, image.width-1);
                
                RGB targRGB = ImageUtils::rgbAt(image, imgX, imgY);

                // multiply weight and accumulate
                float w = kern[ki];
                newRGB.r += w * targRGB.r;
                newRGB.g += w * targRGB.g;
                newRGB.b += w * targRGB.b;
            }

            // add to newPixels
            int imageInd = r * image.width + c;
            newPixels[imageInd] = newRGB;

        }

    }
    // make new image
    newImage = image;
    newImage.pixels = newPixels;
    return newImage;
    
}
