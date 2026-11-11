#include "bof3/bof3.h"

/* Main-RAM cells shared with the other overlays. 0x80145E5D is the append
 * index of the commu pending-notification queue, advanced once per
 * notification stored. A notification is the byte pair at 0x80145E60 + n * 2:
 * its first byte is the notification kind and its second byte selects the row
 * label. The commu00 lifts record the same roles for that queue, and this
 * overlay's own step 0 reads the append index as the pending count. */
typedef struct Commu0507Notification {
    u8 kind;
    u8 label;
} Commu0507Notification;

extern volatile u8 D_80145E5D; /* @source 0x80145E5D @kind unknown */
/* @source 0x80145E60 @kind unknown */
extern volatile Commu0507Notification D_80145E60[];

/* Five-byte-stride label table in main RAM. The sibling row drawer at
 * 0x801D1048 hands the address of one entry to func_8014F800 as a row's label
 * pointer, so an entry is a five-character label. */
extern u8 D_801457E8[][5]; /* @source 0x801457E8 @kind table */

/* Shared name buffer, twelve bytes wide. The sibling copy routine at 0x801D39CC
 * fills its first five bytes and clears byte 5 in the same shape. */
extern u8 D_801490D8[]; /* @source 0x801490D8 @kind unknown */

/* Fixed halfword of the image offset table that sits on the 0x80010000 main-RAM
 * base: added to that base it is the handle func_8014F800 takes as its fifth
 * argument, the same way a row handle is formed. */
extern u16 D_800101D8; /* @source 0x800101D8 @kind data */

/* This overlay's per-kind row table, held in the data run based at 0x801D1950:
 * the text id that selects the row image from the 0x80010000 table, followed by
 * the flag that gates the row label copy. On disc the eight entries hold text
 * ids 0xD9 through 0xE0 with the flag set on the first four. */
typedef struct Commu0507RowEntry {
    u16 textId;
    u16 labelFlag;
} Commu0507RowEntry;

/* @source 0x801D197C @kind table */
extern Commu0507RowEntry D_801D197C[];

/* @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* @source 0x801D14C8
 * @behavior Submits the panel and the eight pending-notification rows of this
 *           overlay's fairy list. It first submits the panel image through
 *           func_8014F800 at (arg0 + 0x52, arg1) with colour 0 and flag 0xFF,
 *           the handle being the 0x80010000 main-RAM base plus the fixed table
 *           halfword at 0x800101D8. It then walks up to eight queue entries
 *           and stops at the first row whose position, arg2 * 8 + i, the
 *           pending append index at 0x80145E5D does not reach. Per row it
 *           reads the notification kind from 0x80145E60 and looks that kind up
 *           in the local eight-entry table at 0x801D197C: when the entry flag
 *           is set it copies eight bytes of the five-byte-stride label run at
 *           0x801457E8 selected by the notification's second byte into the
 *           shared name buffer at 0x801490D8 and clears that buffer's byte 5,
 *           and it always submits the row image at (arg0 + 0x14,
 *           arg1 + i * 0x10 + 0x1C) with the handle taken from the 0x80010000
 *           table entry the row text id names. Both the pending index and the
 *           queue bytes are re-read as volatile cells, so the kind is loaded
 *           once for the flag test and again for the row handle; the name
 *           buffer is the only memory this function writes.
 * @status partial
 * @match 98.11
 * @residual 104/106 instructions and 424 of 424 bytes: the row's y argument
 *           sums the row stride product and arg1 last in the original, while
 *           every spelling of that sum resolves to the same value with the
 *           0x1C constant grouped onto arg1, so the constant's addiu lands on
 *           the other operand (two instructions differ in operand order, byte
 *           count already equal). A source-level local for the row offset
 *           reproduces the original order but costs a callee-saved register
 *           and 12 extra bytes, because the compiler then reduces it to a
 *           stride-0x10 giv.
 */
void drawPendingNotificationRowsCommu0507(s16 arg0, s16 arg1, u8 arg2) {
    s32 index;
    s32 i;
    s32 j;

    func_8014F800(arg0 + 0x52, arg1, 0, 0xFF, 0x80010000u + D_800101D8);

    for (i = 0; i < 8; i++) {
        index = arg2 * 8 + i;
        if (index >= D_80145E5D) {
            break;
        }
        if (D_801D197C[D_80145E60[index].kind].labelFlag != 0) {
            for (j = 0; j < 8; j++) {
                D_801490D8[j] = D_801457E8[D_80145E60[index].label][j];
            }
            D_801490D8[5] = 0;
        }
        func_8014F800(arg0 + 0x14, (i * 0x10 + 0x1C) + arg1, 0, 0xFF,
                      0x80010000u + ((u16*)0x80010000)[D_801D197C[D_80145E60[arg2 * 8 + i].kind].textId]);
    }
}
