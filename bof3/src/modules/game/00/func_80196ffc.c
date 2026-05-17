#include "internal.h"

struct GameData {
    u8 pad_3b90[0x3b90];
    volatile u16 entry0_state;
    u8 pad_3b92_5988[0x1df6];
    volatile u8 palette_serial;
};

#define GAME_DATA ((struct GameData*)0x80140000)

void func_80196ffc(void) {
    volatile u16* const ent = (volatile u16*)((volatile u8*)GAME_DATA + 0x3b90u);
    u16 ev;
    u8 pv;

    emi_stream_init_slot(0x268u);

    while (!func_80162d00()) {
        func_8014b87c(1u);
    }

    func_8014e284();

    pv = GAME_DATA->palette_serial;
    ev = *ent;
    pv++;
    ev++;
    GAME_DATA->palette_serial = pv;
    *ent = ev;
}
