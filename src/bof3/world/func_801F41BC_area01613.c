#include "bof3/world/area01613_internal.h"

extern int rand(void);

/* @source 0x801F41BC
 * @behavior When bit `0` of the shared byte at `0x80146871` is set it runs the
 * shared `0x80196070` helper and returns; otherwise, while the low ten bits of
 * the shared frame counter at `0x80143E6C` are clear, it fills a random number
 * of front-end records: the count is `rand() & 3` and the field it stores at
 * record offset `0x08` is `(rand() & 0x33) >> 4`; each iteration takes the next
 * free record index from the allocator `func_8019601C` and returns as soon as
 * that index is `0xFF`, otherwise it marks the record at `0x80143FC8` selected
 * by that index (`0x74` bytes per record) with the occupancy byte `0x00` = 1,
 * the selector `0x16` at offset `0x05`, the byte at `0x01` = 1, the loop
 * counter at offset `0x06` and the random field at offset `0x08`, and finally
 * stores the shared cursor pair `0x80145EC4`/`0x80145EC8` passed through the
 * shared `func_8015477C` distance helper, biased by `0x400`, into the signed
 * timer at record offset `0x3E`.
 * @status partial
 * @match 75.00
 * @residual Register allocation and preheader shape only: every mnemonic, field offset and shift of the loop body is identical, but gcc-2.7.2 hoists the `0xFF` allocator sentinel into a callee-saved register (`li s6,0xFF` + `beq v0,s6`) where the original rematerializes it per iteration (`andi v1,v0,0xFF` + `li v0,0xFF` + `beq v1,v0`), so the frame saves eight words instead of seven (same 0x38 frame, shifted save offsets) and the shared cursor base lands in `$s4` instead of `$s2`; the loop entry test is the folded `beqz s2` instead of the original's `blez v0` plus its `move s4,v0` bound copy. First live diff = the fourth instruction (`sw ra,52(sp)` vs `sw ra,48(sp)`); 60/80 instructions. Reverted clean-C levers: bound-expression loop, explicit signed guard, do-while/while forms, direct scalar cursor reads, address-cast cursor views, mask/shift statement order, and count expression variants (`div`/`break`-free, no profile change).
 */
void func_801F41BC(void) {
  u8 index;
  s32 i;
  u32 value;
  u32 masked;
  u32 shaped;
  s32 count;
  s32* source;

  if ((D_80146871 & 1) != 0) {
    func_80196070();
    return;
  }
  if ((D_80143E6C & 0x3FF) != 0) {
    return;
  }

  value = (u32)rand();
  masked = value & 0x33;
  count = (s32)(value & 3);
  shaped = masked >> 4;
  source = &D_80145EC4;

  for (i = 0; i < count; i++) {
    index = func_8019601C();
    if (index == 0xFF) {
      return;
    }
    D_80143FC8[index].flags_00 = 1;
    D_80143FC8[index].unk_05 = 0x16;
    D_80143FC8[index].unk_01 = 1;
    D_80143FC8[index].unk_06 = i;
    D_80143FC8[index].unk_08 = shaped;
    D_80143FC8[index].timer_3E = func_8015477C(source[0], source[1]) + 0x400;
  }
}
