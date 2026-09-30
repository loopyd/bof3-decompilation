#include "bof3/battle/battle03_internal.h"

/* @behavior copies the current local battler templates selected by byte `0x13c`
 * into each active local work record's inline block at offset `0x74`.
 * @source 0x801DD08C
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyLocalTemplates(void) {
  u8                                index;
  volatile u8*                      count_ptr;
  const Battle03TemplateRecord*     templates;
  u8*                               work_base;
  volatile u8*                      loop_count;

  count_ptr = &D_801462F0;
  index = 0u;
  if (*count_ptr != 0) {
    do {
      s32                                work_offset;
      s32                                template_index;
      const Battle03TemplateRecord*      src;
      Battle03TemplateRecord*            dst;

      templates = D_80144968;
      work_base = (u8*)D_80145E90 + 0x74;
      loop_count = count_ptr;
      work_offset = (s32)index * 0x140;
      template_index = ((u8*)D_80145E90)[work_offset + 0x13c];
      src = (const Battle03TemplateRecord*)((const u8*)templates +
                                            template_index * 0xa4);
      dst = (Battle03TemplateRecord*)(work_offset + work_base);
      *dst = *src;
      index += 1u;
    } while (index < *loop_count);
  }
}
