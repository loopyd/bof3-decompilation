#include "bof3/bof3.h"
#include "gpu/prim.h"

typedef struct Magic114Endpoint {
  s32 unk_00;
  s32 unk_04;
} Magic114Endpoint;

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern Magic114Endpoint D_801F11F4[]; /* @source 0x801F11F4 @kind data */
extern void func_801EF6A8(void); /* @source 0x801EF6A8 */
extern void func_801DD350(s32 work); /* @source 0x801DD350 */
extern void func_801EF280(s32 *keys); /* @source 0x801EF280 */
extern void func_800AF0F8(u32 x, u32 y, s32 *keys, u8 *packets,
                         u8 count, u8 stride, u8 extra); /* @source 0x800AF0F8 */

/* @source 0x801EF128
 * @behavior Call setup helper 0x801EF6A8, then build eight triangles through
 *           helper 0x801EF280. For each triangle scale two successive pairs
 *           from the closed four-edge table at 0x801F11F4 by work byte A,
 *           transform each through helper 0x801DD350, and retain the first
 *           endpoint in work words 34 and 38. Publish the triangle index in
 *           work byte B. Submit eight 28-byte packets with their depth keys
 *           through helper 0x800AF0F8 at record 0x801EB4E0 words 34 and 38,
 *           then PopMatrix. Takes no arguments and returns no value.
 * @status partial
 * @match 70.45
 * @residual 62/88 instructions; 344 original bytes versus 352 current.
 *           Work and scaled-index registers differ at the loop entry;
 *           two call-argument copies and tail scheduling remain unresolved.
 */
void drawEightScaledEndpointTrianglesAndSubmitBmagicMagic11403_801EF128(void) {
  s32 keys[8];
  u32 index;
  u32 edge;
  u8 *packets;
  s32 work;

  func_801EF6A8();
  index = 0;
  packets = g_PrimCursor;
  do {
    edge = index & 3;
    work = (s32)g_battle_work;
    FIELD_REF(s32, work, 0xCu) = D_801F11F4[edge].unk_00 * FIELD_REF(u8, work, 0xAu);
    FIELD_REF(s32, work, 0x10u) = D_801F11F4[edge].unk_04 * FIELD_REF(u8, work, 0xAu);
    func_801DD350(work);
    work = (s32)g_battle_work;
    FIELD_REF(s32, work, 0x34u) = FIELD_REF(s32, work, 0xCu);
    FIELD_REF(s32, work, 0x38u) = FIELD_REF(s32, work, 0x10u);
    edge++;
    FIELD_REF(s32, work, 0xCu) = D_801F11F4[edge].unk_00 * FIELD_REF(u8, work, 0xAu);
    FIELD_REF(s32, work, 0x10u) = D_801F11F4[edge].unk_04 * FIELD_REF(u8, work, 0xAu);
    func_801DD350(work);
    g_battle_work[0xB] = index;
    func_801EF280(keys);
    index++;
  } while (index < 8);
  func_800AF0F8(FIELD_REF(u32, D_801EB4E0, 0x34u),
                FIELD_REF(u32, D_801EB4E0, 0x38u), keys, packets, 8, 0x1C, 2);
  PopMatrix();
}
