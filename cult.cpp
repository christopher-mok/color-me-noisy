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

    //#pragma omp parallel for
    for(int i = 0; i < m_frames.size(); i++){

        // //old test frames
        // Image cur_frame = ImageUtils::readImage(framePaths[i], true);
        // Image output_frame = processFrame(cur_frame, prevOutput); //main loop happens here
        Image output_frame = processFrame(m_frames[i], prevOutput);

        m_outputFrames[i] = output_frame;
        std::cout<<"Processed frame #"<<i<<std::endl;
        break; // <----- KEEP TO PRINT 1 FRAME
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

    Image currentResult = framePyramid.back();
    Image output_frame = framePyramid.back(); // start at coarsest/most blurred (end of pyramid)

    Image s_deformed = deformImage(m_sourceTexture);

    //REPLACE WITH BORDER
    Image s_border = s_deformed;

    QString deformed_outPath = QString("../color-me-noisy/debug_pyramid/source_deformed.png");
    ImageUtils::writeImage(s_deformed, deformed_outPath);
    std::cout<<"Deformed source texture"<<std::endl;

    std::vector<Image> sourcePyramid = ImagePyramid::make_gaussian_pyramid(s_deformed, FILTER_STRENGTH);
    // std::cout<<"Created source pyramids"<<std::endl;
    std::vector<Image> borderPyramid = ImagePyramid::make_gaussian_pyramid(s_border, FILTER_STRENGTH);

    for(int i = 0; i < sourcePyramid.size(); i++){
        Image cur_image = sourcePyramid[i];
        QString outPath = QString("../color-me-noisy/debug_pyramid/texpyramid_level_%1.png").arg(i);
        ImageUtils::writeImage(cur_image, outPath);
    }

    //Original
    NNF prevNNF;
    for (int level = framePyramid.size() - 1; level >= 0; level--) {
        // deform, patch-match, upsample output to seed next level
        QString outPath = QString("../color-me-noisy/debug_pyramid/framepyramid_level_%1.png").arg(level);
        //debugging
        Image cur_image = framePyramid[level];
        ImageUtils::writeImage(cur_image, outPath);

//        Image& cur_target = framePyramid[level];
        Image& cur_source = sourcePyramid[level];
        Image& cur_border = borderPyramid[level];
        if (level < (int)framePyramid.size() - 1) {
            currentResult = ImagePyramid::upsample(currentResult);
        }


        NNF* seedNNF = nullptr;
        NNF upscaled;
        if (level < (int)framePyramid.size() - 1) {
            upscaled = Patchmatch::upscaleNNF(prevNNF, framePyramid[level+1].width, framePyramid[level+1].height,
                                              currentResult.width, currentResult.height);
            seedNNF = &upscaled;
        }

        prevNNF = Patchmatch::run_patchmatch(currentResult, cur_source, cur_border, PATCH_RADIUS, PATCHMATCH_ITERATIONS, seedNNF);
        output_frame = vote(currentResult, cur_source, cur_source, prevNNF);
        currentResult = output_frame;

        QString dbgPath = QString("../color-me-noisy/debug_pyramid/framepyramid_dbg_level_%1.png").arg(level);
        ImageUtils::writeImage(output_frame, dbgPath);

    }

    return output_frame;

}

Image Cult::patchmatch(const Image& target, const Image& source, const Image& boundarySource){
    Image output_image;

    //std::cout<<"Running Patchmatch"<<std::endl;
    NNF nnf = Patchmatch::run_patchmatch(target, source, PATCH_RADIUS, PATCHMATCH_ITERATIONS);
    output_image = vote(target, source, boundarySource, nnf);
    //std::cout<<"Finished Patchmatch and Voting"<<std::endl;
    return output_image;
}

Image Cult::vote(const Image& target, const Image& source, const Image& boundary, NNF& nnf){


    Image processed_frame = target;
    processed_frame.pixels.resize(target.width * target.height);

    for(int y = 0; y < target.height; y++){
        for(int x = 0; x < target.width; x++){
            std::vector<RGB> candidates;

            for(int dy = -PATCH_RADIUS; dy <= PATCH_RADIUS; dy++){
                for(int dx = -PATCH_RADIUS; dx <= PATCH_RADIUS; dx++){
                    int px = x + dx;
                    int py = y + dy;

                    if(!Patchmatch::isValidPatch(target, px, py, PATCH_RADIUS))
                        continue;

                    Match match = nnf[py*target.width + px];

                    int sx = match.u - dx;
                    int sy = match.v - dy;

                    if(sx >= 0 && sx < source.width && sy >= 0 && sy < source.height){
                        candidates.push_back(ImageUtils::rgbAt(source, sx, sy));
                    }
                }
            }

            if(!candidates.empty()){
                processed_frame.pixels[y * target.width + x] = modeVote(candidates);
            } else {
                processed_frame.pixels[y * target.width + x] = ImageUtils::rgbAt(target, x, y);
            }
        }
    }

    return processed_frame;
}

RGB Cult::modeVote(const std::vector<RGB>& votes) {
    struct Bucket {
        int count = 0;
        float r = 0, g = 0, b = 0;
    };

    std::unordered_map<int, Bucket> hist;

    for (const RGB& c : votes) {
        int r = std::clamp((int)std::round(c.r * 255.f), 0, 255);
        int g = std::clamp((int)std::round(c.g * 255.f), 0, 255);
        int b = std::clamp((int)std::round(c.b * 255.f), 0, 255);

        // 5 bits per channel: 32x32x32 color bins
        int rq = r >> 3;
        int gq = g >> 3;
        int bq = b >> 3;
        int key = (rq << 10) | (gq << 5) | bq;

        auto& bucket = hist[key];
        bucket.count++;
        bucket.r += c.r;
        bucket.g += c.g;
        bucket.b += c.b;
    }

    int bestKey = -1;
    int bestCount = -1;

    for (const auto& [key, bucket] : hist) {
        if (bucket.count > bestCount) {
            bestCount = bucket.count;
            bestKey = key;
        }
    }

    const Bucket& best = hist[bestKey];

    RGB out;
    out.r = best.r / best.count;
    out.g = best.g / best.count;
    out.b = best.b / best.count;
    out.a = 255;
    return out;

}

//mode vote
// Image Cult::vote(const Image& target, const Image& source, NNF& nnf) {
//     Image processed = target;
//     processed.pixels.resize(target.width * target.height);

//     for (int y = 0; y < target.height; y++) {
//         for (int x = 0; x < target.width; x++) {
//             std::vector<RGB> votes;

//             for (int dy = -PATCH_RADIUS; dy <= PATCH_RADIUS; dy++) {
//                 for (int dx = -PATCH_RADIUS; dx <= PATCH_RADIUS; dx++) {
//                     int px = x + dx;
//                     int py = y + dy;

//                     if (!Patchmatch::isValidPatch(target, px, py, PATCH_RADIUS)) {
//                         continue;
//                     }

//                     Match m = nnf[py * target.width + px];

//                     int sx = m.u - dx;
//                     int sy = m.v - dy;

//                     if (sx < 0 || sx >= source.width || sy < 0 || sy >= source.height) {
//                         continue;
//                     }

//                     votes.push_back(ImageUtils::rgbAt(source, sx, sy));
//                 }
//             }

//             if (!votes.empty()) {
//                 processed.pixels[y * target.width + x] = modeVote(votes);
//             } else {
//                 processed.pixels[y * target.width + x] = ImageUtils::rgbAt(target, x, y);
//             }
//         }
//     }

//     return processed;
// }

Image Cult::deformImage(const Image& image){
    int gridSize = GRID_SIZE; //make this class variable

    thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> angleDist(0.f, 2.f * M_PI);
    // std::uniform_real_distribution<float> magnitudeDist(15.f, 25.f);
    std::uniform_real_distribution<float> magnitudeDist(2.f, 7.f);

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

//Note: switch the order of deformed and original points to get swirls or boxes
Image Cult::arapMLS(const Image& image, std::vector<Eigen::Vector2f>& deformedPoints,
              std::vector<Eigen::Vector2f>& originalPoints){
    Image output = image;
    output.pixels.reserve(image.width*image.height);

    float alpha = 1.0f;

    #pragma omp parallel for
    for(int y = 0; y < image.height; y++){
        for(int x = 0; x < image.width; x++){
            Eigen::Vector2f v(x, y);

            float weightSum = 0.f;
            std::vector<float> weights;
            for(int i = 0; i < originalPoints.size(); i++){
                Eigen::Vector2f p = originalPoints[i];
                float dist2 = std::max((p - v).squaredNorm(), 1e-6f);
                float w = 1.f / (pow(dist2, alpha));
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
                Eigen::Vector2f q_hat = deformedPoints[i] - q_star;

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

