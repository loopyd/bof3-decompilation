#include "bof3/bof3.h"

extern s8 D_801448EC; /* @source 0x801448EC @kind data */
extern u8 D_80146254; /* @source 0x80146254 @kind data */
extern u8 D_80145EC6[]; /* @source 0x80145EC6 @kind data */
extern u8 D_80145ECA[]; /* @source 0x80145ECA @kind data */
extern u8 D_801F510C[]; /* @source 0x801F510C @kind data */
extern u8 D_801F510D[]; /* @source 0x801F510D @kind data */
extern u8 D_801F510E[]; /* @source 0x801F510E @kind data */
extern u8 D_801F510F[]; /* @source 0x801F510F @kind data */

/* @source 0x801F3D44
 * @behavior Returns 1 if any of the 0x80146254 work records has both position
 *           halfwords at +0x36 and +0x3A within the unsigned modular bounds
 *           selected by whether the mode byte at 0x801448EC is nonzero.
 *           The records have stride 0x140 from 0x80145E90. The selected
 *           four-byte row at 0x801F510C supplies the two origins and extents.
 *           Returns 0 when the count is zero or no record matches.
 * @status partial
 * @match 78.85
 * @residual Entry allocation differs: the index uses a1 instead of a0 and
 *           the scaled mode offset uses a0 instead of a1; count-copy and
 *           mode-scale scheduling also differ. The 208-byte size agrees.
 */
u8 anyWorkPositionInModeBoundsWorld01Area05213_801F3D44(void) {
  u8 i;
  s32 mode;
  s32 offset;

  i = 0;
  mode = D_801448EC != 0;
  if (D_80146254 != 0) {
    mode *= 4;
    do {
      offset = i * 0x140;
      if ((u16)(*(u16 *)(D_80145EC6 + offset) - D_801F510C[mode]) <
              D_801F510E[mode] &&
          (u16)(*(u16 *)(D_80145ECA + offset) - D_801F510D[mode]) <
              D_801F510F[mode]) {
        return 1;
      }
      i++;
    } while (i < D_80146254);
  }
  return 0;
}
