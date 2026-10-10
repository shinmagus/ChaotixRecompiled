#include "platform/pad_press_filter.h"

#include <cstdlib>

namespace chaotix {

bool menu_takes_pad(const std::string& player_one_device, const std::vector<SDL_JoystickID>& pads,
                    SDL_JoystickID which) {
    if (player_one_device.rfind("pad", 0) != 0) {
        return true;
    }
    const long number = std::strtol(player_one_device.c_str() + 3, nullptr, 10);
    if (number < 1 || size_t(number) > pads.size()) {
        return true;
    }
    return pads[size_t(number - 1)] == which;
}

bool DuplicatePressFilter::take(SDL_JoystickID device, Uint8 button, Uint64 now_ms) {
    if (taken_any && device != taken_device && button == taken_button && now_ms >= taken_ms &&
        now_ms - taken_ms <= kWindowMs) {
        last_gap = now_ms - taken_ms;
        return false;
    }
    taken_any = true;
    taken_device = device;
    taken_button = button;
    taken_ms = now_ms;
    return true;
}

}  // namespace chaotix
