#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the twenty-four code pointers from 0x801D078C
 * through 0x801D07E8, this payload's functions with the shared main-executable
 * handler 0x801B2D54 at entries 2 and 21. The reviewed Splat layout places all
 * twenty-four words in the one `textbin` row at 0x801D078C, so the run is only
 * referenced here. This dispatcher indexes it from 0x801D0798, so its base is
 * entry 3 of that pointer data and the entries it reaches at indices 0 to 2 are
 * 0x801CE654, 0x801CE820 and 0x801CE92C. */
extern void (*D_801D0798[])(void); /* @source 0x801D0798 @kind data */

/* Seeded from the lift of the same body at 0x801CE404 in this target, which
 * measured exact before this seed was written. A comparison of the two payload
 * spans shows their 68 bytes agree in every word except the scaled table load,
 * whose offset names 0x78C there and 0x798 here inside the same 0x801D page, so
 * the statement order, the macro spelling and the declaration style were carried
 * over with this function's own table symbol and name.
 */
/* @source 0x801CE610
 * @behavior Overlay step dispatch: reads the work-record pointer published at
 *           the scratchpad pointer cell 0x1F800044 once, takes that record's
 *           unsigned byte at offset 2, scales it by four and calls the matching
 *           entry of this overlay's handler run based at 0x801D0798 with no
 *           arguments, discarding any result. The original does not bound the
 *           index before the scaled table load, so an index above 2 reads on
 *           into the rest of the same run. It reads no other state and writes
 *           none; the pointer-cell load precedes frame creation, the 0x18-byte
 *           frame only keeps $ra at 0x10 across the indirect call, and the slots
 *           after the index byte and after the table load stay nops. Its only
 *           in-image reference is the word 0x801CE610 at 0x801D07FC, entry 3 of
 *           the seven-word code-pointer run at 0x801D07EC, so the overlay loader
 *           that steps that run is its caller and the body takes no parameters.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callStateTable0798, D_801D0798);
