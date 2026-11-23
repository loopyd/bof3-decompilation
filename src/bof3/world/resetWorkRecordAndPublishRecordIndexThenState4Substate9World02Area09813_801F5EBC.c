#include "bof3/bof3.h"

/* Opaque 0x98-byte work-record view. The shared game work-area header declares
 * the record array at 0x80146888 as an array of the 0x98-byte record, and the
 * original publishes the index of a record as a pointer difference between
 * records, which GCC 2.7.2 turns into that stride's exact division. The field
 * roles at the offsets this body touches are not established, so they stay
 * offset-anchored. */
typedef struct WorkRecordBytes {
  u8 bytes[0x98];
} WorkRecordBytes;

/* Pointer cell publishing the active work record; every record access below
 * re-reads it because a record store may alias the cell. */
extern WorkRecordBytes *D_80148208;
extern WorkRecordBytes D_80146888[];
extern u8 D_80146867;
extern u8 D_801448EB;
extern u8 D_801448ED;

void func_8015C088(void);

/* @source 0x801F5EBC
 * @behavior Overlay entry handler: runs the shared front-end startup helper at
 *           0x8015C088, then resets the work record published through the
 *           pointer cell 0x80148208 by storing 4 into its byte at +1, 0 into
 *           its byte at 0x77 and 0 into its halfword at 0x7E. It then
 *           publishes into the shared byte 0x80146867 the position of that
 *           record inside the 0x98-byte record array based at 0x80146888, and
 *           finally writes 4 into the shared state entry byte 0x801448EB and 9
 *           into the shared substate byte 0x801448ED, leaving the middle byte
 *           0x801448EC untouched. Takes no arguments and returns 0.
 * @status partial
 * @match 48.72
 * @residual Allocation and scheduling: the original keeps the shared 4 in a2,
 *           the third record pointer in a1 and division temporaries in a0/v1.
 *           This body keeps them in a0, v1 and v0/v1 respectively. The array
 *           base materialization follows the byte +0x77 clear rather than
 *           preceding it, and the final state/substate stores are reversed.
 *           The three pointer reloads, access widths and signed stride
 *           division (`negu` then `sra`) are preserved.
 *
 * The record index is an element difference between 0x98-byte record pointers.
 * The corresponding sequence in emi/world00/area032/13 at 0x801F3C18 and
 * emi/world00/area008/13 at 0x801F4424 uses the same pointer-difference form.
 * The nearest shape siblings are emi/world01/area068/13 at 0x801F3F48 and
 * emi/world01/area059/13 at 0x801F2F34. Their stored literals differ; this body
 * uses the literals encoded in its own span.
 */
s32 resetWorkRecordAndPublishRecordIndexThenState4Substate9World02Area09813_801F5EBC(
    void) {
  s32 index;
  u8 state;

  func_8015C088();
  state = 4;
  D_80148208->bytes[1] = state;
  do {
    D_80148208->bytes[0x77] = 0;
    index = D_80148208 - D_80146888;
    *(u16 *)((u8 *)D_80148208 + 0x7E) = 0;
  } while (0);
  D_80146867 = (u8)index;
  D_801448EB = state;
  D_801448ED = 9;
  return 0;
}
