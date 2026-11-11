#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Overlay-local vertex scratch shared by this body and the quad emitter it
 * calls: the eight-vertex table at 0x801FA240, whose 16-byte records the
 * emitter at 0x801F732C indexes with (index & 0xFF) << 4. The body walks the
 * seven first words of those records through its own cursor and stamps the
 * sixteen y and z words through the absolute cells declared below. */
extern s32 D_801FA240[];
extern s32 D_801FA244;
extern s32 D_801FA248;
extern s32 D_801FA254;
extern s32 D_801FA258;
extern s32 D_801FA264;
extern s32 D_801FA268;
extern s32 D_801FA274;
extern s32 D_801FA278;
extern s32 D_801FA284;
extern s32 D_801FA288;
extern s32 D_801FA294;
extern s32 D_801FA298;
extern s32 D_801FA2A4;
extern s32 D_801FA2A8;
extern s32 D_801FA2B4;
extern s32 D_801FA2B8;

/* @source 0x801AFE18 @kind unknown
 * Shared projection setup: it consumes the caller's 0x20-byte scratch buffer
 * and publishes the projection the per-record quad emitter reads.
 */
extern void func_801AFE18(void* arg0);

void drawTranslucentGouraudQuadFromFourIndexedRecordsWithTwoShadeLevelsScenarioScena0400_801F732C(
    u8 arg0, u8 arg1, u8 arg2, u8 arg3, s32 arg4, s32 arg5);

/* @source 0x801F7084
 * @behavior Drawing entry of this overlay's work-byte-1 handler run at
 *           0x801FA180: its three callers, entries 1, 3 and 4 at 0x801F6DD4,
 *           0x801F6F2C and 0x801F6FF8, pass the record at the work object's
 *           +0x0C, which carries two 16.16 points and their half-extents -
 *           point A at +0x00/+0x04/+0x08 with a y half-extent at +0x10 and a z
 *           half-extent at +0x14, and point B at +0x18/+0x1C/+0x20 with the
 *           matching pair at +0x28/+0x2C.
 *           It arms the emitter the way this overlay's drawing entries do:
 *           GetTPage(0, 1, 0x3C0, 0) yields the texture page, SetDrawMode
 *           installs it on the packet published at g_PrimCursor with dfe 0 and
 *           dtd 1, and func_8014E5A0 queues the 0xC-byte DR_MODE under OT slot
 *           1. It then stamps the eight 16-byte vertex records of the
 *           overlay-local table at 0x801FA240, x first and then y and z: the
 *           four records 0..3 hold point A, whose x word is +0x00 and whose y
 *           and z words are +0x04 plus and minus the +0x10 half-extent and
 *           +0x08 plus the +0x14 half-extent for records 0 and 1 while records
 *           2 and 3 take +0x08 without that addition, and records 4..7 repeat
 *           that pattern on point B, +0x18 as x, +0x1C plus and minus +0x28 as
 *           y and +0x20 plus +0x2C then +0x20 alone as z. It publishes the
 *           table through the shared projection setup at 0x801AFE18, passing
 *           the cursor the last record left at 0x801FA2A0, and then emits five
 *           vertex-indexed quads through the emitter at 0x801F732C, whose
 *           16-byte indexing makes the tuples (0, 1, 2, 3), (0, 1, 4, 5),
 *           (2, 0, 6, 4), (1, 3, 5, 7) and (2, 3, 6, 7) select the four faces
 *           of the box plus the one face that joins the two point pairs. The
 *           trailing pair of every emission is the grey-level pair 1 and 1,
 *           1 and 0, 1 and 0, 1 and 0, 1 and 0, so only the first quad carries
 *           two lit corners. It takes one argument, returns no value, and its
 *           0x20-byte frame keeps $s0 and $ra across the three calls; the
 *           constant 1 the emissions share is materialised into $s0 in the
 *           delay slot of the projection call. The cursor the projection call
 *           receives is taken before the final increment, so that increment
 *           folds into the last store's displacement and the call reuses the
 *           register that already holds 0x801FA2A0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFiveTranslucentGouraudQuadsOfBoxFromRecordPointsAndExtentsScenarioScena0400_801F7084(
    u8* record) {
  u16 tpage;
  s32* v;
  s32* scratch;

  tpage = GetTPage(0, 1, 0x3C0, 0);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, tpage, 0);
  func_8014E5A0(1, 0xC);
  v = D_801FA240;
  v[0] = *(s32*)(record + 0x00);
  D_801FA244 = *(s32*)(record + 0x04) + *(s32*)(record + 0x10);
  D_801FA248 = *(s32*)(record + 0x08) + *(s32*)(record + 0x14);
  v += 4;
  v[0] = *(s32*)(record + 0x00);
  D_801FA254 = *(s32*)(record + 0x04) - *(s32*)(record + 0x10);
  D_801FA258 = *(s32*)(record + 0x08) + *(s32*)(record + 0x14);
  v += 4;
  v[0] = *(s32*)(record + 0x00);
  D_801FA264 = *(s32*)(record + 0x04) + *(s32*)(record + 0x10);
  D_801FA268 = *(s32*)(record + 0x08);
  v += 4;
  v[0] = *(s32*)(record + 0x00);
  D_801FA274 = *(s32*)(record + 0x04) - *(s32*)(record + 0x10);
  D_801FA278 = *(s32*)(record + 0x08);
  v += 4;
  v[0] = *(s32*)(record + 0x18);
  D_801FA284 = *(s32*)(record + 0x1C) + *(s32*)(record + 0x28);
  D_801FA288 = *(s32*)(record + 0x20) + *(s32*)(record + 0x2C);
  v += 4;
  v[0] = *(s32*)(record + 0x18);
  D_801FA294 = *(s32*)(record + 0x1C) - *(s32*)(record + 0x28);
  D_801FA298 = *(s32*)(record + 0x20) + *(s32*)(record + 0x2C);
  v += 4;
  v[0] = *(s32*)(record + 0x18);
  D_801FA2A4 = *(s32*)(record + 0x1C) + *(s32*)(record + 0x28);
  D_801FA2A8 = *(s32*)(record + 0x20);
  scratch = v;
  v += 4;
  v[0] = *(s32*)(record + 0x18);
  D_801FA2B4 = *(s32*)(record + 0x1C) - *(s32*)(record + 0x28);
  D_801FA2B8 = *(s32*)(record + 0x20);
  func_801AFE18(scratch);
  drawTranslucentGouraudQuadFromFourIndexedRecordsWithTwoShadeLevelsScenarioScena0400_801F732C(0, 1, 2, 3, 1, 1);
  drawTranslucentGouraudQuadFromFourIndexedRecordsWithTwoShadeLevelsScenarioScena0400_801F732C(0, 1, 4, 5, 1, 0);
  drawTranslucentGouraudQuadFromFourIndexedRecordsWithTwoShadeLevelsScenarioScena0400_801F732C(2, 0, 6, 4, 1, 0);
  drawTranslucentGouraudQuadFromFourIndexedRecordsWithTwoShadeLevelsScenarioScena0400_801F732C(1, 3, 5, 7, 1, 0);
  drawTranslucentGouraudQuadFromFourIndexedRecordsWithTwoShadeLevelsScenarioScena0400_801F732C(2, 3, 6, 7, 1, 0);
}
