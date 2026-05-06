#include "patchmatch.h"
#include <iostream>
#include <omp.h>



//TARGET: the frame we are stylizing
//SOURCE: the texture we are copying pixels from
NNF Patchmatch::run_patchmatch(const Image& target,
                               const Image& source,
                               int patchRadius,
                               int iterations, NNF* prevNNF){
    NNF nnf;
    rng.seed(std::random_device{}());

    initializeNNF(target, source, nnf, patchRadius, prevNNF);

    std::vector<bool> edgeMask = createEdgeMask(target);

    for(int i = 0; i < iterations; i++){
        if(i%2==0){ //Forward propogation
            for(int y = 0; y < target.height; y++){
                for(int x = 0; x < target.width; x++){
                    propogateForward(x, y, target, source, nnf, patchRadius);
                    randomSearch(x, y, target, source, nnf, patchRadius);

                    //std::cout<<"Matching pixel "<<x + y*target.width<<std::endl;
                }
            }
        }else{ //Backward propogation
            for(int y = 0; y < target.height; y++){
                for(int x = 0; x < target.width; x++){
                    propogateBackward(x, y, target, source, nnf, patchRadius);
                    randomSearch(x, y, target, source, nnf, patchRadius);

                    //std::cout<<"Matching pixel "<<x + y*target.width<<std::endl;
                }
            }
        }

        // std::cout<<"Iteration "<<i<<" done"<<std::endl;
    }

    return nnf;
}


std::mt19937 Patchmatch::rng(std::random_device{}());

//
float Patchmatch::patchDistance(const Image& target, int tx, int ty,
                                const Image& source, int sx, int sy,
                                int patchRadius){
    float dist = 0.f;

    for(int dx = -patchRadius; dx <= patchRadius; dx++){
        for(int dy = -patchRadius; dy <= patchRadius; dy++){
            RGB target_rgb = ImageUtils::rgbAt(target, tx+dx, ty+dy);
            RGB source_rgb = ImageUtils::rgbAt(source, sx+dx, sy+dy);

            float dr = target_rgb.r - source_rgb.r;
            float dg = target_rgb.g - source_rgb.g;
            float db = target_rgb.b - source_rgb.b;

            //dist += dr*dr + dg*dg + db*db;
            dist += std::abs(dr) + std::abs(dg) + std::abs(db);
        }
    }

    return dist;
}

bool Patchmatch::isValidPatch(const Image& image, int x, int y, int patchRadius){
    return (x-patchRadius >= 0) && (x+patchRadius < image.width)
           && (y-patchRadius >= 0) && (y+patchRadius < image.height);
}

void Patchmatch::initializeNNF(const Image& target, const Image& source,
                    NNF& nnf, int patchRadius, NNF* prevNNF){

    thread_local std::mt19937 localRng(std::random_device{}());



    nnf.resize(target.width*target.height);


    int minX = patchRadius;
    int maxX = source.width - 1 - patchRadius;
    int minY = patchRadius;
    int maxY = source.height - 1 - patchRadius;

    if(maxX < minX || maxY < minY){
        std::cerr << "Source image too small for patch radius " << patchRadius << std::endl;
        return;
    }

    std::uniform_int_distribution<int> distX(minX, maxX);
    std::uniform_int_distribution<int> distY(minY, maxY);

    for(int y = 0; y < target.height; y++){
        for(int x = 0; x < target.width; x++){
            int sx;
            int sy;
            // do{
            //     //randomize sx to 0 to source.width
            //     sx = distX(localRng);
            //     sy = distY(localRng);
            //     //randomize sy to 0 to source.height
            // }while(!isValidPatch(source, sx, sy, patchRadius));

            if(prevNNF != nullptr){
                // Use upsampled match from coarser level as starting point
                Match prev = (*prevNNF)[y * target.width + x];
                sx = std::clamp(prev.u, minX, maxX);
                sy = std::clamp(prev.v, minY, maxY);
            }
            else{
                sx = distX(localRng);
                sy = distY(localRng);
            }

            Match match;
            match.u = sx;
            match.v = sy;
            match.dist = std::numeric_limits<float>::max();

            //match.dist = patchDistance(target, x, y, source, sx, sy, patchRadius);
            if(isValidPatch(target, x, y, patchRadius)){
                match.dist = patchDistance(target, x, y, source, sx, sy, patchRadius);
            }

            nnf[y*target.width + x] = match;
        }
    }

    // std::cout<<"NNF initialized"<<std::endl;
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


    // candidate 1
    if (!isValidPatch(target, x, y, patchRadius)) return;

    Match current = nnf[y*target.width + x];

    // candidate 2
    if(isValidPatch(target, x-1, y, patchRadius)){
        Match left = nnf[y*target.width + x-1];
        int cx = left.u + 1;
        int cy = left.v;
        float c_dist = patchDistance(target, x, y, source, cx, cy, patchRadius);
        if(isValidPatch(source, cx, cy, patchRadius)){
            float c_dist = patchDistance(target, x, y, source, cx, cy, patchRadius);
            if(c_dist < current.dist){
                current.dist = c_dist;
                current.u = cx;
                current.v = cy;
            }
        }
    }

    // candidate 3
    if(isValidPatch(target, x, y-1, patchRadius)){
        Match left = nnf[(y-1)*target.width + x];
        int cx = left.u;
        int cy = left.v + 1;

        if(isValidPatch(source, cx, cy, patchRadius)){
            float c_dist = patchDistance(target, x, y, source, cx, cy, patchRadius);
            if(c_dist < current.dist){
                current.dist = c_dist;
                current.u = cx;
                current.v = cy;
            }
        }
    }

    nnf[y*target.width + x] = current;
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
     if (!isValidPatch(target, x, y, patchRadius)) return;

    // candidate 1:
    Match curr = nnf[y*target.width + x];

    // candidate 2:
    if (isValidPatch(target, x+1, y, patchRadius)){

        Match neigh = nnf[y*target.width + x+1];
        int cx = neigh.u-1;
        int cy = neigh.v;
        //float dist = patchDistance(target, x, y, source, sx, sy, patchRadius);

        //bool neighValid = isValidPatch(source, cx, cy, patchRadius);
        if(isValidPatch(source, cx, cy, patchRadius)){
            float c_dist = patchDistance(target, x, y, source, cx, cy, patchRadius);
            if(c_dist < curr.dist){
                curr.dist = c_dist;
                curr.u = cx;
                curr.v = cy;
            }
        }
    }

    // candidate 3:
    if (isValidPatch(target, x, y+1, patchRadius)){

        Match neigh = nnf[(y+1)*target.width + x];
        int cx = neigh.u;
        int cy = neigh.v-1;

        if(isValidPatch(source, cx, cy, patchRadius)){  // ← check first
            float c_dist = patchDistance(target, x, y, source, cx, cy, patchRadius);
            if(c_dist < curr.dist){
                curr.dist = c_dist;
                curr.u = cx;
                curr.v = cy;
            }
        }
    }

    nnf[y*target.width + x] = curr;



}

void Patchmatch::randomSearch(int x, int y, const Image& target, const Image& source,
                  NNF& nnf, int patchRadius){
    //init sx and sy to nnf[x,y], this is current best

    if(!isValidPatch(target, x, y, patchRadius)) return;

    Match bestMatch = nnf[y*target.width + x];
    int sx = bestMatch.u;
    int sy = bestMatch.v;
    //initialize search radius
    int radius = std::max(source.width, source.height);

    thread_local std::mt19937 localRng(std::random_device{}());

    std::uniform_real_distribution<float> distX(-1.f, 1.f);
    std::uniform_real_distribution<float> distY(-1.f, 1.f);

    float bestDist = bestMatch.dist;
    int bestX = sx;
    int bestY = sy;

    //while rad > 1
    while(radius > 1){
        int cx, cy;
        //randomly sample in search radius around current best
        // do{
        //     cx = std::clamp(sx + (int)(distX(localRng)*((float)radius)), 0, source.width);
        //     cy = std::clamp(sy + (int)(distY(localRng)*((float)radius)), 0, source.height);
        // }while(!isValidPatch(source, cx, cy, patchRadius));

        cx = std::clamp(sx + (int)(distX(localRng)*radius), patchRadius, source.width-1-patchRadius);
        cy = std::clamp(sy + (int)(distY(localRng)*radius), patchRadius, source.height-1-patchRadius);
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

    nnf[y*target.width + x] = bestMatch;
}


NNF Patchmatch::upscaleNNF(const NNF& nnf, int oldWidth, int oldHeight,
                           int newWidth, int newHeight){
    NNF upsampled(newWidth*newHeight);

    for(int y = 0; y < newHeight; y++){
        for(int x = 0; x < newWidth; x++){
            int cx = std::clamp((int)(x * 0.85f), 0, oldWidth - 1);
            int cy = std::clamp((int)(y * 0.85f), 0, oldHeight - 1);

            Match match = nnf[cy * oldWidth + cx];

            Match upscaleMatch;
            upscaleMatch.u = (int)(match.u / 0.85f);
            upscaleMatch.v = (int)(match.v / 0.85f);
            upscaleMatch.dist = match.dist;

            upsampled[y*newWidth + x] = upscaleMatch;
        }
    }

    return upsampled;
}

std::vector<bool> Patchmatch::createEdgeMask(const Image& target){
    std::vector<bool> edgeMask(target.height*target.width, false);

    int WHITE_THRESHOLD = 200;
    int BOUNDARY_DIST = 1;

    for(int y = 1; y < target.height-1; y++){
        for(int x = 1; x < target.width-1; x++){
            RGB color = ImageUtils::rgbAt(target, x, y);
            bool isWhite = (color.r >= WHITE_THRESHOLD)
                           && (color.g >= WHITE_THRESHOLD)
                           && (color.b >= WHITE_THRESHOLD);

            bool hasNeighbor = false;

            for(int dy = -BOUNDARY_DIST; dy < BOUNDARY_DIST; dy++){
                for(int dx = - BOUNDARY_DIST; dx < BOUNDARY_DIST; dx++){
                    RGB n = ImageUtils::rgbAt(target, dx, dy);
                    if(n.r < WHITE_THRESHOLD && n.g < WHITE_THRESHOLD && n.b < WHITE_THRESHOLD)
                        hasNeighbor = true;
                }
            }

            edgeMask[y*target.width + x] = isWhite && hasNeighbor || !isWhite && hasNeighbor;

        }
    }

    return edgeMask;
}

