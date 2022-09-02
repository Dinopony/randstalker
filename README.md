![version](https://img.shields.io/badge/Version-1.8.0-blue)
![compile](https://github.com/Dinopony/randstalker/workflows/Compile/badge.svg)
<a href="https://discord.gg/XNA76xc9sU">
  <img src="https://img.shields.io/badge/-Discord-lightgrey?logo=discord" alt="Join Discord">
</a>

# Randstalker: The Landstalker Randomizer

Randstalker is a randomizer for the famous Megadrive / Genesis classic "Landstalker : The Treasure of King Nole". It
works on a US ROM of the game by randomizing item sources and altering the game to make it more enjoyable in a
randomizer format.

## Notable differences compared to original game

### Game flow

- Game starts at a given spawn location (usually randomized) and intro is fully skipped.
- Most cutscenes and scenario related checks / blockers are removed to put the focus on gameplay & exploration
- You will need as many jewels as specified in the settings to use the teleporter to go to Kazalt
- All other conditions from the original game stay as-is (Safety Pass to reach Mercator, Gola's Eye to reach King Nole's
  Cave, Axe Magic to cut trees, all three Gola pieces to reach King Nole's fight...)
- The secondary shop of Mercator requiring to do the traders sidequest in the original game is now unlocked by having
  Buyer Card in your inventory
- The **Key** is now a unique item and can open several doors without being consumed. All key doors are gone, except
  three of them :
    * the Mercator castle backdoor (giving access to Greenmaze sector)
    * Thieves Hideout middle door (cutting the level in half)
    * King Nole's Labyrinth door near entrance

### New item features

- On default settings, you start with two unusual items:
    - a **Record Book** allowing you to save anywhere and pontentially save time by strategically save-scumming
    - a **Spell Book** allowing you to warp back to your spawn point
- You will also be able to find the **Statue of Jypta** which is reintroduced as a real item giving gold passively as
  you walk. Free riches!

### Hints

- Hints are provided by Foxy, an ubiquitous wizard fox that can appear in more than a hundred of spots. Keep your eyes
  wide open to find it wherever it appears (full list with screenshots [there](https://imgur.com/a/FnN7Akx))
- Using the Lithograph will give you hints where to find both jewels
- Using the Oracle Stone will give you a hint on an important item
- The Fortune Teller in Mercator will give you a hint leading to one of the Gola items

### Fixes

- You won't be able to go to Mir Tower without the Armlet by doing the Armlet skip since it has been fixed. Sorry!
- All known places where you usually can't go back and could potentially softlock now stay opened to be able to come
  back afterwards (e.g. Crypt, King Nole's Labyrinth raft sector, Swamp Shrine boss reward chest...)

### Misc details

- Depressed shopkeeper in Mercator (giving Buyer's Card in vanilla game) is replaced by a chest in the shop backroom,
  since traders' sidequest is only available in a very limited timeframe in the game
- By default, armors work as "armor upgrades", meaning you will always get them one tier after another (this can be
  disabled as an option)
- The teleporter between Mercator and King Nole's Cave appears as soon as you enter the room with white golems right
  before it. You can go straight to Kazalt without worrying about it.
- Tibor trees connections can be optionally randomized

## Usage

### Release versions

Using the provided release packages (on the right panel of this webpage) is really straightforward since everything
(excepted the original ROM) is included for you to start randomizing.

You just have to put your copy of the original US game ROM inside the folder named `ìnput.md`, and use the 
`gen_preset.bat` script on Windows (Linux users are expected to know how to use CLI).

### Building from source

If you want to get the very last version, you can also compile and use the executable version on your own device. You
will need CMake to do so, and then do:

- On Windows, with Visual Studio 2019 installed, you can execute `build_win_vs2019.bat`
- On Unix, you can execute `build_unix.sh`

You will then need to execute the compiled binary it through the command line with the following options being
available:

- **--permalink=PERMALINK**: use the given permalink to populate all other settings. Permalinks are given at then end of
  a generation and allow for other generations of the exact same seed.
- **--inputRom=PATH**: set the path to the ROM used as input (e.g. `--inputRom="myROM.md"`)
- **--outputRom=PATH**: set the path where the randomized ROM will be outputted (e.g. `--outputROM="./seeds/"`)
- **--outputLog=PATH**: set the path where the spoiler log will be outputted (e.g. `--outputLog=spoiler.log`)
- **--preset=PATH**: set the path where the JSON preset file containing settings is located (
  e.g. `--preset=preset.json`)
- **--noPause**: if set, no "press any key" prompt will be put at the end of generation, especially useful for automated
  generators (e.g. `--noPause`)
- **--ingameTracker**: adds greyed out key items in inventory to have a working in-game item tracker
