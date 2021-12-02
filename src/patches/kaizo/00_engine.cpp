#include "kaizo.hpp"

void neutralize_one_time_events(md::ROM& rom)
{
    rom.set_word(0x1A974, 0xFFFF);
    rom.mark_empty_chunk(0x1A976, 0x1A9BE);
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
    rom.set_byte(0x9D1CD, 0x0F); // 0x0F = Turn invisible
    rom.set_word(0x9D1CE, 0x0000); // 0x0000 = End of behavior
}

void add_miniaturizer(md::ROM& rom)
{
    constexpr uint8_t TRANSFORM_SPRITE = 0x4D;

    md::Code func_miniaturize;
    func_miniaturize.trap(0, { 0x00, 0x5B }); // Play dog transform sound
    func_miniaturize.moveb(0x3, addr_(0xFF1153)); // Forbid attacking and picking up liftable items
	func_miniaturize.movew(0x8100 + TRANSFORM_SPRITE, addr_(0xFF540A));
    func_miniaturize.moveb(0x00, addr_(0xFF540C));
    func_miniaturize.moveb(0x0E, addr_(0xFF543B)); // Sprite dog
    func_miniaturize.moveb(0x01, addr_(0xFF546F));
    func_miniaturize.moveb(0x0F, addr_(0xFF5405)); // Set lowered height
    func_miniaturize.subib(0x10, addr_(0xFF5455)); // Lower hitbox height by one tile
    func_miniaturize.lea(0xFF5400, reg_A1);
    func_miniaturize.moveb(addr_(0xFF5404), reg_D1);
    func_miniaturize.jsr(0x1979C); // SetSpriteRotationAnimFlags
    func_miniaturize.rts();
    uint32_t addr_miniaturize = rom.inject_code(func_miniaturize);

    md::Code func_deminiaturize;
    func_deminiaturize.trap(0, { 0x00, 0x5B }); // Play dog transform sound
    func_deminiaturize.moveb(0x0, addr_(0xFF1153)); // Forbid attacking and picking up liftable items
	func_deminiaturize.movew(0x8100, addr_(0xFF540A));
	func_deminiaturize.moveb(0x00, addr_(0xFF540C));
    func_deminiaturize.moveb(0x00, addr_(0xFF543B));
    func_deminiaturize.moveb(0x00, addr_(0xFF546F));
    func_deminiaturize.moveb(0x1F, addr_(0xFF5405)); // Set base height
    func_deminiaturize.addib(0x10, addr_(0xFF5455)); // Raise hitbox height by one tile
    func_deminiaturize.lea(0xFF5400, reg_A1);
    func_deminiaturize.moveb(addr_(0xFF5404), reg_D1);
    func_deminiaturize.jsr(0x1979C); // SetSpriteRotationAnimFlags
    func_deminiaturize.rts();
    uint32_t addr_deminiaturize = rom.inject_code(func_deminiaturize);

    // Inject toggle on Einstein Whistle use

    md::Code injector;
    injector.cmpib(TRANSFORM_SPRITE, addr_(0xFF540B));
    injector.beq(3);
        injector.jsr(addr_miniaturize);
        injector.bra(2);
    injector.jsr(addr_deminiaturize);
    injector.jmp(0x8BBA); // ReturnSuccess
    rom.set_code(0x8886, injector);

    // Neutralize PostUseEinsteinWhistle
    rom.set_code(0x8BF2, md::Code().rts());
    rom.mark_empty_chunk(0x8BF4, 0x8C7C);

    // Deminiaturize on room change
    md::Code room_change_handler;
    room_change_handler.jsr(0x76EC); // UpdatePlayerDefence
    room_change_handler.cmpib(TRANSFORM_SPRITE, addr_(0xFF540B));
    room_change_handler.bne(2);
        room_change_handler.moveb(0x0F, addr_(0xFF5405)); // Set lowered height
    room_change_handler.rts();
    uint32_t addr_room_change_handler = rom.inject_code(room_change_handler);
    rom.set_code(0x2890, md::Code().jmp(addr_room_change_handler));

    // Deminiaturize and OHKO on hit
    md::Code ohko_handler;
    ohko_handler.cmpib(TRANSFORM_SPRITE, addr_(0xFF540B));
    ohko_handler.bne(3);
        ohko_handler.jsr(addr_deminiaturize);
        ohko_handler.movew(0x0000, addr_(0xFF543E)); // Set health to 0
    ohko_handler.jsr(0x16928); // RefreshCurrentHealthHUD
    ohko_handler.rts();
    uint32_t addr_ohko_handler = rom.inject_code(ohko_handler);
    rom.set_code(0x17842, md::Code().jmp(addr_ohko_handler));

    // Redirect the previous branch to a rts instruction later since we had to remove this one to inject the jmp
    rom.set_byte(0x17841, 0x0E);
}


void patch_engine_for_kaizo(World& world, md::ROM& rom)
{
    neutralize_one_time_events(rom);
    empty_all_persistence_flags(world);
    add_set_persistence_flag_behavior_operand(rom);
    add_invisible_behavior(rom);
    add_miniaturizer(rom);
}