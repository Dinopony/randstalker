#pragma once

#include <map>
#include <string>
#include "GameROM.h"

void alterROM(GameROM& rom, const std::map<std::string, std::string>& options);