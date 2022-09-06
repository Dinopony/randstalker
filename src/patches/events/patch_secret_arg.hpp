#pragma once

#include <landstalker_lib/patches/game_patch.hpp>
#include "../../randomizer_options.hpp"

class PatchSecretARG : public GamePatch
{
private:
    bool _has_golas_blessing;

public:
    explicit PatchSecretARG(const RandomizerOptions& options) :
        _has_golas_blessing(options.hash_sentence().find("Gola") != std::string::npos)
    {}

    void inject_code(md::ROM& rom, World& world) override;
};
