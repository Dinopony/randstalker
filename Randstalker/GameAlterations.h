#pragma once

namespace md { class ROM; }
class RandomizerOptions;

void alterRomBeforeRandomization(md::ROM& rom, const RandomizerOptions& options);
void alterRomAfterRandomization(md::ROM& rom, const RandomizerOptions& options);