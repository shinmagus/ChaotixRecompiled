// A six-button pad as the game sees it through port 1 (runtime/md_bus.cpp,
// Machine::pad_read): the sequence a game steps through by toggling TH, and
// the pad forgetting that sequence once TH has rested for about 1.5 ms.
#include "runtime/system.h"
#include "test_framework.h"
#include <memory>

using namespace chaotix;

namespace {

constexpr uint32_t kPort1Data = 0xA10003;
constexpr uint32_t kPort1Control = 0xA10009;
constexpr uint32_t kThHigh = 0x40;
constexpr uint32_t kThLow = 0x00;
// The 68K runs at about 7.67 MHz; the pad gives up its sequence after 1.5 ms.
constexpr uint64_t kM68kCyclesPerMillisecond = 7670;

// The six button lines, as a game reads them: a pressed button reads as 0.
uint32_t read_port1(Machine& machine) {
    return machine.m68k_read8(kPort1Data) & 0x3F;
}

void set_th(Machine& machine, uint32_t level) {
    machine.m68k_write8(kPort1Data, level);
}

std::unique_ptr<Machine> machine_with(bool six_button, uint16_t pressed) {
    auto machine = std::make_unique<Machine>();
    machine->input.six_button[0] = six_button;
    machine->input.pad[0] = pressed;
    machine->m68k_write8(kPort1Control, kThHigh);  // TH is driven by the 68K
    set_th(*machine, kThHigh);
    // That first write was an edge of its own; the pad has rested since.
    machine->m68k_clock += 2 * kM68kCyclesPerMillisecond;
    return machine;
}

// The six TH edges a game sends to reach the extra buttons, ending high.
void six_edges(Machine& machine) {
    for (int pulse = 0; pulse < 3; ++pulse) {
        set_th(machine, kThLow);
        set_th(machine, kThHigh);
    }
}

} // namespace

TEST(six_button_pad, the_extra_buttons_answer_after_the_fifth_edge) {
    auto machine = machine_with(true, PAD_UP | PAD_X);
    // Before any edge, TH high: C B RIGHT LEFT DOWN UP, up pressed.
    CHECK_EQ(read_port1(*machine), 0x3Eu);
    set_th(*machine, kThLow);
    set_th(*machine, kThHigh);
    set_th(*machine, kThLow);
    set_th(*machine, kThHigh);
    // The fifth edge: the pad identifies itself with 0000 in the low nibble.
    set_th(*machine, kThLow);
    CHECK_EQ(read_port1(*machine) & 0x0Fu, 0x0u);
    // The sixth: MODE X Y Z in the low nibble, X pressed.
    set_th(*machine, kThHigh);
    CHECK_EQ(read_port1(*machine) & 0x0Fu, 0x0Bu);
}

TEST(six_button_pad, a_three_button_pad_never_identifies_itself) {
    auto machine = machine_with(false, PAD_UP);
    for (int edge = 0; edge < 5; ++edge) {
        set_th(*machine, edge % 2 ? kThHigh : kThLow);
    }
    // TH low after five edges: START A 0 0 DOWN UP, up pressed, never 0000.
    CHECK_EQ(read_port1(*machine), 0x32u);
}

TEST(six_button_pad, the_pad_forgets_the_sequence_once_th_has_rested) {
    auto machine = machine_with(true, PAD_UP);
    six_edges(*machine);
    // The sixth edge selected the extra buttons, none of them pressed.
    CHECK_EQ(read_port1(*machine) & 0x0Fu, 0x0Fu);
    // Virtua Racing Deluxe reads the pad every third frame and starts each
    // read before it toggles TH, so 50 ms after the sixth edge it expects the
    // directions again, not the extra buttons the sixth edge selected.
    machine->m68k_clock += 50 * kM68kCyclesPerMillisecond;
    CHECK_EQ(read_port1(*machine), 0x3Eu);
    // And within the sequence the extra buttons are still there.
    six_edges(*machine);
    CHECK_EQ(read_port1(*machine) & 0x0Fu, 0x0Fu);
}
