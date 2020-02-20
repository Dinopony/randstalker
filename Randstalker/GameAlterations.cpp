#include "GameAlterations.h"
#include "Constants/ItemCodes.h"
#include "RandomizerOptions.h"
#include "GameText.h"
#include <cstdint>
#include <vector>

constexpr uint16_t OPCODE_MOVB =	0x13FC;
constexpr uint16_t OPCODE_MOVW =	0x33FC;
constexpr uint16_t OPCODE_MOVL =    0x23FC;
constexpr uint16_t OPCODE_RTS =		0x4E75;
constexpr uint16_t OPCODE_JSR =		0x4EB9;
constexpr uint16_t OPCODE_JMP =		0x4EF9;
constexpr uint16_t OPCODE_NOP =		0x4E71;
constexpr uint16_t OPCODE_BRA =		0x6000;
constexpr uint16_t OPCODE_BNE =		0x6600;
constexpr uint16_t OPCODE_BEQ =		0x6700;
constexpr uint16_t OPCODE_BLT =		0x6D00;
constexpr uint16_t OPCODE_BGT =		0x6E00;

void alterGameStart(GameROM& rom, const RandomizerOptions& options)
{
    // ------- Remove no music flag ---------
    // Replace the bitset of the no music flag by a jump to the injected flags init function located at the end of the rom
    
    // 0x002700:
        // Before: 	[08F9] bset 3 -> $FF1027
        // After:	[4EB9] jsr $1FFAD0 ; [4E71] nop
    rom.setWord(0x002700, OPCODE_JSR);
    rom.setLong(0x002702, rom.getCurrentInjectionAddress());
    rom.setWord(0x002706, OPCODE_NOP); // transform last two bytes into a NOP to keep the ROM padding intact
    
    // ------- Remove cutscene flag (no input allowed) ---------
    // Usually, when starting a new game, it is automatically put into "cutscene mode" to let the intro roll without allowing the player
    // to move or pause, or do anything at all. We need to remove that cutscene flag to enable the player actually playing the game.

    // 0x00281A:
        // Before:	[33FC] move.w 0x00FE -> $FF12DE
        // After:	[4E71] nop (4 times)
    for(uint32_t addr=0x00281A ; addr <= 0x002820 ; addr += 0x2)
        rom.setWord(addr, OPCODE_NOP);
    
    // ------- Inject flags init function ---------
    // Init function used to set story flags to specific values at the very beginning of the game, opening some usually closed paths
    // and removing some useless cutscenes (considering them as "already seen").

    uint8_t flagArray[0x60];
    for(int i=0 ; i<0x60 ; ++i)
        flagArray[i] = 0x0;

    // Setup story flags
    flagArray[0x00] = 0xB1;
    flagArray[0x02] = 0xD5;
    flagArray[0x03] = 0xE0;
    flagArray[0x04] = 0xDF;
    flagArray[0x05] = 0x60;
    flagArray[0x06] = 0x7E;
    flagArray[0x07] = 0xB4;
    flagArray[0x08] = 0xFE;
    flagArray[0x09] = 0x7E;
    flagArray[0x12] = 0x1E;
    flagArray[0x13] = 0x43;
    flagArray[0x14] = 0x81;
    flagArray[0x17] = 0x48;
    flagArray[0x20] = 0x70;
    flagArray[0x26] = 0x80;
    flagArray[0x27] = 0x22;
    flagArray[0x28] = 0xE0;
    flagArray[0x29] = 0x40;
    flagArray[0x2A] = 0x81;
    flagArray[0x2B] = 0x82;

    // Give Record book
    if(options.useRecordBook())
        flagArray[0x51] |= 0x20;

    // Setup inventory tracker if needed
    if (options.addIngameItemTracker())
    {
        flagArray[0x4B] |= 0x10;
        flagArray[0x4C] |= 0x10;
        flagArray[0x4D] |= 0x11;
        flagArray[0x4F] |= 0x11;
        flagArray[0x50] |= 0x01;
        flagArray[0x55] |= 0x10;
        flagArray[0x57] |= 0x10;
        flagArray[0x58] |= 0x10;
        flagArray[0x59] |= 0x11;
        flagArray[0x5B] |= 0x10;
        flagArray[0x5C] |= 0x11;
    }

    for(int i=0 ; i<0x60 ; i+=0x2)
    {
        if( flagArray[i] != 0 || flagArray[i+1] != 0)
        {
            rom.injectWord(OPCODE_MOVW);
            rom.injectByte(flagArray[i]);
            rom.injectByte(flagArray[i+1]);
            rom.injectLong(0x00FF1000 + i);
        }
    }

    // Set the orientation byte of Nigel to 88 (south-west) on game start
    rom.injectWord(OPCODE_MOVB);
    rom.injectWord(0x0088);
    rom.injectLong(0x00FF5404);

    // rts (return from function)
    rom.injectWord(OPCODE_RTS);
}

void fixAxeMagicCheck(GameROM& rom)
{
    // Changes the Axe Magic check when slashing a tree from bit 0 of flag 1003 to "Axe Magic owned"
    // 0x16262:
        // Before:	[0839] btst 0 in FF1003
        // After:	[0839] btst 5 in FF104B
    rom.setWord(0x016264, 0x0005);
    rom.setWord(0x016268, 0x104B);
}

void fixSafetyPassCheck(GameROM& rom)
{
    // Change Mercator door opened check from bit 7 of flag 1004 to "Safety Pass owned"
    // 0x004FF8:
        // Before:	04 0F (0F & 7 = 7 -----> bit 7 of FF1004)
        // After:	59 0D (0D & 7 = 5 -----> bit 5 of FF1059)
    rom.setWord(0x004FF8, 0x590D);
}

void fixArmletCheck(GameROM& rom)
{
    // Change Armlet check from bit 6 of flag 1014 to "Armlet owned".
    // This one was tricky because there are TWO checks to change (the one to remove the "repelled" textbox + tornado, 
    // and another one to remove the invisible wall added in US version), and any of those checks remove the armlet from inventory
    // on completion.

    // Actually changed the flag which is check for both triggers
    // 0x09C803:
        // Before:	14 06 (bit 6 of FF1014)
        // After:	4F 05 (bit 5 of FF104F)
    rom.setWord(0x09C803, 0x4F05);
    // 0x09C7F3: same as above
    rom.setWord(0x09C7F3, 0x4F05);
    
    // Prevent the game from removing the armlet from the inventory
    // 0x013A80: put a RTS instead of the armlet removal and all (not exactly sure why it works perfectly, but it does)
        // Before:  4E F9
        // After:	4E 75 (rts)
    rom.setWord(0x013A8A, OPCODE_RTS);
}

void fixSunstoneCheck(GameROM& rom)
{
    // Change Sunstone check for repairing the lighthouse from bit 2 of flag 1026 to "Susntone owned"
    // 0x09D091:
        // Before:	26 02 (bit 2 of FF1026)
        // After:	4F 01 (bit 1 of FF104F)
    rom.setWord(0x09D091, 0x4F01);
}

void fixDogTalkingCheck(GameROM& rom)
{
    // Change doggo talking check from bit 4 of flag 1024 to "Einstein Whistle owned"
    // 0x0253C0:
        // Before:	01 24 (0124 >> 3 = 24 and 0124 & 7 = 04 -----> bit 4 of FF1024)
        // After:	02 81 (0281 >> 3 = 50 and 0281 & 7 = 01 -----> bit 1 of FF1050)
    rom.setWord(0x0253C0, 0x0281);
}

void fixCryptBehavior(GameROM& rom)
{
    // 1) Remove the check "if shadow mummy was beaten, raft mummy never appears again"
    // 0x019DF6:
        // Before:	0839 0006 00FF1014 (btst bit 6 in FF1014) ; 66 14 (bne $19E14)
        // After:	4EB9 00019E14 (jsr $19E14; 4E71 4E71 (nop nop)
    rom.setWord(0x019DF6, OPCODE_NOP);
    rom.setWord(0x019DF8, OPCODE_NOP);
    rom.setLong(0x019DFA, OPCODE_NOP);
    rom.setWord(0x019DFC, OPCODE_NOP);
    rom.setWord(0x019DFE, OPCODE_NOP);

    // 2) Change the room exit check and shadow mummy appearance from "if armlet is owned" to "chest was opened"
    // 0x0117E8:
        // Before:	103C 001F ; 4EB9 00022ED0 ; 4A41 ; 6B00 F75C (bmi $10F52)
        // After:	0839 0002 00FF1097 (btst 2 FF1097)	; 6700 F75C (bne $10F52)
    rom.setWord(0x0117E8, 0x0839);		// btst
    rom.setWord(0x0117EA, 0x0002);		// bit 2
    rom.setLong(0x0117EC, 0x00FF1097);	// of flag FF1097
    rom.setWord(0x0117F0, OPCODE_NOP);	// nop
    rom.setWord(0x0117F2, OPCODE_NOP);	// nop
    rom.setWord(0x0117F4, OPCODE_BEQ);	// beq $10F52
}

void fixMirAfterLakeShrineCheck(GameROM& rom)
{
    // In the original game, coming back to Mir room after Lake Shrine would softlock you because Mir
    // would not be there. This check is removed to prevent any softlock and allow fighting Mir after having
    // done Lake Shrine.

    // 0x01AA22:
        // Before:	0310 2A A2 (in map 310, check bit 5 of flag 102A)
        // After:	0000 5F E2 (in map 0, check bit 7 of flag 105F - never true)
    rom.setWord(0x01AA22, 0x0000);
    rom.setWord(0x01AA24, 0x5FE2);
}

void fixLogsRoomExitCheck(GameROM& rom)
{
    // Remove logs check
    rom.setWord(0x011EC4, OPCODE_BRA);
}

void fixArmletSkip(GameROM& rom)
{
    // Fix armlet skip by putting the tornado way higher, preventing any kind of buffer-jumping on it
    // 0x02030C:
        // Before:  0x82 (Tornado pos Y = 20)
        // After:   0x85 (Tornado pos Y = 50)
    rom.setByte(0x02030C, 0x85);
}

void fixTreeCuttingGlitch(GameROM& rom)
{
    // Call the injected function when killing an enemy
    rom.setWord(0x01625C, OPCODE_JSR);
    rom.setLong(0x01625E, rom.getCurrentInjectionAddress());

    // Inject a new function which fixes the money value check on an enemy when it is killed, causing the tree glitch to be possible
    // tst.b ($36,A5) [4A2D 0036]
    rom.injectWord(0x4A2D);
    rom.injectWord(0x0036);

    // beq to rts +0x0E
    rom.injectWord(0x670E);

    // cmpi.w #0126, ($A,A5) [0C6D 0126]
    rom.injectWord(0x0C6D);
    rom.injectWord(0x0126);
    rom.injectWord(0x000A);

    // beq to rts +0x6
    rom.injectWord(0x6706);

    // jmp to $16284
    rom.injectWord(OPCODE_JMP);
    rom.injectLong(0x00016284);

    // rts
    rom.injectWord(OPCODE_RTS);
}

void fixMirTowerPriestRoomItems(GameROM& rom)
{
    // Remove the "shop/church" flag on the priest room of Mir Tower to make its items on ground work everytime
    // 0x024E5A:
        // Before:	0307
        // After:	7F7F
    rom.setWord(0x024E5A, 0x7F7F);
}

void fixKingNolesLabyrinthRafts(GameROM& rom)
{
    // Change the rafts logic so we can take them several times in a row, preventing from getting softlocked by missing chests
    // The trick here is to use flag 1001 (which resets on every map change) to correctly end the cutscene while discarding the "raft already taken" state 
    // as early as the player moves to another map.
    rom.setWord(0x09E031, 0x0100);
    rom.setWord(0x09E034, 0x0100);
    rom.setWord(0x09E04E, 0x0100);
    rom.setWord(0x09E051, 0x0100);

    // Second raft is always here, so no need for two logs anymore, set the maximum amount of logs to one
    rom.setByte(0x0293C0, 0x01);
}

void fixFaraLifestockChest(GameROM& rom)
{
    // Make it so Lifestock chest near Fara in Swamp Shrine appears again when going back into the room afterwards, preventing any softlock there.
    // jsr FUNC
    rom.setWord(0x019BE0, OPCODE_JSR);
    rom.setLong(0x019BE2, rom.getCurrentInjectionAddress());

    // nop (for padding)
    rom.setLong(0x019BE6, OPCODE_NOP);

    // --------- Function to remove all entities but the chest when coming back in the room ---------
    // movem(store registers) [48E7 FFFE]
    rom.injectWord(0x48E7);
    rom.injectWord(0xFFFE);

    // lea $FF5480, A0 [41F9 00FF5480] 
    rom.injectWord(0x41F9);
    rom.injectLong(0x00FF5480);

    // moveq.b #D, D0 [7012]
    rom.injectWord(0x700D);

    // move.w $7F7F, (A0) [30BC 7F7F]
    rom.injectWord(0x30BC);
    rom.injectWord(0x7F7F);

    // adda $80, A0 [D1FC 0080]
    rom.injectWord(0xD1FC);
    rom.injectLong(0x00000080);

    // dbra D0, -12 [51C8 FFF4]
    rom.injectWord(0x51C8);
    rom.injectWord(0xFFF4);

    // movem(restore registers) [4CDF 7FFF]
    rom.injectWord(0x4CDF);
    rom.injectWord(0x7FFF);

    // rts (4E75)
    rom.injectWord(OPCODE_RTS);

    // --------- Moving the chest to the ground ---------
    rom.setWord(0x01BF6C, 0x1A93);
    rom.setWord(0x01BF6E, 0x0000);
    rom.setWord(0x01BF70, 0x0012);
    rom.setWord(0x01BF72, 0x0400);
}

void alterArthurCheck(GameROM& rom)
{
    // Change the Arthur check giving casino tickets for him to be always here, instead of only after Lake Shrine

    // 0x01A904: 
        // Before:  AAA0 (bit 5 of flag 2A, affecting entity 0) 
        // After:   8000 (bit 0 of flag 00, affecting entity 0)
    rom.setWord(0x01A904, 0x8000);

    // 0x01A908:
        // Before:	AAA1 (bit 5 of flag 2A, affecting entity 1) 
        // After:	8001 (bit 0 of flag 00, affecting entity 1)
    rom.setWord(0x01A908, 0x8001);
}

void alterMercatorSecondaryShopCheck(GameROM& rom)
{
    // Change the Mercator secondary shop check so that it sells item as long as you own Buyer's Card

    // 0x00A574:
        // Before:	2A 04 (bit 4 of FF102A)
        // After:	4C 05 (bit 5 of FF104C)
    rom.setWord(0x00A574, 0x4C05);
}

void alterWaterfallShrineSecretStairsCheck(GameROM& rom)
{
    // Change Waterfall Shrine entrance check from "Talked to Prospero" to "What a noisy boy!", removing the need
    // of talking to Prospero (which we couldn't do anyway because of the story flags).

    // 0x005014:
        // Before:	00 08 (bit 0 of FF1000)
        // After:	02 09 (bit 1 of FF1002)
    rom.setWord(0x005014, 0x0209);
}

void alterVerlaBoulderCheck(GameROM& rom)
{
    // Change the removal check for the boulder between Verla and Mercator so that it disappears as soon as you sail with the boat.
    // This means you don't need to do anything in Verla to be able to go back to the rest of the island, preventing any softlock there

    // Mercator side map (exterior)
    //  5071 08 => 0C
    // 1A8C9 08 => 88 (>> 5 = 4 --> bit 4)
    rom.setByte(0x005071, 0x0C);
    rom.setByte(0x01A8C9, 0x88);

    // Verla side map (inside tunnel)
    //  5075 08 => 0C
    // 1A965 06 => 86 (>> 5 = 4 --> bit 4)
    rom.setByte(0x005075, 0x0C);
    rom.setByte(0x01A965, 0x86);
}

void alterBlueRibbonStoryCheck(GameROM& rom)
{
    // The "falling ribbon" item source is pretty dependant from the scenario to happen. In the original game,
    // the timeframe to get it is really tight. We try to get rid of any conditions here, apart from checking
    // if item has already been obtained.

    // 0x00A466:
        // Before:  20 05 (bit 5 of flag 1020)
        // After:   3F 07 (bit 7 of flag 103F - never true)
    rom.setWord(0x00A466, 0x3F07);

    // 0x01A724:
        // Before:	16 22 (bit 1 of flag 1016)
        // After:	3F E2 (bit 7 of flag 103F - never true)
    rom.setWord(0x1A724, 0x3FE2);

    // Change falling item position from 1F to 20 to ensure it is taken by Nigel whatever its original position is
    rom.setByte(0x09C59E, 0x20);

    // Remove the servant guarding the door, setting her position to 00 00
    rom.setWord(0x01BFCA, 0x0000);
}

void alterKingNolesCaveTeleporterCheck(GameROM& rom)
{
    // Change the flag checked for teleporter appearance from "saw the duke Kazalt cutscene" to "has visited four white golems room in King Nole's Cave"
    // 0x0050A0:
        // Before:	27 09 (bit 1 of flag 1027)
        // After:	D0 09 (bit 1 of flag 10D0)
    rom.setWord(0x0050A0, 0xD009);
    // 0x00509C:
        // Before:	27 11 (bit 1 of flag 1027)
        // After:	D0 11 (bit 1 of flag 10D0)
    rom.setWord(0x00509C, 0xD011);

    // We need to inject a procedure checking "is D0 equal to FF" to replace the "bmi" previously used which was preventing
    // from checking flags above 0x80 (the one we need to check is 0xD0).

    // Replace the (move.b, bmi, ext.w) by a jmp to the injected procedure
    // clr.w D0
    rom.setWord(0x004E18, 0x4240);
    
    // jmp to procedure
    rom.setWord(0x004E1A, OPCODE_JMP);
    rom.setLong(0x004E1C, rom.getCurrentInjectionAddress());

    // Inject the actual procedure
    // move.b ($2,A0), D0
    rom.injectWord(0x1028);
    rom.injectWord(0x0002);

    // cmpi.b #FF, D0
    rom.injectWord(0x0C00);
    rom.injectWord(0x00FF);

    // bne $(PC,0x06)
    rom.injectWord(0x6606);

    // jmp $4E2E
    rom.injectWord(OPCODE_JMP);
    rom.injectLong(0x00004E2E);

    // jmp $4E1E
    rom.injectWord(OPCODE_JMP);
    rom.injectLong(0x00004E20);
}

void alterMercatorDocksShopCheck(GameROM& rom)
{
    // 0x01AA26:
        // Before:	0284 2A A2 (in map 284, check bit 5 of flag 102A)
        // After:	0000 5F E2 (in map 0, check bit 7 of flag 105F - never true)
    rom.setWord(0x01AA26, 0x0000);
    rom.setWord(0x01AA28, 0x5FE2);
}


void alterLanternIntoPassiveItem(GameROM& rom)
{
 
    std::vector<uint16_t> darkRooms = {
        0x0170, 0x017D, 0x017F, 0x0178, 0x0185, 0x018D,
        0x018E, 0x0187, 0x01A0, 0x01A1, 0x0176, 0x017B
    };

    // Change all "is room lit" flags by "is lantern owned"
    for (uint8_t i = 0; i < 24; ++i)
    {
        uint32_t address = 0x8800 + i * 0x2;
        if (i < darkRooms.size())
            rom.setWord(address, darkRooms[i]);
        else
            rom.setWord(address, 0xFFFF);
    }
 
    // addq #2,A0 (instead of adding 4)
    rom.setWord(0x0087D2, 0x5488);

    // btst #1, $FF104D (0839 0005 00FF1045)
    rom.setWord(0x0087D6, 0x0839);
    rom.setWord(0x0087D8, 0x0001);
    rom.setLong(0x0087DA, 0x00FF104D);

    for (uint32_t addr = 0x87DE; addr <= 0x87E8; addr += 0x02)
        rom.setWord(addr, OPCODE_NOP);

//    for (uint32_t addr = 0x87DD; addr >= 0x87C2; addr -= 0x01)
//        rom.setByte(addr + 2, rom.getByte(addr));

    // lea ADDR_DARK_ROOMS, A0
//    rom.setWord(0x0087BE, 0x41F9);
//    rom.setLong(0x0087C0, rom.getStoredAddress("data_dark_rooms"));
}

void alterItemOrderInMenu(GameROM& rom)
{
    std::vector<uint8_t> itemOrder = {
        ITEM_EKEEKE,        ITEM_RECORD_BOOK,
        ITEM_DAHL,          ITEM_RESTORATION,
        ITEM_GOLDEN_STATUE, ITEM_GAIA_STATUE,
        ITEM_DETOX_GRASS,   ITEM_MIND_REPAIR,
        ITEM_ANTI_PARALYZE, ITEM_ORACLE_STONE,
        ITEM_KEY,           ITEM_GARLIC,
        ITEM_LOGS,          ITEM_IDOL_STONE,
        ITEM_GOLA_EYE,      ITEM_GOLA_NAIL,
        ITEM_GOLA_HORN,     ITEM_GOLA_FANG,
        ITEM_DEATH_STATUE,  ITEM_STATUE_JYPTA,
        ITEM_SHORT_CAKE,    ITEM_PAWN_TICKET,
        ITEM_CASINO_TICKET, ITEM_LITHOGRAPH,
        ITEM_LANTERN,       ITEM_BELL,
        ITEM_SAFETY_PASS,   ITEM_ARMLET,
        ITEM_SUN_STONE,     ITEM_BUYER_CARD,
        ITEM_AXE_MAGIC,     ITEM_EINSTEIN_WHISTLE,
        ITEM_RED_JEWEL,     ITEM_PURPLE_JEWEL,
        ITEM_BLUE_RIBBON,   ITEM_SPELL_BOOK,
        0xFF,               0xFF,
        0xFF,               0xFF
    };

    uint32_t baseAddress = 0x00D55C;
    for (int i = 0; baseAddress + i < 0x00D584; ++i)
        rom.setByte(baseAddress + i, itemOrder[i]);
}

void alterGoldRewardsHandling(GameROM& rom)
{
    // In the original game, only 3 item IDs are reserved for gold rewards (3A, 3B, 3C)
    // Here, we moved the table of gold rewards to the end of the ROM so that we can handle 64 rewards up to 255 golds each.
    // In the new system, all item IDs after the "empty item" one (0x40 and above) are now gold rewards.
    
    rom.setByte(0x0070DF, 0x40); // cmpi 3A, D0 >>> cmpi 40, D0
    rom.setByte(0x0070E5, 0x40); // subi 3A, D0 >>> subi 40, D0

    // Before:      add D0,D0   ;   move.w (PC, D0, 42), D0
    // After:       jsr to new procedure
    rom.setWord(0x0070E8, OPCODE_JSR);
    rom.setLong(0x0070EA, rom.getCurrentInjectionAddress());

    // ------------- Procedure to put gold reward value in D0 ----------------
    // Input: D0 = gold reward ID (offset from 0x40)
    // Output: D0 = gold reward value

    // store A0 into -(A7)
    rom.injectWord(0x48E7);
    rom.injectWord(0x0080);
     
    // lea $1FFFC0, A0
    rom.injectWord(0x41F9);
    rom.injectLong(rom.getStoredAddress("data_gold_values"));

    // move.b (A0, D0), D0
    rom.injectWord(0x1030);
    rom.injectWord(0x0000);

    // restore A0 from (A7)+
    rom.injectWord(0x4CDF);
    rom.injectWord(0x0100);

    // rts
    rom.injectWord(OPCODE_RTS);
}

void alterLifestockHandlingInShops(GameROM& rom)
{
    // Make Lifestock prices the same over all shops
    for (uint32_t addr = 0x024D34; addr <= 0x024EAE; addr += 0xE)
        rom.setByte(addr + 0x03, 0x10); //rom.getByte(addr + 0x02));

    // Remove the usage of "bought lifestock in shop X" flags 
    for (uint32_t addr = 0x009D18; addr <= 0x009D33; addr += 0xE)
        rom.setByte(addr, 0xFF);
}

void removeMercatorCastleBackdoorGuard(GameROM& rom)
{
    // There is a guard staying in front of the Mercator castle backdoor to prevent you from using
    // Mir Tower keys on it. He appears when Crypt is finished and disappears when Mir Tower is finished,
    // but we actually never want him to be there, so we delete him from existence by moving him away from the map.

    // 0x0215A6:
        // Before:	93 9D
        // After:	00 00
    rom.setWord(0x0215A6, 0x0000);
}

void removeSailorInDarkPort(GameROM& rom)
{
    // There is a sailor NPC in the "dark" version of Mercator port who responds badly to story triggers, allowing us to sail to Verla
    // even without having repaired the lighthouse. To prevent this from being exploited, we removed him altogether.

    // 0x021646:
        // Before:	23 EC
        // After:	00 00
    rom.setWord(0x021646, 0x0000);
}

void addNewlineHandlingInDynamicText(GameROM& rom)
{
    // Inject a new condition in characters processing to handle newlines in dynamic text when 0x6C character is encountered
    rom.setWord(0x0230A2, OPCODE_JMP);
    rom.setLong(0x0230A4, rom.getCurrentInjectionAddress());

    // cmpi.w #FFFF, D0
    rom.injectWord(0x0C40);
    rom.injectWord(0xFFFF);

    // bne to next case
    rom.injectWord(OPCODE_BNE + 0x06);

    // jmp to 230C0
    rom.injectWord(OPCODE_JMP);
    rom.injectLong(0x000230C0);

    // cmpi.w #006C, D0
    rom.injectWord(0x0C40);
    rom.injectWord(0x006C);

    // bne to next case
    rom.injectWord(OPCODE_BNE + 0x0C);

    // jsr "add newline" function
    rom.injectWord(OPCODE_JSR);
    rom.injectLong(0x00022F7C);

    // jmp to 23094 (process next character)
    rom.injectWord(OPCODE_JMP);
    rom.injectLong(0x00023094);

    // jmp to 230A8 (resume back to usual code)
    rom.injectWord(OPCODE_JMP);
    rom.injectLong(0x000230A8);
}

void addJewelsCheckForTeleporterToKazalt(GameROM& rom)
{
    GameText text("King Nole: Only the bearers of \nthe jewels are worthy of \nentering my domain...\t");
    rom.injectDataBlock(text.getBytes(), "data_text_jewels_alert");

    // ----------- Hijack text with dynamic text func ----------------------
    uint32_t hijackTextFunc = rom.getCurrentInjectionAddress();

    // move.w FFFF, D0
    rom.injectWord(0x303C);
    rom.injectWord(0xFFFF);

    // movem D0-A6, -(A7)
    rom.injectWord(0x48E7);
    rom.injectWord(0xFFFE);


    // jsr ($22FCC)
    rom.injectWord(OPCODE_JSR);
    rom.injectLong(0x00022FCC);

    // set hint addr ($FF1844)
    rom.injectWord(OPCODE_MOVL);
    rom.injectLong(rom.getStoredAddress("data_text_jewels_alert"));
    rom.injectLong(0x00FF1844);

    // jmp ($22F34)
    rom.injectWord(OPCODE_JMP);
    rom.injectLong(0x00022F34);


    // ----------- Jewel textbox handling ----------------------
    // This adds the purple & red jewel as a requirement for the Kazalt teleporter to work correctly
    rom.setWord(0x0062F4, OPCODE_JMP);
    rom.setLong(0x0062F6, rom.getCurrentInjectionAddress());

    // btst #$1, ($FF1054)    - Test if red jewel is owned
    rom.injectWord(0x0839);
    rom.injectWord(0x0001);
    rom.injectLong(0x00FF1054);

    // beq to REMOVEPORTAL
    rom.injectWord(OPCODE_BEQ + 0x0A);

    // btst #$1, ($FF1055)    - Test if purple jewel is owned
    rom.injectWord(0x0839);
    rom.injectWord(0x0001);
    rom.injectLong(0x00FF1055);

    // bne to TELEPORT
    rom.injectWord(OPCODE_BNE + 0x1C);

    // movem(store registers) [48E7 FFFE] [REMOVEPORTAL:]
    rom.injectWord(0x48E7);
    rom.injectWord(0xFFFE);

    // jsr OPEN TEXTBOX
    rom.injectWord(OPCODE_JSR);
    rom.injectLong(0x22EE8);

    // jsr hijackTextFunc
    rom.injectWord(OPCODE_JSR);
    rom.injectLong(hijackTextFunc);

    // jsr CLOSE TEXTBOX
    rom.injectWord(OPCODE_JSR);
    rom.injectLong(0x22EA0);

    // movem(restore registers) [4CDF 7FFF]
    rom.injectWord(0x4CDF);
    rom.injectWord(0x7FFF);

    // rts
    rom.injectWord(OPCODE_RTS);

    // moveq 7, D0  [TELEPORT:]
    rom.injectWord(0x7007);
    
    // jsr E110
    rom.injectWord(OPCODE_JSR);
    rom.injectLong(0x00E110);

    // jmp back to the regular code
    rom.injectWord(OPCODE_JMP);
    rom.injectLong(0x0062FA);
}

void addStatueOfJyptaGoldsOverTime(GameROM& rom)
{
    constexpr uint16_t goldsPerCycle = 0x0002;

    for(uint32_t addr = 0x16696 ; addr <= 0x1669E ; addr += 0x2)
        rom.setWord(addr, OPCODE_NOP);

    rom.setWord(0x166D0, OPCODE_JSR);
    rom.setLong(0x166D2, rom.getCurrentInjectionAddress());

    for (uint32_t addr = 0x166D6; addr <= 0x166DC; addr += 0x2)
        rom.setWord(addr, OPCODE_NOP);

    // btst #$5, ($FF104E)    - Test if statue of jypta is owned
    rom.injectWord(0x0839);
    rom.injectWord(0x0005);
    rom.injectLong(0x00FF104E);

    // beq to next condition
    rom.injectWord(0x670A);

    // move.w 2, D0
    rom.injectWord(0x303C);
    rom.injectWord(goldsPerCycle);

    // jsr $177DC
    rom.injectWord(OPCODE_JSR);
    rom.injectLong(0x000177DC);

    // cmpi.b $7, ($FF1150)
    rom.injectWord(0x0C39);
    rom.injectWord(0x0007);
    rom.injectLong(0x00FF1150);

    // bne to rts
    rom.injectWord(0x6610);

    // move.w 100, D0
    rom.injectWord(0x303C);
    rom.injectWord(0x0100);
    
    // lea $FF5400, A5
    rom.injectWord(0x4BF9);
    rom.injectLong(0x00FF5400);

    // jsr $1780E
    rom.injectWord(OPCODE_JSR);
    rom.injectLong(0x0001780E);

    // rts
    rom.injectWord(OPCODE_RTS);
}

void addLithographChestInKazaltTeleporterRoom(GameROM& rom)
{
    // Negate map trigger removing entities once the Duke cutscene was seen in OG
    rom.setWord(0x01AA0E, 0x7F7F);
    
    // Replace the first entity in the map by a chest
    rom.setWord(0x01CFA6, 0x1D9D);  // X&Z pos, orientation and palette info
    rom.setWord(0x01CFA8, 0x0000);
    rom.setWord(0x01CFAA, 0x0012);  // Type 0x0012 = chest
    rom.setWord(0x01CFAC, 0x0100);  // Y pos = 0x10

    // Remove other entities in the map
    for (uint32_t addr = 0x1CFAE; addr <= 0x1CFC5; ++addr)
        rom.setByte(addr, 0xFF);

    // Change the map base chest ID
    rom.setByte(0x09E838, 0x1E);
}


void replaceLumberjackByChest(GameROM& rom)
{
    // Set base index for chests in map to "1A" instead of "A8" to have room for a second chest in the map
    for (uint32_t addr = 0x9E9BA; addr <= 0x9E9BE; ++addr)
        rom.setByte(addr, 0x1A);

    // Transform the lumberjack into a chest
    rom.setWord(0x020B9C, 0xD5B1);  // First word: position, orientation and palette (5571 => D5B1)
    rom.setWord(0x020B9E, 0x0000);  // Second word: ??? (2200 => 0000)
    rom.setWord(0x020BA0, 0x0012);  // Third word: type (0550 = Lumberjack NPC => 0012 = chest)
    rom.setWord(0x020BA2, 0x8000);  // Fourth word: behavior (00C1 => 8000 works for some reason)
}

void replaceSickMerchantByChest(GameROM& rom)
{
    // Neutralize map entrance triggers for both the shop and the backroom to remove the "sidequest complete" check.
    // Either we forced them to be always true or always false
    rom.setWord(0x0050B4, 0x0008);  // Before: 0x2A0C (bit 4 of 102A) | After: 0x0008 (bit 0 of 1000 - always true)
    rom.setWord(0x00A568, 0x3F07);	// Before: 0x2A04 (bit 4 of 102A) | After: 0x3F07 (bit 7 of 103F - always false)
    rom.setWord(0x00A56E, 0x3F07);	// Before: 0x2A03 (bit 3 of 102A) | After: 0x3F07 (bit 7 of 103F - always false)
    rom.setWord(0x01A6F8, 0x3FE0);	// Before: 0x2A80 (bit 4 of 102A) | After: 0x3FE0 (bit 7 of 103F - always false)
    
    // Set the index for added chest in map to "0E" instead of "C2"
    rom.setByte(0x09EA48, 0x0E);

    // Transform the sick merchant into a chest
    rom.setWord(0x021D0E, 0xCE92);  // First word: position, orientation and palette (CE16 => CE92)
    rom.setWord(0x021D10, 0x0000);  // Second word: ??? (3000 => 0000)
    rom.setWord(0x021D12, 0x0012);  // Third word: type (006D = Lumberjack NPC => 0012 = chest)
//	rom.setWord(0x021D14, 0x0000); 

    // Move the kid to hide the fact that the bed looks broken af
    rom.setWord(0x021D16, 0x5055);
}

void replaceFaraInElderHouseByChest(GameROM& rom)
{
    // Neutralize a map specific trigger which broke chests inside it
    // 0x019C82:
        // Before:	0x0251 (map ID)
        // After:	0xFFFF 
    rom.setWord(0x019C82, 0xFFFF);
    
    // Move the elder to his right position
    rom.setWord(0x020FA2, 0x1113);

    // Replace Fara entity by a chest
    rom.setWord(0x020FAA, 0x134F);
    rom.setWord(0x020FAC, 0x0000);
    rom.setWord(0x020FAE, 0x0012);
    rom.setWord(0x020FB0, 0x0000);

    // Remove bed wakeup cutscene
    rom.setWord(0x020FB2, 0x7F7F);
    rom.setWord(0x020FB4, 0x0000);
    rom.setWord(0x020FB6, 0x0000);
    rom.setWord(0x020FB8, 0x0000);

    // Set map base chest index to 0x17
    rom.setByte(0x09E9DF, 0x17);
}

void handleArmorUpgrades(GameROM& rom)
{
    // --------------- Alter item in D0 register function ---------------
    uint32_t alterItemInD0Function = rom.getCurrentInjectionAddress();

    // Inject the function
    // cmpi.b #09, D0 (0C00 0009)
    rom.injectWord(0x0C00);
    rom.injectWord(0x0009);

    // blt (+0x34 [to rts]) (6D3A)
    rom.injectWord(0x6D34);

    // cmpi.b #0C, D0 (0C00 000C)
    rom.injectWord(0x0C00);
    rom.injectWord(0x000C);

    // bgt (+0x2E [to rts]) (6E2E)
    rom.injectWord(0x6E2E);

    // move.w #C, D0 (303C 000C)
    rom.injectWord(0x303C);
    rom.injectWord(0x000C);

    // btst #5, $FF1045 (0839 0005 00FF1045)
    rom.injectWord(0x0839);
    rom.injectWord(0x0005);
    rom.injectLong(0x00FF1045);

    // bne +0x04 (6604)
    rom.injectWord(0x6604);

    // move.w #B, D0 (303C 000B)
    rom.injectWord(0x303C);
    rom.injectWord(0x000B);

    // btst #1, $FF1045 (0839 0001 00FF1045)
    rom.injectWord(0x0839);
    rom.injectWord(0x0001);
    rom.injectLong(0x00FF1045);

    // bne +0x04 (6604)
    rom.injectWord(0x6604);

    // move.w #A, D0 (303C 000A)
    rom.injectWord(0x303C);
    rom.injectWord(0x000A);

    // btst #5, $FF1044 (0839 0005 00FF1044)
    rom.injectWord(0x0839);
    rom.injectWord(0x0005);
    rom.injectLong(0x00FF1044);

    // bne +0x04 (6604)
    rom.injectWord(0x6604);

    // move.w #9, D0 (303C 0009)
    rom.injectWord(0x303C);
    rom.injectWord(0x0009);

    // rts (4E75)
    rom.injectWord(OPCODE_RTS);

    // --------------- Change item in reward box function ---------------
    uint32_t changeItemInRewardBoxFunction = rom.getCurrentInjectionAddress();

    // jsr ALTERATION FUNC
    rom.injectWord(OPCODE_JSR);
    rom.injectLong(alterItemInD0Function);

    // move.w D0, $FF1196 (33C0 00FF1196)
    rom.injectWord(0x33C0);
    rom.injectLong(0x00FF1196);

    // rts (4E75)
    rom.injectWord(OPCODE_RTS);

    // --------------- Change item given by taking item on ground function ---------------
    uint32_t changeItemGivenByItemOnGroundFunction = rom.getCurrentInjectionAddress();

    // movem D7,A0 -(A7)	(48E7 0180)
    rom.injectWord(0x48E7);
    rom.injectWord(0x0180);
    
    // cmpi.b #0C, D0 (0C00 000C)
    rom.injectWord(0x0C00);
    rom.injectWord(0x000C);

    // bgt to movem (6E22)
    rom.injectWord(OPCODE_BGT + 0x22);

    // cmpi.b #09, D0 (0C00 0009)
    rom.injectWord(0x0C00);
    rom.injectWord(0x0009);

    // blt to movem (6D1C)
    rom.injectWord(OPCODE_BLT + 0x1C);

    // jsr ALTERATION FUNC
    rom.injectWord(OPCODE_JSR);
    rom.injectLong(alterItemInD0Function);

    // move ($3B,A5), D7	(1E2D 003B)
    rom.injectWord(0x1E2D);
    rom.injectWord(0x003B);

    // subi #C9, D7	 (0407)
    rom.injectWord(0x0407);
    rom.injectWord(0x00C9);

    // cmpa #00FF5400, A5	(BBF9 5400)
    rom.injectWord(0xBBFC);
    rom.injectLong(0x00FF5400);

    // blt to movem (6D06)
    rom.injectWord(OPCODE_BLT + 0x06);

    // bset D7, FF103F	(0FF9 00FF103F)
    rom.injectWord(0x0FF9);
    rom.injectLong(0x00FF103F);

    // movem (A7)+, D7,A0	(4CDF 0180)
    rom.injectWord(0x4CDF);
    rom.injectWord(0x0180);
     
    // lea FF1040, A0 (41F9 00FF1040)
    rom.injectWord(0x41F9);
    rom.injectLong(0x00FF1040);

    // rts (4E75)
    rom.injectWord(OPCODE_RTS);

    // --------------- Change visible item for items on ground function ---------------
    uint32_t changeItemVisibleOnGroundFunction = rom.getCurrentInjectionAddress();

    // movem D7,A0 -(A7)	(48E7 0180)
    rom.injectWord(0x48E7);
    rom.injectWord(0x0180);

    // subi #C0, D0  (0400 00C0)
    rom.injectWord(0x0400);
    rom.injectWord(0x00C0);

    // cmpi.b #0C, D0 (0C00 000C)
    rom.injectWord(0x0C00);
    rom.injectWord(0x000C);

    // bgt (+0x20 [to movem]) (6E20)
    rom.injectWord(OPCODE_BGT + 0x20);

    // cmpi.b #09, D0 (0C00 0009)
    rom.injectWord(0x0C00);
    rom.injectWord(0x0009);

    // blt (+0x1A [to movem]) (6D1A)
    rom.injectWord(OPCODE_BLT + 0x1A);

    // move D0, D7 (001E)
    rom.injectWord(0x1E00);

    // subi #9, D7	 (0407)
    rom.injectWord(0x0407);
    rom.injectWord(0x0009);

    // btest D7, FF103F	(0F39 00FF103F)
    rom.injectWord(0x0F39);
    rom.injectLong(0x00FF103F);

    // bne (+0x08 [to move 3F into item]) (6608)
    rom.injectWord(OPCODE_BNE + 0x08);

    // jsr ALTERATION FUNC
    rom.injectWord(OPCODE_JSR);
    rom.injectLong(alterItemInD0Function);

    // bra +0x04
    rom.injectWord(OPCODE_BRA + 0x04);

    // move.w #3F, D0 (303C 003F)
    rom.injectWord(0x303C);
    rom.injectWord(0x003F);

    // move D0, ($36,A1) (1340 0036)
    rom.injectWord(0x1340);
    rom.injectWord(0x0036);

    // movem (A7)+, D7,A0	(4CDF 0180)
    rom.injectWord(0x4CDF);
    rom.injectWord(0x0180);

    // rts (4E75)
    rom.injectWord(OPCODE_RTS);

    // --------------- Hooks ---------------

    // In 'chest reward' function, replace the item ID move by the injected function
    rom.setWord(0x0070BE, OPCODE_JSR);
    rom.setLong(0x0070C0, changeItemInRewardBoxFunction);

    // In 'NPC reward' function, replace the item ID move by the injected function
    rom.setWord(0x028DD8, OPCODE_JSR);
    rom.setLong(0x028DDA, changeItemInRewardBoxFunction);

    // In 'item on ground reward' function, replace the item ID move by the injected function
    rom.setWord(0x024ADC, 0x3002); // put the move D2,D0 before the jsr because it helps us while changing nothing to the usual logic
    rom.setWord(0x024ADE, OPCODE_JSR);
    rom.setLong(0x024AE0, changeItemInRewardBoxFunction);

    // Replace 2928C lea (41F9 00FF1040) by a jsr to injected function
    rom.setWord(0x02928C, OPCODE_JSR);
    rom.setLong(0x02928E, changeItemGivenByItemOnGroundFunction);

    // Replace 1963C - 19644 (0400 00C0 ; 1340 0036) by a jsr to a replacement function
    rom.setWord(0x01963C, OPCODE_JSR);
    rom.setLong(0x01963E, changeItemVisibleOnGroundFunction);
    rom.setWord(0x019642, OPCODE_NOP);
}

void addFunctionToItemsOnUse(GameROM& rom)
{
    // ------------- Lithograph hint function -------------
    uint32_t lithographHintFunctionAddr = rom.getCurrentInjectionAddress();

    // move.w FFFF, D0
    rom.injectWord(0x303C);
    rom.injectWord(0xFFFF);

    // movem D0-A6, -(A7)
    rom.injectWord(0x48E7);
    rom.injectWord(0xFFFE);

    // jsr ($22FCC)
    rom.injectWord(OPCODE_JSR);
    rom.injectLong(0x00022FCC);

    // set hint addr ($FF1844)
    rom.injectWord(OPCODE_MOVL);
    rom.injectLong(rom.getStoredAddress("data_lithograph_hint_text"));
    rom.injectLong(0x00FF1844);

    // jmp ($22F34)
    rom.injectWord(OPCODE_JMP);
    rom.injectLong(0x00022F34);

    // ------------- Extended item handling function -------------

    rom.setWord(0x00DBA8, OPCODE_JSR);
    rom.setLong(0x00DBAA, rom.getCurrentInjectionAddress());
    rom.setWord(0x00DBAE, OPCODE_NOP);
    rom.setWord(0x00DBB0, OPCODE_NOP);
    rom.setWord(0x00DBB2, OPCODE_NOP);
    rom.setWord(0x00DBB4, OPCODE_NOP);

    // cmpi.b #23, D0
    rom.injectWord(0x0C00);
    rom.injectWord(ITEM_RECORD_BOOK);

    // bne to next case
    rom.injectWord(0x6608);

    // Call the "save game" function
    rom.injectWord(OPCODE_JSR);
    rom.injectLong(0x00001592);

    // Eject out to the "success" address
    rom.injectWord(OPCODE_RTS);

    // cmpi.b #18, D0
    rom.injectWord(0x0C00);
    rom.injectWord(ITEM_SPELL_BOOK);

    // bne to next case
    rom.injectWord(0x6608);

    // jsr $DC1C       ("Abracadabra...")
    rom.injectWord(OPCODE_JSR);
    rom.injectLong(0x0000DC1C);

    // Eject out to the "success" address
    rom.injectWord(OPCODE_RTS);

    // cmpi.b #27, D0
    rom.injectWord(0x0C00);
    rom.injectWord(ITEM_LITHOGRAPH);

    // bne to next case
    rom.injectWord(0x6606);

    // jsr LITHOGRAPH HINT FUNCTION
    rom.injectWord(OPCODE_JSR);
    rom.injectLong(lithographHintFunctionAddr);

    // rts
    rom.injectWord(OPCODE_RTS);

    // -------------------- Other modifications ---------------------

    // To remove the "Nothing happened..." text, the item must be put in a list which has a finite size.
    // We replace the Blue Ribbon (0x18) by the Record Book (0x23) to do so.
    rom.setByte(0x008642, 0x23);
    // Same for Lithograph (0x27) remplacing Lantern (0x1A)
    rom.setWord(0x008647, 0x6627);
}

void alterRomBeforeRandomization(GameROM& rom, const RandomizerOptions& options)
{
    // Rando core
    alterGameStart(rom, options);
    alterWaterfallShrineSecretStairsCheck(rom);
    alterVerlaBoulderCheck(rom);
    alterBlueRibbonStoryCheck(rom);
    alterKingNolesCaveTeleporterCheck(rom);
    alterMercatorDocksShopCheck(rom);
    alterMercatorSecondaryShopCheck(rom);
    alterArthurCheck(rom);
    alterItemOrderInMenu(rom);
    alterLifestockHandlingInShops(rom);

    fixAxeMagicCheck(rom);
    fixSafetyPassCheck(rom);
    fixArmletCheck(rom);
    fixSunstoneCheck(rom);
    fixDogTalkingCheck(rom);
    fixCryptBehavior(rom);
    fixMirAfterLakeShrineCheck(rom);
    fixLogsRoomExitCheck(rom);
    fixMirTowerPriestRoomItems(rom);
    fixKingNolesLabyrinthRafts(rom);
    fixFaraLifestockChest(rom);

    removeMercatorCastleBackdoorGuard(rom);
    removeSailorInDarkPort(rom);

    addNewlineHandlingInDynamicText(rom);
    addJewelsCheckForTeleporterToKazalt(rom);
    addStatueOfJyptaGoldsOverTime(rom);
    addLithographChestInKazaltTeleporterRoom(rom);

    // Glitch prevention
    fixArmletSkip(rom);
    fixTreeCuttingGlitch(rom);

    // Rando extensions (non-vanilla content)
    replaceLumberjackByChest(rom);
    replaceSickMerchantByChest(rom);
    replaceFaraInElderHouseByChest(rom);

    if(options.useArmorUpgrades())
        handleArmorUpgrades(rom);
}

void alterRomAfterRandomization(GameROM& rom, const RandomizerOptions& options)
{
    addFunctionToItemsOnUse(rom);
    alterGoldRewardsHandling(rom);
    alterLanternIntoPassiveItem(rom);
}