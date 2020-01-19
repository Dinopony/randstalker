![](https://github.com/Dilandau/randstalker/workflows/Compile/badge.svg)

# Randstalker

Randstalker is a randomizer for the famous Megadrive / Genesis classic "Landstalker : The Treasure of King Nole".
It works on a US ROM of the game by randomizing item sources and altering the game so that it is more enjoyable in a randomizer format.

## Notable differences compared to original game

- Game starts at Massan Inn and intro is skipped
- Most cutscenes and scenario related checks / blockers are removed
- All key doors are gone, except one (the Mercator castle backdoor), same goes for the keys. The key becomes a unique item to find which gives you access to Greenmaze sector.
- You can go to the Mercator Casino freely without any story trigger and without Casino Ticket
- The "secret" stairs at the entrance of Waterfall Shrine now appears with Prospero saying "What a noisy boy!" as you approach him at the end of the level (no need to talk to him anymore).
- You can go back to the Crypt after having done it a first time 
- The secondary shop of Mercator requiring to do the Traders sidequest in the original game is now unlocked solely by having Buyer Card in your inventory

## Usage

Either launch Randstalker.exe and follow the instructions, or execute it through the command line with the following syntax:

`Randstalker.exe [input_filename] [seed] [output_filename] [output_log]`
