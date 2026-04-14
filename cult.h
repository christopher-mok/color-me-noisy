#ifndef CULT_H
#define CULT_H

#include <iostream>
#include <stdlib.h>
#include <QStringList>

#include "image_utils.h"


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
    
};

#endif // CULT_H
