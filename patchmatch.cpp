#include "patchmatch.h"


//TARGET: the frame we are stylizing
//SOURCE: the texture we are copying pixels from
NNF Patchmatch::run_patchmatch(const Image& target,
                               const Image& source,
                               int patchRadius,
                               int iterations){
    NNF nnf;
    rng.seed(std::random_device{}());
    initializeNNF(target, source, nnf, patchRadius);

    for(int i = 0; i < iterations; i++){
        if(i%2==0){ //Forward propogation
            for(int y = 0; y < target.height; y++){
                for(int x = 0; x < target.width; x++){
                    propogateForward(x, y, target, source, nnf, patchRadius);
                    randomSearch(x, y, target, source, nnf, patchRadius);
                }
            }
        }else{ //Backward propogation
            for(int y = 0; y < target.height; y++){
                for(int x = 0; x < target.width; x++){
                    propogateBackward(x, y, target, source, nnf, patchRadius);
                    randomSearch(x, y, target, source, nnf, patchRadius);
                }
            }
        }
    }

    return nnf;
}


std::mt19937 Patchmatch::rng(std::random_device{}());

float Patchmatch::patchDistance(const Image& target, int tx, int ty,
                                const Image& source, int sx, int sy,
                                int patchRadius){
    float dist = 0.f;

    for(int dx = -patchRadius; dx <= patchRadius; dx++){
        for(int dy = -patchRadius; dy <= patchRadius; dy++){
            RGB target_rgb = ImageUtils::rgbAt(target, tx+dx, ty+dy);
            RGB source_rgb = ImageUtils::rgbAt(source, sx+dx, sy+dy);

            float r2 = std::pow(target_rgb.r-source_rgb.r, 2);
            float g2 = std::pow(target_rgb.g - source_rgb.g, 2);
            float b2 = std::pow(target_rgb.b - source_rgb.b, 2);

            dist+= r2 + g2 + b2;
        }
    }

    return dist;
}

bool Patchmatch::isValidPatch(const Image& image, int x, int y, int patchRadius){
    return (x-patchRadius >= 0) && (x+patchRadius < image.width)
           && (y-patchRadius >= 0) && (y+patchRadius < image.height);
}

void Patchmatch::initializeNNF(const Image& target, const Image& source,
                    NNF& nnf, int patchRadius){

    std::uniform_int_distribution<int> distX(0, source.width - 1);
    std::uniform_int_distribution<int> distY(0, source.height - 1);

    for(int y = 0; y < target.height; y++){
        for(int x = 0; x < target.width; x++){
            int sx;
            int sy;
            do{
                //randomize sx to 0 to source.width
                sx = distX(rng);
                sy = distY(rng);
                //randomize sy to 0 to source.height
            }while(isValidPatch(source, sx, sy, patchRadius));

            Match match;
            match.u = sx;
            match.v = sy;
            match.dist = patchDistance(target, x, y, source, sx, sy, patchRadius);

            nnf[x][y] = match;
        }
    }
}

void Patchmatch::propogateForward(int x, int y, const Image& target, const Image& source,
                      NNF& nnf, int patchRadius){

}

void Patchmatch::propogateBackward(int x, int y, const Image& target,
                       const Image& source, NNF& nnf, int patchRadius){
}

void Patchmatch::randomSearch(int x, int y, const Image& target, const Image& source,
                  NNF& nnf, int patchRadius){

}

