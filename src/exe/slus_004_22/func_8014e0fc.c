#include "internal.h"

extern void* func_8014E0A8(s32 size, void* buffer, s32 sectors);

extern void* D_801459F8;
extern u32   D_80143E70[];
extern void* D_80143E88;
extern s32   D_80143E8C;

/* @behavior reads one EXE file into RAM in two sector-sized steps and copies the
 * first staged header block into the boot scratch area.
 * @source 0x8014E0FC
 */
s32 func_8014E0FC(const char* path) {
  CdlFILE     file;
  u32*        src;
  u32*        dst;
  u32*        end;
  s32         attempts;
  s32         sectors;
  s32*        exe_size;
  const char* current_path;

  current_path = path;
  sectors = 0x80;
  exe_size = &D_80143E8C;
  attempts = 0;
  while (1) {
    if (CdSearchFile(&file, current_path) != NULL) {
      CdControl(CdlSetloc, (u_char*)&file, NULL);
      if (func_8014E0A8(0x800, D_801459F8, sectors) == 0) {
        src = D_801459F8;
        dst = D_80143E70;
        end = src + 0x20;

        do {
          u32* cur_dst;
          u32* cur_src;
          u32  word1;
          u32  word2;
          u32  word3;

          cur_dst = dst;
          cur_src = src;
          word1 = cur_src[1];
          word2 = cur_src[2];
          word3 = cur_src[3];
          cur_dst[0] = cur_src[0];
          cur_dst[1] = word1;
          cur_dst[2] = word2;
          cur_dst[3] = word3;
          src = cur_src + 4;
          dst = cur_dst + 4;
        } while (src != end);

        dst[0] = src[0];
        dst[1] = src[1];
        CdIntToPos(CdPosToInt(&file.pos) + 1, &file.pos);
        CdControlB(CdlSetloc, (u_char*)&file, NULL);

        attempts += 1;
        if (func_8014E0A8(exe_size[0], D_80143E88, sectors) == 0) {
          return 0;
        }

        if (attempts < 10) {
          continue;
        }

        return -1;
      }
    }

    attempts += 1;
    if (attempts >= 10) {
      return -1;
    }
  }
}
