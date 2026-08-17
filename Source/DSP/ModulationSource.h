#pragma once

namespace gexex
{
    // Tiny interface so a future second LFO / mod envelope can be added
    // without touching how modulation gets applied (see the build plan's
    // §2). Only the Lfo implements this for now.
    struct ModulationSource
    {
        virtual ~ModulationSource() = default;
        virtual float getCurrentValue() const noexcept = 0;
    };
}
