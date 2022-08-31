#pragma once

#include "landstalker_lib/patches/game_patch.hpp"

/**
 * This patch gives the possibility to increase the amount of HP & Max HP granted by getting a Life Stock.
 */
class PatchSetLifestocksHealth : public GamePatch
{
private:
    uint8_t _health_per_lifestock;

public:
    PatchSetLifestocksHealth(uint8_t health_per_lifestock) : _health_per_lifestock(health_per_lifestock) {}

    void alter_rom(md::ROM& rom) override
    {
        uint16_t health_per_lifestock_formatted = _health_per_lifestock << 8;
        rom.set_word(0x291E2, health_per_lifestock_formatted);
        rom.set_word(0x291F2, health_per_lifestock_formatted);
        rom.set_word(0x7178, health_per_lifestock_formatted);
        rom.set_word(0x7188, health_per_lifestock_formatted);
    }
};
