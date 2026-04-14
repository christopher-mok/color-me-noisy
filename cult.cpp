#include "cult.h"
#include "image_utils.h"
#include "image_pyramid.h"

Cult::Cult() {
}

void Cult::run(const QStringList &framePaths, const QString &texturePath) {

    //Make source texture image pyramid
    Image tex = ImageUtils::readImage(texturePath, false);
    std::vector<Image> tex_pyramid = ImagePyramid::make_gaussian_pyramid(tex, 0.5f);

    //Final Image initiation
    

    for (const auto& path: framePaths) {
        Image cur_frame = ImageUtils::readImage(path, true);
        Image output_frame;
        
        // build the pyramid for the cur_frame, call gaussian blur
        std::vector<Image> cur_pyramid = ImagePyramid::make_gaussian_pyramid(cur_frame, 0.5f);
        
        //MAIN LOOP
        for (Image& level : cur_pyramid) {
            //S_deformed = deformImage()
            //for each patch in Image:
                //best_patch = find_match(patch, s_deformed)
                //Copy best patch to output image
                m_outputFrames.push_back(output_frame);
        }
        
        
    
        // save all of this frame's pyramids so we can see and debug
        for (int i = 0; i < cur_pyramid.size(); i++){
            QString outPath = QString("../color-me-noisy/debug_pyramid/pyramid_level_%1.png").arg(i);
            ImageUtils::writeImage(cur_pyramid[i], outPath);
        }
    }

    std::cout<<"Cult is running! Send us your SSN and credit card number to join :D"<< std::endl;
    std::cout<<"Color Cult loves you <3" << std::endl;

    saveFrames("../color-me-noisy/output_frames");
    std::cout<<"Good job team!" << std::endl;
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

