#include "bof3/bof3.h"

typedef struct Area15113WorkRecord {
  u8  unk_00;
  u8  mode;
  u8  unk_02[0x07];
  u8  countdown;
  u8  unk_0a[0x36];
  u32 unk_40;
  u8  unk_44[0x04];
  u8  unk_48;
} Area15113WorkRecord;

/* Work record published through the scratchpad pointer cell at 0x1F800044. The
 * body reads the cell once for the countdown store and the word retreat, a
 * second time for the expiry test and the 0x48 store, and a third time for the
 * mode store, so it is read through this cell each time rather than through one
 * cached pointer.
 * @source 0x1F800044 @kind data
 */
extern Area15113WorkRecord* D_1F800044;

/* Shared mode byte of the shared main executable, outside this payload: the
 * countdown-expiry arm routes to the shared work-area reset only while this byte
 * reads 5, and takes the in-place mode re-arm for every other value.
 * @source 0x80143BB0 @kind data
 */
extern u8 D_80143BB0;

/* Local seeder of this overlay at 0x801F3370: it writes 0xA0 to work halfword
 * 0x2E and 0x50 to work halfword 0x30 through the same scratchpad pointer cell
 * and takes no arguments.
 * @source 0x801F3370
 */
extern void setWorkHalfwordsWorld03Area15113_801F3370(void);

/* Shared work-area reset of the main executable at 0x80196070, outside this
 * payload: the call site passes nothing and reads no result. Splat binds the
 * address through this target's generated undefined_funcs_auto.txt.
 * @source 0x80196070
 */
extern void func_80196070(void);

/* Shared per-frame helper of the main executable at 0x8014D4E0, outside this
 * payload: the countdown-still-running arm calls it and reads no result. Splat
 * binds the address through this target's generated undefined_funcs_auto.txt.
 * @source 0x8014D4E0
 */
extern void func_8014D4E0(void);

/* @source 0x801F32D0
 * @behavior Seeds the work halfwords through the local 0x801F3370 helper, then
 *           decrements the countdown byte at 0x09 of the work record published
 *           through scratchpad pointer cell 0x1F800044 and retreats the word at
 *           0x40 of that record by 0x2000. It reads the cell again and, once the
 *           countdown has reached zero, calls the shared work-area reset
 *           0x80196070 while the shared mode byte 0x80143BB0 reads 5, and
 *           otherwise clears work byte 0x48 and writes 1 to the work mode byte
 *           at 0x01 through a third read of the cell. While the countdown is
 *           still running it calls the shared per-frame helper 0x8014D4E0
 *           instead. The payload names no caller of this body - no jal and no
 *           code-pointer run word - and its only in-image reference is the word
 *           0x801F32D0 stored at 0x801F543C in the trailing script/data blob, so
 *           it is a script-invoked overlay entry whose roles stay structural.
 *           Takes no arguments, uses no result, and its 0x18-byte frame holds
 *           the return address only. One cmp of the 0xA0 span against the exact
 *           decrementByte9RetreatWord40By2000ThenResetWorkAreaWhenSharedMode5OrSetState1OnByte9ExpiryWorld00Area03313_801F3294
 *           lift of emi/world00/area033/13 at that payload's own offset left
 *           three differing bytes: its own seed address in the one `jal` word
 *           and the two `j` words whose targets shift by the 0x3C span delta,
 *           so that sibling supplied this body's statement order, operand forms,
 *           record layout and name while this body carries its own seed symbol.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementByte9RetreatWord40By2000ThenResetWorkAreaWhenSharedMode5OrSetState1OnByte9ExpiryWorld03Area15113_801F32D0(
    void) {
  Area15113WorkRecord* work;
  Area15113WorkRecord* state;

  setWorkHalfwordsWorld03Area15113_801F3370();
  work = D_1F800044;
  work->countdown--;
  work->unk_40 -= 0x2000;

  state = D_1F800044;
  if (state->countdown == 0) {
    if (D_80143BB0 == 5) {
      func_80196070();
    } else {
      state->unk_48 = 0;
      work = D_1F800044;
      work->mode = 1;
    }
  } else {
    func_8014D4E0();
  }
}
