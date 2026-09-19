// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <filesystem>

namespace sw {
std::filesystem::path startupStageFile();
// Serializes launch selection across processes. An unavailable state directory
// falls back to random selection so a read-only home never prevents startup.
int chooseStartupStage(std::uint64_t seed,const std::filesystem::path& file);
}
