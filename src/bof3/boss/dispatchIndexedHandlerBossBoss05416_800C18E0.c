#include "bof3/bof3.h"

typedef void (*Boss05416Handler)(void);

/* This payload's byte-indexed handler run at 0x800C20C0, the interior word the
 * scaled index adds to: its three entries are 0x800C1918, 0x800C1920 and
 * 0x800C1928, this overlay's inert handlers, and the run ends at the payload
 * end 0x800C20CC. The word is interior to the reviewed `textbin` row D_800C2078
 * (0x800C2078 through 0x800C20CB), whose first six words are the small record
 * 0x01010000, 0x01010404, 0, 0x06000600, 0xFFFF0602, 0x00000601 and whose
 * twelve-word step-handler run starts at 0x800C2090, so the row stays one data
 * row and this word is named at the offset the scaled index adds. */
extern Boss05416Handler D_800C20C0[]; /* @source 0x800C20C0 @kind data */

/* @source 0x800C18E0
 * @behavior Byte-indexed handler dispatch of this overlay: masks the index
 * argument with 0xFF, scales it by four and calls the entry of the handler run
 * based at 0x800C20C0, so the selected handler's result stays in the return
 * register. The original bounds the index only to a byte and never checks it
 * against the run's three entries; the slot after the scaled table load and the
 * indirect call's delay slot both stay nops, and the indirect call receives no
 * argument of its own beyond the scaled index word the body computed. The
 * 0x18-byte frame only keeps $ra at 0x10 across that call, and the epilogue
 * leaves $v0 untouched. No jal and no literal pointer word in this payload
 * names 0x800C18E0: 0x800C187C materializes the address with lui/addiu and
 * stores it into word +0xE4 of the work record at 0x801EB4E8, the indexed
 * handler slot the work-record stepper calls through its own index. The 0x38
 * byte body is instruction-for-instruction identical to the exact sibling
 * dispatcher dispatchIndexedHandlerBossBoss01316_800C18FC, a 0x38-byte
 * dispatcher of the boss013 overlay lifted before this body, and differs from
 * it in one word only: the %lo displacement of the scaled table load, since
 * both runs sit in the same 0x800C page. Its copied statement order and
 * operand forms met the original bytes on the first measurement, and the
 * payload span appears in no other extracted payload, so no byte-identical
 * twin exists to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss05416_800C18E0(u8 index) {
  D_800C20C0[index]();
}
