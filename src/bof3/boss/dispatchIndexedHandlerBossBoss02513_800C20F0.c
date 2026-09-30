#include "bof3/bof3.h"

typedef void (*Boss02513Handler)(void);

/*
 * Handler table at 0x800C3294, the table this dispatcher selects from:
 * entry 0 = 0x800C2128, entry 1 = 0x800C2160 (raises bit 0x20 of the boss
 * flag byte 0x801462ED), entry 2 = 0x800C217C (inert handler), entry 3 =
 * 0x800C3038, entry 4 = 0x800C3088.
 */
extern Boss02513Handler D_800C3294[]; /* @source 0x800C3294 @kind table */

/* @source 0x800C20F0
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C3294
 * handler table, leaving whatever that handler put in the return register as
 * this function's own result. No in-payload caller: func_800C1DCC installs
 * this function's address at +0xE4 of the structure pointed to by the
 * scratchpad pointer at 0x1F800044 instead of branching to it.
 * Byte-identical to the sibling dispatchers at 0x800C1A68 and 0x800C1D2C of
 * this overlay; only the table address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss02513_800C20F0(u8 index) {
  D_800C3294[index]();
}
