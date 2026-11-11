#include "bof3/bof3.h"

/* Front-end record table of the shared main executable: 0x74-byte rows based at
 * 0x80143FC8. Only the fields this step touches are proven here: the occupancy
 * byte at 0x00 and the handler selector byte at 0x05, the selector of the table
 * at 0x801C7C70 that include/bof3/ui/game00_internal.h and the exact world03
 * area144 sibling record for this table. The remaining bytes keep their offset
 * names. The original keeps `index * 0x74` in one register and folds each store
 * into `lui $at` / `addu $at,$at,$reg` against the table base, so the source
 * keeps an indexed table row rather than a materialised row pointer.
 * @source 0x80143FC8 @kind table
 */
typedef struct World02Area09513FrontEndRecord {
  u8  flags_00;
  u8  unk_01[4];    /* 0x01 - 0x04 */
  u8  handler_05;
  u8  unk_06;
  u8  unk_07;
  u8  unk_08;
  u8  unk_09;
  u8  unk_0A[0x1F]; /* 0x0A - 0x28 */
  u8  unk_29;
  u8  unk_2A[2];    /* 0x2A - 0x2B */
  u16 unk_2C;
  u8  unk_2E[6];    /* 0x2E - 0x33 */
  u32 unk_34;
  u32 unk_38;
  u32 unk_3C;
  u8  pad_40[0x34]; /* 0x40 - 0x73 */
} World02Area09513FrontEndRecord;

extern World02Area09513FrontEndRecord D_80143FC8[];

/* Shared flag bank of the main executable. The flag helper 0x8015B580 takes the
 * bank address as its first argument and the bit index as its second, as the
 * exact world02 area108 record of the same helper states.
 * @source 0x80144F28 @kind data
 */
extern u8 D_80144F28[];

/* Shared route halfword of the main executable, bound by the shared target map
 * row and read here with `lhu` before the equality test against 0x60.
 * @source 0x80143F00 @kind unknown
 */
extern u16 D_80143F00;

/* Signed scenario-state byte read with `lb`; the same width the exact world
 * siblings declare at this address.
 * @source 0x80146870 @kind data
 */
extern s8 D_80146870;

/* Work-record pointer cell of the main executable; this step lowers the
 * unsigned halfword 0x7E of the record it publishes.
 * @source 0x80146884 @kind data
 */
extern u8* D_80146884;

/* Shared record selector of the main executable, outside this payload: it
 * answers the index of a free 0x74-byte row of the table at 0x80143FC8, or 0xFF
 * when every row is occupied. The original masks the answer to a byte before
 * the 0xFF test and before the row chain, so the declared return stays wide and
 * the caller narrows it.
 */
s32 func_8019601C(void);

/* Shared frontend cue dispatcher of the main executable; the cue id arrives
 * zero-extended in the first argument register.
 */
void func_8015DF18(u16 arg0);

/* Shared flag-bit helper of the main executable. */
void func_8015B580(void* bitmap, s32 bit_index);

/* @source 0x801F31F8
 * @behavior Overlay step handler. It asks the shared record selector
 *   func_8019601C for an index and masks the answer to a byte. Unless that byte
 *   reads 0xFF it claims the selected 0x74-byte front-end record of the shared
 *   table at 0x80143FC8, writing in this store order: handler selector 0x37
 *   into record byte 0x05, the word 0x410000 at 0x34, the word 0x2000000 at
 *   0x3C, the halfword 0x58 at 0x2C, byte 0x12 at 0x07, occupancy byte 1 at
 *   0x00, the word 0x3C8000 at 0x38, byte 1 at 0x09, byte 0 at 0x06 and byte 6
 *   at 0x29; it then queues frontend cue 0x202 through func_8015DF18. After the
 *   cue it sets one bit of the shared flag bank 0x80144F28 through
 *   func_8015B580: bit 0x20 when the shared route halfword 0x80143F00 reads
 *   0x60, otherwise bit 0x21 while the signed scenario byte 0x80146870 reads
 *   below 7, and bit 0x22 once that byte reads 7 or more. When the selector
 *   answers 0xFF instead it lowers the unsigned halfword 0x7E of the work
 *   record published at the pointer cell 0x80146884 by 2 and skips every store,
 *   the cue and the flag bit. It takes no arguments, returns nothing, and its
 *   0x18-byte frame saves $ra at 0x10.
 *
 * Name evidence: the claim, the handler selector, the cue id and the three flag
 *   bits are the whole written state of the span, and the names of the shared
 *   main-executable cells it binds come from the reviewed records of those cells
 *   in other targets plus this span own loads: `lhu` on 0x80143F00, `lb` on
 *   0x80146870, and the pointer cell 0x80146884 whose record this step lowers.
 *   The record bytes at 0x06, 0x07, 0x09, 0x29, 0x2C, 0x34, 0x38 and 0x3C carry
 *   no established role, so they stay offset-anchored and the name does not
 *   describe them.
 *
 * Shape: the payload scan over every extracted binary under out/binaries finds
 *   no byte-identical copy of this 0x174-byte span. The nearest sibling is the
 *   world02 area096 row at 0x801F3188, which differs in its three internal `j`
 *   words alone and in each by exactly the 0x70 span delta, so it is this body
 *   at another address; that row is still `asm`, so there is no lifted source to
 *   clone and the clone lever is declined. The record-seeding region is
 *   transcribed from the exact world03 area144 sibling of the same table, which
 *   carries the identical `andi 0xFF` / 0xFF test / row chain head and the same
 *   `$at`-folded stores, and every literal, cell address and cue id here comes
 *   from this span own bytes.
 *
 * Residual (live measurement 87/93 instructions, 93.55%, 372 bytes at the
 *   original size, first difference at instruction 7): the two hoisted
 *   constants of the claim block exchange their caller-saved registers and the
 *   order in which they are materialised. The build holds the value 1 in $a2,
 *   defined in the branch delay slot, and 0x3C8000 in $a1 defined in the two
 *   instructions after it; the original holds the value 1 in $a1, defined after
 *   the row-index chain and before the first store value, and 0x3C8000 in $a2
 *   defined in the delay slot pair. Every other instruction matches: the store
 *   order, the cue call, the three flag arms and the 0xFF retreat arm. Declaring
 *   the repeated 1 as a local assigned ahead of the guard is what keeps the two
 *   1-stores apart; with that assignment inside the block the scheduler sinks
 *   the value to its first use and merges the two stores (83/93).
 * @status partial
 * @match 93.55
 * @residual Constant materialisation order and caller-saved register pair of the
 *   claim block, first difference at instruction 7.
 */
void claimFrontEndRecordSetHandler37QueueCue202AndSetFlagBank20Or21Or22ByRoute60AndScenarioElseLowerWorkHalfword7eBy2World02Area09513_801F31F8(
    void) {
  s32 index;
  u8  one;

  index = func_8019601C() & 0xFF;
  one = 1;
  if (index != 0xFF) {
    D_80143FC8[index].handler_05 = 0x37;
    D_80143FC8[index].unk_34 = 0x410000;
    D_80143FC8[index].unk_3C = 0x2000000;
    D_80143FC8[index].unk_2C = 0x58;
    D_80143FC8[index].unk_07 = 0x12;
    D_80143FC8[index].flags_00 = one;
    D_80143FC8[index].unk_38 = 0x3C8000;
    D_80143FC8[index].unk_09 = one;
    D_80143FC8[index].unk_06 = 0;
    D_80143FC8[index].unk_29 = 6;
    func_8015DF18(0x202);
    if (D_80143F00 == 0x60) {
      func_8015B580(D_80144F28, 0x20);
    } else if (D_80146870 < 7) {
      func_8015B580(D_80144F28, 0x21);
    } else {
      func_8015B580(D_80144F28, 0x22);
    }
  } else {
    *(u16*)(D_80146884 + 0x7E) -= 2;
  }
}
