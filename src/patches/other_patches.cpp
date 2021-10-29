#include "../tools/megadrive/rom.hpp"
#include "../tools/megadrive/code.hpp"
#include "../tools/game_text.hpp"

#include "../randomizer_options.hpp"
#include "../world.hpp"
#include "../exceptions.hpp"

#include <cstdint>
#include <vector>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////////
//       Game & gameplay changes
///////////////////////////////////////////////////////////////////////////////////


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
        ITEM_GREEN_JEWEL,   ITEM_BLUE_JEWEL,
        ITEM_YELLOW_JEWEL,  ITEM_SPELL_BOOK,
        ITEM_BLUE_RIBBON,   0xFF
    };

    uint32_t baseAddress = 0x00D55C;
    for (int i = 0; baseAddress + i < 0x00D584; ++i)
        rom.set_byte(baseAddress + i, itemOrder[i]);
}

void alterLifestockHandlingInShops(md::ROM& rom)
{
    // Make Lifestock prices the same over all shops
    for (uint32_t addr = 0x024D34; addr <= 0x024EAE; addr += 0xE)
        rom.set_byte(addr + 0x03, 0x10);

    // Remove the usage of "bought lifestock in shop X" flags 
    for (uint32_t addr = 0x009D18; addr <= 0x009D33; addr += 0xE)
        rom.set_byte(addr, 0xFF);
}

void alter_fahl_challenge(md::ROM& rom, const World& world)
{
    // Neutralize mid-challenge proposals for money
    rom.set_code(0x12D52, md::Code().nop(24));
    
    // Set the end of the challenge at the number of fahl enemies in the world list
    rom.set_byte(0x12D87, (uint8_t)world.fahl_enemies().size());
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

    uint32_t funcAlterItemInD0Addr = rom.inject_code(funcAlterItemInD0);


    // --------------- Change item in reward box function ---------------
    md::Code funcChangeItemRewardBox;

    funcChangeItemRewardBox.jsr(funcAlterItemInD0Addr);
    funcChangeItemRewardBox.movew(reg_D0, addr_(0xFF1196));
    funcChangeItemRewardBox.rts();

    uint32_t funcChangeItemRewardBoxAddr = rom.inject_code(funcChangeItemRewardBox);

    // --------------- Change item given by taking item on ground function ---------------
    md::Code funcAlterItemGivenByGroundSource;

    funcAlterItemGivenByGroundSource.movem_to_stack({ reg_D7 }, { reg_A0 }); // movem D7,A0 -(A7)	(48E7 0180)

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

    funcAlterItemGivenByGroundSource.movem_from_stack({ reg_D7 }, { reg_A0 }); // movem (A7)+, D7,A0	(4CDF 0180)
    funcAlterItemGivenByGroundSource.lea(0xFF1040, reg_A0);
    funcAlterItemGivenByGroundSource.rts();

    uint32_t funcAlterItemGivenByGroundSourceAddr = rom.inject_code(funcAlterItemGivenByGroundSource);

    // --------------- Change visible item for items on ground function ---------------
    md::Code funcAlterItemVisibleForGroundSource;

    funcAlterItemVisibleForGroundSource.movem_to_stack({ reg_D7 }, { reg_A0 });  // movem D7,A0 -(A7)

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
    funcAlterItemVisibleForGroundSource.movem_from_stack({ reg_D7 }, { reg_A0 }); // movem (A7)+, D7,A0	(4CDF 0180)
    funcAlterItemVisibleForGroundSource.rts();

    uint32_t funcAlterItemVisibleForGroundSourceAddr = rom.inject_code(funcAlterItemVisibleForGroundSource);

    // --------------- Hooks ---------------
    // In 'chest reward' function, replace the item ID move by the injected function
    rom.set_code(0x0070BE, md::Code().jsr(funcChangeItemRewardBoxAddr));

    // In 'NPC reward' function, replace the item ID move by the injected function
    rom.set_code(0x028DD8, md::Code().jsr(funcChangeItemRewardBoxAddr));

    // In 'item on ground reward' function, replace the item ID move by the injected function
    rom.set_word(0x024ADC, 0x3002); // put the move D2,D0 before the jsr because it helps us while changing nothing to the usual logic
    rom.set_code(0x024ADE, md::Code().jsr(funcChangeItemRewardBoxAddr));

    // Replace 2928C lea (41F9 00FF1040) by a jsr to injected function
    rom.set_code(0x02928C, md::Code().jsr(funcAlterItemGivenByGroundSourceAddr));

    // Replace 1963C - 19644 (0400 00C0 ; 1340 0036) by a jsr to a replacement function
    rom.set_code(0x01963C, md::Code().jsr(funcAlterItemVisibleForGroundSourceAddr).nop());
}


///////////////////////////////////////////////////////////////////////////////////
//       Item check changes
///////////////////////////////////////////////////////////////////////////////////



void addJewelsCheckForTeleporterToKazalt(md::ROM& rom, const RandomizerOptions& options)
{
    // ----------- Rejection textbox handling ------------
    md::Code funcRejectKazaltTeleporter;

    funcRejectKazaltTeleporter.jsr(0x22EE8); // open textbox
    funcRejectKazaltTeleporter.movew(0x22, reg_D0);
    funcRejectKazaltTeleporter.jsr(0x28FD8); // display text 
    funcRejectKazaltTeleporter.jsr(0x22EA0); // close textbox
    funcRejectKazaltTeleporter.rts();

    uint32_t addrRejectKazaltTeleport = rom.inject_code(funcRejectKazaltTeleporter);

    // ----------- Jewel checks handling ------------
    md::Code procHandleJewelsCheck;

    if(options.jewel_count() > MAX_INDIVIDUAL_JEWELS)
    {
        procHandleJewelsCheck.movem_to_stack({reg_D1},{});
        procHandleJewelsCheck.moveb(addr_(0xFF1054), reg_D1);
        procHandleJewelsCheck.andib(0x0F, reg_D1);
        procHandleJewelsCheck.cmpib(options.jewel_count(), reg_D1); // Test if red jewel is owned
        procHandleJewelsCheck.movem_from_stack({reg_D1},{});
        procHandleJewelsCheck.bgt(3);
            procHandleJewelsCheck.jsr(addrRejectKazaltTeleport);
            procHandleJewelsCheck.rts();
    }
    else
    {
        if(options.jewel_count() >= 1)
        {
            procHandleJewelsCheck.btst(0x1, addr_(0xFF1054)); // Test if red jewel is owned
            procHandleJewelsCheck.bne(3);
                procHandleJewelsCheck.jsr(addrRejectKazaltTeleport);
                procHandleJewelsCheck.rts();
        }
        if(options.jewel_count() >= 2)
        {
            procHandleJewelsCheck.btst(0x1, addr_(0xFF1055)); // Test if purple jewel is owned
            procHandleJewelsCheck.bne(3);
                procHandleJewelsCheck.jsr(addrRejectKazaltTeleport);
                procHandleJewelsCheck.rts();
        }
        if(options.jewel_count() >= 3)
        {
            procHandleJewelsCheck.btst(0x1, addr_(0xFF105A)); // Test if green jewel is owned
            procHandleJewelsCheck.bne(3);
                procHandleJewelsCheck.jsr(addrRejectKazaltTeleport);
                procHandleJewelsCheck.rts();
        }
        if(options.jewel_count() >= 4)
        {
            procHandleJewelsCheck.btst(0x5, addr_(0xFF1050)); // Test if blue jewel is owned
            procHandleJewelsCheck.bne(3);
                procHandleJewelsCheck.jsr(addrRejectKazaltTeleport);
                procHandleJewelsCheck.rts();
        }
        if(options.jewel_count() >= 5)
        {
            procHandleJewelsCheck.btst(0x1, addr_(0xFF1051)); // Test if yellow jewel is owned
            procHandleJewelsCheck.bne(3);
                procHandleJewelsCheck.jsr(addrRejectKazaltTeleport);
                procHandleJewelsCheck.rts();
        }
    }
    procHandleJewelsCheck.moveq(0x7, reg_D0);
    procHandleJewelsCheck.jsr(0xE110);  // "func_teleport_kazalt"
    procHandleJewelsCheck.jmp(0x62FA);

    uint32_t procHandleJewelsAddr = rom.inject_code(procHandleJewelsCheck);

    // This adds the purple & red jewel as a requirement for the Kazalt teleporter to work correctly
    rom.set_code(0x62F4, md::Code().jmp(procHandleJewelsAddr));
}

void makeSwordOfGaiaWorkInVolcano(md::ROM& rom)
{
    // Add the ability to also trigger the volcano using the Sword of Gaia instead of only Statue of Gaia
    md::Code procTriggerVolcano;

    procTriggerVolcano.cmpiw(0x20A, addr_(0xFF1204));
    procTriggerVolcano.bne(4);
        procTriggerVolcano.bset(0x2, addr_(0xFF1027));
        procTriggerVolcano.jsr(0x16712);
        procTriggerVolcano.rts();
    procTriggerVolcano.jmp(0x16128);

    uint32_t procAddr = rom.inject_code(procTriggerVolcano);

    rom.set_code(0x1611E, md::Code().jmp(procAddr));
}

void fixReverseGreenmazeFountainSoftlock(md::ROM& rom)
{
    // Pressing the button behind the locked door now triggers the flag "Visited Greenmaze Crossroads map"
    rom.set_byte(0x9C66C, 0xF1);
    rom.set_byte(0x9C66D, 0x00);

    // Change the flag checked for lowering fountain to "Visited Greenmaze Crossroads map"
    rom.set_byte(0x500C, 0xF1);
    rom.set_byte(0x500D, 0x08);
}


void addDoorForReverseSafetyPass(md::ROM& rom)
{
    // Alter map variants so that we are in map 0x27A while safety pass is not owned
    rom.set_word(0xA50E, 0x5905); // Byte 1050 bit 5 is required to trigger map variant
    rom.set_word(0xA514, 0x5905); // Byte 1050 bit 5 is required to trigger map variant

    // Modify entities in map variant 0x27A to replace two NPCs by a door blocking the way out of Mercator
    rom.set_bytes(0x2153A, { 0x70, 0x2A, 0x02, 0x00, 0x00, 0x67, 0x01, 0x00, 0x70, 0x2C, 0x02, 0x00, 0x00, 0x67, 0x01, 0x00 });
}

void makeRyumaMayorSaveable(md::ROM& rom)
{
    // --------------- Fixes for thieves hideout treasure room ---------------
    // Disable the cutscene when opening vanilla lithograph chest
    rom.set_code(0x136BE, md::Code().rts());

    // Disable Friday blocker in the treasure room by removing last entity from the map
    rom.set_word(0x9BA62, 0xFEFE); // Why does that even work? I don't know...

    // Set the "Mayor freed" flag to byte 10D6, bit 4 (which happens to be the "Visited treasure room with mayor variant" flag)
    rom.set_word(0xA3F4, 0xD604);

    // Remove the "remove all NPCs on flag set" trigger in treasure room by putting it to an impossible flag
    rom.set_byte(0x1A9C0, 0x01);

    // Inject custom code "on map enter" to set the flag when it is convenient to do so
    md::Code onMapEnterFunction;
    onMapEnterFunction.lea(0xFF10C0, reg_A0); // Do the instruction that was replaced the hook call
    // When entering the cavern where we save Ryuma's mayor, set the flag "mayor is saved"
    onMapEnterFunction.cmpib(0xE0, reg_D0);
    onMapEnterFunction.bne(2);
        onMapEnterFunction.bset(0x01, addr_(0xFF1004));
    onMapEnterFunction.rts();

    uint32_t onMapEnterFunctionAddr = rom.inject_code(onMapEnterFunction);
    rom.set_code(0x2952, md::Code().jsr(onMapEnterFunctionAddr));

    // --------------- Fixes for Ryuma's mayor reward ---------------
    // Change the second reward from "fixed 100 golds" to "item with ID located at 0x2837F"
    rom.set_byte(0x2837E, 0x00);
    rom.set_word(0x28380, 0x17E8);

    // Remove the "I think we need an exorcism" dialogue for the mayor when progression flags are much further in the game
    rom.set_word(0x2648E, 0xF908); // CheckFlagAndDisplayMessage
    rom.set_word(0x26490, 0x0023); // on bit 3 of flag FF1004
    rom.set_word(0x26492, 0x1801); // Script ID for post-reward dialogue
    rom.set_word(0x26494, 0x17FF); // Script ID for reward cutscene
    rom.set_code(0x26496, md::Code().rts());
    // Clear out the rest
    rom.set_long(0x26498, 0xFFFFFFFF);

    // --------------- Fixes for other events depending on those flags ---------------
    // Make Ryuma's shop open without saving the mayor
    rom.set_byte(0x1A64D, 0x00);

    // Make Tibor open without saving the mayor
    rom.set_byte(0x501D, 0x10);
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
    rom.set_word(0x005014, 0x0209);
}

void makeFallingRibbonAlwaysThere(md::ROM& rom)
{
    // The "falling ribbon" item source is pretty dependant from the scenario to happen. In the original game,
    // the timeframe to get it is really tight. We try to get rid of any conditions here, apart from checking
    // if item has already been obtained.

    // 0x00A466:
        // Before:  20 05 (bit 5 of flag 1020)
        // After:   3F 07 (bit 7 of flag 103F - never true)
    rom.set_word(0x00A466, 0x3F07);

    // 0x01A724:
        // Before:	16 22 (bit 1 of flag 1016)
        // After:	3F E2 (bit 7 of flag 103F - never true)
    rom.set_word(0x1A724, 0x3FE2);

    // Change falling item position from 1F to 20 to ensure it is taken by Nigel whatever its original position is
    rom.set_byte(0x09C59E, 0x20);

    // Remove the servant guarding the door, setting her position to 00 00
    rom.set_word(0x01BFCA, 0x0000);
}

void alterKingNolesCaveTeleporterCheck(md::ROM& rom)
{
    // Change the flag checked for teleporter appearance from "saw the duke Kazalt cutscene" to "has visited four white golems room in King Nole's Cave"
    // 0x0050A0:
        // Before:	27 09 (bit 1 of flag 1027)
        // After:	D0 09 (bit 1 of flag 10D0)
    rom.set_word(0x0050A0, 0xD009);
    // 0x00509C:
        // Before:	27 11 (bit 1 of flag 1027)
        // After:	D0 11 (bit 1 of flag 10D0)
    rom.set_word(0x00509C, 0xD011);

    // We need to inject a procedure checking "is D0 equal to FF" to replace the "bmi" previously used which was preventing
    // from checking flags above 0x80 (the one we need to check is 0xD0).
    md::Code procImproveFlagCheck;

    procImproveFlagCheck.moveb(addr_(reg_A0, 0x2), reg_D0);     // 1028 0002
    procImproveFlagCheck.cmpib(0xFF, reg_D0);
    procImproveFlagCheck.bne(2);
    procImproveFlagCheck.jmp(0x4E2E);
    procImproveFlagCheck.jmp(0x4E20);

    uint32_t procAddr = rom.inject_code(procImproveFlagCheck);

    // Replace the (move.b, bmi, ext.w) by a jmp to the injected procedure
    rom.set_code(0x004E18, md::Code().clrw(reg_D0).jmp(procAddr));
}

void makeMercatorDocksShopAlwaysOpen(md::ROM& rom)
{
    // 0x01AA26:
        // Before:	0284 2A A2 (in map 284, check bit 5 of flag 102A)
        // After:	0000 5F E2 (in map 0, check bit 7 of flag 105F - never true)
    rom.set_word(0x01AA26, 0x0000);
    rom.set_word(0x01AA28, 0x5FE2);
}

void makeArthurAlwaysPresentInThroneRoom(md::ROM& rom)
{
    // Change the Arthur check giving casino tickets for him to be always here, instead of only after Lake Shrine
    // 0x01A904: 
        // Before:  AAA0 (bit 5 of flag 2A, affecting entity 0) 
        // After:   8000 (bit 0 of flag 00, affecting entity 0)
    rom.set_word(0x01A904, 0x8000);

    // 0x01A908:
        // Before:	AAA1 (bit 5 of flag 2A, affecting entity 1) 
        // After:	8001 (bit 0 of flag 00, affecting entity 1)
    rom.set_word(0x01A908, 0x8001);
}

void fixCryptBehavior(md::ROM& rom)
{
    // 1) Remove the check "if shadow mummy was beaten, raft mummy never appears again"
    // 0x019DF6:
        // Before:	0839 0006 00FF1014 (btst bit 6 in FF1014) ; 66 14 (bne $19E14)
        // After:	4EB9 00019E14 (jsr $19E14; 4E71 4E71 (nop nop)
    rom.set_code(0x19DF6, md::Code().nop(5));

    // 2) Change the room exit check and shadow mummy appearance from "if armlet is owned" to "chest was opened"
    // 0x0117E8:
        // Before:	103C 001F ; 4EB9 00022ED0 ; 4A41 ; 6B00 F75C (bmi $10F52)
        // After:	0839 0002 00FF1097 (btst 2 FF1097)	; 6700 F75C (bne $10F52)
    md::Code injectChangeCryptExitCheck;
    injectChangeCryptExitCheck.btst(0x2, addr_(0xFF1097));
    injectChangeCryptExitCheck.nop(2);
    injectChangeCryptExitCheck.beq(); // beq $10F52
    rom.set_code(0x117E8, injectChangeCryptExitCheck);
}

void fixKingNolesLabyrinthRafts(md::ROM& rom)
{
    // Change the rafts logic so we can take them several times in a row, preventing from getting softlocked by missing chests
    // The trick here is to use flag 1001 (which resets on every map change) to correctly end the cutscene while discarding the "raft already taken" state 
    // as early as the player moves to another map.
    rom.set_word(0x09E031, 0x0100);
    rom.set_word(0x09E034, 0x0100);
    rom.set_word(0x09E04E, 0x0100);
    rom.set_word(0x09E051, 0x0100);
}

void removeLogsRoomExitCheck(md::ROM& rom)
{
    // Usually, when trying to leave the room where you get Logs in the vanilla game without having taken both logs, a dwarf
    // comes and prevents you from leaving. Here, we remove that check since we cannot softlock anymore on the raft.
    rom.set_code(0x011EC4, md::Code().bra());
}

void fixMirFightAfterLakeShrine(md::ROM& rom)
{
    // In the original game, coming back to Mir room after Lake Shrine would softlock you because Mir
    // would not be there. This check is removed to prevent any softlock and allow fighting Mir after having
    // done Lake Shrine.

    // 0x01AA22:
        // Before:	0310 2A A2 (in map 310, check bit 5 of flag 102A)
        // After:	0000 5F E2 (in map 0, check bit 7 of flag 105F - never true)
    rom.set_word(0x01AA22, 0x0000);
    rom.set_word(0x01AA24, 0x5FE2);
}

void fixMirTowerPriestRoomItems(md::ROM& rom)
{
    // Remove the "shop/church" flag on the priest room of Mir Tower to make its items on ground work everytime
    // 0x024E5A:
        // Before:	0307
        // After:	7F7F
    rom.set_word(0x024E5A, 0x7F7F);
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
    rom.set_word(0x0215A6, 0x0000);
}

void removeSailorInDarkPort(md::ROM& rom)
{
    // There is a sailor NPC in the "dark" version of Mercator port who responds badly to story triggers, allowing us to sail to Verla
    // even without having repaired the lighthouse. To prevent this from being exploited, we removed him altogether.

    // 0x021646:
        // Before:	23 EC
        // After:	00 00
    rom.set_word(0x021646, 0x0000);
}

void fixFaraLifestockChest(md::ROM& rom)
{
    // Make it so Lifestock chest near Fara in Swamp Shrine appears again when going back into the room afterwards, preventing any softlock there.

    // --------- Function to remove all entities but the chest when coming back in the room ---------
    md::Code funcRemoveAllEntitiesButChestInFaraRoom;

    funcRemoveAllEntitiesButChestInFaraRoom.movem_to_stack({ reg_D0_D7 }, { reg_A0_A6 });
    funcRemoveAllEntitiesButChestInFaraRoom.lea(0xFF5480, reg_A0);
    funcRemoveAllEntitiesButChestInFaraRoom.moveq(0xD, reg_D0);
    funcRemoveAllEntitiesButChestInFaraRoom.label("loop_remove_entities");
    funcRemoveAllEntitiesButChestInFaraRoom.movew(0x7F7F, addr_(reg_A0));
    funcRemoveAllEntitiesButChestInFaraRoom.adda(0x80, reg_A0);
    funcRemoveAllEntitiesButChestInFaraRoom.dbra(reg_D0, "loop_remove_entities");
    funcRemoveAllEntitiesButChestInFaraRoom.movem_from_stack({ reg_D0_D7 }, { reg_A0_A6 });
    funcRemoveAllEntitiesButChestInFaraRoom.rts();

    uint32_t funcAddr = rom.inject_code(funcRemoveAllEntitiesButChestInFaraRoom);

    // Call the injected function
    rom.set_code(0x019BE0, md::Code().jsr(funcAddr).nop());

    // --------- Moving the chest to the ground ---------
    rom.set_word(0x01BF6C, 0x1A93);
    rom.set_word(0x01BF6E, 0x0000);
    rom.set_word(0x01BF70, 0x0012);
    rom.set_word(0x01BF72, 0x0400);
}



void replaceSickMerchantByChest(md::ROM& rom)
{
    // Neutralize map variant triggers for both the shop and the backroom to remove the "sidequest complete" check.
    // Either we forced them to be always true or always false
    rom.set_word(0x0050B4, 0x0008);  // Before: 0x2A0C (bit 4 of 102A) | After: 0x0008 (bit 0 of 1000 - always true)
    rom.set_word(0x00A568, 0x3F07);	// Before: 0x2A04 (bit 4 of 102A) | After: 0x3F07 (bit 7 of 103F - always false)
    rom.set_word(0x00A56E, 0x3F07);	// Before: 0x2A03 (bit 3 of 102A) | After: 0x3F07 (bit 7 of 103F - always false)
    rom.set_word(0x01A6F8, 0x3FE0);	// Before: 0x2A80 (bit 4 of 102A) | After: 0x3FE0 (bit 7 of 103F - always false)

    // Set the index for added chest in map to "0E" instead of "C2"
    rom.set_byte(0x09EA48, 0x0E);

    // Transform the sick merchant into a chest
    rom.set_word(0x021D0E, 0xCE92);  // First word: position, orientation and palette (CE16 => CE92)
    rom.set_word(0x021D10, 0x0000);  // Second word: ??? (3000 => 0000)
    rom.set_word(0x021D12, 0x0012);  // Third word: type (006D = sick merchant NPC => 0012 = chest)
//	rom.set_word(0x021D14, 0x0000); 

    // Move the kid to hide the fact that the bed looks broken af
    rom.set_word(0x021D16, 0x5055);
}


///////////////////////////////////////////////////////////////////////////////////
//       Original game bugs & glitches fixes
///////////////////////////////////////////////////////////////////////////////////

void fix_armlet_skip(md::ROM& rom)
{
    // Fix armlet skip by putting the tornado way higher, preventing any kind of buffer-jumping on it
    // 0x02030C:
        // Before:  0x82 (Tornado pos Y = 20)
        // After:   0x85 (Tornado pos Y = 50)
    rom.set_byte(0x02030C, 0x85);
}

void fix_tree_cutting_glitch(md::ROM& rom)
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

    uint32_t funcAddr = rom.inject_code(funcFixTreeCuttingGlitch);

    // Call the injected function when killing an enemy
    rom.set_code(0x01625C, md::Code().jsr(funcAddr));
}



///////////////////////////////////////////////////////////////////////////////////
//       Miscellaneous
///////////////////////////////////////////////////////////////////////////////////

void deactivateRegionCheck(md::ROM& rom)
{
    // Before : jsr $A0A0C | After : nop nop nop
    rom.set_code(0x506, md::Code().nop(3));
}

void changeHUDColor(md::ROM& rom, const RandomizerOptions& options)
{
    // 0x824 is the default purple color from the original game
    uint16_t color = 0x0824;

    std::string hudColor = options.hud_color();
    tools::to_lower(hudColor);

    if (hudColor == "red")              color = 0x228;
    else if (hudColor == "darkred")     color = 0x226;
    else if (hudColor == "green")       color = 0x262;
    else if (hudColor == "blue")        color = 0x842;
    else if (hudColor == "brown")       color = 0x248;
    else if (hudColor == "darkpurple")  color = 0x424;
    else if (hudColor == "darkgray")    color = 0x222;
    else if (hudColor == "gray")        color = 0x444;
    else if (hudColor == "lightgray")   color = 0x666;

    rom.set_word(0xF6D0, color);
    rom.set_word(0xFB36, color);
    rom.set_word(0x903C, color);
//    rom.set_word(0x9020, color);
}


void alterGoldRewardsHandling(md::ROM& rom)
{
    // In the original game, only 3 item IDs are reserved for gold rewards (3A, 3B, 3C)
    // Here, we moved the table of gold rewards to the end of the ROM so that we can handle 64 rewards up to 255 golds each.
    // In the new system, all item IDs after the "empty item" one (0x40 and above) are now gold rewards.
    
    rom.set_byte(0x0070DF, ITEM_GOLDS_START); // cmpi 3A, D0 >>> cmpi 40, D0
    rom.set_byte(0x0070E5, ITEM_GOLDS_START); // subi 3A, D0 >>> subi 40, D0

    // ------------- Function to put gold reward value in D0 ----------------
    // Input: D0 = gold reward ID (offset from 0x40)
    // Output: D0 = gold reward value

    md::Code funcGetGoldReward;

    funcGetGoldReward.movem_to_stack({}, { reg_A0 });
    funcGetGoldReward.lea(rom.stored_address("data_gold_values"), reg_A0);
    funcGetGoldReward.moveb(addr_(reg_A0, reg_D0, md::Size::WORD), reg_D0);  // move.b (A0, D0.w), D0 : 1030 0000
    funcGetGoldReward.movem_from_stack({}, { reg_A0 });
    funcGetGoldReward.rts();

    uint32_t funcAddr = rom.inject_code(funcGetGoldReward);

    // Set the call to the injected function
    // Before:      add D0,D0   ;   move.w (PC, D0, 42), D0
    // After:       jsr to injected function
    rom.set_code(0x0070E8, md::Code().jsr(funcAddr));
}

void shortenMirCutsceneAfterLakeShrine(md::ROM& rom)
{
    // Cut the cutscene script
    rom.set_word(0x28A44, 0xE739);
}

void renameItems(md::ROM& rom, const RandomizerOptions& options)
{
    std::vector<uint8_t> itemNameBytes;
    rom.data_chunk(0x29732, 0x29A0A, itemNameBytes);
    std::vector<std::vector<uint8_t>> itemNames;

    // "Kazalt Jewel" mode is a specific mode when user asked for more jewels than we can provide individual items for.
    // In that case, we only use one generic jewel item type which can be obtained several times, and check against this
    // item's count instead of checking if every jewel type is owned at Kazalt teleporter
    bool kazaltJewelMode = (options.jewel_count() > MAX_INDIVIDUAL_JEWELS);

    // Read item names
    uint32_t addr = 0;
    while(true)
    {
        uint16_t stringSize = itemNameBytes[addr++];
        if(stringSize == 0xFF)
            break;

        // Clear Island Map name to make room for other names
        if(itemNames.size() == ITEM_ISLAND_MAP)
            itemNames.push_back(std::vector<uint8_t>({ 0x00 }));
        // Rename all default equipments with "None"
        else if(itemNames.size() == ITEM_NO_SWORD || itemNames.size() == ITEM_NO_ARMOR || itemNames.size() == ITEM_NO_BOOTS)
            itemNames.push_back({ 0x18, 0x33, 0x32, 0x29 });
        // Rename No52 into Green Jewel
        else if(itemNames.size() == ITEM_GREEN_JEWEL && !kazaltJewelMode)
            itemNames.push_back({ 0x11, 0x36, 0x29, 0x29, 0x32, 0x6A, 0x14, 0x29, 0x3B, 0x29, 0x30 });
        // Rename Detox Book into Blue Jewel
        else if(itemNames.size() == ITEM_BLUE_JEWEL && !kazaltJewelMode)
            itemNames.push_back({ 0x0C, 0x30, 0x39, 0x29, 0x6A, 0x14, 0x29, 0x3B, 0x29, 0x30 });
        // Rename AntiCurse Book into Yellow Jewel
        else if(itemNames.size() == ITEM_YELLOW_JEWEL && !kazaltJewelMode)
            itemNames.push_back({ 0x23, 0x29, 0x30, 0x30, 0x33, 0x3B, 0x6A, 0x14, 0x29, 0x3B, 0x29, 0x30 });
        // Clear "Purple Jewel" name to make room for other names since it's unused in Kazalt Jewel mode
        else if(itemNames.size() == ITEM_PURPLE_JEWEL && kazaltJewelMode)
            itemNames.push_back(std::vector<uint8_t>({ 0x00 }));
        // Rename "Red Jewel" into the more generic "Kazalt Jewel" in Kazalt Jewel mode
        else if(itemNames.size() == ITEM_RED_JEWEL && kazaltJewelMode)
            itemNames.push_back(std::vector<uint8_t>({ 0x15, 0x25, 0x3E, 0x25, 0x30, 0x38, 0x6A, 0x14, 0x29, 0x3B, 0x29, 0x30 }));
        // No specific treatment, just add it back as-is
        else
            itemNames.push_back(std::vector<uint8_t>(itemNameBytes.begin() + addr, itemNameBytes.begin() + addr + stringSize));

        addr += stringSize;
    }

    constexpr uint16_t initialSize = 0x29A0A - 0x29732;

    itemNameBytes.clear();
    for(const std::vector<uint8_t>& itemName : itemNames)
    {
        itemNameBytes.push_back((uint8_t)itemName.size());
        itemNameBytes.insert(itemNameBytes.end(), itemName.begin(), itemName.end());
    }
    itemNameBytes.push_back(0xFF);

    if(itemNameBytes.size() > initialSize)
        throw new RandomizerException("Item names size is above initial game size");
    rom.set_bytes(0x29732, itemNameBytes);
}




void alterCredits(md::ROM& rom)
{
    constexpr uint32_t creditsTextBaseAddr = 0x9ED1A;

    // Change "LANDSTALKER" to "RANDSTALKER"
    rom.set_byte(creditsTextBaseAddr + 0x2, 0x13);

    rom.set_bytes(creditsTextBaseAddr + 0x14, { 
        0x27, 0x1C, 0x29, 0x1F, 0x2E, 0x2F, 0x1C, 0x27, 0x26, 0x20, 0x2D, 0x80, // "landstalker "
        0x2D, 0x1C, 0x29, 0x1F, 0x2A, 0x28, 0x24, 0x35, 0x20, 0x2D              // "randomizer"
    });

    // Widen the space between the end of the cast and the beginning of the rando staff
    rom.set_byte(creditsTextBaseAddr + 0x5C, 0x0F);

    rom.set_bytes(creditsTextBaseAddr + 0x75, { 
        // RANDOMIZER
        0x08, 0xFF, 0x82, 
        0x80, 0x13, 0x80, 0x02, 0x80, 0x0F, 0x80, 0x05, 0x80, 0x10, 0x80, 0x0E, 0x80, 0x0A, 0x80, 0x1B, 0x80, 0x06, 0x80, 0x13, 0x80, 0x00, 
        // developed by
        0x04, 0xFF, 0x81, 
        0x1F, 0x20, 0x31, 0x20, 0x27, 0x2A, 0x2B, 0x20, 0x1F, 0x80, 0x1D, 0x34, 0x00,
        // DINOPONY
        0x03, 0xFF,
        0x05, 0x0A, 0x0F, 0x10, 0x11, 0x10, 0x0F, 0x1A, 0x00,
        // WIZ
        0x08, 0xFF,
        0x18, 0x0A, 0x1B, 0x00,
        // testing
        0x04, 0xFF, 0x81,
        0x2F, 0x20, 0x2E, 0x2F, 0x24, 0x29, 0x22, 0x00,
        // SAGAZ
        0x03, 0xFF,
        0x14, 0x02, 0x08, 0x02, 0x1B, 0x00,
        // STIK
        0x03, 0xFF,
        0x14, 0x15, 0x0A, 0x0C, 0x00,
        // WIZ
        0x03, 0xFF,
        0x18, 0x0A, 0x1B, 0x00,
        // LANDRYLE
        0x08, 0xFF,
        0x0D, 0x02, 0x0F, 0x05, 0x13, 0x1A, 0x0D, 0x06, 0x00,
        // website by
        0x04, 0xFF, 0x81,
        0x32, 0x20, 0x1D, 0x2E, 0x24, 0x2F, 0x20, 0x80, 0x1D, 0x34, 0x00,
        // DILANDAU
        0x08, 0xFF,
        0x05, 0x0A, 0x0D, 0x02, 0x0F, 0x05, 0x02, 0x16, 0x00
    });

    std::vector<uint8_t> rest;
    rom.data_chunk(creditsTextBaseAddr + 0x1D2, creditsTextBaseAddr + 0x929, rest);
    rom.set_bytes(creditsTextBaseAddr + 0xF5, rest);
    for(uint32_t addr = creditsTextBaseAddr + 0xF5 + (uint32_t)rest.size() ; addr <= creditsTextBaseAddr + 0x929 ; ++addr)
        rom.set_byte(addr, 0x00);
}

void apply_other_patches(md::ROM& rom, const RandomizerOptions& options, const World& world)
{
    // Item source behavior changes
    alterGoldRewardsHandling(rom);
    alterLifestockHandlingInShops(rom);
    if (options.use_armor_upgrades())
        handleArmorUpgrades(rom);
    fixFaraLifestockChest(rom);
    alter_fahl_challenge(rom, world);

    // Specific map check changes
    alterWaterfallShrineSecretStairsCheck(rom);
    makeFallingRibbonAlwaysThere(rom);
    alterKingNolesCaveTeleporterCheck(rom);
    makeMercatorDocksShopAlwaysOpen(rom);
    makeArthurAlwaysPresentInThroneRoom(rom);
    fixCryptBehavior(rom);
    fixKingNolesLabyrinthRafts(rom);
    removeLogsRoomExitCheck(rom);
    fixMirFightAfterLakeShrine(rom);
    fixMirTowerPriestRoomItems(rom);
    makeSwordOfGaiaWorkInVolcano(rom);
    fixReverseGreenmazeFountainSoftlock(rom);
    addJewelsCheckForTeleporterToKazalt(rom, options);
    makeRyumaMayorSaveable(rom);

    // Map content changes
    replaceSickMerchantByChest(rom);
    removeMercatorCastleBackdoorGuard(rom);
    removeSailorInDarkPort(rom);
    addDoorForReverseSafetyPass(rom);

    // Fix original game glitches & bugs
    if(options.fix_armlet_skip())
        fix_armlet_skip(rom);
    if(options.fix_tree_cutting_glitch())
        fix_tree_cutting_glitch(rom);

    // UI changes
    alterItemOrderInMenu(rom);
    renameItems(rom, options);
    changeHUDColor(rom, options);

    // Miscellaneous
    deactivateRegionCheck(rom);
    alterCredits(rom);
    shortenMirCutsceneAfterLakeShrine(rom);
}
