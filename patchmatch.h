#ifndef PATCHMATCH_H
#define PATCHMATCH_H

#include "image_utils.h"

struct Match{
    int u;
    int v;
    float dist;
};

using NNF = std::vector<std::vector<Match>>;

class patchmatch
{
public:
    static NNF run_patchmatch(const Image& target,
                              const Image& source,
                              int patchRadius,
                              int iterations);

private:
    static float patchDistance(const Image& target, int tx, int ty,
                               const Image& source, int sx, int sy,
                               int patchRadius);
    static bool isValidPatch(const Image& image, int x, int y, int patchRadius);
    static void initializeNNF(const Image& target,
                              const Image& source,
                              NNF& nnf,
                              int patchRadius);
    static void propogateForward();
    static void propogateBackward();
    static void randomSearch();
};

#endif // PATCHMATCH_H
