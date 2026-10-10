// A gamepad press on its way to the menu (platform/pad_press_filter.h): which
// gamepads the menu takes, and a press that arrives twice.
#include "platform/pad_press_filter.h"
#include "test_framework.h"
#include <vector>

using namespace chaotix;

TEST(pad_press_filter, the_menu_takes_only_the_pad_player_one_chose) {
    const std::vector<SDL_JoystickID> pads{7, 9};
    CHECK(menu_takes_pad("pad1", pads, 7));
    CHECK(!menu_takes_pad("pad1", pads, 9));
    CHECK(menu_takes_pad("pad2", pads, 9));
    CHECK(!menu_takes_pad("pad2", pads, 7));
}

TEST(pad_press_filter, every_pad_drives_the_menu_unless_one_is_chosen_and_connected) {
    const std::vector<SDL_JoystickID> pads{7, 9};
    for (const char* device : {"auto", "keyboard", "none"}) {
        CHECK(menu_takes_pad(device, pads, 7));
        CHECK(menu_takes_pad(device, pads, 9));
    }
    // A chosen pad that is not connected (the setting outlived it) must not
    // lock the menu to a pad that is not there.
    CHECK(menu_takes_pad("pad3", pads, 7));
    CHECK(menu_takes_pad("pad1", {}, 7));
    CHECK(menu_takes_pad("padX", pads, 9));
}

TEST(pad_press_filter, a_press_repeated_by_another_device_within_the_window_is_dropped) {
    DuplicatePressFilter filter;
    // What a PS4 pad listed twice looks like: the real pad, then DS4Windows's
    // virtual pad with the same button a few milliseconds later.
    CHECK(filter.take(3, SDL_GAMEPAD_BUTTON_DPAD_DOWN, 1000));
    CHECK(!filter.take(4, SDL_GAMEPAD_BUTTON_DPAD_DOWN, 1003));
    CHECK_EQ(filter.last_gap_ms(), Uint64(3));
    // A third device repeating the same press is a copy too.
    CHECK(!filter.take(5, SDL_GAMEPAD_BUTTON_DPAD_DOWN, 1010));
    // The next press, half a second on, is new again, and its copy is not.
    CHECK(filter.take(3, SDL_GAMEPAD_BUTTON_DPAD_DOWN, 1500));
    CHECK(!filter.take(4, SDL_GAMEPAD_BUTTON_DPAD_DOWN, 1500));
}

TEST(pad_press_filter, other_presses_are_never_copies) {
    DuplicatePressFilter filter;
    CHECK(filter.take(3, SDL_GAMEPAD_BUTTON_SOUTH, 1000));
    // Another button from the other device: a press of its own.
    CHECK(filter.take(4, SDL_GAMEPAD_BUTTON_EAST, 1002));
    // The same button again on the same device: a quick double press.
    CHECK(filter.take(4, SDL_GAMEPAD_BUTTON_EAST, 1010));
    // The same button from another device, but after the window.
    CHECK(filter.take(3, SDL_GAMEPAD_BUTTON_EAST, 1010 + DuplicatePressFilter::kWindowMs + 1));
    // At the window's edge it is still a copy.
    CHECK(filter.take(3, SDL_GAMEPAD_BUTTON_NORTH, 2000));
    CHECK(!filter.take(4, SDL_GAMEPAD_BUTTON_NORTH, 2000 + DuplicatePressFilter::kWindowMs));
}
