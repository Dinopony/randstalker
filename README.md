![](https://github.com/Dinopony/randstalker/workflows/Compile/badge.svg)

# Randstalker

Randstalker is a randomizer for the famous Megadrive / Genesis classic "Landstalker : The Treasure of King Nole".
It works on a US ROM of the game by randomizing item sources and altering the game so that it is more enjoyable in a randomizer format.

## Notable differences compared to original game

- Game starts at Massan Inn and intro is skipped
- Most cutscenes and scenario related checks / blockers are removed
- All key doors are gone, except one (the Mercator castle backdoor), same goes for the keys. The key becomes a unique item to find which gives you access to Greenmaze sector.
- You can go to the Mercator Casino freely without any story trigger and without Casino Ticket
- You can go back to the Crypt after having done it a first time 
- The "secret" stairs at the entrance of Waterfall Shrine now appears with Prospero saying "What a noisy boy!" as you approach him at the end of the level (no need to talk to him anymore).
- The secondary shop of Mercator requiring to do the traders sidequest in the original game is now unlocked solely by having Buyer Card in your inventory
- Some reward-giving NPCs (such as sick merchant in Mercator or lumberjack in Greenmaze) are replaced by chests because of story triggers sometimes preventing them from giving their items
- Some reward-giving NPCs are not handled at all because it would be too complex to do so right now (Massan elder reward after saving Fara and Ryuma's mayor)
- Tibor trees connections are randomized (will be available as an option later on)

## Usage

You can use the online generator provided by Dilandau (thanks to him!) at this address: https://randstalker.ovh/

The online generator always uses the most up-to-date version of the tool.

You can also use the executable version on your own device.
To do so, either launch randstalker.exe and follow the instructions, or execute it through the command line with the following syntax:

`randstalker [options]`

With the following options being available:

- inputRom: set the path to the ROM used as input (e.g. `--inputRom="myROM.md"`)
- outputRom: set the path where the randomized ROM will be outputted (e.g. `--outputROM="rando.md"`)
- seed: set the seed used for the randomizer. It must be an integer or the string "random" if you want it to be random (e.g. `--seed=2134548740`)
- outputLog: set the path where the spoiler log will be outputted (e.g. `--outputLog=spoiler.log`)
- noPause: if set, no "press any key" prompt will be put at the end of generation, especially useful for automated generators (e.g. `--noPause`)

It also provides a few randomization options:

- shuffleTrees: if set, Tibor trees connections are shuffled (e.g. `--shuffleTrees`)