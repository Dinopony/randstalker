#pragma once

#include <string>
#include <vector>
#include <random>
#include <landstalker-lib/tools/vectools.hpp>
#include "logic_model/randomizer_world.hpp"

class BingoGenerator {
private:
    const RandomizerWorld& _world;
    std::mt19937 _rng;

    std::vector<std::string> _casino_goals;
    std::vector<std::string> _key_item_goals;
    std::vector<std::string> _equipment_goals;
    std::vector<std::string> _global_equipment_goals;
    std::vector<std::string> _inn_goals;
    std::vector<std::string> _empty_shop_goals;
    std::vector<std::string> _buy_in_x_shops_goals;
    std::vector<std::string> _church_goals;
    std::vector<std::string> _lifestock_goals;
    std::vector<std::string> _easy_boss_goals;
    std::vector<std::string> _hard_boss_goals;
    std::vector<std::string> _consumable_goals;
    std::vector<std::string> _beat_all_enemy_colors;
    std::vector<std::string> _painful_checks_goals;
    std::vector<std::string> _goofy_goals;
    std::vector<std::string> _dungeon_goals;
    std::vector<std::string> _tree_cutting_goals;
    std::vector<std::string> _big_tree_goals;
    std::vector<std::string> _ending_goals;
    std::vector<std::string> _debuff_goals;

public:
    BingoGenerator(const RandomizerWorld& world, uint32_t seed) :
        _world  (world),
        _rng    (seed)
    {
        init_casino_goals();
        init_key_item_goals();
        init_equipment_goals();
        init_shop_inn_church_goals();
        init_lifestock_goals();
        init_bosses_goals();
        init_consumable_goals();
        init_enemy_color_goals();
        init_painful_checks_goals();
        init_goofy_goals();
        init_dungeon_goals();
        init_tree_cutting_goals();
        init_big_tree_goals();
        init_ending_goals();
        init_debuff_goals();
    }

    Json generate()
    {
        std::vector<std::string> goals = build_goals_list();
        Json file_json = Json::array();
        for(auto& str : goals)
        {
            Json goal_json = Json::object();
            goal_json["name"] = str;
            file_json.emplace_back(goal_json);
        }

        return file_json;
    }

private:
    void init_casino_goals()
    {
        _casino_goals.emplace_back("Win Greenpea's boulder game 2 times");
        _casino_goals.emplace_back("Win Greenpea's boulder game 4 times");
        _casino_goals.emplace_back("Win the chicken race");
        _casino_goals.emplace_back("Win at the roulette");
        _casino_goals.emplace_back("Win at the roulette 2 times");
        _casino_goals.emplace_back("Throw 5+ chicken in the pit in a game of chicken toss at the casino");
        _casino_goals.emplace_back("Throw 8+ chicken in the pit in a game of chicken toss at the casino");

        vectools::shuffle(_casino_goals, _rng);
    }

    void init_key_item_goals()
    {
        // TODO: Use World to list items appropriately, using ItemDistribution
        _key_item_goals.emplace_back("Get Garlic");
        _key_item_goals.emplace_back("Get Logs");
        _key_item_goals.emplace_back("Get Einstein Whistle");
        _key_item_goals.emplace_back("Get Buyer's Card");
        _key_item_goals.emplace_back("Get Idol Stone");
        _key_item_goals.emplace_back("Get Armlet");
        _key_item_goals.emplace_back("Get Casino Ticket");
        _key_item_goals.emplace_back("Get Lantern");
        _key_item_goals.emplace_back("Get Gola's Eye");
        _key_item_goals.emplace_back("Get Gola's Nail");
        _key_item_goals.emplace_back("Get Gola's Horn");
        _key_item_goals.emplace_back("Get Gola's Fang");
        _key_item_goals.emplace_back("Get Safety Pass");
        _key_item_goals.emplace_back("Get any Jewel");
        _key_item_goals.emplace_back("Get Sun Stone");
        _key_item_goals.emplace_back("Get Key");
        _key_item_goals.emplace_back("Get Axe Magic");
        _key_item_goals.emplace_back("Get both Garlics");
        _key_item_goals.emplace_back("Get both Logs");
        _key_item_goals.emplace_back("Get all Gola items");
        _key_item_goals.emplace_back("Get all Jewels");
        vectools::shuffle(_key_item_goals, _rng);
    }
    
    void init_equipment_goals()
    {
        _equipment_goals.emplace_back("Get Iron Boots");
        _equipment_goals.emplace_back("Get Snow Spikes");
        _equipment_goals.emplace_back("Get Fireproof Boots");
        _equipment_goals.emplace_back("Get Healing Boots");
        _equipment_goals.emplace_back("Get Venus Stone");
        _equipment_goals.emplace_back("Get Saturn Stone");
        _equipment_goals.emplace_back("Get Mars Stone");
        _equipment_goals.emplace_back("Get Moon Stone");
        _equipment_goals.emplace_back("Get Magic Sword");
        _equipment_goals.emplace_back("Get Thunder Sword");
        _equipment_goals.emplace_back("Get Ice Sword");
        _equipment_goals.emplace_back("Get Sword of Gaia");
        _equipment_goals.emplace_back("Get Steel Breast");
        _equipment_goals.emplace_back("Get Chrome Breast");
        _equipment_goals.emplace_back("Get Shell Breast");
        _equipment_goals.emplace_back("Get Hyper Breast");
        vectools::shuffle(_equipment_goals, _rng);

        _global_equipment_goals.emplace_back("Get all swords");
        _global_equipment_goals.emplace_back("Get all armors");
        _global_equipment_goals.emplace_back("Get all rings");
        _global_equipment_goals.emplace_back("Get all boots");
        _global_equipment_goals.emplace_back("Equip 2nd column fully (Magic)");
        _global_equipment_goals.emplace_back("Equip 3rd column fully (Thunder)");
        _global_equipment_goals.emplace_back("Equip 4th column fully (Ice)");
        _global_equipment_goals.emplace_back("Equip 5th column fully (Gaia)");
        _global_equipment_goals.emplace_back("Equip a diagonal of equipments");
        vectools::shuffle(_global_equipment_goals, _rng);
    }

    void init_shop_inn_church_goals()
    {
        _inn_goals.emplace_back("Sleep in an inn");
        _inn_goals.emplace_back("Sleep in an inn while having 1 HP");
        for(int i = 2 ; i <= 5 ; ++i)
            _inn_goals.emplace_back("Sleep in " + std::to_string(i) + " different inns");
        vectools::shuffle(_inn_goals, _rng);

        for(int i = 4 ; i <= 9 ; ++i)
            _buy_in_x_shops_goals.emplace_back("Buy an item in " + std::to_string(i) + " different shops");
        vectools::shuffle(_buy_in_x_shops_goals, _rng);

        _empty_shop_goals.emplace_back("Buy each item at least once in Massan shop");
        _empty_shop_goals.emplace_back("Buy each item at least once in Gumi shop");
        _empty_shop_goals.emplace_back("Buy each item at least once in Ryuma main shop");
        _empty_shop_goals.emplace_back("Buy each item at least once in Mercator town shop");
        _empty_shop_goals.emplace_back("Buy each item at least once in Mercator docks shop");
        _empty_shop_goals.emplace_back("Buy each item at least once in Mercator special shop");
        _empty_shop_goals.emplace_back("Buy each item at least once in Verla shop");
        _empty_shop_goals.emplace_back("Buy each item at least once in Kelketo Waterfalls shop");
        _empty_shop_goals.emplace_back("Buy each item at least once in Destel main shop");
        _empty_shop_goals.emplace_back("Buy each item at least once in Greedly's shop");
        _empty_shop_goals.emplace_back("Buy each item at least once in Kazalt shop");
        _empty_shop_goals.emplace_back("Buy the item in both Destel and Ryuma inns");
        vectools::shuffle(_empty_shop_goals, _rng);

        _church_goals.emplace_back("Use Spell Book while in a church");
        _church_goals.emplace_back("Cure a debuff using a book in a church");
        for(int i=2 ; i<=7 ; ++i)
            _church_goals.emplace_back("Save in " + std::to_string(i) + " different churches");
        vectools::shuffle(_church_goals, _rng);
    }

    void init_lifestock_goals()
    {
        for(int i = 30 ; i <= 70 ; i += 10)
            _lifestock_goals.emplace_back("Reach " + std::to_string(i) + " total life");
        for(int i = 2 ; i <= 6 ; ++i)
            _lifestock_goals.emplace_back("Buy " + std::to_string(i) + " Life Stock in shops");
        vectools::shuffle(_lifestock_goals, _rng);
    }

    void init_bosses_goals()
    {
        _easy_boss_goals.emplace_back("Beat Orc Kings in Swamp Shrine");
        _easy_boss_goals.emplace_back("Beat Silver Knight in Thieves Hideout");
        _easy_boss_goals.emplace_back("Beat Stone Quake");
        _easy_boss_goals.emplace_back("Beat the Duke");
        _easy_boss_goals.emplace_back("Beat White Spinner");
        _easy_boss_goals.emplace_back("Beat Miro once");
        vectools::shuffle(_easy_boss_goals, _rng);

        _hard_boss_goals.emplace_back("Beat King Nole");
        _hard_boss_goals.emplace_back("Beat Firedemon");
        _hard_boss_goals.emplace_back("Beat Miro twice");
        _hard_boss_goals.emplace_back("Beat Mir");
        _easy_boss_goals.emplace_back("Beat Red Spinner");
        _easy_boss_goals.emplace_back("Beat Golden Quake");
        _hard_boss_goals.emplace_back("Beat Zak");
        vectools::shuffle(_hard_boss_goals, _rng);
    }

    void init_consumable_goals()
    {
        _consumable_goals.emplace_back("Have 9 EkeEke");
        for(int i = 6 ; i <= 9 ; ++i)
        {
            _consumable_goals.emplace_back("Have " + std::to_string(i) +" Detox Grass");
            _consumable_goals.emplace_back("Have " + std::to_string(i) +" Dahl");
            _consumable_goals.emplace_back("Have " + std::to_string(i) +" Anti-Paralyze");
            _consumable_goals.emplace_back("Have " + std::to_string(i) +" Restoration");
            _consumable_goals.emplace_back("Have " + std::to_string(i) +" Statue of Gaia");
            _consumable_goals.emplace_back("Have " + std::to_string(i) +" Golden Statue");
        }
        vectools::shuffle(_consumable_goals, _rng);
        // Short Cakes ? Pawn Tickets ?
    }

    void init_enemy_color_goals()
    {
        _beat_all_enemy_colors.emplace_back("Beat all 7 color variants of Slimes");
        _beat_all_enemy_colors.emplace_back("Beat all 3 color variants of Orcs");
        _beat_all_enemy_colors.emplace_back("Beat all 3 color variants of Mushrooms");
        _beat_all_enemy_colors.emplace_back("Beat all 3 color variants of Cyclops");
        _beat_all_enemy_colors.emplace_back("Beat all 3 color variants of Knights");
        _beat_all_enemy_colors.emplace_back("Beat all 3 color variants of Mummies");
        _beat_all_enemy_colors.emplace_back("Beat all 3 color variants of Lizards");
        vectools::shuffle(_beat_all_enemy_colors, _rng);
    }

    void init_painful_checks_goals()
    {
        _painful_checks_goals.emplace_back("Morph back to human form after being a dog");
        _painful_checks_goals.emplace_back("Beat Fahl in his dojo");
        _painful_checks_goals.emplace_back("Beat the Mummy in the Crypt");
        _painful_checks_goals.emplace_back("Beat Slasher in Verla mines");
        _painful_checks_goals.emplace_back("Beat Dex in Verla mines");
        _painful_checks_goals.emplace_back("Call the lumberjack using the whistle");
        _painful_checks_goals.emplace_back("Fix the lighthouse using the Sun Stone");
        _painful_checks_goals.emplace_back("Visit all rooms of Massan Cave");
        _painful_checks_goals.emplace_back("Free Tibor");
        vectools::shuffle(_painful_checks_goals, _rng);
    }

    void init_goofy_goals()
    {
        _goofy_goals.emplace_back("Win a beauty contest using the Blue Ribbon");
        _goofy_goals.emplace_back("Use Death Statue");
        _goofy_goals.emplace_back("Give Shortcake to Friday");
        _goofy_goals.emplace_back("Use Pawn Ticket with 1 HP");
        _goofy_goals.emplace_back("Jump on Foxy's head");
        _goofy_goals.emplace_back("Jump on Massan statue (no dog throw)");
        _goofy_goals.emplace_back("Break all jars inside grandma's house in Mercator");
        _goofy_goals.emplace_back("Use a Dahl while at 1 HP");
        _goofy_goals.emplace_back("Enlighten a room using Lantern");
        vectools::shuffle(_goofy_goals, _rng);
    }

    void init_dungeon_goals()
    {
        _dungeon_goals.emplace_back("Enter Waterfall Shrine");
        _dungeon_goals.emplace_back("Enter Massan Cave");
        _dungeon_goals.emplace_back("Enter Swamp Shrine");
        _dungeon_goals.emplace_back("Enter Thieves Hideout");
        _dungeon_goals.emplace_back("Enter Mercator Dungeon");
        _dungeon_goals.emplace_back("Enter Verla Mines");
        _dungeon_goals.emplace_back("Enter Destel Well");
        _dungeon_goals.emplace_back("Enter Lake Shrine");
        _dungeon_goals.emplace_back("Enter King Nole's Palace");
        vectools::shuffle(_dungeon_goals, _rng);
    }

    void init_tree_cutting_goals()
    {
        for(int i=4 ; i <= 12 ; ++i)
            _tree_cutting_goals.emplace_back("Cut " + std::to_string(i) + " Sacred Trees using Axe Magic");
        vectools::shuffle(_tree_cutting_goals, _rng);
    }

    void init_big_tree_goals()
    {
        _big_tree_goals.emplace_back("Enter big tree near Ryuma");
        _big_tree_goals.emplace_back("Enter big tree near Massan");
        _big_tree_goals.emplace_back("Enter big tree near Mercator gate");
        _big_tree_goals.emplace_back("Enter big tree near Verla");
        _big_tree_goals.emplace_back("Enter big tree near Mir Tower");
        _big_tree_goals.emplace_back("Enter big tree in Mountainous Area");
        _big_tree_goals.emplace_back("Enter big tree near Destel");
        _big_tree_goals.emplace_back("Enter big tree near Lake Shrine");
        _big_tree_goals.emplace_back("Enter big tree at Greenmaze entrance");
        _big_tree_goals.emplace_back("Enter big tree near Greenmaze end");

        for(int i=2 ; i<=10 ; ++i)
            _big_tree_goals.emplace_back("Enter in " + std::to_string(i) + " unique trees");

        vectools::shuffle(_big_tree_goals, _rng);
    }

    void init_ending_goals()
    {
        _ending_goals.emplace_back("Get any ending");
        _ending_goals.emplace_back("Get the white ending");
        _ending_goals.emplace_back("Get the blue ending");
        vectools::shuffle(_ending_goals, _rng);
    }

    void init_debuff_goals()
    {
        for(int i = 2 ; i <= 7 ; ++i)
            _debuff_goals.emplace_back("Get poisoned " + std::to_string(i) + " times");

        for(int i = 2 ; i <= 6 ; ++i)
            _debuff_goals.emplace_back("Get immobilized " + std::to_string(i) + " times");
        _debuff_goals.emplace_back("Heal from immobilization without dying");

        for(int i = 2 ; i <= 5 ; ++i)
            _debuff_goals.emplace_back("Get confused " + std::to_string(i) + " times");
        _debuff_goals.emplace_back("Heal from confusion without dying");

        for(int i = 2 ; i <= 3 ; ++i)
            _debuff_goals.emplace_back("Get cursed " + std::to_string(i) + " times");
        _debuff_goals.emplace_back("Heal from a curse without dying");
        vectools::shuffle(_debuff_goals, _rng);
    }

    std::vector<std::string> build_goals_list()
    {
        // We need to build a list with 25 goals (5x5 grid)
        std::vector<std::string> goals;
        goals.resize(25, "Uncompletable");

        // 1 casino goal
        goals[0] = _casino_goals[0];

        // 2 "get key item" goals
        goals[1] = _key_item_goals[0];
        goals[2] = _key_item_goals[1];
        goals[22] = _key_item_goals[2];

        // 2 equipment-related goals
        goals[3] = _equipment_goals[0];
        goals[4] = _global_equipment_goals[0];

        // 3 shops, inns & churches related goals
        std::vector<std::string> shops_inns_churches_goals {
            _inn_goals[0], _buy_in_x_shops_goals[0], _empty_shop_goals[0], _church_goals[0]
        };
        vectools::shuffle(shops_inns_churches_goals, _rng);
        goals[5] = shops_inns_churches_goals[0];
        goals[6] = shops_inns_churches_goals[1];
        goals[7] = shops_inns_churches_goals[2];

        // 1 gold related goal
        std::vector<uint32_t> sums { 1000, 1200, 1400, 1600, 1800, 2000, 2200, 2400 };
        vectools::shuffle(sums, _rng);
        goals[8] = "Have " + std::to_string(sums[0]) + " gold in your pockets";

        // 1 Life-stock related goal
        goals[9] = _lifestock_goals[0];

        // 2 boss-related goals
        goals[10] = _easy_boss_goals[0];
        goals[11] = _hard_boss_goals[0];

        // 1 consumable goal
        goals[12] = _consumable_goals[0];

        // 1 enemy color goal
        goals[13] = _beat_all_enemy_colors[0];

        // 2 painful checks goals
        goals[14] = _painful_checks_goals[0];
        goals[15] = _painful_checks_goals[1];

        // 2 goofy goals
        goals[16] = _goofy_goals[0];
        goals[17] = _goofy_goals[1];

        // 1 dungeon goal
        goals[18] = _dungeon_goals[0];

        // 1 big tree goal
        goals[19] = _big_tree_goals[0];

        // 1 ending goal
        goals[20] = _ending_goals[0];

        // 1 debuff goal
        goals[21] = _debuff_goals[0];

        // 1 tree cutting goal
        goals[23] = _tree_cutting_goals[0];

        goals[24] = "Uncompletable";

        vectools::shuffle(goals, _rng);
        return goals;
    }
};
