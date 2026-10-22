#include "bof3/core/slus_internal.h"

/* Body-read staging pair: the load buffer pointer is immediately followed by
 * the byte count read into it, so both fields are reached through one object. */
typedef struct ExeImageLoad {
  void* buffer;
  s32   size;
} ExeImageLoad;

extern u32*         D_801459F8;
extern u32          D_80143E70[];
extern ExeImageLoad D_80143E88;

extern s32 cdReadSync(s32 size, void* buffer, s32 sectors);

/* @behavior searches the mounted disc for the requested EXE image, reads one
 * 0x800-byte sector into the staged read buffer, copies the 0x88-byte image
 * header into the boot header area, advances the file position by one sector,
 * then reads the image body into the staged load buffer.  A missing file and a
 * failed body read both retry, up to ten attempts in total.
 * @source 0x8014E0FC
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_8014E0FC(const char* path) {
  CdlFILE file;
  s32     attempts;
  s32     sectors;

  sectors = 0x80;
  attempts = 0;
  do {
    if (CdSearchFile(&file, (char*)path) != NULL) {
      CdControl(CdlSetloc, (u_char*)&file, NULL);
      if (cdReadSync(0x800, D_801459F8, sectors) == 0) {
        __builtin_memcpy(&D_80143E70, D_801459F8, 0x88);
        CdIntToPos(CdPosToInt(&file.pos) + 1, &file.pos);
        CdControlB(CdlSetloc, (u_char*)&file, NULL);
        if (cdReadSync(D_80143E88.size, D_80143E88.buffer, sectors) != 0) {
          attempts++;
          continue;
        }
        return 0;
      }
    }
    attempts++;
  } while (attempts < 10);
  return -1;
}
