#include "bof3/bof3.h"

/* @source 0x80143F03 @kind unknown */
extern u8 D_80143F03;
/* @source 0x801468C0 @kind unknown */
extern s32 D_801468C0[];
/* @source 0x801468BC @kind unknown */
extern s32 D_801468BC[];
/* @source 0x80145EC8 @kind unknown */
extern s32 D_80145EC8;
/* @source 0x80145EC4 @kind unknown */
extern s32 D_80145EC4;
extern void setProgress1FAndRequestPrimaryState7ScenarioScena0200_801FCB54(void);

/* @source 0x801FC518
 * @behavior When D_80143F03 is zero, test the shared signed word pair against
 * bounds from the selected 152-byte-stride record. D_80145EC8 must lie between
 * the indexed word at 0x801468C0 minus 0x20000 and that word plus 0x28000.
 * D_80145EC4 must lie between the indexed word at 0x801468BC and that word
 * plus 0x40000. All bounds are inclusive. If all tests pass, call the helper
 * that sets progress 0x1F and requests primary state 7. The caller passes
 * index 3; only its low byte is consumed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState7OnIndexedOffsetBoundsScenarioScena0200_801FC518(u8 index) {
    s32 offset;
    s32 bound;
    s32 current;
    s32 second_current;
    s32 second_bound;

    if (D_80143F03 != 0) {
        return;
    }
    offset = index * 38;
    bound = D_801468C0[offset];
    current = D_80145EC8;
    if (bound + 0x28000 < current || current < bound - 0x20000) {
        return;
    }
    second_current = D_80145EC4;
    second_bound = D_801468BC[offset];
    if (second_current < second_bound || second_bound + 0x40000 < second_current) {
        return;
    }
    setProgress1FAndRequestPrimaryState7ScenarioScena0200_801FCB54();
}
