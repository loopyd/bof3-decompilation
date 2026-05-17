#ifndef BOF3_CONTEXT_01_SYMBOLS_H
#define BOF3_CONTEXT_01_SYMBOLS_H

/* address and table pointer definitions */

#define BOF3_GAME_FRONT_START_MASK         0x0800u
#define BOF3_GAME_FRONT_POPUP_PENDING_MASK 0x00ffff00u
#define BOF3_GAME_FRONT_POPUP_PENDING_OPEN 0x00020000u
#define BOF3_GAME_FRONT_SELECTION_FX_TABLE ((const volatile u8*)0x80181ebau)
#endif
