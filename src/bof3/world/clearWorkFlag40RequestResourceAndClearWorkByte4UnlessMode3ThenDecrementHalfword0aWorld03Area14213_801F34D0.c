#include "bof3/bof3.h"

/* Shared resource-request helper reached by the `jal` at 0x801F3514. It has no
 * lifted source and no semantic owner name anywhere in the tree, so the
 * address-anchored name stays. */
void func_8014D6B8(u32 flag); /* @source 0x8014D6B8 */

/* Shared mode byte at 0x80143BB0. The neighbouring handler 0x801F3458 tests
 * the same byte against the same 3. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind unknown */

/* Pointer cell of the shared record whose halfword at 0x0A this step consumes. */
extern u16* D_80148210; /* @source 0x80148210 @kind unknown */

/* @source 0x801F34D0
 * @behavior Scratch work-record handler that complements the 0x801F3458
 *           arm/disarm pair. When the shared mode byte at 0x80143BB0 is not 3
 *           it clears the work-record flag bit 0x40 published through the
 *           pointer cell at 0x1F800044: the record byte 0 is loaded into a
 *           value register, the bit is masked off and stored back through the
 *           record pointer. A second load of the same cell then passes record
 *           byte 8 to the resource helper func_8014D6B8, and a third load
 *           writes 0 into record byte 4. Each access reloads the cell, so the
 *           original re-materialises the cell address for every step. Whatever
 *           the mode byte reads, the step then consumes two units of the
 *           halfword at 0x0A of the record published through the global
 *           pointer cell 0x80148210. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkFlag40RequestResourceAndClearWorkByte4UnlessMode3ThenDecrementHalfword0aWorld03Area14213_801F34D0(
    void) {
  u8** slots;
  u8* work;
  u8 value;

  if (D_80143BB0 != 3) {
    slots = SPAD_PTR_TABLE(u8);
    work = slots[0x11];
    value = work[0];
    value &= 0xBF;
    work[0] = value;
    slots = SPAD_PTR_TABLE(u8);
    func_8014D6B8(slots[0x11][8]);
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][4] = 0;
  }
  D_80148210[5] -= 2;
}
