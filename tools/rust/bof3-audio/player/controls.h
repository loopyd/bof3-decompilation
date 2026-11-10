#ifndef PLAYER_CONTROLS_H
#define PLAYER_CONTROLS_H

#include "psx.h"

#define BUTTON_L1 0x0004
#define BUTTON_CIRCLE 0x0020
#define BUTTON_CROSS 0x0040
#define BUTTON_SQUARE 0x0080
#define BUTTON_UP 0x1000
#define BUTTON_DOWN 0x4000
#define BUTTON_LEFT 0x8000
#define BUTTON_RIGHT 0x2000

struct ControlsState {
    u32 held;
    u32 pressed;
    u32 help;
};
extern volatile struct ControlsState player_controls;

void controls_initialize(void);
u32 controls_read(void);

#endif
