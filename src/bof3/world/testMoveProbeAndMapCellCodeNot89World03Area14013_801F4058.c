#include "bof3/bof3.h"

struct GameWorkArea;

/* Work-area record pointer cell of the shared main executable, immediately
 * before the D_80146888 work-area array. It is outside this overlay's image, so
 * the neighbouring lifts of this overlay family read the same cell, and the
 * only access here is the pointer the movement probe below consumes.
 * @source 0x80146884 @kind data
 */
extern struct GameWorkArea* D_80146884;

/* Movement probe of the shared main executable, outside this payload: it adds
 * the movement offsets selected by the work-area route byte to the work-area
 * coordinate pair and answers whether the party can reach that projected
 * position, taking the work-area record read through 0x80146884. This body uses
 * the answer as a plain flag.
 * @source 0x801A1AE4
 */
extern s32 func_801A1AE4(struct GameWorkArea* work);

/* Map code probe of the shared main executable, outside this payload: it
 * answers the one-byte code of the map cell at the supplied signed column and
 * row. The cell offsets are signed sixteen-bit values, so the original converts
 * each argument to that width at this call.
 * @source 0x80166CB0
 */
extern u8 func_80166CB0(s16 col, s16 row);

/* @source 0x801F4058
 * @behavior Tests the map cell at the supplied signed column and row against the
 *           map code 0x89 once the shared movement probe accepts the work-area
 *           record published through 0x80146884. The probe runs first and its
 *           zero answer returns 0 without probing the cell; otherwise the cell
 *           code is masked to its low byte and the answer is 1 only when that
 *           code differs from 0x89, so 0x89 itself answers 0. Both arguments are
 *           kept across the first call in their own saved registers and are
 *           converted to signed halfwords only for the second call, and the
 *           answer is a zero-or-one flag the caller masks with 0xFF.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 testMoveProbeAndMapCellCodeNot89World03Area14013_801F4058(s32 col, s32 row) {
    if (func_801A1AE4(D_80146884) != 0) {
        if ((func_80166CB0(col, row) & 0xFF) != 0x89) {
            return 1;
        }
    }
    return 0;
}
