#include "controls.h"

volatile struct ControlsState player_controls;

void controls_initialize(void)
{
    CALL(0x80174668, 0, 0, 0, 0);
    player_controls.held = (u32)CALL(0x801746b8, 0, 0, 0, 0) & 0xffff;
    player_controls.pressed = 0;
    player_controls.help = 0;
}

u32 controls_read(void)
{
    u32 held, pressed;
    held = (u32)CALL(0x801746b8, 0, 0, 0, 0) & 0xffff;
    pressed = held & ~player_controls.held;
    player_controls.held = held;
    player_controls.pressed = 0;
    if (pressed & BUTTON_L1) {
        player_controls.help = !player_controls.help;
        return 0;
    }
    if (player_controls.help) return 0;
    player_controls.pressed = pressed;
    return pressed;
}
