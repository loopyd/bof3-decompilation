#include "bof3/bof3.h"

/* Shared commu queue count in main RAM, outside every overlay payload: the
 * number of entries in the list section this overlay draws second. Step 0 of
 * this overlay's handler run turns it into that section's row count at 0x14
 * entries per row, and the third section takes its own count from the sibling
 * cell 0x80145E5D. */
extern u8 D_80145E5C; /* @source 0x80145E5C @kind data */

/* One-byte entry run of that section in main RAM: the byte at a position selects
 * one label of the five-byte-stride name table below. This body indexes the run
 * with a stride of 0xF while the row position it tests the count with advances
 * 0x14 per row, so the run is bound as flat bytes. */
extern u8 D_80145E48[]; /* @source 0x80145E48 @kind table */

/* Five-byte-stride label table in main RAM. The sibling row drawer at
 * 0x801D1048 hands the address of one entry to func_8014F800 as a row's label
 * pointer, so an entry is a five-character label. */
extern u8 D_801457E8[][5]; /* @source 0x801457E8 @kind table */

/* Shared text buffer the section count is printed into before it is submitted;
 * the sibling copy routine at 0x801D39CC fills it in the same shape. */
extern u8 D_80145AD4[]; /* @source 0x80145AD4 @kind unknown */

/* Two-digit field format "%2d" in the data run at the head of this overlay's
 * own payload, beside the "%d/%d" format its step 1 also prints through. */
extern const char D_801D0C0C[]; /* @source 0x801D0C0C @kind data */

/* The section title as this overlay's data run holds it: the plural "faeries"
 * first and the singular "fairy" 8 bytes later. */
extern const char D_801D196C[]; /* @source 0x801D196C @kind data */
extern const char D_801D1974[]; /* @source 0x801D1974 @kind data */

/* Fixed halfwords of the image offset table that sits on the 0x80010000 main-RAM
 * base: added to that base each is the handle func_8014F800 takes as its fifth
 * argument, the same way a row handle is formed. */
extern u16 D_8001006E; /* @source 0x8001006E @kind data */
extern u16 D_80010076; /* @source 0x80010076 @kind data */

/* @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* @source 0x8014FF0C */
extern void func_8014FF0C(s16 arg0, s16 arg1, s32 arg2, const void* arg3);

/* @source 0x801D0F50 */
extern void drawRowIconSpriteCommu0507(s16 arg0, s16 arg1, u8 arg2);

/* @source 0x801D129C
 * @behavior Submits the header and entry grid of the second of this overlay's
 *           three list sections, the one whose row drawer the step 1 dispatcher
 *           at 0x801D0D4C selects when its walk lands on section 1 and calls it
 *           with (0x28, 0x28, row). It first submits the section image through
 *           func_8014F800 at (arg0 + 0x28, arg1) with colour 0 and flag 0xFF, the
 *           handle being the 0x80010000 main-RAM base plus the fixed table
 *           halfword at 0x8001006E, draws the section icon 0 through
 *           drawRowIconSpriteCommu0507 at (arg0 + 8, arg1 + 0x14), and submits a
 *           second image at (arg0 + 0x18, arg1 + 0x14) with the halfword at
 *           0x80010076 on the same base. It prints the section's entry count, the
 *           shared count byte at 0x80145E5C, through the overlay format "%2d" at
 *           0x801D0C0C into the shared text buffer at 0x80145AD4 and submits that
 *           buffer through func_8014FF0C at (arg0 + 0x6C, arg1 + 0x14), then
 *           submits the section title at (arg0 + 0x84, arg1 + 0x14) with mask 7:
 *           the plural "faeries" at 0x801D196C, or the singular "fairy" at
 *           0x801D1974 once the count byte reads 1. It then walks up to 0x14
 *           entries of a grid three columns wide and seven rows deep, stopping at
 *           the first entry whose position, i + arg2 * 0x14, the count byte does
 *           not reach. Entry i sits at column i % 3 and row i / 3, so it is
 *           submitted through func_8014F800 at (arg0 + 0x12 + 0x48 * column,
 *           arg1 + 0x2C + 0x10 * row) with colour 0 and mask 5, carrying the
 *           five-character label that the entry byte at 0x80145E48 + arg2 * 0xF +
 *           i selects from the name table at 0x801457E8. The count byte is re-read
 *           on every loop test, the entry byte is read once per entry, and the
 *           body writes no memory of its own and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFairySectionRowsCommu0507(s16 arg0, s16 arg1, u8 arg2) {
    s32 i;

    func_8014F800(arg0 + 0x28, arg1, 0, 0xFF, 0x80010000u + D_8001006E);
    drawRowIconSpriteCommu0507(arg0 + 8, arg1 + 0x14, 0);
    func_8014F800(arg0 + 0x18, arg1 + 0x14, 0, 0xFF, 0x80010000u + D_80010076);
    sprintf((char *)D_80145AD4, (const char *)D_801D0C0C, D_80145E5C);
    func_8014FF0C(arg0 + 0x6C, arg1 + 0x14, 0, D_80145AD4);
    if (D_80145E5C == 1) {
        func_8014F800(arg0 + 0x84, arg1 + 0x14, 0, 7, (u32)D_801D1974);
    } else {
        func_8014F800(arg0 + 0x84, arg1 + 0x14, 0, 7, (u32)D_801D196C);
    }

    for (i = 0; i < 0x14; i++) {
        if (i + arg2 * 20 >= D_80145E5C) {
            break;
        }
        func_8014F800(arg0 + 0x12 + (i % 3) * 0x48,
                      arg1 + 0x2C + (i / 3) * 0x10, 0, 5,
                      (u32)D_801457E8[D_80145E48[arg2 * 15 + i]]);
    }
}
