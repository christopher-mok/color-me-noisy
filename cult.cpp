#include "cult.h"
#include "image_utils.h"
#include "image_pyramid.h"

Cult::Cult() {
}

void Cult::run(const QStringList &framePaths, const QString &texturePath) {

    //Make source texture image pyramid - only do this once!
    Image tex = ImageUtils::readImage(texturePath, false);
    m_texPyramid = ImagePyramid::make_gaussian_pyramid(tex, 0.5f);

    Image prevOutput;
    //main loop
    for (const auto& path: framePaths) {
        Image cur_frame = ImageUtils::readImage(path, true);

        Image output_frame = processFrame(cur_frame, prevOutput); //main loop happens here
        m_outputFrames.push_back(output_frame);
        prevOutput = m_outputFrames.back();


        //save image for debug
        QString outPath = QString("../color-me-noisy/debug_pyramid/pyramid_level_%1.png").arg(output_frame.frameNumber);
        ImageUtils::writeImage(output_frame, outPath);

    }

    std::cout<<"Cult is running! Send us your SSN and credit card number to join :D"<< std::endl;
    std::cout<<"Color Cult loves you <3" << std::endl;

    saveFrames("../color-me-noisy/output_frames");
    std::cout<<"Good job team!" << std::endl;
}

Image Cult::processFrame(const Image& frame, const Image& prevOutput){
    std::vector<Image> framePyramid = ImagePyramid::make_gaussian_pyramid(frame, 0.5f);
    Image output_frame = framePyramid.back(); // start at coarsest/most blurred (end of pyramid)


    //        //MAIN LOOP
    //        for (Image& level : cur_pyramid) {
    //            //S_deformed = deformImage()
    //            //for each patch in Image:
    //                //best_patch = find_match(patch, s_deformed)
    //                //Copy best patch to output image
    //                m_outputFrames.push_back(output_frame);
    //        }


    for (int level = framePyramid.size() - 1; level >= 0; level--) {
        // deform, patch-match, upsample output to seed next level

        Image s_deformed = deformImage();
        output_frame = patchmatch(m_texPyramid[level], s_deformed);

        if (level < framePyramid.size() - 1) {
            output_frame = ImagePyramid::upsample(output_frame);
        }
    }
    return output_frame;

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

