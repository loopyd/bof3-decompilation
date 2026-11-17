#include "bof3/world/area03004_internal.h"

extern u16 D_800100A6;   /* @source 0x800100A6 @kind unknown */
extern u16 D_800100A8;   /* @source 0x800100A8 @kind unknown */
extern u8 D_8014428A;    /* @source 0x8014428A @kind unknown */
extern s32 D_8014428C;   /* @source 0x8014428C @kind unknown */
extern s32 D_80144290;   /* @source 0x80144290 @kind unknown */
extern s32 D_80144298;   /* @source 0x80144298 @kind unknown */
extern s32 D_8014429C;   /* @source 0x8014429C @kind unknown */
extern s16 D_801442B6;   /* @source 0x801442B6 @kind unknown */
extern u16 D_801442B8;   /* @source 0x801442B8 @kind unknown */
extern s16 D_801442BA;   /* @source 0x801442BA @kind unknown */
extern u8 D_801C9E8A[];  /* @source 0x801C9E8A @kind unknown */

/* @source 0x801DCFD0
 * @behavior Decode shared pad direction and handle entry 2 of D_801E22F0.
 * Cancel plays cue 0x106, clears flag mask 2 and decrements work byte 4.
 * Otherwise flag bit 0 selects one of two record-list paths. Each draws its
 * fixed text row, handles confirmation and directional list movement, and
 * publishes a record value or 0xFFFF followed by the list marker position.
 * Confirmation tests the selected record's byte at stride 0x14 against 0x0B
 * or 0x0A, invokes the existing slot helpers when permitted, plays cue 0x103,
 * clears flag mask 2 and decrements work byte 4. The second path also handles
 * nine-row paging and its existing gate/countdown stores.
 * @status partial
 * @match 43.33
 * @residual Complete native bytes still differ. The original 1684-byte
 * interval remains retained until exact reconstruction.
 */
void updateSelectedRecordAndListPositionOnPadWorld00Area03004_801DCFD0(
    void) {
    u16* pad;
    u8* selected;
    u8* flag;
    u8* work;
    u8 record;
    u8 previous;
    u8 flags;
    u16 buttons;
    u16 page;
    u16 next_page;
    u16 cue;
    s32 direction;
    s32 position;
    s32 limit;
    s32 row;
    s32 count;

    pad = &D_80145AA8;
    direction = func_8014E640(*pad & 0x5000);
    if (*pad & D_80145AC4) {
        cue = 0x106;
        goto finish_selection;
    }

    if ((D_80144287 & 1) == 0) {
        func_8014F800(
            0x1D, 0x14, 0, 0xFFu, 0x80010000u + (u32)D_800100A6);

        if (*pad & D_80145AC2) {
            record = D_801451CC[D_801442B6];
            if (D_801C9E8A[record * 0x14] == 0x0B) {
                func_801665A0(3, record, 1, 0);
                selected = &D_80145028;
                previous = *selected;
                if (previous != 0) {
                    func_801650B4(3, previous, 1, 0);
                }
                *selected = record;
            }
            goto selection_confirmed;
        }

        if (direction & 0x1000) {
            if (D_8014428C != 0) {
                func_8015DF18(0x100);
                D_8014428C = (s32)((u32)D_8014428C - 1u);
            }
        } else if (direction & 0x4000) {
            if (D_8014428C < (s32)((u32)D_80144298 - 1u)) {
                func_8015DF18(0x100);
                D_8014428C = (s32)((u32)D_8014428C + 1u);
            }
        }

        record = D_801451CC[D_801442B6];
        if (D_801C9E8A[record * 0x14] == 0x0B) {
            selectedRecordValueState = record + 4;
        } else {
            selectedRecordValueState = 0xFFFF;
        }
        position = D_8014428C;
        goto draw_position;
    }

    func_8014F800(
        0x1D, 0x14, 0, 0xFFu, 0x80010000u + (u32)D_800100A8);

    if (D_8014428A == 0) {
        buttons = *pad;
        if (buttons & D_80145AC2) {
            record = D_801451CC[D_801442BA];
            if (D_801C9E8A[record * 0x14] != 0x0A) {
                goto selection_confirmed;
            }

            selected = &D_80145026;
            if (*selected == record) {
                goto selection_confirmed;
            }

            func_801665A0(3, record, 1, 0);
            previous = *selected;
            if (previous != 0) {
                func_801650B4(3, previous, 1, 0);
            }
            *selected = record;
            D_80145027 = 1;

            count = 0;
            for (row = 0; row < 0x80; row++) {
                record = D_801451CC[row];
                if (D_801C9E8A[record * 0x14] == 0x0A) {
                    count++;
                }
            }

            if (D_8014429C < count) {
                goto selection_confirmed;
            }
            page = D_801442B8;
            if (page != 0 && count - 9 < page) {
                D_801442B8 = page - 1;
            }
            goto selection_confirmed;
        }

        if (direction & 0x1000) {
            position = D_80144290;
            if (position == 0) {
                if (D_801442B8 != 0) {
                    func_8015DF18(0x100);
                    gateByteWorld00Area03004_80144288 = 4;
                    page = D_801442B8;
                    D_8014428A = 3;
                    D_801442B8 = page - 1;
                }
            } else {
                func_8015DF18(0x100);
                D_80144290 = (s32)((u32)D_80144290 - 1u);
            }
        } else if (direction & 0x4000) {
            position = D_80144290;
            if (position == 8) {
                page = D_801442B8;
                limit = D_8014429C;
                if (page + 8 < limit) {
                    func_8015DF18(0x100);
                    page = D_801442B8;
                    gateByteWorld00Area03004_80144288 = 0;
                    D_8014428A = 3;
                    D_801442B8 = page + 1;
                }
            } else if (position < D_8014429C) {
                func_8015DF18(0x100);
                D_80144290 = (s32)((u32)D_80144290 + 1u);
            }
        } else if (buttons & 4) {
            page = D_801442B8;
            next_page = (u16)(page - 9);
            D_801442B8 = next_page;
            if ((s16)next_page < 0) {
                D_801442B8 = 0;
                if (page == 0) {
                    if (D_80144290 != 0) {
                        func_8015DF18(0x100);
                        D_80144290 = 0;
                    }
                    goto publish_second_record;
                }
            }
            func_8015DF18(0x100);
        } else if (buttons & 8) {
            limit = D_8014429C;
            if (limit >= 9) {
                page = D_801442B8;
                next_page = (u16)(page + 9);
                D_801442B8 = next_page;
                if (next_page + 8 >= limit) {
                    next_page = (u16)(limit - 8);
                    D_801442B8 = next_page;
                    if (next_page == page) {
                        if (D_80144290 < 8) {
                            func_8015DF18(0x100);
                            D_80144290 = 8;
                        }
                        goto publish_second_record;
                    }
                }
                func_8015DF18(0x100);
            } else if (D_80144290 < limit) {
                func_8015DF18(0x100);
                D_80144290 = D_8014429C;
            }
        }
    }

publish_second_record:
    record = D_801451CC[D_801442BA];
    if (D_801C9E8A[record * 0x14] == 0x0A) {
        selectedRecordValueState = record + 4;
    } else {
        selectedRecordValueState = 0xFFFF;
    }
    position = D_80144290;

draw_position:
    func_801647C4(
        0xA8, (u16)((u32)position * 13u + 0x5Au), 0);
    return;

selection_confirmed:
    cue = 0x103;

finish_selection:
    func_8015DF18(cue);
    flag = &D_80144287;
    flags = *flag;
    work = D_1F800044;
    *flag = flags & 0xFD;
    work[4] = work[4] - 1;
}
