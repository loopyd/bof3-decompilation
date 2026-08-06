#include "internal.h"

/* @behavior copies the current local battler templates selected by byte `0x13c`
 * into each active local work record's inline block at offset `0x74`.
 * @source 0x801DD08C
 */
void battle03_copy_local_templates(void) {
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

      /*
       * MATCHING_AID:
       * These three invariants are assigned inside the loop so GCC 2.7 loop.c
       * hoists them as late-created pseudos: the original preheader is
       * `la t4,D_80144968; la t3,D_80145E90+0x74; move t2,v1` (t2 copying the
       * count pointer materialized in v1 at entry), and the body addus use
       * index-first operand order (`addu a3,v0,t3` / `addu a2,v0,t4`).
       * Hoisting them in source before the loop flips the addu operand order
       * and drops the entry copy; a plain single count pointer allocates the
       * address straight into t2. Exhausted rungs: declaration/statement
       * reordering, temporaries, pointer hoists, profile search, pin probe.
       * The immediately following live `bin/byte-match` was exact.
       * Remove when the entry allocator behavior is understood structurally.
       */
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
