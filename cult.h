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
    RGB modeVote(const std::vector<RGB>& votes);
    
    const int PATCH_RADIUS = 8;
    const int PATCHMATCH_ITERATIONS = 4;
    const int WEXLER_ITERS = 2;
    const float FILTER_STRENGTH = 0.5;
    const int GRID_SIZE = 100;
};

#endif // CULT_H
