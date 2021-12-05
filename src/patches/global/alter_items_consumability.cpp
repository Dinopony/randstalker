#include <md_tools.hpp>

/**
 * The effect of Pawn Ticket fits very well one of a consumable item, but didn't
 * work this way in the original game. Instead, a story flag was set that prevented
 * to use the item again, but it was still in inventory.
 * This function changes this.
 */
void make_pawn_ticket_consumable(md::ROM& rom)
{
    md::Code proc_consume_pawn_ticket;
    proc_consume_pawn_ticket.jsr(0x8B98); // ConsumeItem
    proc_consume_pawn_ticket.nop(3);
    rom.set_code(0x88D2, proc_consume_pawn_ticket);
}

/**
 * This function makes the key not consumed on use, since it is now a unique item
 * that needs to be used to open several doors.
 */
void make_key_not_consumed_on_use(md::ROM& rom)
{
    rom.set_code(0x8B34, md::Code().nop());
}
