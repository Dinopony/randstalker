#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include <landstalker-lib/model/entity_type.hpp>
#include "../../randomizer_options.hpp"

/**
 * Apply all of the difficulty factor settings on the enemy internal values.
 * This must be done as a patch and not as a function inside "apply_randomizer_options" mostly because there is a
 * "NormalizeBossesHP" patch (which needs to be run BEFORE this one) which make drastic changes in the way
 * bosses HP are stored and affected the end result by a lot.
 *
 * Having this as its own patch enables running this AFTER normalization, giving coherent results.
 */
class PatchApplyDifficultyFactors : public GamePatch
{
private:
    const RandomizerOptions& _options;

public:
    explicit PatchApplyDifficultyFactors(const RandomizerOptions& options) : _options(options)
    {}

    void alter_world(World& world) override
    {
        // Apply multiplicators from randomizer settings
        for (auto& [id, entity_type] : world.entity_types())
        {
            if(entity_type->type_name() != "enemy")
                continue;
            EnemyType* enemy_type = reinterpret_cast<EnemyType*>(entity_type);

            enemy_type->apply_damage_factor(_options.enemies_damage_factor());
            enemy_type->apply_health_factor(_options.enemies_health_factor());
            enemy_type->apply_armor_factor(_options.enemies_armor_factor());
            enemy_type->apply_golds_factor(_options.enemies_golds_factor());
            enemy_type->apply_drop_chance_factor(_options.enemies_drop_chance_factor());
        }
    }
};
