#include "bof3/battle/battle03_internal.h"

/* @source 0x801DCFD0
 * @behavior Writes each active local work record's inline 0xA4-byte block at
 * +0x74 back into the shared template record selected by that record's byte
 * +0x13C (D_80145FCC), for the first D_80146254 records.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The two base pointers are named inside the loop so GCC hoists them as
 * late-created pseudos in source order: the original preheader materializes the
 * template base in t3 and the local-work block base in t2, and the body addus
 * use index-first operand order (`addu a3,v0,t2`, `addu a2,v0,t3`). Computing
 * both addresses inline in their body expressions instead allocates t3/t2
 * swapped (asm-diff 43/47, first mismatch +0x0018).
 */
void copyLocalWorkToTemplates(void) {
  u8                      index;
  Battle03TemplateRecord* templates;
  u8*                     work_base;

  index = 0;
  if (D_80146254 != 0) {
    do {
      s32                           work_offset;
      u8                            template_index;
      const Battle03TemplateRecord* src;
      Battle03TemplateRecord*       dst;

      templates = D_80144968;
      work_base = (u8*)D_80145E90 + 0x74;
      work_offset = (s32)index * 0x140;
      src = (const Battle03TemplateRecord*)(work_offset + work_base);
      template_index = D_80145FCC[work_offset];
      dst = (Battle03TemplateRecord*)((u8*)templates + template_index * 0xa4);
      *dst = *src;
      index += 1;
    } while (index < D_80146254);
  }
}
