#ifndef CULT_H
#define CULT_H

#include <iostream>
#include <QStringList>
#include <Eigen/Dense>

#include "image_utils.h"
#include "patchmatch.h"
#include "image_pyramid.h"


class Cult
{
public:
    Cult();
    void run(const QStringList &framePaths, const QString &texturePath);

private:
    std::vector<Image> m_frames;
    std::vector<Image> m_outputFrames;
    std::vector<Image> m_texPyramid;

    Image m_targetFrame;
    Image m_sourceTexture;

    Image processFrame(const Image& frame, const Image& prevOutput);
    void initFrames(const QStringList &framePaths);
    void saveFrames(const QString &outputDir);

    Image deformImage(const Image& image);
    Image patchmatch(const Image& target, const Image& source);
    
    const int PATCH_RADIUS = 1;
    const int PATCHMATCH_ITERATIONS = 1;
};

#endif // CULT_H
