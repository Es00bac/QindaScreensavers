// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
namespace saver {
enum class Cue { Hit, Shield, Special, Jump, Pickup, Knockout, Boost, Start };
struct SoundEvent {std::uint64_t serial=0;double time=0;Cue cue=Cue::Hit;int character=0;float pan=0;};
}
