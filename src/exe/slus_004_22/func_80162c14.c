#include "internal.h"

extern u32  DAT_80146454;
extern vu32 DAT_80146458;
extern u32  DAT_80146464;
extern s8   DAT_80146489;

/* @behavior copies the next EMI transfer chunk from the CD sector buffer,
 * wrapping into the active slot's sector when a partial sector remains.
 * @source 0x80162c14 func_80162c14
 */
void func_80162c14(void) {
  u32  transfer_words;
  u32  transfer_size;
  u32  transfer_address;
  u32* transfer_size_ptr;

  transfer_size_ptr = &DAT_80146454;
  transfer_size = *transfer_size_ptr;
  if (transfer_size >= 0x801u) {
    *transfer_size_ptr = transfer_size - 0x800u;
    CdGetSector((void*)DAT_80146458, 0x200);
    DAT_80146458 += 0x800u;
  } else {
    transfer_address = DAT_80146458;
    transfer_words = (transfer_size + 3u) >> 2;
    CdGetSector((void*)transfer_address, transfer_words);

    if (transfer_words != 0x200u) {
      CdGetSector((void*)(DAT_80146464 + ((s32)DAT_80146489 << 11)),
                  0x200u - transfer_words);
    }

    *transfer_size_ptr = 0;
    DAT_80146458 += transfer_words << 2;
  }
}
