#include "bof3/bof3.h"

/* One 164-byte row of the main-RAM record table at 0x80144968. Its five
 * leading bytes are the record's card run: this body copies them out and writes
 * the dealt run back over them, and the companion body at 0x801DA8E8 installs
 * the same run into the five-byte active-record rows of the same table area. */
typedef struct MainRamRecordRow {
    u8 cards[164];
} MainRamRecordRow;

/* Card-id work run of this overlay: the deal routine at 0x801D3864 fills its
 * leading bytes with distinct ids in 1 through 9 and the card draw bodies read
 * them as ids, 0xFF standing for a face-down slot. */
extern u8 D_801DB080[]; /* @source 0x801DB080 @kind data */

/* Record ordinal of this overlay: the card bodies pass it to the slot finders
 * that map it onto the record tables. */
extern u8 D_801DB0B4; /* @source 0x801DB0B4 @kind data */

/* Base of the 0xAC-byte stride main-RAM record table whose five leading bytes
 * hold the card run committed here. */
extern u8 D_80144968[]; /* @source 0x80144968 @kind data */

/* First shared work buffer of the main executable, holding the five record
 * bytes this body snapshots. */
extern u8 D_801490D8[]; /* @source 0x801490D8 @kind data */

/* Second shared work buffer of the main executable, holding the six card-run
 * bytes this body snapshots. */
extern u8 D_801490F8[]; /* @source 0x801490F8 @kind data */

/* Frontend mode byte of the main executable; 2 marks the frontend busy. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Step group of this overlay: byte 0 selects the handler run the step
 * dispatcher indexes, bytes 1 and 2 the handler of its byte-1 view. */
extern u8 D_801F294C; /* @source 0x801F294C @kind data */
extern u8 D_801F294D; /* @source 0x801F294D @kind data */
extern u8 D_801F294E; /* @source 0x801F294E @kind data */

/* Action starter of the main executable: runs the action the id names. */
extern void func_80150224(s32 action_id); /* @source 0x80150224 */

extern u8 findFlaggedFieldRecordSlot(u8 ordinal);

/* @source 0x801DA7A4
 * @behavior Snapshots the selected flagged field record and the dealt card run,
 *           commits the run into that record, then restarts the card step. In
 *           each of the two five-byte loops it asks
 *           findFlaggedFieldRecordSlot for the record of the ordinal held at
 *           0x801DB0B4 and walks the 0x80144968 + 0xAC * slot row of the record
 *           table. The first loop copies the five record card bytes into the
 *           shared buffer at 0x801490D8 and clears that buffer's terminator at
 *           0x801490DD; the loop of six bytes copies the card run at 0x801DB080
 *           into the shared buffer at 0x801490F8; the third five-byte loop
 *           writes the same card run back over the record prefix, so the record
 *           ends up holding the dealt run. Action 0xF5 is then started through
 *           func_80150224, the frontend mode byte at 0x80143BB0 is latched to 2,
 *           and the step group at 0x801F294C is set to 3 with its bytes 1 and 2
 *           cleared. Nothing is returned, and its only callers are the two card
 *           confirm bodies at 0x801D7648 and 0x801D7C54, each of which calls it
 *           once the overlay phase byte 0x801DB0C4 reaches 4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void commitCardRunIntoFlaggedFieldRecordAndStartActionF5(void) {
    s32 i;

    for (i = 0; i < 5; i++) {
        D_801490D8[i] =
            ((MainRamRecordRow*)D_80144968)[findFlaggedFieldRecordSlot(
                D_801DB0B4)]
                .cards[i];
    }
    D_801490D8[5] = 0;
    for (i = 0; i < 6; i++) {
        D_801490F8[i] = D_801DB080[i];
    }
    for (i = 0; i < 5; i++) {
        ((MainRamRecordRow*)D_80144968)[findFlaggedFieldRecordSlot(
            D_801DB0B4)]
            .cards[i] = D_801DB080[i];
    }
    func_80150224(0xF5);
    D_80143BB0 = 2;
    D_801F294C = 3;
    D_801F294D = 0;
    D_801F294E = 0;
}
