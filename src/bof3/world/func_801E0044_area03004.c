#include "bof3/world/area03004_internal.h"

/* @source 0x801E0044
 * @behavior AREA030 work-record movement integrator: adds the step words at
 * offsets 0x0C/0x10 into the 32-bit position words at 0x34/0x38 of the scratch
 * work record published at the scratchpad cursor 0x1F800044 and counts the
 * record byte 0x09 down; when that count reaches zero it collapses both
 * positions to their sign bit, arms record byte 0x01, clears both step words
 * and record byte 0x02, then always runs the shared helper func_8014D978.
 * @status partial
 * @match 94.87
 * @residual live asm-diff first=+0x005c[o-/c23]: 37/39 instructions, 156/156
 * bytes. The head, the cursor re-reads and the branch/delay-slot layout match
 * byte for byte; the sole divergence is the scheduling of the third scratchpad
 * cursor read (`armed = D_1F800044;`, emitted as `lui $v1,(0x1F800044 >> 16);
 * lw $v1,..($v1)`): the original issues it between `sb $v1,1($a1)` and the
 * `andi`/`sh` of the first halfword mask pair, while this source leaves it
 * after that pair (the original then fills the `lw $v1` load-delay slot with
 * that `andi`). Moving the corresponding statement before the mask pair
 * re-allocates the record value to $a0 and the constant 1 to $v0 and costs 7
 * instructions (30/39), so this is a delay-slot/scheduling residual rather than
 * a statement-order fact. Clean-C facts established for this selector: the
 * volatile cursor cell must be read into a fresh local before the final
 * position store (`next = D_1F800044;` between the step load and that store) or
 * the head re-orders and the function loses 16 instructions; the record stays
 * non-volatile (volatile record views pin every access and add four load-delay
 * `nop`s, 20/43); the two mask pairs are plain read-modify-writes
 * (`*(u16*)(next + 0x34) = *(u16*)(next + 0x34) & 0x8000;`) because routing them
 * through a `position` local swaps the $v0/$a0 pair (32/39). Next untried rung:
 * one bounded `bin/permute emi/world00/area030/04@0x801E0044 --time-limit 60`
 * (parent opt-in only) once this delay-slot residual is classified.
 */
void func_801E0044(void) {
  u8* work;
  u8* next;
  u8* armed;
  s32 x;
  s32 y;
  s32 dx;
  s32 dy;
  u8 count;

  work = D_1F800044;
  x = *(s32*)(work + 0x34);
  dx = *(s32*)(work + 0x0C);
  count = *(u8*)(work + 9);
  *(s32*)(work + 0x34) = x + dx;
  y = *(s32*)(work + 0x38);
  count = (u8)(count - 1);
  *(u8*)(work + 9) = count;
  dy = *(s32*)(work + 0x10);
  next = D_1F800044;
  *(s32*)(work + 0x38) = y + dy;
  if (*(u8*)(next + 9) == 0) {
    *(u8*)(next + 1) = 1;
    *(u16*)(next + 0x34) = *(u16*)(next + 0x34) & 0x8000;
    armed = D_1F800044;
    *(u32*)(next + 0x0C) = 0;
    *(u32*)(next + 0x10) = 0;
    *(u16*)(next + 0x38) = *(u16*)(next + 0x38) & 0x8000;
    *(u8*)(armed + 2) = 0;
  }
  func_8014D978();
}
