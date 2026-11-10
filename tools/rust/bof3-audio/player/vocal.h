#ifndef PLAYER_VOCAL_H
#define PLAYER_VOCAL_H

#include "psx.h"

struct VocalStatus {
    u32 state;
    u32 repeat;
};
extern volatile struct VocalStatus vocal_status;

void vocal_initialize(void);
void vocal_update(void);
void vocal_play(void);
void vocal_stop(void);
void vocal_repeat(void);

#endif
