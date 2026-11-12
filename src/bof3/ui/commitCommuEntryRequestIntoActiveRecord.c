#include "bof3/bof3.h"

/* Current commu request of the commu01/07 entry handlers: mode byte plus its
 * two parameter bytes. */
extern u8 D_801D4DF4; /* @source 0x801D4DF4 @kind data */
extern u8 D_801D4DF8; /* @source 0x801D4DF8 @kind data */
extern u8 D_801D4DFC; /* @source 0x801D4DFC @kind data */

/* Snapshot of the request bytes above, taken by the confirm path at 0x801D0CAC. */
extern u8 D_801D4E00; /* @source 0x801D4E00 @kind data */
extern u8 D_801D4E04; /* @source 0x801D4E04 @kind data */
extern u8 D_801D4E08; /* @source 0x801D4E08 @kind data */

/* Pending world id; the commu entry count of a world is one byte of the table at
 * 0x801F286D indexed by it. */
extern u16 D_80143F00; /* @source 0x80143F00 @kind data */
extern u8 D_801F286D[]; /* @source 0x801F286D @kind data */

/* Shared commu list cursor: the list row this overlay's handlers act on. */
extern u8 D_801448ED; /* @source 0x801448ED @kind data */

/* Request row byte and ordinal byte of the current commu request. */
extern u8 D_801D4E28; /* @source 0x801D4E28 @kind data */
extern u8 D_801D4E24; /* @source 0x801D4E24 @kind data */

/* Byte views of the shared stride-8 active records at 0x801455C8: byte 1 is the
 * record kind, bytes 2 and 3 its state bytes and the word at +4 its
 * progress-anchor word. */
extern u8 D_801455C9[]; /* @source 0x801455C9 @kind data */
extern u8 D_801455CA[]; /* @source 0x801455CA @kind data */
extern u8 D_801455CB[]; /* @source 0x801455CB @kind data */
extern u8 D_801455CC[]; /* @source 0x801455CC @kind data */

/* Shared elapsed battle counter, written as the record progress anchor. */
extern u32 D_8014502C; /* @source 0x8014502C @kind data */

/* Overlay step byte the enclosing handler run advances as its request completes. */
extern u8 D_801F294C; /* @source 0x801F294C @kind data */

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; consumes one u32 cue ID.
 */
extern void dispatchSoundCue(u32 cue_id);

u8 countActiveRecordsByKind(s32 kind);
u8 findActiveRecordSlotByKind(u8 kind, u8 ordinal);

/* @source 0x801D482C
 * @behavior Commits the current commu request of the commu01/07 entry handlers
 * into the shared stride-8 active records at 0x801455C8 and reports whether the
 * commit went through. It switches on the request mode byte 0x801D4DFC. Mode 1
 * first requires fewer than three in-use records whose kind byte 0x801455C9
 * equals the request row byte 0x801D4E28, and a pending world commu entry count,
 * the byte at 0x801F286D indexed by the world id halfword 0x80143F00, that is
 * not below that request row read as a signed byte. Modes 0 and 2 take no such
 * check, and any other mode value reports no commit. Every mode that reaches the
 * commit body leaves when the request row byte equals the commu list row
 * 0x801448ED. Otherwise it copies the saved request bytes 0x801D4E00, 0x801D4E04
 * and 0x801D4E08 into the current request bytes 0x801D4DF4, 0x801D4DF8 and
 * 0x801D4DFC, takes the active-record slot that findActiveRecordSlotByKind
 * returns for the list row 0x801448ED and the request ordinal 0x801D4E24 less
 * the offset this function is handed, and writes into that slot at eight times
 * the slot index: the request row byte as its kind byte at +1, zero into its
 * state bytes at +2 and +3, and the shared battle counter 0x8014502C as its
 * progress-anchor word at +4. It then rolls the overlay step byte 0x801F294C
 * back by one and records the commit. Each path queues resident sound cue 0x103
 * when the commit happened and 0x107 when it did not, through the shared cue
 * dispatcher 0x8015DF18, and returns the commit flag, 0 or 1, as a signed word.
 * The exact callers 0x801D1630 and 0x801D1158 pass offset 1 and offset 0 and
 * test the returned low byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 commitCommuEntryRequestIntoActiveRecord(s32 ordinalOffset) {
    s32 committed;
    u8 slot;
    u8 step;
    u32 anchor;

    committed = 0;
    switch (D_801D4DFC) {
    case 1:
        if (countActiveRecordsByKind(D_801D4E28) == 3) {
            break;
        }
        if (D_801F286D[D_80143F00] < (s8)D_801D4E28) {
            break;
        }
        /* fall through */
    case 0:
    case 2:
        if ((s8)D_801D4E28 == D_801448ED) {
            break;
        }
        D_801D4DF4 = D_801D4E00;
        D_801D4DF8 = D_801D4E04;
        D_801D4DFC = D_801D4E08;
        slot = findActiveRecordSlotByKind(D_801448ED, D_801D4E24 - ordinalOffset);
        D_801455C9[slot << 3] = D_801D4E28;
        anchor = D_8014502C;
        D_801455CA[slot << 3] = 0;
        D_801455CB[slot << 3] = 0;
        step = D_801F294C - 1;
        *(u32 *)(D_801455CC + (slot << 3)) = anchor;
        D_801F294C = step;
        committed = 1;
        break;
    }
    dispatchSoundCue((committed & 0xFF) ? 0x103 : 0x107);
    return committed;
}
