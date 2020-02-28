#include "GameAlterations.h"
#include "ItemCodes.h"
#include "RandomizerOptions.h"
#include "GameText.h"
#include "MegadriveTools/MdRom.h"
#include "MegadriveTools/MdCode.h"
#include <cstdint>
#include <vector>

constexpr uint32_t customTextStorageMemoryAddress = 0xFF0014;

///////////////////////////////////////////////////////////////////////////////////
//       Technical additions
///////////////////////////////////////////////////////////////////////////////////

void addNewlineHandlingInDynamicText(md::ROM& rom)
{
    // Inject a new condition in characters processing to handle newlines in dynamic text when 0x6C character is encountered
    md::Code procHandleNewlineInCustomText;

    procHandleNewlineInCustomText.cmpiw(0xFFFF, reg_D0);
    procHandleNewlineInCustomText.bne(2);
        procHandleNewlineInCustomText.jmp(0x230C0);
    procHandleNewlineInCustomText.cmpiw(0x006C, reg_D0);
    procHandleNewlineInCustomText.bne(3);
        procHandleNewlineInCustomText.jsr(0x22F7C); // "func_textbox_line_feed"
        procHandleNewlineInCustomText.jmp(0x23094); // process next character
    procHandleNewlineInCustomText.jmp(0x230A8); // resume to calling code

    uint32_t procAddr = rom.injectCode(procHandleNewlineInCustomText, "proc_handle_newline_in_custom_text");

    // Jump to the injected procedure
    rom.setCode(0x0230A2, md::Code().jmp(procAddr));
}

void addCustomTextHandling(md::ROM& rom)
{
    // --------------- Custom text handling procedure ---------------------
    // This procedure replaces the text that supposed to be written if an address is stored in 0xFF0014.l,
    // using this address instead as dynamic text.

    md::Code procHandleCustomText;

    procHandleCustomText.movemToStack({ reg_D0_D7 }, { reg_A0_A6 });
    procHandleCustomText.nop(3); // Let some space to insert the custom text usage function (check addCustomTextUsage())
    procHandleCustomText.tstl(addr_(customTextStorageMemoryAddress));
    procHandleCustomText.beq(11);
        // If memory address stored is negative (e.g. 0xFFFFFFFF), it means we don't want to display text
        procHandleCustomText.bpl(4);
            procHandleCustomText.movel(0x00000000, addr_(customTextStorageMemoryAddress));
            procHandleCustomText.movemFromStack({ reg_D0_D7 }, { reg_A0_A6 });
            procHandleCustomText.rts();
        // If there is already custom text address in 0xFF0014, use it
        procHandleCustomText.movew(0xFFFF, reg_D0);
        procHandleCustomText.moveb(0x00, addr_(0xFF1144));  // to reset textbox state
        procHandleCustomText.jsr(0x22FCC);
        procHandleCustomText.movel(addr_(customTextStorageMemoryAddress), addr_(0xFF1844));
        procHandleCustomText.movel(0x00000000, addr_(customTextStorageMemoryAddress));
        procHandleCustomText.bra(2);
    // Otherwise, print default text
    procHandleCustomText.jsr(0x22FCC);

    procHandleCustomText.jmp(0x22F34);

    uint32_t procHandleCustomTextAddr = rom.injectCode(procHandleCustomText, "proc_handle_custom_text");

    // Inject the custom text handling procedure at the beginning of regular text handling function
    rom.setCode(0x22F2C, md::Code().jmp(procHandleCustomTextAddr).nop());
    rom.storeAddress("func_display_text", 0x22F2C);

    // --------------- Full textbox handling function ---------------------
    md::Code funcDisplayTextWithTextbox;

    funcDisplayTextWithTextbox.movemToStack({ reg_D0_D7 }, { reg_A0_A6 });
    funcDisplayTextWithTextbox.jsr(0x22EE8); // open textbox
    funcDisplayTextWithTextbox.jsr(0x22F2C); // display text
    funcDisplayTextWithTextbox.jsr(0x22EA0); // close textbox
    funcDisplayTextWithTextbox.movemFromStack({ reg_D0_D7 }, { reg_A0_A6 });
    funcDisplayTextWithTextbox.rts();

    rom.injectCode(funcDisplayTextWithTextbox, "func_display_text_with_textbox");
}

void addTextReplacementFunction(md::ROM& rom)
{
    constexpr uint32_t regularTextTableAddress = 0x277F6;
    constexpr uint32_t regularTextEnd = 0x29000;

    // --------------- Text replacement function ---------------------
    // This function analyzes the text about to be displayed in the textbox and replaces it with custom text
    // if it's appropriate to do so.
    // For instance, if we interact with a road sign, it puts a hint instead of the usual sign indications.

    md::Code funcFindCustomText;

    funcFindCustomText.movemToStack({ reg_D0, reg_D1 }, { reg_A1 });
    funcFindCustomText.cmpa(lval_(0x29000), reg_A0);
    funcFindCustomText.bgt(12);
        // Iterate over text replacement table to find the related text
        funcFindCustomText.movel(reg_A0, reg_D0);
        funcFindCustomText.subil(regularTextTableAddress, reg_D0);
        funcFindCustomText.movel(rom.getStoredAddress("data_text_replacement_table"), reg_A1);
        funcFindCustomText.label("loop_start");
        funcFindCustomText.movew(addr_(reg_A1), reg_D1);
        funcFindCustomText.bmi(7);
            funcFindCustomText.cmpw(reg_D1, reg_D0);
            funcFindCustomText.bne(3);
                funcFindCustomText.movel(addr_(reg_A1, 0x2), addr_(customTextStorageMemoryAddress));
                funcFindCustomText.bra(3);
            funcFindCustomText.adda(0x6, reg_A1);
            funcFindCustomText.bra("loop_start");
    funcFindCustomText.movemFromStack({ reg_D0, reg_D1 }, { reg_A1 });
    funcFindCustomText.rts();

/*    
    funcFindCustomText.movemToStack({ reg_D0 }, { reg_A1 });
    funcFindCustomText.cmpa(lval_(0xFF5480), reg_A4);
    funcFindCustomText.blt(12);
        // If this is a road sign...
        funcFindCustomText.cmpiw(0x0128, addr_(reg_A4, 0xA));
        funcFindCustomText.bne(10);
            // Iterate over signs table to find the related text
            funcFindCustomText.movel(rom.getStoredAddress("data_signs_table"), reg_A0);
            funcFindCustomText.label("loop_start");
            funcFindCustomText.movew(addr_(reg_A0), reg_D0);
            funcFindCustomText.bmi(7);
                funcFindCustomText.cmpw(addr_(0xFF1204), reg_D0);
                funcFindCustomText.bne(3);
                    funcFindCustomText.movel(addr_(reg_A0, 0x2), addr_(customTextStorageMemoryAddress));
                    funcFindCustomText.bra(3);
                funcFindCustomText.adda(0x6, reg_A0);
                funcFindCustomText.bra("loop_start");
    funcFindCustomText.movemFromStack({ reg_D0, reg_D1 }, { reg_A1 });
    funcFindCustomText.rts();
    */
    uint32_t funcFindCustomTextAddr = rom.injectCode(funcFindCustomText);
    uint32_t injectionAddress = rom.getStoredAddress("proc_handle_custom_text") + 0x4;
    rom.setCode(injectionAddress, md::Code().jsr(funcFindCustomTextAddr)); // Try to find an appropriate custom text for this context
}



///////////////////////////////////////////////////////////////////////////////////
//       Game & gameplay changes
///////////////////////////////////////////////////////////////////////////////////

void alterGameStart(md::ROM& rom, const RandomizerOptions& options)
{
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

    md::Code funcInitFlags;

    // Set the orientation byte of Nigel to 88 (south-west) on game start
    funcInitFlags.moveb(0x88, addr_(0xFF5404));

    for(int i=0 ; i<0x60 ; i+=0x2)
    {
        uint16_t value = (static_cast<uint16_t>(flagArray[i]) << 8) + static_cast<uint16_t>(flagArray[i+1]);
        if(value)
            funcInitFlags.movew(value, addr_(0xFF1000+i));
    }

    funcInitFlags.rts();

    uint32_t funcInitFlagsAddr = rom.injectCode(funcInitFlags);

    // ------- Remove no music flag ---------
    // Replace the bitset of the no music flag by a jump to the injected flags init function located at the end of the rom

    // 0x002700:
        // Before: 	[08F9] bset 3 -> $FF1027
        // After:	[4EB9] jsr $1FFAD0 ; [4E71] nop
    rom.setCode(0x002700, md::Code().jsr(funcInitFlagsAddr).nop());

    // ------- Remove cutscene flag (no input allowed) ---------
    // Usually, when starting a new game, it is automatically put into "cutscene mode" to let the intro roll without allowing the player
    // to move or pause, or do anything at all. We need to remove that cutscene flag to enable the player actually playing the game.

    // 0x00281A:
        // Before:	[33FC] move.w 0x00FE -> $FF12DE
        // After:	[4E71] nop (4 times)
    rom.setCode(0x281A, md::Code().nop(4));
}

void alterItemOrderInMenu(md::ROM& rom)
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

void alterLifestockHandlingInShops(md::ROM& rom)
{
    // Make Lifestock prices the same over all shops
    for (uint32_t addr = 0x024D34; addr <= 0x024EAE; addr += 0xE)
        rom.setByte(addr + 0x03, 0x10); //rom.getByte(addr + 0x02));

    // Remove the usage of "bought lifestock in shop X" flags 
    for (uint32_t addr = 0x009D18; addr <= 0x009D33; addr += 0xE)
        rom.setByte(addr, 0xFF);
}

void addStatueOfJyptaGoldsOverTime(md::ROM& rom)
{
    constexpr uint16_t goldsPerCycle = 0x0002;

    // ============== Function to handle walk abilities (healing boots, jypta statue...) ==============
    md::Code funcHandleWalkAbilities;

    // If Statue of Jypta is owned, gain gold over time
    funcHandleWalkAbilities.btst(0x5, addr_(0xFF104E));
    funcHandleWalkAbilities.beq(3);
    funcHandleWalkAbilities.movew(goldsPerCycle, reg_D0);
    funcHandleWalkAbilities.jsr(0x177DC);   // rom.getStoredAddress("func_earn_gold");

// If Healing boots are equipped, gain life over time
    funcHandleWalkAbilities.cmpib(0x7, addr_(0xFF1150));
    funcHandleWalkAbilities.bne(4);
    funcHandleWalkAbilities.movew(0x100, reg_D0);
    funcHandleWalkAbilities.lea(0xFF5400, reg_A5);
    funcHandleWalkAbilities.jsr(0x1780E);   // rom.getStoredAddress("func_heal_hp");

    funcHandleWalkAbilities.rts();

    uint32_t funcAddr = rom.injectCode(funcHandleWalkAbilities);

    // ============== Hook the function inside game code ==============

    rom.setCode(0x16696, md::Code().nop(5));

    rom.setCode(0x166D0, md::Code().jsr(funcAddr).nop(4));
}

void quickenGaiaEffect(md::ROM& rom)
{
    constexpr uint8_t factor = 3;

    rom.setWord(0x1686C, rom.getWord(0x1686C) * factor);
    rom.setWord(0x16878, rom.getWord(0x16878) * factor);
    rom.setWord(0x16884, rom.getWord(0x16884) * factor);
}

void handleArmorUpgrades(md::ROM& rom)
{
    // --------------- Alter item in D0 register function ---------------
    md::Code funcAlterItemInD0;

    // Check if item ID is between 09 and 0C (armors). If not, branch to return.
    funcAlterItemInD0.cmpib(ITEM_STEEL_BREAST, reg_D0);
    funcAlterItemInD0.blt(13);
    funcAlterItemInD0.cmpib(ITEM_HYPER_BREAST, reg_D0);
    funcAlterItemInD0.bgt(11);

    // By default, put Hyper breast as given armor
    funcAlterItemInD0.movew(ITEM_HYPER_BREAST, reg_D0);

    // If Shell breast is not owned, put Shell breast
    funcAlterItemInD0.btst(0x05, addr_(0xFF1045));
    funcAlterItemInD0.bne(2);
    funcAlterItemInD0.movew(ITEM_SHELL_BREAST, reg_D0);

    // If Chrome breast is not owned, put Chrome breast
    funcAlterItemInD0.btst(0x01, addr_(0xFF1045));
    funcAlterItemInD0.bne(2);
    funcAlterItemInD0.movew(ITEM_CHROME_BREAST, reg_D0);

    // If Steel breast is not owned, put Steel breast
    funcAlterItemInD0.btst(0x05, addr_(0xFF1044));
    funcAlterItemInD0.bne(2);
    funcAlterItemInD0.movew(ITEM_STEEL_BREAST, reg_D0);

    funcAlterItemInD0.rts();

    uint32_t funcAlterItemInD0Addr = rom.injectCode(funcAlterItemInD0);


    // --------------- Change item in reward box function ---------------
    md::Code funcChangeItemRewardBox;

    funcChangeItemRewardBox.jsr(funcAlterItemInD0Addr);
    funcChangeItemRewardBox.movew(reg_D0, addr_(0xFF1196));
    funcChangeItemRewardBox.rts();

    uint32_t funcChangeItemRewardBoxAddr = rom.injectCode(funcChangeItemRewardBox);

    // --------------- Change item given by taking item on ground function ---------------
    md::Code funcAlterItemGivenByGroundSource;

    funcAlterItemGivenByGroundSource.movemToStack({ reg_D7 }, { reg_A0 }); // movem D7,A0 -(A7)	(48E7 0180)

    funcAlterItemGivenByGroundSource.cmpib(ITEM_HYPER_BREAST, reg_D0);
    funcAlterItemGivenByGroundSource.bgt(9); // to movem
    funcAlterItemGivenByGroundSource.cmpib(ITEM_STEEL_BREAST, reg_D0);
    funcAlterItemGivenByGroundSource.blt(7);  // to movem

    funcAlterItemGivenByGroundSource.jsr(funcAlterItemInD0Addr);
    funcAlterItemGivenByGroundSource.moveb(addr_(reg_A5, 0x3B), reg_D7);  // move ($3B,A5), D7	(1E2D 003B)
    funcAlterItemGivenByGroundSource.subib(0xC9, reg_D7);
    funcAlterItemGivenByGroundSource.cmpa(lval_(0xFF5400), reg_A5);
    funcAlterItemGivenByGroundSource.blt(2);    // to movem
    funcAlterItemGivenByGroundSource.bset(reg_D7, addr_(0xFF103F)); // set a flag when an armor is taken on the ground for it to disappear afterwards

    funcAlterItemGivenByGroundSource.movemFromStack({ reg_D7 }, { reg_A0 }); // movem (A7)+, D7,A0	(4CDF 0180)
    funcAlterItemGivenByGroundSource.lea(0xFF1040, reg_A0);
    funcAlterItemGivenByGroundSource.rts();

    uint32_t funcAlterItemGivenByGroundSourceAddr = rom.injectCode(funcAlterItemGivenByGroundSource);

    // --------------- Change visible item for items on ground function ---------------
    md::Code funcAlterItemVisibleForGroundSource;

    funcAlterItemVisibleForGroundSource.movemToStack({ reg_D7 }, { reg_A0 });  // movem D7,A0 -(A7)

    funcAlterItemVisibleForGroundSource.subib(0xC0, reg_D0);
    funcAlterItemVisibleForGroundSource.cmpib(ITEM_HYPER_BREAST, reg_D0);
    funcAlterItemVisibleForGroundSource.bgt(10); // to move D0 in item slot
    funcAlterItemVisibleForGroundSource.cmpib(ITEM_STEEL_BREAST, reg_D0);
    funcAlterItemVisibleForGroundSource.blt(8); // to move D0 in item slot
    funcAlterItemVisibleForGroundSource.moveb(reg_D0, reg_D7);
    funcAlterItemVisibleForGroundSource.subib(ITEM_STEEL_BREAST, reg_D7);
    funcAlterItemVisibleForGroundSource.btst(reg_D7, addr_(0xFF103F));
    funcAlterItemVisibleForGroundSource.bne(3);
    // Item was not already taken, alter the armor inside
    funcAlterItemVisibleForGroundSource.jsr(funcAlterItemInD0Addr);
    funcAlterItemVisibleForGroundSource.bra(2);
    // Item was already taken, remove it by filling it with an empty item
    funcAlterItemVisibleForGroundSource.movew(ITEM_NONE, reg_D0);
    funcAlterItemVisibleForGroundSource.moveb(reg_D0, addr_(reg_A1, 0x36)); // move D0, ($36,A1) (1340 0036)
    funcAlterItemVisibleForGroundSource.movemFromStack({ reg_D7 }, { reg_A0 }); // movem (A7)+, D7,A0	(4CDF 0180)
    funcAlterItemVisibleForGroundSource.rts();

    uint32_t funcAlterItemVisibleForGroundSourceAddr = rom.injectCode(funcAlterItemVisibleForGroundSource);

    // --------------- Hooks ---------------
    // In 'chest reward' function, replace the item ID move by the injected function
    rom.setCode(0x0070BE, md::Code().jsr(funcChangeItemRewardBoxAddr));

    // In 'NPC reward' function, replace the item ID move by the injected function
    rom.setCode(0x028DD8, md::Code().jsr(funcChangeItemRewardBoxAddr));

    // In 'item on ground reward' function, replace the item ID move by the injected function
    rom.setWord(0x024ADC, 0x3002); // put the move D2,D0 before the jsr because it helps us while changing nothing to the usual logic
    rom.setCode(0x024ADE, md::Code().jsr(funcChangeItemRewardBoxAddr));

    // Replace 2928C lea (41F9 00FF1040) by a jsr to injected function
    rom.setCode(0x02928C, md::Code().jsr(funcAlterItemGivenByGroundSourceAddr));

    // Replace 1963C - 19644 (0400 00C0 ; 1340 0036) by a jsr to a replacement function
    rom.setCode(0x01963C, md::Code().jsr(funcAlterItemVisibleForGroundSourceAddr).nop());
}




///////////////////////////////////////////////////////////////////////////////////
//       Item check changes
///////////////////////////////////////////////////////////////////////////////////

void fixAxeMagicCheck(md::ROM& rom)
{
    // Changes the Axe Magic check when slashing a tree from bit 0 of flag 1003 to "Axe Magic owned"
    // 0x16262:
        // Before:	[0839] btst 0 in FF1003
        // After:	[0839] btst 5 in FF104B
    rom.setWord(0x016264, 0x0005);
    rom.setWord(0x016268, 0x104B);
}

void fixSafetyPassCheck(md::ROM& rom)
{
    // Change Mercator door opened check from bit 7 of flag 1004 to "Safety Pass owned"
    // 0x004FF8:
        // Before:	04 0F (0F & 7 = 7 -----> bit 7 of FF1004)
        // After:	59 0D (0D & 7 = 5 -----> bit 5 of FF1059)
    rom.setWord(0x004FF8, 0x590D);
}

void fixArmletCheck(md::ROM& rom)
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
    rom.setCode(0x013A8A, md::Code().rts());
}

void fixSunstoneCheck(md::ROM& rom)
{
    // Change Sunstone check for repairing the lighthouse from bit 2 of flag 1026 to "Susntone owned"
    // 0x09D091:
        // Before:	26 02 (bit 2 of FF1026)
        // After:	4F 01 (bit 1 of FF104F)
    rom.setWord(0x09D091, 0x4F01);
}

void fixDogTalkingCheck(md::ROM& rom)
{
    // Change doggo talking check from bit 4 of flag 1024 to "Einstein Whistle owned"
    // 0x0253C0:
        // Before:	01 24 (0124 >> 3 = 24 and 0124 & 7 = 04 -----> bit 4 of FF1024)
        // After:	02 81 (0281 >> 3 = 50 and 0281 & 7 = 01 -----> bit 1 of FF1050)
    rom.setWord(0x0253C0, 0x0281);
}

void alterMercatorSecondaryShopCheck(md::ROM& rom)
{
    // Change the Mercator secondary shop check so that it sells item as long as you own Buyer's Card

    // 0x00A574:
        // Before:	2A 04 (bit 4 of FF102A)
        // After:	4C 05 (bit 5 of FF104C)
    rom.setWord(0x00A574, 0x4C05);
}

void addJewelsCheckForTeleporterToKazalt(md::ROM& rom)
{
    GameText text("King Nole: Only the bearers of the jewels are worthy of entering my domain...");
    rom.injectDataBlock(text.getBytes(), "data_text_jewels_alert");

    // ----------- Jewel textbox handling ----------------------
    md::Code procHandleJewelsCheck;

    procHandleJewelsCheck.btst(0x1, addr_(0xFF1054)); // Test if red jewel is owned
    procHandleJewelsCheck.beq(3);
    procHandleJewelsCheck.btst(0x1, addr_(0xFF1055)); // Test if purple jewel is owned
    procHandleJewelsCheck.bne(4);
        procHandleJewelsCheck.movel(rom.getStoredAddress("data_text_jewels_alert"), addr_(0xFF0014));
        procHandleJewelsCheck.jsr(rom.getStoredAddress("func_display_text_with_textbox"));
        procHandleJewelsCheck.rts();
    procHandleJewelsCheck.moveq(0x7, reg_D0);
    procHandleJewelsCheck.jsr(0xE110);  // "func_teleport_kazalt"
    procHandleJewelsCheck.jmp(0x62FA);

    uint32_t procHandleJewelsAddr = rom.injectCode(procHandleJewelsCheck, "proc_handle_jewels_check");

    // This adds the purple & red jewel as a requirement for the Kazalt teleporter to work correctly
    rom.setCode(0x62F4, md::Code().jmp(procHandleJewelsAddr));
}

void fixSwordOfGaiaEffectInVolcano(md::ROM& rom)
{
    md::Code procTriggerVolcano;

    procTriggerVolcano.cmpiw(0x20A, addr_(0xFF1204));
    procTriggerVolcano.bne(4);
        procTriggerVolcano.bset(0x2, addr_(0xFF1027));
        procTriggerVolcano.jsr(0x16712);
        procTriggerVolcano.rts();
    procTriggerVolcano.jmp(0x16128);

    uint32_t procAddr = rom.injectCode(procTriggerVolcano);

    rom.setCode(0x1611E, md::Code().jmp(procAddr));
}




///////////////////////////////////////////////////////////////////////////////////
//       Map check changes
///////////////////////////////////////////////////////////////////////////////////

void alterWaterfallShrineSecretStairsCheck(md::ROM& rom)
{
    // Change Waterfall Shrine entrance check from "Talked to Prospero" to "What a noisy boy!", removing the need
    // of talking to Prospero (which we couldn't do anyway because of the story flags).

    // 0x005014:
        // Before:	00 08 (bit 0 of FF1000)
        // After:	02 09 (bit 1 of FF1002)
    rom.setWord(0x005014, 0x0209);
}

void alterVerlaBoulderCheck(md::ROM& rom)
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

void alterBlueRibbonStoryCheck(md::ROM& rom)
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

void alterKingNolesCaveTeleporterCheck(md::ROM& rom)
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

    md::Code procImproveFlagCheck;

    procImproveFlagCheck.moveb(addr_(reg_A0, 0x2), reg_D0);     // 1028 0002
    procImproveFlagCheck.cmpib(0xFF, reg_D0);
    procImproveFlagCheck.bne(2);
    procImproveFlagCheck.jmp(0x4E2E);
    procImproveFlagCheck.jmp(0x4E20);

    uint32_t procAddr = rom.injectCode(procImproveFlagCheck);

    // Replace the (move.b, bmi, ext.w) by a jmp to the injected procedure
    rom.setCode(0x004E18, md::Code().clrw(reg_D0).jmp(procAddr));
}

void alterMercatorDocksShopCheck(md::ROM& rom)
{
    // 0x01AA26:
        // Before:	0284 2A A2 (in map 284, check bit 5 of flag 102A)
        // After:	0000 5F E2 (in map 0, check bit 7 of flag 105F - never true)
    rom.setWord(0x01AA26, 0x0000);
    rom.setWord(0x01AA28, 0x5FE2);
}

void alterArthurCheck(md::ROM& rom)
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

void fixCryptBehavior(md::ROM& rom)
{
    // 1) Remove the check "if shadow mummy was beaten, raft mummy never appears again"
    // 0x019DF6:
        // Before:	0839 0006 00FF1014 (btst bit 6 in FF1014) ; 66 14 (bne $19E14)
        // After:	4EB9 00019E14 (jsr $19E14; 4E71 4E71 (nop nop)
    rom.setCode(0x19DF6, md::Code().nop(5));

    // 2) Change the room exit check and shadow mummy appearance from "if armlet is owned" to "chest was opened"
    // 0x0117E8:
        // Before:	103C 001F ; 4EB9 00022ED0 ; 4A41 ; 6B00 F75C (bmi $10F52)
        // After:	0839 0002 00FF1097 (btst 2 FF1097)	; 6700 F75C (bne $10F52)
    md::Code injectChangeCryptExitCheck;
    injectChangeCryptExitCheck.btst(0x2, addr_(0xFF1097));
    injectChangeCryptExitCheck.nop(2);
    injectChangeCryptExitCheck.beq(); // beq $10F52
    rom.setCode(0x117E8, injectChangeCryptExitCheck);
}

void fixKingNolesLabyrinthRafts(md::ROM& rom)
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

void fixLogsRoomExitCheck(md::ROM& rom)
{
    // Remove logs check
    rom.setCode(0x011EC4, md::Code().bra());
}

void fixMirAfterLakeShrineCheck(md::ROM& rom)
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

void fixMirTowerPriestRoomItems(md::ROM& rom)
{
    // Remove the "shop/church" flag on the priest room of Mir Tower to make its items on ground work everytime
    // 0x024E5A:
        // Before:	0307
        // After:	7F7F
    rom.setWord(0x024E5A, 0x7F7F);
}





///////////////////////////////////////////////////////////////////////////////////
//       Map content changes
///////////////////////////////////////////////////////////////////////////////////

void removeMercatorCastleBackdoorGuard(md::ROM& rom)
{
    // There is a guard staying in front of the Mercator castle backdoor to prevent you from using
    // Mir Tower keys on it. He appears when Crypt is finished and disappears when Mir Tower is finished,
    // but we actually never want him to be there, so we delete him from existence by moving him away from the map.

    // 0x0215A6:
        // Before:	93 9D
        // After:	00 00
    rom.setWord(0x0215A6, 0x0000);
}

void removeSailorInDarkPort(md::ROM& rom)
{
    // There is a sailor NPC in the "dark" version of Mercator port who responds badly to story triggers, allowing us to sail to Verla
    // even without having repaired the lighthouse. To prevent this from being exploited, we removed him altogether.

    // 0x021646:
        // Before:	23 EC
        // After:	00 00
    rom.setWord(0x021646, 0x0000);
}

void fixFaraLifestockChest(md::ROM& rom)
{
    // Make it so Lifestock chest near Fara in Swamp Shrine appears again when going back into the room afterwards, preventing any softlock there.

    // --------- Function to remove all entities but the chest when coming back in the room ---------
    md::Code funcRemoveAllEntitiesButChestInFaraRoom;

    funcRemoveAllEntitiesButChestInFaraRoom.movemToStack({ reg_D0_D7 }, { reg_A0_A6 });
    funcRemoveAllEntitiesButChestInFaraRoom.lea(0xFF5480, reg_A0);
    funcRemoveAllEntitiesButChestInFaraRoom.moveq(0xD, reg_D0);
    funcRemoveAllEntitiesButChestInFaraRoom.label("loop_remove_entities");
    funcRemoveAllEntitiesButChestInFaraRoom.movew(0x7F7F, addr_(reg_A0));
    funcRemoveAllEntitiesButChestInFaraRoom.adda(0x80, reg_A0);
    funcRemoveAllEntitiesButChestInFaraRoom.dbra(reg_D0, "loop_remove_entities");
    funcRemoveAllEntitiesButChestInFaraRoom.movemFromStack({ reg_D0_D7 }, { reg_A0_A6 });
    funcRemoveAllEntitiesButChestInFaraRoom.rts();

    uint32_t funcAddr = rom.injectCode(funcRemoveAllEntitiesButChestInFaraRoom);

    // Call the injected function
    rom.setCode(0x019BE0, md::Code().jsr(funcAddr).nop());

    // --------- Moving the chest to the ground ---------
    rom.setWord(0x01BF6C, 0x1A93);
    rom.setWord(0x01BF6E, 0x0000);
    rom.setWord(0x01BF70, 0x0012);
    rom.setWord(0x01BF72, 0x0400);
}

void addLithographChestInKazaltTeleporterRoom(md::ROM& rom)
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

void replaceLumberjackByChest(md::ROM& rom)
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

void replaceSickMerchantByChest(md::ROM& rom)
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

void replaceFaraInElderHouseByChest(md::ROM& rom)
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





///////////////////////////////////////////////////////////////////////////////////
//       Original game bugs & glitches fixes
///////////////////////////////////////////////////////////////////////////////////

void fixArmletSkip(md::ROM& rom)
{
    // Fix armlet skip by putting the tornado way higher, preventing any kind of buffer-jumping on it
    // 0x02030C:
        // Before:  0x82 (Tornado pos Y = 20)
        // After:   0x85 (Tornado pos Y = 50)
    rom.setByte(0x02030C, 0x85);
}

void fixTreeCuttingGlitch(md::ROM& rom)
{
    // Inject a new function which fixes the money value check on an enemy when it is killed, causing the tree glitch to be possible
    md::Code funcFixTreeCuttingGlitch;
 
    funcFixTreeCuttingGlitch.tstb(addr_(reg_A5, 0x36));  // tst.b ($36,A5) [4A2D 0036]
    funcFixTreeCuttingGlitch.beq(4);
        // Only allow the "killable because holding money" check if the enemy is not a tree
        funcFixTreeCuttingGlitch.cmpiw(0x126, addr_(reg_A5, 0xA));
        funcFixTreeCuttingGlitch.beq(2);
            funcFixTreeCuttingGlitch.jmp(0x16284);
    funcFixTreeCuttingGlitch.rts();

    uint32_t funcAddr = rom.injectCode(funcFixTreeCuttingGlitch);

    // Call the injected function when killing an enemy
    rom.setCode(0x01625C, md::Code().jsr(funcAddr));
}



///////////////////////////////////////////////////////////////////////////////////
//       Post-generation stuff
///////////////////////////////////////////////////////////////////////////////////

void alterLanternIntoPassiveItem(md::ROM& rom)
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
    rom.setCode(0x0087D6, md::Code().btst(0x1, addr_(0xFF104D)).nop(6));

//    for (uint32_t addr = 0x87DD; addr >= 0x87C2; addr -= 0x01)
//        rom.setByte(addr + 2, rom.getByte(addr));

    // lea ADDR_DARK_ROOMS, A0
//    rom.setWord(0x0087BE, 0x41F9);
//    rom.setLong(0x0087C0, rom.getStoredAddress("data_dark_rooms"));
}

void alterGoldRewardsHandling(md::ROM& rom)
{
    // In the original game, only 3 item IDs are reserved for gold rewards (3A, 3B, 3C)
    // Here, we moved the table of gold rewards to the end of the ROM so that we can handle 64 rewards up to 255 golds each.
    // In the new system, all item IDs after the "empty item" one (0x40 and above) are now gold rewards.
    
    rom.setByte(0x0070DF, 0x40); // cmpi 3A, D0 >>> cmpi 40, D0
    rom.setByte(0x0070E5, 0x40); // subi 3A, D0 >>> subi 40, D0

    // ------------- Function to put gold reward value in D0 ----------------
    // Input: D0 = gold reward ID (offset from 0x40)
    // Output: D0 = gold reward value

    md::Code funcGetGoldReward;

    funcGetGoldReward.movemToStack({}, { reg_A0 });
    funcGetGoldReward.lea(rom.getStoredAddress("data_gold_values"), reg_A0);
    funcGetGoldReward.moveb(addr_(reg_A0, reg_D0, md::Size::WORD), reg_D0);  // move.b (A0, D0.w), D0 : 1030 0000
    funcGetGoldReward.movemFromStack({}, { reg_A0 });
    funcGetGoldReward.rts();

    uint32_t funcAddr = rom.injectCode(funcGetGoldReward);

    // Set the call to the injected function
    // Before:      add D0,D0   ;   move.w (PC, D0, 42), D0
    // After:       jsr to injected function
    rom.setCode(0x0070E8, md::Code().jsr(funcAddr));
}

void addFunctionToItemsOnUse(md::ROM& rom)
{
    // ------------- Extended item handling function -------------

    md::Code funcExtendedItemHandling;

    funcExtendedItemHandling.cmpib(ITEM_RECORD_BOOK, reg_D0);
    funcExtendedItemHandling.bne(3);
        funcExtendedItemHandling.jsr(0x1592); // "func_save_game"
        funcExtendedItemHandling.rts();
    funcExtendedItemHandling.cmpib(ITEM_SPELL_BOOK, reg_D0);
    funcExtendedItemHandling.bne(3);
        funcExtendedItemHandling.jsr(0xDC1C); // "func_abracadabra"
        funcExtendedItemHandling.rts();
    funcExtendedItemHandling.cmpib(ITEM_LITHOGRAPH, reg_D0);
    funcExtendedItemHandling.bne(3);
        funcExtendedItemHandling.movel(rom.getStoredAddress("data_lithograph_hint_text"), addr_(0xFF0014));
        funcExtendedItemHandling.jsr(rom.getStoredAddress("func_display_text"));
    funcExtendedItemHandling.rts();
    
    uint32_t funcExtendedItemHandlingAddr = rom.injectCode(funcExtendedItemHandling);

    rom.setCode(0x00DBA8, md::Code().jsr(funcExtendedItemHandlingAddr).nop(4));

    // -------------------- Other modifications ---------------------

    // To remove the "Nothing happened..." text, the item must be put in a list which has a finite size.
    // We replace the Blue Ribbon (0x18) by the Record Book (0x23) to do so.
    rom.setByte(0x008642, 0x23);
    // Same for Lithograph (0x27) remplacing Lantern (0x1A)
    rom.setWord(0x008647, 0x6627);
}




void alterRomBeforeRandomization(md::ROM& rom, const RandomizerOptions& options)
{
    // Technical additions
    addNewlineHandlingInDynamicText(rom);
    addCustomTextHandling(rom);

    // Game & gameplay changes
    alterGameStart(rom, options);
    alterItemOrderInMenu(rom);
    alterLifestockHandlingInShops(rom);
    addStatueOfJyptaGoldsOverTime(rom);
    quickenGaiaEffect(rom);
    if (options.useArmorUpgrades())
        handleArmorUpgrades(rom);

    // Item check changes
    fixAxeMagicCheck(rom);
    fixSafetyPassCheck(rom);
    fixArmletCheck(rom);
    fixSunstoneCheck(rom);
    fixDogTalkingCheck(rom);
    alterMercatorSecondaryShopCheck(rom);
    addJewelsCheckForTeleporterToKazalt(rom);
    fixSwordOfGaiaEffectInVolcano(rom);

    // Map check changes
    alterWaterfallShrineSecretStairsCheck(rom);
    alterVerlaBoulderCheck(rom);
    alterBlueRibbonStoryCheck(rom);
    alterKingNolesCaveTeleporterCheck(rom);
    alterMercatorDocksShopCheck(rom);
    alterArthurCheck(rom);
    fixCryptBehavior(rom);
    fixKingNolesLabyrinthRafts(rom);
    fixLogsRoomExitCheck(rom);
    fixMirAfterLakeShrineCheck(rom);
    fixMirTowerPriestRoomItems(rom);

    // Map content changes
    removeMercatorCastleBackdoorGuard(rom);
    removeSailorInDarkPort(rom);
    fixFaraLifestockChest(rom);
    addLithographChestInKazaltTeleporterRoom(rom);
    replaceLumberjackByChest(rom);
    replaceSickMerchantByChest(rom);
    replaceFaraInElderHouseByChest(rom);

    // Fix original game glitches & bugs
    fixArmletSkip(rom);
    fixTreeCuttingGlitch(rom);
}

void alterRomAfterRandomization(md::ROM& rom, const RandomizerOptions& options)
{
    // Post-generation stuff
    addFunctionToItemsOnUse(rom);
    alterGoldRewardsHandling(rom);
    alterLanternIntoPassiveItem(rom);
    addTextReplacementFunction(rom);
}