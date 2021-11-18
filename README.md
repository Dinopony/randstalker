![](https://github.com/Dinopony/randstalker/workflows/Compile/badge.svg)

# Randstalker

Randstalker is a randomizer for the famous Megadrive / Genesis classic "Landstalker : The Treasure of King Nole".
It works on a US ROM of the game by randomizing item sources and altering the game so that it is more enjoyable in a randomizer format.

## Notable differences compared to original game

### Game flow

- Game starts at a given spawn location (usually randomized) and intro is fully skipped.
- Most cutscenes and scenario related checks / blockers are removed
- You will need as many jewels as specified in the settings to use the teleporter to go to Kazalt
- All other conditions from the original game stay as-is (Safety Pass to reach Mercator, Gola's Eye to reach King Nole's Cave, Axe Magic to cut trees, all three Gola pieces to reach King Nole's fight...)
- The secondary shop of Mercator requiring to do the traders sidequest in the original game is now unlocked by having Buyer Card in your inventory
- The key is now a unique item and can open several doors without being consumed. All key doors are gone, except two : 
	* the Mercator castle backdoor (giving access to Greenmaze sector) 
	* Thieves Hideout middle door (cutting the level in half)
- Some reward-giving NPCs are replaced by chests for technical reasons:
	* Lumberjack in Greenmaze (Einstein Whistle in OG)
	* Depressed shopkeeper in Mercator (Buyer's Card in OG)
- Tibor trees connections can be optionally randomized

### Hints 

- Road signs which used to give directions now give hints instead
- Using the Lithograph will give you hints where to find both jewels
- Using the Oracle Stone will give you a hint on an important item
- The Fortune Teller in Mercator will give you a hint leading to one of the Gola items

### Fixes

- Original game glitches (tree cutting glitch, armlet skip...) are fixed. Sorry!
- All known places where you usually can't go back and could potentially softlock now stay opened (e.g. Crypt, King Nole's Labyrinth raft sector, Swamp Shrine boss reward chest...)

### New item features

- You start with a Record Book in your inventory allowing you to save anywhere and pontentially save time by strategically save-scumming (this can be disabled as an option)
- Spell Book warps you back to your spawn point when used
- Statue of Jypta is reintroduced as a real item giving gold passively as you walk. Free riches!
- Armors work by default as "armor upgrades", meaning you will always get them one tier after another (this can be disabled as an option)

### Misc details

- The "secret" stairs at the entrance of Waterfall Shrine now appears with Prospero saying "What a noisy boy!" as you approach him at the end of the level (no need to talk to him anymore).
- The teleporter between Mercator and King Nole's Cave appears as soon as you enter the room with white golems right before it. You can go straight to Kazalt without worrying about it.

## Known issues / non-issues

- Dex won't be in his map in Verla Mines for technical reasons. This is not a problem by itself, but can be surprising at first.
- Trees next to Sun Stone cannot be cut using Axe Magic, only by the lumberjack using Einstein Whistle. This is the intended behavior in original game, and it's pretty convenient for the rando logic.

## Usage

You can use the online generator provided by Dilandau (thanks to him!) at this address: https://randstalker.ovh/

The online generator always uses an up-to-date stable version of the tool, but sometimes not the very latest build which might be buggy / unstable.
To get the very last version, you can also compile and use the executable version on your own device.
To do so, either launch randstalker.exe and follow the instructions, or execute it through the command line with the following syntax:

`randstalker [options]`

With the following options being available:

- **--permalink=PERMALINK**: use the given permalink to populate all other settings. Permalinks are given at then end of a generation and allow for other generations of the exact same seed.
- **--inputRom=PATH**: set the path to the ROM used as input (e.g. `--inputRom="myROM.md"`)
- **--outputRom=PATH**: set the path where the randomized ROM will be outputted (e.g. `--outputROM="./seeds/"`)
- **--outputLog=PATH**: set the path where the spoiler log will be outputted (e.g. `--outputLog=spoiler.log`)
- **--preset=PATH**: set the path where the JSON preset file containing settings is located (e.g. `--preset=preset.json`)
- **--noPause**: if set, no "press any key" prompt will be put at the end of generation, especially useful for automated generators (e.g. `--noPause`)
- **--ingameTracker**: adds greyed out key items in inventory to have a working in-game item tracker

You can use the `example_preset.json` located at the root of the repository as a starting point for your own preset files.

## Build instructions

If you want to build the binary from the source code, you will need CMake.

- On Windows, with Visual Studio 2019 installed, you can execute `build_win_vs2019.bat`
- On Unix, you can execute `build_unix.sh`
