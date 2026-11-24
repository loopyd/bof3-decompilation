#include "bof3/bof3.h"

typedef s32 (*ScenarioScena0500LookupHandler)(void);

/* Word argument view: the resident helper consumes the low byte of each
 * coordinate, as shown by its entry masks at 0x801A7AF4 and 0x801A7AF8. */
s32 func_801A7AD8(u8 *records, s32 count, u16 coordA, u16 coordB);

/* The overlay's textbin row at 0x801FD668 begins with the two five-byte lookup
 * records this selector scans: key byte 0x34 with base coordinate 0x84 and
 * second coordinate 0x10, then the same key and base with second coordinate
 * 0x13, both carrying flags 0x07 and span count 2. The two bytes at
 * 0x801FD672..0x801FD673 are alignment, and the handler run starts at
 * 0x801FD674. The row's own Splat label is the map row this target spells.
 * @source 0x801FD668 @kind table
 */
extern u8 coordinateScanRecords[];

/* The code-pointer run that begins immediately after those records, at
 * 0x801FD674: eighteen overlay handler addresses from 0x801FD178 through
 * 0x801FD580, followed by a zero word at 0x801FD6BC. With two records the
 * lookup can only answer 0 or 1, so the reachable entries are word 0,
 * func_801FD178 (0x801FD178), and word 1, func_801FD208 (0x801FD208); both are
 * still unlifted 0x90-byte spans of this overlay whose bytes differ only in the
 * constant 0x26 versus 0x25 they pass to func_8015B580 and the sub-state 1
 * versus 0x19 they store at D_80146875. The base sits inside the textbin rows
 * above, so the target map does not own it and splat's generated symbol file
 * binds the interior word.
 */
extern ScenarioScena0500LookupHandler D_801FD674[];

/* @source 0x801FD110
 * @behavior Overlay coordinate-record selector, and word 4 (0x801FD5D8) of the
 * five-word code-pointer run at 0x801FD5C8 that precedes the three-word
 * outer-state row at 0x801FD5DC: it passes the two unsigned halfword
 * coordinates its own argument registers hold and the two five-byte lookup
 * records at 0x801FD668, with the length 2, to the resident helper
 * func_801A7AD8 (0x801A7AD8), which matches a record's key byte against the
 * shared world byte D_80143F00, its coordinate span against those coordinates
 * and its low-nibble category against the shared record byte at 0x80145E98. It
 * reads that answer as a signed byte, returns -1 when the index is negative and
 * otherwise calls that entry of the eighteen-word code-pointer run at
 * 0x801FD674 with no arguments, returning the handler's signed low byte. Only
 * indices 0 and 1 are reachable with two records and they hold 0x801FD178 and
 * 0x801FD208, so a matching pair always runs one of those two bodies; the
 * records differ only in their second coordinate (0x10 versus 0x13) and their
 * keys and flags are equal, so that coordinate picks between the two handlers.
 * It makes no stores and reads no state of its own; the 0x18-byte frame only
 * keeps $ra across the two calls. No in-image jal targets the address and its
 * only image word is 0x801FD5D8, so the run at 0x801FD5C8 is reached from
 * outside this payload and the two coordinate values are structural rather than
 * a contractual parameter pair. Its 104 bytes compare equal word for word with
 * the exact dispatchCoordinateLookupHandlerScenarioScena1200_801FCC54
 * (0x801FCC54 of emi/scenario/scena12/00) in every word except the record-table
 * low immediate, the lookup length (2 here, 4 there), the handler-table low
 * immediate and the absolute `j` word, which shifts by exactly the 0x4BC offset
 * delta between the two spans; that sibling supplied the statement order,
 * operand forms, declaration style and name this body writes, and every literal
 * here is this span's own immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 dispatchCoordinateLookupHandler(u16 coordA, u16 coordB) {
  s8 index;

  index = func_801A7AD8(coordinateScanRecords, 2, coordA, coordB);
  if (index < 0) {
    return -1;
  }
  return (s8)D_801FD674[index]();
}
