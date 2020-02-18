#pragma once

#include <string>
#include "GameROM.h"
#include "RandomizerOptions.h"

void alterRomBeforeRandomization(GameROM& rom, const RandomizerOptions& options);
void alterRomAfterRandomization(GameROM& rom, const RandomizerOptions& options);