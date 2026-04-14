#include "patchmatch.h"


//TARGET: the frame we are stylizing
//SOURCE: the texture we are copying pixels from
NNF Patchmatch::run_patchmatch(const Image& target,
                               const Image& source,
                               int patchRadius,
                               int iterations){
    NNF nnf;
    initializeNNF(target, source, nnf, patchRadius);

    return nnf;
}

float Patchmatch::patchDistance(const Image& target, int tx, int ty,
                                const Image& source, int sx, int sy,
                                int patchRadius){
    float dist = 0.f;

    return dist;
}

bool Patchmatch::isValidPatch(const Image& image, int x, int y, int patchRadius){
    return false;
}

void Patchmatch::initializeNNF(const Image& target, const Image& source,
                    NNF& nnf, int patchRadius){

}

void Patchmatch::propogateForward(int x, int y, const Image& target, const Image& source,
                      int patchRadius, int iterations){

}

void Patchmatch::propogateBackward(int x, int y, const Image& target,
                       const Image& source, int patchRadius, int iterations){

}

void Patchmatch::randomSearch(int x, int y, const Image& target, const Image& source,
                  NNF& nnf, int patchRadius){

}

