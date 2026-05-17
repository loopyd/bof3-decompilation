#ifndef BOF3_SRC_CORE_GAME_FRONT_INTERNAL_H
#define BOF3_SRC_CORE_GAME_FRONT_INTERNAL_H

#include "bof3/core/callback_scheduler.h"
#include "bof3/core/game_front.h"
#include "bof3/context.h"

#define GAME_FRONT_EFFECT_BUSY (*(volatile u16*)0x80143c40u)
#define GAME_FRONT_LOCAL_MODE  (*(volatile u16*)0x80143c90u)

/* does: slot-2 frontend-local callback body selected by the local mode value.
 * @source: 0x8014ed6c
 */
void game_front_local_mode_callback_loop(void);

#endif
