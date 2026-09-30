#include "bof3/world/area01613_internal.h"

/* @source 0x801F2C38
 * @behavior when the shared mode byte at 0x801448EC is 0 it calls the shared
 * func_8015C088 helper, probes the shared scroll pair at 0x80145EC6/0x80145ECA
 * through func_80166CB0, and then either searches the local 0x801F4FC4 table
 * (16 halfwords per entry) for the entry whose key equals the shared
 * 0x80143F10 word and starts the halfword at its 0x80146870 column through
 * func_80150224, or scans the four-byte rows at 0x801F4DA0 for the row whose
 * first byte equals 0x80145EC6 and whose second byte equals 0x80145ECA and
 * uses that row's fourth byte to select one of the three five-byte records at
 * 0x801F50E4: the four 0x20-byte records at 0x801490D8 are then each filled
 * with twelve 0x5C bytes when the selected code's flag byte at 0x80144FE4 is
 * clear, or copied from the local 0x801F50F4 table (code 0x16) or from
 * func_80165D48(0, code + 0x38) otherwise, clearing the byte at 0x801490E4
 * plus the record offset, and action (selection + 0x16) is started through
 * func_80150224; every path then writes 2 to the shared phase byte at
 * 0x80143BB0 and increments the mode byte; when the shared mode byte is 1 and
 * the phase byte is not 2 it calls the shared func_8015C058 helper and clears
 * 0x801448EB, the mode byte and 0x801448ED.
 * @status partial
 * @match 94.22
 * @residual 163/173 instructions (692/692 bytes) match, first difference at
 * +0x98. Everything except the halfword-search exit and the fill preheader matches
 * instruction for instruction: the shared 0x801F4FC4 table search, the 0x801F4DA0
 * row scan, the 0x801F50E4 record selection, both twelve-byte fill/copy arms, the
 * cross-jumped func_80150224 edges and the case-1 reset. The residual is
 * allocator/scheduler only: at the search exit this candidate duplicates the
 * `sll $v0,$s0,0x5` index scaling into the `beq` delay slot (the original leaves
 * `nop` there and recomputes `sll $v1,$s0,0x5` after the base materialisation),
 * which shifts the exit block's `$v0`/`$v1` roles, and the fill loop emits the
 * hoisted `li $a0,0x5C` after `li $v1,0xB` instead of before it. Rejected clean-C
 * levers (each measured against the live diff): byte-offset vs element-index vs
 * whole-expression forms of the search result pointer, `entry` pointer local with
 * and without a cast chain, do-while/while/goto spellings of the fill loop,
 * preheader statement permutations, and splitting the search byte offset from the
 * 0x801490E4 record offset. Compiler-profile and permuter rungs are opt-in and were
 * not authorised for this selector.
 */
void func_801F2C38(void) {
  const u8* row;
  const s16* entry;
  u8* codes;
  u8* dst;
  u8* src;
  u8 code;
  s32 index;
  s32 count;
  s32 offset;
  s32 record_offset;
  s32 i;

  switch (*(s8*)D_801448EC) {
  case 0:
    func_8015C088();
    code = (u8)func_80166CB0(D_80145EC6[0], D_80145ECA);
    if (code == 0xA1) {
      for (index = 0; index < 9; index++) {
        offset = index * 32;
        if (D_80143F10 == *(u16*)((u8*)D_801F4FC4 + offset)) {
          break;
        }
      }
      entry = (const s16*)D_801F4FC4 + index * 16;
      func_80150224(entry[D_80146870]);
    } else {
      row = &D_801F4DA0[0][0];
      while (1) {
        if (D_80145EC6[0] == row[0] && D_80145EC6[2] == row[1]) {
          break;
        }
        row += 4;
      }
      for (index = 0; index < 3; index++) {
        offset = index * 5;
        if (row[3] == D_801F50E4[offset]) {
          break;
        }
      }
      count = 1;
      record_offset = 0;
      dst = D_801490D8;
      codes = &D_801F50E5[index * 5];
    area01613Record:
      code = *codes;
      if (code == 0xFF) {
        goto area01613RecordsDone;
      }
      if (D_80144FE4[code] != 0) {
        if (code == 0x16) {
          src = D_801F50F4;
        } else {
          src = func_80165D48(0, code + 0x38);
        }
        for (i = 0; i < 0xC; i++) {
          dst[i] = src[i];
        }
      } else {
        i = 0xB;
        do {
          dst[i] = 0x5C;
          i -= 1;
        } while (i >= 0);
      }
      D_801490E4[record_offset] = 0;
      record_offset += 0x20;
      dst += 0x20;
      codes += 1;
      count += 1;
      if (count < 5) {
        goto area01613Record;
      }
    area01613RecordsDone:
      func_80150224((s16)(index + 0x16));
    }
    D_80143BB0 = 2;
    D_801448EC[0] = D_801448EC[0] + 1;
    break;
  case 1:
    if (D_80143BB0 == 2) {
      break;
    }
    func_8015C058();
    D_801448EB = 0;
    D_801448EC[0] = 0;
    D_801448ED = 0;
    break;
  }
}
