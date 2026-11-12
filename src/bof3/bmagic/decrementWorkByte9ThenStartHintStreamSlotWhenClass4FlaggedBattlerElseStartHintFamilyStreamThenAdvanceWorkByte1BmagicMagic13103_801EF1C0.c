#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers. The address folds into each load (lui $vN,0x1F80 / lw
 * $vN,68($vN)), so the two reads this body makes stay separate.
 * @source 0x1F800044 @kind data */
extern u8 *g_battle_work;

/* Shared main-executable bytes outside this overlay's image: the EMI stream hint
 * byte at 0x80145024, the local work-record count at 0x80146254 and the battle
 * mode byte at 0x801462EC whose 0x2 bit selects the second variant of every slot
 * this body starts. */
extern u8 D_80145024; /* @source 0x80145024 @kind unknown */
extern u8 D_80146254; /* @source 0x80146254 @kind unknown */
extern u8 D_801462EC; /* @source 0x801462EC @kind unknown */

/* Fields of the local work record based at 0x80145E90, 0x140 bytes per record:
 * the class byte at +0x79 and the flag word at +0x128. The original reaches each
 * with its own displacement (lui $at,%hi(X) / addu at,at,index*0x140 / load
 * %lo(X)(at)), so both keep a flat symbol instead of one shared record base. */
extern u8 D_80145F09[]; /* @source 0x80145F09 @kind data */
extern u8 D_80145FB8[]; /* @source 0x80145FB8 @kind data */

/* Shared main-executable helpers, both outside this payload and both bound by
 * the generated undefined_funcs_auto.txt rather than by a map row: the EMI
 * loader slot opener, and the wrapper that records a hint byte and starts that
 * family's variant slot (family 1 base 0x1DB, family 2 base 0x1EE). */
extern void func_80161FDC(u32 slot_id); /* @source 0x80161FDC */
extern void func_8016728C(u8 index, u8 family); /* @source 0x8016728C */

/* @source 0x801EF1C0
 * @behavior Entry index 4 (the word at 0x801EEC14) of the eight-entry code
 *           pointer run at 0x801EEC04 that the overlay frame dispatcher
 *           func_801EEC24 copies to its stack and calls through the work
 *           object's byte 1, so it runs as that stage handler. It reads the
 *           scratchpad work-object pointer cell at 0x1F800044 once and
 *           decrements work byte 9 at byte width (lbu / addiu -1 / sb); the
 *           guard compares the stored register truncated to eight bits
 *           (andi / bnez), so no second cell read supplies the compared value.
 *           Only when that countdown reaches zero does it scan the D_80146254
 *           local work records, 0x140 bytes apart, setting a found flag
 *           whenever a record's class byte at +0x79 (0x80145F09) is 4 and its
 *           flag word at +0x128 (0x80145FB8) has bit 0 set; the scan does not
 *           stop at the first match, so it tests every record.
 *           When the flag is set it compares the hint byte 0x80145024 against
 *           7, 0xD, 0xE and 0xF in four separate statements and, for each hit,
 *           starts the EMI stream slot 0x201, 0x202, 0x203 or 0x204, or the
 *           variant 0x205, 0x206, 0x207 or 0x208 when the battle mode byte
 *           0x801462EC has its 0x2 bit set; the last statement emits its two
 *           calls separately instead of one call with a selected argument.
 *           Otherwise it tests the same mode bit and calls the shared wrapper
 *           at 0x8016728C from each arm with the hint byte masked to seven bits
 *           and the family selector 2 (mode bit set) or 1 (clear), which
 *           records the hint byte and starts that family's variant slot; the
 *           two calls differ only in the selector, so the compiler keeps one
 *           call and duplicates the hint load across the exclusive arms. Every
 *           statement re-reads the hint byte and the mode byte after a call, so
 *           each test keeps its own load. It then re-reads the scratchpad cell
 *           and increments work byte 1, advancing the dispatcher to the next
 *           table entry. Takes no arguments, returns nothing, and the 0x18-byte
 *           frame only keeps $ra at 0x10 across the five calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenStartHintStreamSlotWhenClass4FlaggedBattlerElseStartHintFamilyStreamThenAdvanceWorkByte1BmagicMagic13103_801EF1C0(void) {
    u8 *work;
    u8 countdown;
    u8 found;
    u8 i;
    u8 count;
    u32 offset;
    u32 slot;

    work = g_battle_work;
    countdown = work[9] - 1;
    work[9] = countdown;
    if (countdown == 0) {
        found = 0;
        count = D_80146254;
        for (i = 0; i < count; i++) {
            offset = i * 0x140;
            if ((D_80145F09[offset] == 4) &&
                ((*(u32 *)&D_80145FB8[offset] & 1) != 0)) {
                found = 1;
            }
        }
        if (found != 0) {
            if (D_80145024 == 7) {
                if ((D_801462EC & 2) != 0) {
                    slot = 0x205;
                } else {
                    slot = 0x201;
                }
                func_80161FDC(slot);
            }
            if (D_80145024 == 0xD) {
                if ((D_801462EC & 2) != 0) {
                    slot = 0x206;
                } else {
                    slot = 0x202;
                }
                func_80161FDC(slot);
            }
            if (D_80145024 == 0xE) {
                if ((D_801462EC & 2) != 0) {
                    slot = 0x207;
                } else {
                    slot = 0x203;
                }
                func_80161FDC(slot);
            }
            if (D_80145024 == 0xF) {
                if ((D_801462EC & 2) != 0) {
                    func_80161FDC(0x208);
                } else {
                    func_80161FDC(0x204);
                }
            }
        } else {
            if ((D_801462EC & 2) != 0) {
                func_8016728C(D_80145024 & 0x7F, 2);
            } else {
                func_8016728C(D_80145024 & 0x7F, 1);
            }
        }
        work = g_battle_work;
        work[1]++;
    }
}
