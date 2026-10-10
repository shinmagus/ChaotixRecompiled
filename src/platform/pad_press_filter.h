#pragma once

// A gamepad press on its way to the menu: which gamepads the menu takes, and
// a press that arrives twice.
//
// A controller the system lists twice sends every press twice, from two
// devices a few milliseconds apart: DS4Windows, for one, adds a virtual Xbox
// 360 pad beside the real PS4 pad unless it is told to hide the real one.
// Taken twice, a press moves the menu two rows, flips a setting on and off
// again, or opens and closes the menu. The game itself is not affected the
// same way: it reads the pad chosen under CONTROLS, or every pad OR-ed
// together.

#include <SDL3/SDL.h>

#include <string>
#include <vector>

namespace chaotix {

// Whether the menu takes a press from the gamepad `which`. When CONTROLS >
// 1P DEVICE names a gamepad ("pad1", "pad2"...) that is connected, only that
// one drives the menu. AUTO, KEYBOARD, NONE, or a named pad that is not
// connected (the setting can outlive the pad), leave every gamepad able to.
// `pads` holds the open gamepads' ids in the order the setting numbers them.
bool menu_takes_pad(const std::string& player_one_device, const std::vector<SDL_JoystickID>& pads,
                    SDL_JoystickID which);

// Drops the second copy of a press: the same button from another device
// within kWindowMs of one just taken. A second press on the same device is
// never a copy, however quick.
class DuplicatePressFilter {
public:
    static constexpr Uint64 kWindowMs = 50;
    // True when the press is new and the caller should act on it.
    bool take(SDL_JoystickID device, Uint8 button, Uint64 now_ms);
    // How long after the press it copied the last dropped one came, for a log line.
    Uint64 last_gap_ms() const { return last_gap; }

private:
    bool taken_any = false;
    SDL_JoystickID taken_device = 0;
    Uint8 taken_button = 0;
    Uint64 taken_ms = 0;
    Uint64 last_gap = 0;
};

}  // namespace chaotix
