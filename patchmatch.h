#ifndef PATCHMATCH_H
#define PATCHMATCH_H

#include <vector>
#include "image_utils.h"
#include <random>

struct Match{
    int u;
    int v;
    float dist;
};

using NNF = std::vector<Match>;

class Patchmatch
{
public:
    static NNF run_patchmatch(const Image& target,
                              const Image& source,
                              const std::vector<bool>& targetBoundaryMask,
                              const std::vector<bool>& sourceBoundaryMask,
                              int patchRadius,
                              int iterations, NNF* prevNNF = nullptr);
    static bool isValidPatch(const Image& image, int x, int y, int patchRadius);

    static NNF upscaleNNF(const NNF& nnf, int oldWidth, int oldHeight,
                                        int newWidth, int newHeight);

    static std::vector<bool> createEdgeMask(const Image& target);

private:
    static std::mt19937 rng;

    static float patchDistance(const Image& target, int tx, int ty,
                               const Image& source, int sx, int sy,
                               int patchRadius);
    static void initializeNNF(const Image& target,
                              const Image& source,
                              const std::vector<bool>& targetBoundaryMask,
                              const std::vector<bool>& sourceBoundaryMask,
                              NNF& nnf,
                              int patchRadius, NNF* prevNNF = nullptr);
    static bool isAllowedSourcePatch(const Image& source,
                                     const std::vector<bool>& sourceBoundaryMask,
                                     int sx,
                                     int sy,
                                     int patchRadius,
                                     bool requireBoundary);
    static void propogateForward(int x, int y,
                                 const Image& target,
                                 const Image& source,
                                 const std::vector<bool>& sourceBoundaryMask,
                                 NNF& nnf,
                                 int patchRadius,
                                 bool requireBoundary);
    static void propogateBackward(int x, int y,
                                  const Image& target,
                                  const Image& source,
                                  const std::vector<bool>& sourceBoundaryMask,
                                  NNF& nnf,
                                  int patchRadius,
                                  bool requireBoundary);
    static void randomSearch(int x, int y,
                             const Image& target,
                             const Image& source,
                             const std::vector<bool>& sourceBoundaryMask,
                             NNF& nnf,
                             int patchRadius,
                             bool requireBoundary);


};

#endif // PATCHMATCH_H
