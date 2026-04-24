#include "cult.h"
#include "image_utils.h"
#include "image_pyramid.h"
#include <random>
#include <omp.h>


Cult::Cult() {
}

void Cult::run(const QStringList &framePaths, const QString &texturePath) {

    //Make source texture image pyramid - only do this once!
    std::cout << texturePath.toStdString() << std::endl;
    Image tex = ImageUtils::readImage(texturePath, false);
    m_sourceTexture = tex;
    m_texPyramid = ImagePyramid::make_gaussian_pyramid(tex, 0.5f);

    //save for debug
    for (int i = 0; i < m_texPyramid.size(); i++) {
        QString outPath = QString("../color-me-noisy/debug_pyramid/tex_pyramid_level_%1.png").arg(i);
        ImageUtils::writeImage(m_texPyramid[i], outPath);
    }

    m_frames = ImageUtils::readVideo("../color-me-noisy/source_videos/walker.mov");

    Image prevOutput;
    // Image cur_frame = ImageUtils::readImage(framePaths[0], true);
    // Image output_frame = processFrame(cur_frame, prevOutput); //main loop happens here
    std::cout<<"Beginning frame processing"<<std::endl;

    m_outputFrames.resize(m_frames.size());

    #pragma omp parallel for
    for(int i = 0; i < m_frames.size(); i++){

        // //old test frames
        // Image cur_frame = ImageUtils::readImage(framePaths[i], true);
        // Image output_frame = processFrame(cur_frame, prevOutput); //main loop happens here
        Image output_frame = processFrame(m_frames[i], prevOutput);

        m_outputFrames[i] = output_frame;
        std::cout<<"Processed frame #"<<i<<std::endl;
    }

//    Image prevOutput;
//    //main loop
//    for (const auto& path: framePaths) {
//        Image cur_frame = ImageUtils::readImage(path, true);

//        Image output_frame = processFrame(cur_frame, prevOutput); //main loop happens here
//        m_outputFrames.push_back(output_frame);
//        prevOutput = m_outputFrames.back();


//        //save image for debug
//        QString outPath = QString("../color-me-noisy/debug_pyramid/pyramid_level_%1.png").arg(output_frame.frameNumber);
//        ImageUtils::writeImage(output_frame, outPath);

//    }

    std::cout<<"Cult is running on Tuesdays! Send us your SSN and credit card number to join :D"<< std::endl;
    std::cout<<"Color Cult loves you <3" << std::endl;

    saveFrames("../color-me-noisy/output_frames");
    std::cout<<"Good job team!" << std::endl;

}

Image Cult::processFrame(const Image& frame, const Image& prevOutput){
    std::cout<<"Processing frame"<<std::endl;
    std::vector<Image> framePyramid = ImagePyramid::make_gaussian_pyramid(frame, FILTER_STRENGTH);
    //std::cout<<"Created frame pyramids"<<std::endl;

    Image output_frame = framePyramid.back(); // start at coarsest/most blurred (end of pyramid)

    //        //MAIN LOOP
    //        for (Image& level : cur_pyramid) {
    //            //S_deformed = deformImage()
    //            //for each patch in Image:
    //                //best_patch = find_match(patch, s_deformed)
    //                //Copy best patch to output image
    //                m_outputFrames.push_back(output_frame);
    //        }

    Image s_deformed = deformImage(m_sourceTexture);
    QString deformed_outPath = QString("../color-me-noisy/debug_pyramid/source_deformed.png");
    ImageUtils::writeImage(s_deformed, deformed_outPath);

    std::vector<Image> sourcePyramid = ImagePyramid::make_gaussian_pyramid(s_deformed, FILTER_STRENGTH);
    // std::cout<<"Created source pyramids"<<std::endl;

    // std::cout<<"Target image dimensions: "<<frame.width<<"x"<<frame.height<<std::endl;

    // std::cout << "Frame pyramid levels: " << framePyramid.size() << std::endl;
    // for (int i = 0; i < framePyramid.size(); i++) {
    //     std::cout << "  framePyramid[" << i << "]: " << framePyramid[i].width << "x" << framePyramid[i].height << std::endl;
    // }

    NNF prevNNF;
    for (int level = framePyramid.size() - 1; level >= 0; level--) {
        // deform, patch-match, upsample output to seed next level
        QString outPath = QString("../color-me-noisy/debug_pyramid/framepyramid_level_%1.png").arg(level);
        //debugging
        Image cur_image = framePyramid[level];
        ImageUtils::writeImage(cur_image, outPath);

        Image& cur_target = framePyramid[level];
        Image& cur_source = sourcePyramid[level];

        // if (level == (int)framePyramid.size() - 1) {
        //     // Coarsest level: no seed, just match directly
        //     output_frame = patchmatch(cur_target, cur_source);
        // } else {
        //     output_frame = patchmatch(output_frame, cur_source);
        // }


        NNF* seedNNF = nullptr;
        NNF upscaled;
        if (level < (int)framePyramid.size() - 1) {
            upscaled = Patchmatch::upscaleNNF(prevNNF, framePyramid[level+1].width, framePyramid[level+1].height,
                                              cur_target.width, cur_target.height);
            seedNNF = &upscaled;
        }

        prevNNF = Patchmatch::run_patchmatch(cur_target, cur_source, PATCH_RADIUS, PATCHMATCH_ITERATIONS, seedNNF);
        output_frame = vote(cur_target, cur_source, prevNNF);

        QString dbgPath = QString("../color-me-noisy/debug_pyramid/framepyramid_dbg_level_%1.png").arg(level);
        ImageUtils::writeImage(output_frame, dbgPath);

        // if (level > 0) { //dont upsample at the last level
        //     output_frame = ImagePyramid::upsample(output_frame);
        // }
    }

    return output_frame;

}

Image Cult::patchmatch(const Image& target, const Image& source){
    Image output_image;

    //std::cout<<"Running Patchmatch"<<std::endl;
    NNF nnf = Patchmatch::run_patchmatch(target, source, PATCH_RADIUS, PATCHMATCH_ITERATIONS);
    output_image = vote(target, source, nnf);
    //std::cout<<"Finished Patchmatch and Voting"<<std::endl;
    return output_image;
}

Image Cult::vote(const Image& target, const Image& source, NNF& nnf){
    Image processed_frame = target;
    std::vector<RGB> processed_pixels;
    //For each pixel
    const std::vector<RGB>& pixels = target.pixels;
    for(int i = 0; i < pixels.size(); i++){
        int x = i%target.width;
        int y = i/target.width;
        RGB accum;
        accum.r = 0,accum.g = 0, accum.b = 0;
        int count = 0;
        //For each pixel in the patch radius around iterated pixel
        for(int dy = -PATCH_RADIUS; dy <= PATCH_RADIUS; dy++){
            for(int dx = -PATCH_RADIUS; dx <= PATCH_RADIUS; dx++){
                //Offset position
                int px = x + dx;
                int py = y + dy;

                //patch validity check
                if(Patchmatch::isValidPatch(target, px, py, PATCH_RADIUS)){
                    Match match = nnf[py*target.width + px];

                    //match to source
                    int sx = match.u - dx;
                    int sy = match.v - dy;

                    if(sx>=0 && sx < source.width && sy>=0 && sy<source.height){
                        RGB rgb = ImageUtils::rgbAt(source, sx, sy);
                        accum.r += rgb.r;
                        accum.g += rgb.g;
                        accum.b += rgb.b;
                        count++;
                    }

                }
            }
        }
        RGB outRGB;
        if(count==0) std::cout<<"No matches found in voting";
        outRGB.r = accum.r/count;
        outRGB.g = accum.g/count;
        outRGB.b = accum.b/count;
        outRGB.a = 255;
        processed_pixels.push_back(outRGB);
    }
    processed_frame.pixels = processed_pixels;
    return processed_frame;
}

Image Cult::deformImage(const Image& image){
    int gridSize = 50; //make this class variable

    thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> angleDist(0.f, 2.f * M_PI);
    // std::uniform_real_distribution<float> magnitudeDist(15.f, 25.f);
    std::uniform_real_distribution<float> magnitudeDist(40.f, 60.f);

    std::vector<Eigen::Vector2f> originalPoints;
    std::vector<Eigen::Vector2f> deformedPoints;

    for (int y = 0; y <= image.height; y += gridSize) {
        for (int x = 0; x <= image.width; x += gridSize) {
            originalPoints.push_back({(float)x, (float)y});

            float angle = angleDist(rng);
            float magnitude = magnitudeDist(rng);

            deformedPoints.push_back({
                x + magnitude * std::cos(angle),
                y + magnitude * std::sin(angle)
            });
        }
    }

    //still todo

    return arapMLS(image, originalPoints, deformedPoints);
}

Image Cult::arapMLS(const Image& image, std::vector<Eigen::Vector2f>& originalPoints,
              std::vector<Eigen::Vector2f>& deformedPoints){
    Image output = image;
    output.pixels.reserve(image.width*image.height);

    float alpha = 2.0f;

    #pragma omp parallel for
    for(int y = 0; y < image.height; y++){
        for(int x = 0; x < image.width; x++){
            Eigen::Vector2f v(x, y);

            float weightSum = 0.f;
            std::vector<float> weights;
            for(int i = 0; i < originalPoints.size(); i++){
                Eigen::Vector2f p = originalPoints[i];
                float w = 1.f / (pow((p-v).norm(), 2.f*alpha));
                weights.push_back(w);
                weightSum += w;
            }

            Eigen::Vector2f p_star(0,0);
            Eigen::Vector2f q_star(0,0);
            for(int i = 0; i < originalPoints.size(); i++){
                p_star += (weights[i] * originalPoints[i]) / weightSum;
                q_star += (weights[i] * deformedPoints[i]) / weightSum;
            }

            std::vector<Eigen::Vector2f> centeredSources;
            std::vector<Eigen::Vector2f> centeredTargets;
            for(int i = 0; i < originalPoints.size(); i++){
                Eigen::Vector2f p_hat = originalPoints[i] - p_star;
                Eigen::Vector2f q_hat = deformedPoints[i] - p_star;

                centeredSources.push_back(p_hat);
                centeredTargets.push_back(q_hat);
            }

            Eigen::Matrix2f M = Eigen::Matrix2f::Zero();
            for(int i = 0; i < originalPoints.size(); i++){
                M += weights[i]*centeredSources[i]*centeredTargets[i].transpose();
            }

            Eigen::JacobiSVD<Eigen::Matrix2f> svd(M, Eigen::ComputeFullU | Eigen::ComputeFullV);
            Eigen::Matrix2f R = svd.matrixU() * svd.matrixV().transpose();

            Eigen::Vector2f deformed = R * (v - p_star)+q_star;

            int sx = std::clamp((int)deformed.x(), 0, image.width - 1);
            int sy = std::clamp((int)deformed.y(), 0, image.height - 1);
            output.pixels[y * image.width + x] = ImageUtils::rgbAt(image, sx, sy);
        }
    }


    return output;
}

void Cult::initFrames(const QStringList &framePaths) {
    for (const QString& frame : framePaths) {
        Image cur_frame = ImageUtils::readImage(frame, true);
        
        // build the pyramid for the cur_frame
        std::vector<Image> cur_pyramid = ImagePyramid::make_gaussian_pyramid(cur_frame, 0.5f);
        // maybe make a map of frames to pyramids


    }
}

void Cult::saveFrames(const QString &outputDir) {
    for (int i = 0; i < m_outputFrames.size(); i++) {
        QString outPath = QString("%1/frame_%2.png").arg(outputDir).arg(i);
        ImageUtils::writeImage(m_outputFrames[i], outPath);
    }
}

