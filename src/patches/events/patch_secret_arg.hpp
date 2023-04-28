#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include "../../randomizer_options.hpp"

class PatchSecretARG : public GamePatch
{
private:
    bool _has_golas_blessing;

public:
    PatchSecretARG() = default;

    void inject_code(md::ROM& rom, World& world) override;
};
