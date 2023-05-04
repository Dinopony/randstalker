#include "patches.hpp"

#include "../randomizer_options.hpp"
#include "../personal_settings.hpp"

#include <landstalker-lib/patches/patches.hpp>

#include "edit_content/patch_edit_dialogues.hpp"
#include "edit_content/patch_shorten_cutscenes.hpp"
#include "edit_content/patch_fahl_challenge.hpp"
#include "edit_content/patch_rando_world_edits.hpp"
#include "edit_content/patch_title_screen.hpp"
#include "edit_content/patch_credits.hpp"
#include "edit_content/patch_make_ryuma_mayor_saveable.hpp"
#include "edit_content/patch_add_foxies.hpp"

#include "gameplay_tweaks/patch_remove_tibor_requirement.hpp"
#include "gameplay_tweaks/patch_allow_whistle_usage_behind_trees.hpp"
#include "gameplay_tweaks/patch_remove_ekeeke_auto_revive.hpp"
#include "gameplay_tweaks/patch_remove_tree_cutting_glitch_drops.hpp"
#include "gameplay_tweaks/patch_sword_of_gaia_in_volcano.hpp"
#include "gameplay_tweaks/patch_faster_gaia_effect.hpp"
#include "gameplay_tweaks/patch_faster_pawn_ticket.hpp"
#include "gameplay_tweaks/patch_set_lifestocks_health.hpp"
#include "gameplay_tweaks/patch_armor_upgrades.hpp"
#include "gameplay_tweaks/patch_show_all_books_in_churches.hpp"
#include "gameplay_tweaks/patch_remove_continue_prompt_after_saving.hpp"
#include "gameplay_tweaks/patch_alternative_goals.hpp"

#include "item_effects/patch_lithograph_hint_on_use.hpp"
#include "item_effects/patch_oracle_stone_hint_on_use.hpp"
#include "item_effects/patch_spell_book_teleport_on_use.hpp"
#include "item_effects/patch_record_book_save_on_use.hpp"
#include "item_effects/patch_consumable_pawn_ticket.hpp"
#include "item_effects/patch_permanent_key.hpp"

#include "technical/patch_apply_item_sources.hpp"
#include "technical/patch_fix_armlet_skip.hpp"
#include "technical/patch_fix_softlocks.hpp"
#include "technical/patch_handle_jewels.hpp"
#include "technical/patch_remove_music.hpp"
#include "technical/patch_swap_overworld_music.hpp"
#include "technical/patch_remove_story_dependencies.hpp"
#include "technical/patch_save_select_show_hash_option.hpp"
#include "technical/patch_randomizer_adaptations.hpp"
#include "technical/patch_fix_item_checks.hpp"
#include "technical/patch_improve_gold_rewards_handling.hpp"
#include "technical/patch_improve_lantern_handling.hpp"
#include "technical/patch_flags_for_ground_items.hpp"
#include "technical/patch_handle_archipelago.hpp"
#include "technical/patch_update_teleport_tree_connections.hpp"
#include "technical/patch_on_walk_effects.hpp"
#include "technical/patch_individual_price_for_shop_sources.hpp"
#include "technical/patch_handle_custom_dialogues.hpp"

#include "optimization/patch_optimize_maps.hpp"
#include "optimization/patch_optimize_destel_well_map.hpp"
#include "optimization/patch_optimize_swamp_shrine_map.hpp"
#include "optimization/patch_optimize_tibor_maps.hpp"
#include "optimization/patch_optimize_lake_shrine_platforms_map.hpp"

#include "events/patch_christmas_event.hpp"
#include "events/patch_secret_arg.hpp"
#include "edit_content/patch_add_golas_heart.hpp"

void apply_randomizer_patches(md::ROM& rom, RandomizerWorld& world, const RandomizerOptions& options, const PersonalSettings& personal_settings)
{
    std::vector<GamePatch*> patches;

    // =======================================================
    // Engine / Technical improvements
    patches.emplace_back(new PatchExtendROM());
    patches.emplace_back(new PatchExtendTileSwapFlagRange());
    patches.emplace_back(new PatchExtendVisitedFlagRange());
    patches.emplace_back(new PatchImproveInGameTimer());
    patches.emplace_back(new PatchAddSoftReset());
    patches.emplace_back(new PatchNormalizeBossesHP());
    patches.emplace_back(new PatchNewMapFormat());
    patches.emplace_back(new PatchImproveItemUseHandling());
    patches.emplace_back(new PatchImproveGoldRewardsHandling());
    patches.emplace_back(new PatchImproveLanternHandling());
    patches.emplace_back(new PatchOptimizeMaps());
    patches.emplace_back(new PatchOptimizeDestelWellMap());
    patches.emplace_back(new PatchOptimizeSwampShrineMap());
    patches.emplace_back(new PatchOptimizeTiborMaps());
    patches.emplace_back(new PatchOptimizeLakeShrinePlatformsMap());
    patches.emplace_back(new PatchOptimizeAILookup());
    patches.emplace_back(new PatchOptimizeReorderDrawOrderList());
    patches.emplace_back(new PatchOptimizeCollisionDetect());
    patches.emplace_back(new PatchFlagsForGroundItems(options));
    patches.emplace_back(new PatchDisableRegionCheck());

    // =======================================================
    // Randomizer adjustments to make it playable / interesting
    patches.emplace_back(new PatchNewGame(personal_settings.add_ingame_item_tracker()));
    patches.emplace_back(new PatchApplyItemSources());
    patches.emplace_back(new PatchRandoWorldEdits());
    patches.emplace_back(new PatchRandomizerAdaptations());
    patches.emplace_back(new PatchFixItemChecks());
    patches.emplace_back(new PatchFixSoftlocks());
    patches.emplace_back(new PatchSwordOfGaiaInVolcano());
    patches.emplace_back(new PatchConsumablePawnTicket());
    patches.emplace_back(new PatchPermanentKey());
    patches.emplace_back(new PatchAddFoxies());
    patches.emplace_back(new PatchRemoveStoryDependencies());
    patches.emplace_back(new PatchHandleJewels(options.jewel_count()));
    patches.emplace_back(new PatchMakeRyumaMayorSaveable());
    patches.emplace_back(new PatchFahlChallenge());
    patches.emplace_back(new PatchShowAllBooksInChurches());
    patches.emplace_back(new PatchSetLifestocksHealth(options.health_gained_per_lifestock()));
    patches.emplace_back(new PatchUpdateTeleportTreeConnections());
    patches.emplace_back(new PatchIndividualPriceForShopSources());
    patches.emplace_back(new PatchAlternativeGoals(options.goal()));
    if(options.remove_tree_cutting_glitch_drops())
        patches.emplace_back(new PatchRemoveTreeCuttingGlitchDrops());
    if(options.use_armor_upgrades())
        patches.emplace_back(new PatchArmorUpgrades());
    if(options.fix_armlet_skip())
        patches.emplace_back(new PatchFixArmletSkip());
    if (options.remove_tibor_requirement())
        patches.emplace_back(new PatchRemoveTiborRequirement());
    if(options.allow_whistle_usage_behind_trees())
        patches.emplace_back(new PatchAllowWhistleUsageBehindTrees());
    if(!options.ekeeke_auto_revive())
        patches.emplace_back(new PatchRemoveEkeEkeAutoRevive());

    if(options.archipelago_world())
        patches.emplace_back(new PatchHandleArchipelago(options));

    // =======================================================
    // Randomizer-specific item effects
    patches.emplace_back(new PatchLithographHintOnUse());
    patches.emplace_back(new PatchOracleStoneHintOnUse());
    patches.emplace_back(new PatchSpellBookTeleportOnUse(options.consumable_spell_book()));
    patches.emplace_back(new PatchRecordBookSaveOnUse(options.consumable_record_book()));
    patches.emplace_back(new PatchOnWalkEffects(options.archipelago_world()));

    // =======================================================
    // Fluff / quality of life
    patches.emplace_back(new PatchTitleScreen());
    patches.emplace_back(new PatchCredits(30));
    patches.emplace_back(new PatchEditDialogues());
    patches.emplace_back(new PatchShortenCutscenes());
    patches.emplace_back(new PatchFasterGaiaEffect());
    patches.emplace_back(new PatchFasterPawnTicket());
    patches.emplace_back(new PatchSaveSelectShowHashOption(options.hash_sentence()));
    patches.emplace_back(new PatchAlterInventoryOrder(personal_settings.inventory_order()));
    patches.emplace_back(new PatchAlterNigelColors(personal_settings.nigel_colors()));
    patches.emplace_back(new PatchAlterUIColor(personal_settings.hud_color()));
    patches.emplace_back(new PatchRemoveContinuePromptAfterSaving());
    patches.emplace_back(new PatchFixHUDTilemap());
    if(options.fast_transitions())
        patches.emplace_back(new PatchFasterTransitions());
    if(personal_settings.remove_music())
        patches.emplace_back(new PatchRemoveMusic());
    if(personal_settings.swap_overworld_music())
        patches.emplace_back(new PatchSwapOverworldMusic());

    // =======================================================
    // Special events
    if(options.christmas_event())
        patches.emplace_back(new PatchChristmasEvent());
    if(options.goal() == "beat_dark_nole" || options.secret_event())
        patches.emplace_back(new PatchAddGolasHeart());
    if(options.secret_event())
        patches.emplace_back(new PatchSecretARG());

    // Custom dialogues patch needs to be applied last as it needs to process dialogues introduced by other patches
    patches.emplace_back(new PatchHandleCustomDialogues());

    execute_patches(patches, rom, world);
}
