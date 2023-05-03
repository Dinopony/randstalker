#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include <landstalker-lib/model/entity.hpp>
#include <landstalker-lib/model/map.hpp>
#include "../../logic_model/randomizer_world.hpp"
#include "landstalker-lib/tools/game_text.hpp"

/**
 * This patch provides a generic custom dialogue text system that can be applied to any NPC in any map
 */
class PatchHandleCustomDialogues : public GamePatch
{
private:
    /// A table containing words with the following format: (map_id.12 | entity_id.4)
    ByteArray _custom_text_lut;
    uint32_t _custom_text_lut_addr = 0xFFFFFFFF;

    /// A table of script words triggering the appropriate dialog, with matching indexes with the LUT above
    ByteArray _custom_text_script_words;
    uint32_t _custom_text_script_words_addr = 0xFFFFFFFF;

public:
    static constexpr uint8_t CUSTOM_TEXT_SPEAKER_ID = 0x33; // Dexter script in vanilla

    void alter_world(World& world) override
    {
        RandomizerWorld& randomizer_world = reinterpret_cast<RandomizerWorld&>(world);

        // Apply all custom dialogues
        for(const auto& [entity, custom_string] : randomizer_world.custom_dialogues())
        {
            Map* map = entity->map();

            Map* parent_map = map;
            while(parent_map->parent_map())
                parent_map = parent_map->parent_map();

            // Find the index of CUSTOM_TEXT_SPEAKER_ID among speaker IDs for the parent map (shared with all children)
            std::vector<uint16_t>& speaker_ids = parent_map->speaker_ids();
            uint8_t custom_text_speaker_index = 0xFF;
            for(size_t i=0 ; i<speaker_ids.size() ; ++i)
            {
                if(speaker_ids[i] == CUSTOM_TEXT_SPEAKER_ID)
                {
                    custom_text_speaker_index = i;
                    break;
                }
            }

            // If we didn't find it, it means it's not yet in there. Add it!
            if(custom_text_speaker_index == 0xFF)
            {
                custom_text_speaker_index = speaker_ids.size();
                speaker_ids.emplace_back(CUSTOM_TEXT_SPEAKER_ID);
            }

            entity->talkable(true);
            entity->dialogue(custom_text_speaker_index);

            // Add a map/entity combo inside LUT ByteArray
            uint16_t map_entity_combo = map->id() << 4;
            map_entity_combo |= map->entity_id(entity);
            _custom_text_lut.add_word(map_entity_combo);

            // Inject string in textbanks and get string ID
            if(custom_string.size() >= 254)
                throw LandstalkerException("Custom text is too long.");

            uint16_t string_id = world.first_empty_game_string_id(0x4D);
            world.game_strings()[string_id] = custom_string;

            // Add a script word to trigger display of the injected string from textbanks
            uint16_t script_word = 0xE000 | ((string_id - 0x4D) & 0x1FFF);
            _custom_text_script_words.add_word(script_word);
        }

        _custom_text_lut.add_word(0xFFFF);
    }

    void inject_data(md::ROM& rom, World& world) override
    {
        _custom_text_lut_addr = rom.inject_bytes(_custom_text_lut);
        _custom_text_script_words_addr = rom.inject_bytes(_custom_text_script_words);
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        md::Code func;
        func.movem_to_stack({ reg_D0, reg_D1, reg_D2 }, { reg_A0 });
        {
            func.movew(addr_(0xFF1204), reg_D1);  // D1 = (null.4 | current_map_id.12)
            func.lslw(4, reg_D1);                 // D1 = (current_map_id.12 | null.4)

            func.movel(addr_(0xFF187C), reg_D0);  // D0 = Current NPC address (0x00FF5580)
            func.lsrw(4, reg_D0);                 // D0 =                           (0x58)
            func.subib(0x48, reg_D0);             // D0 =                           (0x10)
            func.lsrb(3, reg_D0);                 // D0 = Current NPC ID             (0x2)
            func.orb(reg_D0, reg_D1);             // D1 = (current_map_id.12 | entity_id.4) --> value to find in LUT

            func.movew(0x0001, reg_D0);           // From now on, D0 = offset in script words table
            func.lea(_custom_text_lut_addr, reg_A0);
            func.label("loop_start");
            {
                func.movew(addr_postinc_(reg_A0), reg_D2); // D2 = (map_id.12 | entity_id.4) from LUT
                func.bmi("return"); // Reached end of list without finding a valid combination (should not happen)

                func.cmpw(reg_D1, reg_D2);  // If we talking to the right entity in the right map...
                func.bne("next_loop");
                {
                    // ... display the matching custom text
                    func.lea(_custom_text_script_words_addr, reg_A0);
                    func.jsr(0x253F8); // RunTextCmd function with D0 being the offset to the good script word
                    func.bra("return");
                }
                func.label("next_loop");
                // Not the right map/entity combo, point on next value and loop back
                func.addqw(0x2, reg_D0);
            }
            func.bra("loop_start");
        }
        func.label("return");
        func.movem_from_stack({ reg_D0, reg_D1, reg_D2 }, { reg_A0 });
        func.rts();

        uint32_t func_process_custom_text = rom.inject_code(func);
        rom.set_code(0x27294, md::Code().jmp(func_process_custom_text)); // Replaces Dexter script (0x33)
    }
};
