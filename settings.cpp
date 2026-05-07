#include "settings.h"

namespace AppSettings {

// Input texture sampled by PatchMatch. Larger textures offer more variety but
// increase pyramid construction, deformation, and search space cost.
const QString texturePath = "../color-me-noisy/textures/dithertone.png";

// Input video used for target frames. Longer/larger videos increase decode and
// synthesis time; current processing still stops after the first frame.
const QString videoPath = "../color-me-noisy/source_videos/cicero_1.mov";

// Enables boundary-aware sampling. true preserves watercolor border texture but
// adds mask creation and more constrained PatchMatch search; false is faster and
// uses the original single-source PatchMatch behavior.
const bool useBorder = false;

// Deforms the source texture once per processed frame before building its
// pyramid. true adds organic texture variation but costs a full deformation pass
// per frame. false is faster and keeps the source texture stable/repeatable.
const bool deformTexture = false;

// Patch radius in pixels. Larger patches preserve broader texture structure but
// are much slower because each candidate compares (2r+1)^2 pixels. Smaller
// patches are faster and more local, but can look noisier or less coherent.
const int patchRadius = 6;

// PatchMatch sweep count. More iterations usually improve matches and temporal/
// spatial coherence, but runtime scales roughly linearly. Fewer iterations are
// faster but can leave obviously wrong colors/patches.
const int patchmatchIterations = 4;

// Reserved for outer Wexler-style refinement passes. Increasing would multiply
// the cost of the synthesis loop if wired in; currently kept as a tuning value.
const int wexlerIterations = 1;

// Source deformation control grid spacing. Larger values deform the texture more
// smoothly and cheaply. Smaller values allow more local warping but cost more in
// the deformation pass.
const int gridSize = 100;

// Pyramid blur/downsampling control. Larger values create more/coarser pyramid
// smoothing and can improve large-scale coherence, but add pyramid levels/work.
// Smaller values are faster and sharper, but may make matching less stable.
const float filterStrength = 0.5f;

// Pixels with all RGB channels above this are treated as white background by the
// automatic border detector. Raising it makes background detection stricter;
// lowering it includes more off-white paper/wash as background.
const float whiteThreshold = 0.78f;

// Width of the automatic border band around watercolor/background transitions.
// Larger values preserve a wider edge region but constrain more pixels and can
// slow matching. Smaller values are faster and affect only a thin boundary.
const int boundaryDistance = 6;

// Random candidates tested during initial NNF setup for non-boundary pixels.
// More trials reduce bad initial matches but directly increase initialization
// time. Fewer trials are faster but rely more on propagation/random search.
const int initialRandomTrials = 12;

// Random candidates tested at each search radius for normal pixels. More trials
// improve color/patch matching but are one of the main runtime costs. Fewer
// trials are faster but can miss good source locations.
const int randomSearchTrials = 8;

// Same as randomSearchTrials, but for boundary-constrained pixels. This often
// needs to be higher because valid boundary candidates are sparse; increasing it
// improves border quality but can noticeably slow boundary-heavy frames.
const int boundaryRandomSearchTrials = 32;

}
