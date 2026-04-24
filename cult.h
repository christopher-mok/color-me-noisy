#ifndef CULT_H
#define CULT_H

#include <iostream>
#include <vector>
#include <QStringList>
#include "Eigen/Core"
#include "Eigen/Eigen"
#include "Eigen/Dense"

#include "image_utils.h"
#include "patchmatch.h"
#include "image_pyramid.h"

using NNF = std::vector<Match>;

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
    Image arapMLS(const Image& image, std::vector<Eigen::Vector2f>& originalPoints,
                  std::vector<Eigen::Vector2f>& deformedPoints);
    Image patchmatch(const Image& target, const Image& source);

    Image vote(const Image& target, const Image& source, NNF& nnf);
    
    const int PATCH_RADIUS = 5;
    const int PATCHMATCH_ITERATIONS = 4;
    const float FILTER_STRENGTH = 2.0;
    const int GRID_SIZE = 50;
};

#endif // CULT_H
