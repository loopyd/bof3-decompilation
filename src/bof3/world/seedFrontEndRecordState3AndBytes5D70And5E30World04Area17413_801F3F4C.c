#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materialises its address
 * and re-reads it for every store, so the source keeps the named symbol form.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;
/* Front-end record table base 0x80143FC8 and the field bases of its 0x74-byte
 * rows. The original re-materialises `lui at,0x8014` for every store, so each
 * field is addressed as its own address-derived symbol rather than through one
 * struct base.
 * @source 0x80143FC9 @kind unknown
 */
extern u8 D_80143FC9[];
/* @source 0x80144025 @kind unknown */
extern u8 D_80144025[];
/* @source 0x80144026 @kind unknown */
extern u8 D_80144026[];

/* @source 0x801F3F4C
 * @behavior Seeds the row of the shared 0x74-byte front-end record table at
 * 0x80143FC8 that the work byte 0x0B of the work object published at scratchpad
 * 0x1F800044 selects: record byte 0x01 = 3, record byte 0x5D = 0x70 and record
 * byte 0x5E = 0x30. Every store re-reads the cursor cell and then the index
 * byte, as the original does, so the scaled record address is rebuilt for each
 * store. It takes no arguments, returns nothing, calls nothing and touches no
 * other state. Nothing in this payload branches or calls into it, so its entry
 * point is reachable only from outside the overlay; the neighbouring
 * 0x801F3E68 seeds the same table with its own 1/0x9D/1 and derived field
 * values, and this 0x74-byte table is the one the scenario overlays already
 * call the front-end record table, selected in both by work byte 0x0B.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedFrontEndRecordState3AndBytes5D70And5E30World04Area17413_801F3F4C(
    void) {
  D_80143FC9[(u32)D_1F800044[0x0B] * 0x74u] = 3u;
  D_80144025[(u32)D_1F800044[0x0B] * 0x74u] = 0x70u;
  D_80144026[(u32)D_1F800044[0x0B] * 0x74u] = 0x30u;
}
