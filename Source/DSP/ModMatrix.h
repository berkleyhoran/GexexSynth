#pragma once

#include "ModTarget.h"

namespace gexex
{
    // Deliberately tiny for v1 -- a single (target, depth) pair, not a
    // generalized N-source x M-target matrix (see the build plan's §2).
    // Extending to multiple sources later means adding more of these plus
    // more ModulationSource implementations; ModTarget and each consumer's
    // point-of-use application don't need to change.
    struct ModRoute
    {
        ModTarget target = ModTarget::Off;
        float depth = 0.0f;

        // The fully-resolved, per-target-scaled modulation amount for a
        // given raw LFO reading (-1..+1). 0 if this route targets nothing.
        float resolve(float lfoValue) const noexcept
        {
            return target == ModTarget::Off ? 0.0f : lfoValue * depth * modDepthRange(target);
        }
    };
}
