#include "bof3/bof3.h"

typedef struct Area12114WorkRecord {
  u8  unk_00;
  u8  mode;
  u8  unk_02[0x05];
  u8  unk_07;
  u8  unk_08;
  u8  unk_09;
  u8  unk_0a;
  u8  state;
  u8  unk_0c[0x0c];
  u32 unk_18;
  u8  unk_1c[0x24];
  u32 unk_40;
  u32 unk_44;
  u8  unk_48;
} Area12114WorkRecord;

/* @source 0x1F800044 @kind unknown */
extern Area12114WorkRecord* area12114WorkCursor_1F800044;

extern u8  D_801F64D8[];
extern u16 D_80143F10;

void func_8014D6B8(u32 resource);

/* @source 0x801F3104
 * @behavior Mode-1 handler of the work-record mode table at 0x801F67C4, which
 * the overlay's update at 0x801F2F04 indexes with the work-record byte at 0x01.
 * It latches the interaction byte at 0x0B into work byte 0x07 and then selects
 * the resource the entry requests. For interaction 1 it searches the local
 * four-byte-row table 0x801F64D8 for the row whose first halfword equals the
 * shared context seed 0x80143F10 and takes that row's byte at offset 2.
 * Interaction 2 requests resource 3, interaction 3 requests resource 0 and
 * interaction 4 requests resource 1; every other value returns without a
 * request. Each requesting path clears work word 0x40, stores 0x10000 in word
 * 0x44, stores 2 in work byte 0x48 and 8 in work byte 0x09, and only the
 * interaction-1 path publishes the matched seed in work word 0x18. The
 * selected resource is then requested through the shared resident helper at
 * 0x8014D6B8 and the mode byte at 0x01 is set to 2. Takes no arguments and
 * returns nothing.
 * @status partial
 * @match 82.05
 * @residual The build matches 64 of 78 compared instructions and occupies
 * 308 bytes versus the original's 312. The search lacks the original offset
 * copy, reverses the equality operands, and schedules the mode constant in
 * place of the word-0x44 constant. The common arm also reverses those constant
 * loads. Compiler-source evidence is unavailable; the ladder is not exhausted.
 */
void requestResourceForInteractionByteThenAdvanceMode2World03Area12114_801F3104(void) {
  Area12114WorkRecord* work;
  u32                  value;
  u32                  offset;
  u32                  next;

  work = area12114WorkCursor_1F800044;
  work->unk_07 = work->state;

  if (area12114WorkCursor_1F800044->state == 1) {
    value = D_80143F10;
    offset = 0;
  search:
    if (FIELD_REF(u16, D_801F64D8, offset) == value) {
      goto seed_found;
    }
    offset += 4;
    goto search;
  } else {
    work = SPAD_PTR_SLOT(Area12114WorkRecord, 0x44);
    value = work->state;
    if (value == 2) {
      value = 3;
      goto arm;
    } else if (value == 3) {
      value = 0;
      goto arm;
    } else {
      goto interaction_four;
    }
  }
seed_found:
  work = area12114WorkCursor_1F800044;
  next = 2;
  work->unk_18 = value;
  work->unk_40 = 0;
  work->unk_44 = 0x10000;
  work->unk_48 = next;
  area12114WorkCursor_1F800044->unk_09 = 8;
  value = FIELD_REF(u8, D_801F64D8, offset + 2);
  goto request;

interaction_four:
  if (value != 4) {
    return;
  }
  value = 1;
arm:
  next = 2;
  work->unk_40 = 0;
  work->unk_44 = 0x10000;
  work->unk_48 = next;
  area12114WorkCursor_1F800044->unk_09 = 8;

request:
  func_8014D6B8(value);
  area12114WorkCursor_1F800044->mode = next;
}
