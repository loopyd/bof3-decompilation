#include "internal.h"

extern void func_80158E20(void);

extern u8 D_80148330[];

void func_8009B20C(void)
{
    u8 i;
    u8 *base;
    u8 *ptr;

    i = 0x10;
    base = D_80148330;
    do {
        ptr = base + i * 0x24;
        D_80148648 = (Bof3PanelTask *)ptr;
        ptr = ptr;
        i += 1;
        func_80158E20();
    } while (i < 0x14);
}