#include "bof3/bof3.h"

/* Resident flag tester of the shared flag bank: returns nonzero when the flag
 * number passed as its second argument reads set in the bank at the address
 * passed as its first, the same shape its exact callers in this tree use. */
s32 func_8015B5D4(u32 bank, s32 flag);

/* Shared work-area reset clearWorkFlags in emi/etc/game/00: zeroes work-object
 * bytes 0x00-0x04 and so returns the overlay to work state 0. */
void func_80196070(void);

void mathKeepGreenUnscaled(
    s32 scale);

/* Shared flag bank at 0x80144ED0 in main RAM, the bank the resident tester
 * func_8015B5D4 reads; this overlay probes flag 0x3E of it.
 * @source 0x80144ED0 @kind unknown
 */
extern u8 D_80144ED0;

/* Scratchpad work-record pointer cell. The original materialises the cell
 * address with `lui` plus an offset load, and a call to the shared reset can
 * republish the cell, so the source reads the named symbol once per group.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* @source 0x80117A74
 * @behavior Step handler 0 of the thirteen in-image pointers the Splat textbin
 * row D_8011818C places: word 8 at 0x801181AC, the base
 * callWorkByte2Table8 indexes from the
 * published work record's byte at offset 2, so this is the handler the overlay
 * runs while that byte still reads 0. It probes flag 0x3E of the shared flag
 * bank 0x80144ED0 through the resident tester func_8015B5D4 and, when the flag
 * reports set, scales the shared palette row's red and blue channels to half
 * through mathKeepGreenUnscaled
 * with the factor 0x40, half the 0x80 unity scale, carrying green unscaled,
 * and then resets the
 * scratchpad work object through the shared func_80196070 (clearWorkFlags in
 * emi/etc/game/00), which zeroes work-object bytes 0x00-0x04 and so returns the
 * overlay to work state 0. Every frame, taken or not, it then re-reads the
 * pointer cell and increments the record's byte at offset 2, the byte
 * callWorkByte2Table8 scales by four, so the
 * overlay moves on to entry 1 (func_80117ACC) from the next frame on. The
 * flag-bank address is materialized before the frame is created, the 0x18-byte
 * frame only keeps $ra at 0x10 across the three calls, the slot after the
 * probe's test stays a nop, and the slots after the pointer-cell load and after
 * the record byte load stay nops. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void mathRedBlueHalf(
    void) {
  if (func_8015B5D4((u32)&D_80144ED0, 0x3E) != 0) {
    mathKeepGreenUnscaled(
        0x40);
    func_80196070();
  }

  D_1F800044[2]++;
}
