#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

namespace AppSettings {

extern const QString texturePath;
extern const QString videoPath;

extern const bool useBorder;
extern const bool deformTexture;
extern const bool backgroundIsWhite;

extern const int patchRadius;
extern const int patchmatchIterations;
extern const int wexlerIterations;
extern const int gridSize;

extern const float filterStrength;
extern const float whiteThreshold;
extern const int boundaryDistance;

extern const int initialRandomTrials;
extern const int randomSearchTrials;
extern const int boundaryRandomSearchTrials;

}

#endif // SETTINGS_H
