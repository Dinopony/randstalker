#include "kaizo.hpp"
#include "../../world_model/item_source.hpp"

void clear_item_sources(World& world)
{
//    std::vector<ItemSource*> kept_item_sources;    
    for(ItemSource* source : world.item_sources())
        delete source;
    
    world.item_sources().clear();
//    world.item_sources().insert(world.item_sources().end(), kept_item_sources.begin(), kept_item_sources.end());
}

void neutralize_one_time_events(md::ROM& rom)
{
    rom.set_word(0x1A974, 0xFFFF);
    rom.mark_empty_chunk(0x1A976, 0x1A9BE);
}

void neutralize_custom_room_actions(md::ROM& rom)
{
    rom.set_code(0x19B62, md::Code().rts());
    rom.mark_empty_chunk(0x19B64, 0x1A30A);
}

void empty_all_persistence_flags(World& world)
{
    for(auto& [map_id, map] : world.maps())
    {
        for(Entity* entity : map->entities())
        {
            if(entity->entity_type_id() != ENTITY_SACRED_TREE)
                entity->clear_persistence_flag();
        }
    }
}

void add_set_persistence_flag_behavior_operand(md::ROM& rom)
{
    md::Code set_persistence_flag_operand;
    set_persistence_flag_operand.movem_to_stack({ reg_D0, reg_D1, reg_D2 }, { reg_A0 });
    set_persistence_flag_operand.lea(offsets::PERSISTENCE_FLAGS_TABLE, reg_A0);
    set_persistence_flag_operand.moveq(0xFF, reg_D0);
    set_persistence_flag_operand.jsr(0x1A4D0); // FindNextRoomFlagMatch function
    set_persistence_flag_operand.tstb(reg_D0);
    set_persistence_flag_operand.bmi(3);
        set_persistence_flag_operand.lea(0xFF1000, reg_A0);
        set_persistence_flag_operand.bset(reg_D0, addr_(reg_A0, reg_D1));
    set_persistence_flag_operand.movem_from_stack({ reg_D0, reg_D1, reg_D2 }, { reg_A0 });
    set_persistence_flag_operand.rts();

    uint32_t addr = rom.inject_code(set_persistence_flag_operand);

    // Replace the "set_speed_to_8_and_follow_up" operand by our new set_persistence_flag operand
    rom.set_code(0x18252, md::Code().jsr(addr));

    // Update behavior 544 to become the new go-to behavior for persistent buttons
    rom.set_word(0x9D26E, 0x2200);
}

void add_invisible_behavior(md::ROM& rom)
{
    rom.set_word(0x9D1CD, 0x0000); // 0x0000 = End of behavior

    // Make entities invisible by bypassing the behavior system which is too slow and
    // makes the player faintly see entities on room load
    md::Code func_turn_entities_invisible;
    func_turn_entities_invisible.add_bytes(rom.data_chunk(0x19520, 0x19526));
    func_turn_entities_invisible.movem_to_stack({}, { reg_A5 });
    func_turn_entities_invisible.lea(0xFF5480, reg_A5);
    func_turn_entities_invisible.label("loop_start");
    func_turn_entities_invisible.cmpiw(0xFFFF, addr_(reg_A5));
    func_turn_entities_invisible.bne(3);
        func_turn_entities_invisible.movem_from_stack({}, { reg_A5 });
        func_turn_entities_invisible.rts();
    func_turn_entities_invisible.cmpiw(0xD1CD, addr_(reg_A5, 0x34));
    func_turn_entities_invisible.bne(2);
        func_turn_entities_invisible.orib(0x40, addr_(reg_A5, 0x0C));
    func_turn_entities_invisible.adda(0x80, reg_A5);
    func_turn_entities_invisible.bra("loop_start");

    uint32_t addr_hide = rom.inject_code(func_turn_entities_invisible);
    rom.set_code(0x19520, md::Code().jsr(addr_hide));

    // Make a function to reveal invisible entities 
    md::Code func_reveal_invisible_entities;
    func_reveal_invisible_entities.movem_to_stack({}, { reg_A5 });
    func_reveal_invisible_entities.trap(0, { 0x00, 0x6B }); // Play magic sound
    func_reveal_invisible_entities.lea(0xFF5480, reg_A5);
    func_reveal_invisible_entities.label("loop_start");
    func_reveal_invisible_entities.cmpiw(0xFFFF, addr_(reg_A5));
    func_reveal_invisible_entities.bne(3);
        func_reveal_invisible_entities.movem_from_stack({}, { reg_A5 });
        func_reveal_invisible_entities.rts();
    func_reveal_invisible_entities.cmpiw(0xD1CD, addr_(reg_A5, 0x34));
    func_reveal_invisible_entities.bne(2);
        func_reveal_invisible_entities.andib(0xBF, addr_(reg_A5, 0x0C));
    func_reveal_invisible_entities.adda(0x80, reg_A5);
    func_reveal_invisible_entities.bra("loop_start");

    // Attach reveal function on Gola's Eye item use
    uint32_t addr_reveal = rom.inject_code(func_reveal_invisible_entities);
    md::Code reveal_injector;
    reveal_injector.jsr(addr_reveal);
    reveal_injector.jmp(0x8BBA); // ReturnSuccess
    rom.set_code(0x892E, reveal_injector);
    rom.mark_empty_chunk(0x892E + reveal_injector.size(), 0x897A);

    // Neutralize Gola's Eye post-use
    rom.set_code(0x8C7B, md::Code().rts());
}

void handle_morpher(md::ROM& rom)
{
    constexpr uint8_t TRANSFORM_SPRITE = 0x4D;

    md::Code func_smolifier;
    func_smolifier.trap(0, { 0x00, 0x5B }); // Play dog transform sound
    func_smolifier.moveb(0x3, addr_(0xFF1153)); // Forbid attacking and picking up liftable items
	func_smolifier.movew(0x8100 + TRANSFORM_SPRITE, addr_(0xFF540A));
    func_smolifier.moveb(0x00, addr_(0xFF540C));
    func_smolifier.moveb(0x0E, addr_(0xFF543B)); // Sprite dog
    func_smolifier.moveb(0x01, addr_(0xFF546F));
    func_smolifier.moveb(0x0F, addr_(0xFF5405)); // Set lowered height
    func_smolifier.subib(0x10, addr_(0xFF5455)); // Lower hitbox height by one tile
    func_smolifier.lea(0xFF5400, reg_A1);
    func_smolifier.moveb(addr_(0xFF5404), reg_D1);
    func_smolifier.jsr(0x1979C); // SetSpriteRotationAnimFlags
    func_smolifier.rts();
    uint32_t addr_smolifier = rom.inject_code(func_smolifier);

    md::Code func_desmolifier;
    func_desmolifier.trap(0, { 0x00, 0x5B }); // Play dog transform sound
    func_desmolifier.moveb(0x0, addr_(0xFF1153)); // Forbid attacking and picking up liftable items
	func_desmolifier.movew(0x8100, addr_(0xFF540A));
	func_desmolifier.moveb(0x00, addr_(0xFF540C));
    func_desmolifier.moveb(0x00, addr_(0xFF543B));
    func_desmolifier.moveb(0x00, addr_(0xFF546F));
    func_desmolifier.moveb(0x1F, addr_(0xFF5405)); // Set base height
    func_desmolifier.addib(0x10, addr_(0xFF5455)); // Raise hitbox height by one tile
    func_desmolifier.lea(0xFF5400, reg_A1);
    func_desmolifier.moveb(addr_(0xFF5404), reg_D1);
    func_desmolifier.jsr(0x1979C); // SetSpriteRotationAnimFlags
    func_desmolifier.rts();
    uint32_t addr_desmolifier = rom.inject_code(func_desmolifier);

    // Inject toggle on Einstein Whistle use
    md::Code injector;
    injector.cmpib(TRANSFORM_SPRITE, addr_(0xFF540B));
    injector.beq(3);
        injector.jsr(addr_smolifier);
        injector.bra(2);
    injector.jsr(addr_desmolifier);
    injector.jmp(0x8BBA); // ReturnSuccess
    rom.set_code(0x8886, injector);

    // Neutralize PostUseEinsteinWhistle
    rom.set_code(0x8BF2, md::Code().rts());
    rom.mark_empty_chunk(0x8BF4, 0x8C7C);

    // Make smolification stay across room changes
    md::Code room_change_handler;
    room_change_handler.lea(0xFF5400, reg_A1); // Do the erased instruction needed to inject this one
    room_change_handler.cmpib(TRANSFORM_SPRITE, addr_(0xFF540B));
    room_change_handler.bne(2);
        room_change_handler.moveb(0x0F, addr_(0xFF5405)); // Set lowered height
    room_change_handler.rts();
    uint32_t addr_room_change_handler = rom.inject_code(room_change_handler);
    rom.set_code(0x284C, md::Code().jsr(addr_room_change_handler));

    // Desmolify and OHKO on hit
    md::Code ohko_handler;
    ohko_handler.cmpib(TRANSFORM_SPRITE, addr_(0xFF540B));
    ohko_handler.bne(3);
        ohko_handler.jsr(addr_desmolifier);
        ohko_handler.movew(0x0000, addr_(0xFF543E)); // Set health to 0
    ohko_handler.jsr(0x16928); // RefreshCurrentHealthHUD
    ohko_handler.rts();
    uint32_t addr_ohko_handler = rom.inject_code(ohko_handler);
    rom.set_code(0x17842, md::Code().jmp(addr_ohko_handler));

    // Redirect the previous branch to a rts instruction later since we had to remove this one to inject the jmp
    rom.set_byte(0x17841, 0x0E);
}

void make_damage_independant_from_lifestocks(md::ROM& rom)
{
    md::Code func_get_base_damage;
    func_get_base_damage.moveb(addr_(0xFF1BF1), reg_D6);
    func_get_base_damage.bne(3);
        // Sword 0
        func_get_base_damage.movew(SWORD_0_BASE_POWER * 0x100, reg_D0);
        func_get_base_damage.rts();
    func_get_base_damage.cmpib(0x2, reg_D6);
    func_get_base_damage.bgt(6);
    func_get_base_damage.beq(3);
        // Sword 1
        func_get_base_damage.movew(SWORD_1_BASE_POWER * 0x100, reg_D0);
        func_get_base_damage.rts();  
        // Sword 2
        func_get_base_damage.movew(SWORD_2_BASE_POWER * 0x100, reg_D0);
        func_get_base_damage.rts();
    func_get_base_damage.cmpib(0x3, reg_D6); 
    func_get_base_damage.bne(3);
        // Sword 3
        func_get_base_damage.movew(SWORD_3_BASE_POWER * 0x100, reg_D0);
        func_get_base_damage.rts();
    // Sword 4
    func_get_base_damage.movew(SWORD_4_BASE_POWER * 0x100, reg_D0);
    func_get_base_damage.rts();

    uint32_t func_addr = rom.inject_code(func_get_base_damage);
    rom.set_code(0x16584, md::Code().jsr(func_addr));
}

void make_lifestocks_give_double_health(md::ROM& rom)
{
    constexpr uint16_t health_gained_on_lifestock = 0x0200;
    rom.set_word(0x291E2, health_gained_on_lifestock);
    rom.set_word(0x291F2, health_gained_on_lifestock);
    rom.set_word(0x7178, health_gained_on_lifestock);
    rom.set_word(0x7188, health_gained_on_lifestock);
}

void patch_engine_for_kaizo(World& world, md::ROM& rom)
{
    clear_item_sources(world);
    neutralize_one_time_events(rom);
    neutralize_custom_room_actions(rom);
    empty_all_persistence_flags(world);
    add_set_persistence_flag_behavior_operand(rom);
    add_invisible_behavior(rom);
    handle_morpher(rom);
    make_damage_independant_from_lifestocks(rom);
    make_lifestocks_give_double_health(rom);
}