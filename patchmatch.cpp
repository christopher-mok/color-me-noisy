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

//For neighbors left and above
void Patchmatch::propogateForward(int x, int y, const Image& target, const Image& source,
                      NNF& nnf, int patchRadius){
    //TODO
    //init:
        //candidate 1: input x,y nnf
        //candidate 2: nnf[x-1, y] + (1, 0)
        //candidate 3: nnf[x, y-1] + (0, 1)
    //set nnf[x,y] to candidate with lowest patchDistance
    Match current = nnf[x][y];
    if(isValidPatch(target, x-1, y, patchRadius)){
        Match left = nnf[x-1][y];
        int cx = left.u + 1;
        int cy = left.v;
        float c_dist = patchDistance(target, x, y, source, cx, cy, patchRadius);

        if(isValidPatch(source, cx, cy, patchRadius) && c_dist < current.dist){
            current.dist = c_dist;
            current.u = cx;
            current.v = cy;
        }
    }

    if(isValidPatch(target, x, y-1, patchRadius)){
        Match left = nnf[x][y-1];
        int cx = left.u;
        int cy = left.v + 1;
        float c_dist = patchDistance(target, x, y, source, cx, cy, patchRadius);

        if(isValidPatch(source, cx, cy, patchRadius) && c_dist < current.dist){
            current.dist = c_dist;
            current.u = cx;
            current.v = cy;
        }
    }

    nnf[x][y] = current;
}

//for neighbors right and below
void Patchmatch::propogateBackward(int x, int y, const Image& target,
                       const Image& source, NNF& nnf, int patchRadius){
    //TODO
    //init:
        //candidate 1: input x,y nnf
        //candidate 2: nnf[x+1, y] - (1, 0)
        //candidate 3: nnf[x, y+1] - (0, 1)
    //set nnf[x,y] to candidate with lowest patchDistance
}

void Patchmatch::randomSearch(int x, int y, const Image& target, const Image& source,
                  NNF& nnf, int patchRadius){
    //init sx and sy to nnf[x,y], this is current best
    Match bestMatch = nnf[x][y];
    int sx = bestMatch.u;
    int sy = bestMatch.v;
    //initialize search radius
    int radius = std::max(source.width, source.height);

    std::uniform_real_distribution<float> distX(-1.f, 1.f);
    std::uniform_real_distribution<float> distY(-1.f, 1.f);

    float bestDist = bestMatch.dist;
    int bestX = sx;
    int bestY = sy;

    //while rad > 1
    while(radius > 1){
        int cx, cy;
        //randomly sample in search radius around current best
        do{
            cx = std::clamp(sx + (int)(distX(rng)*((float)radius)), 0, source.width);
            cy = std::clamp(sy + (int)(distY(rng)*((float)radius)), 0, source.height);
        }while(isValidPatch(source, cx, cy, patchRadius));
        //if valid, check if patchdist of new sample < best sample
        float c_distance = patchDistance(target, x, y, source, cx, cy, patchRadius);
        if(c_distance < bestDist){
            bestDist = c_distance;
            bestX = cx;
            bestY = cy;
        }
        //if so, set nnf[x, y] to new match

        //shrink radius by half
        radius *= 0.5f;
    }

    bestMatch.dist = bestDist;
    bestMatch.u = bestX;
    bestMatch.v = bestY;

    nnf[x][y] = bestMatch;
}

