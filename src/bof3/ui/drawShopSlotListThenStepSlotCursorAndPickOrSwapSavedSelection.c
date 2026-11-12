#include "bof3/ui/shop00_internal.h"

/* @source 0x801DE4C8
 * @behavior phase handler at entry 2 of the shop phase table D_801E5BEC, which
 *           the dispatcher func_801DE188 indexes with the UI phase byte
 *           D_80148651: it draws the slot list with its selection cursors
 *           through drawShopSlotListWithSelectionCursors, then submits the shop
 *           panel row through func_8014F800 at 0x1A by 0x14 with the texture
 *           0x60 while the published saved-selection id D_801E6254 still holds
 *           the 0xFF no-selection sentinel, and with 0x61 plus that id's
 *           164-byte field record D_80144968[164 * D_80181B10[id]] through
 *           func_8016620C(0, 5, record) once a saved id exists. It then reads
 *           the pad direction func_8014E640 returns for the 0xF000 mask of the
 *           pad state D_80145AA8 and the visible-list length func_801BDB7C(0):
 *           a 0x8000 or 0x2000 direction while the off-screen list D_801E6250
 *           holds a slot flips the cursor kind byte D_80148658 between the
 *           visible id list and the off-screen one, and the cursor slot byte
 *           D_80148659 is then clamped to the last slot of whichever list that
 *           kind selects. Direction bit 0x1000 decrements the slot byte and
 *           wraps it to that list's last slot once it turns negative, while bit
 *           0x4000 increments it and wraps it back to zero once it passes the
 *           last slot; when either step moved the cursor pair the handler plays
 *           the cue 0x100. When the pad state shares the confirm mask
 *           D_80145AC2 it plays cue 0x103 and copies the cursor into the saved
 *           pair D_8014865A/D_8014865D while that pair still holds the 0x7F
 *           sentinel, and otherwise publishes the id swapSelectionIds returns
 *           into D_801E6254, plays cue 0x107 for a real id or cue 0x103 for the
 *           0xFF sentinel, and re-arms the saved pair with 0x7F. When the pad
 *           state shares the cancel mask D_80145AC4 instead it plays cue 0x106
 *           and clears the saved pair, or, with nothing saved, clears the UI
 *           sub-step byte D_80148652, increments the UI phase byte D_80148651
 *           and switches the frontend-local mode through func_8014ECAC with
 *           the constant 2.
 * @status partial
 * @match 87.61
 * @residual The 0x4000 increment arm schedules six instructions shorter than
 *           the original: this candidate keeps the incremented byte in a live
 *           pointer local, so psyq fills both load-delay slots and the branch
 *           delay slot and rematerializes the (s8) cast once per arm, while the
 *           original leaves a nop after each of its two byte loads and one more
 *           after the kind branch and carries a single sign extension into both
 *           comparisons. The arithmetic, both list bounds and both wrap stores
 *           agree; every other instruction of the function matches, and the
 *           branch displacements that follow the arm are the same branches
 *           shifted by the same delta. Thirteen further clean-C spellings of
 *           that arm (a shared sign-extended local, plain cell comparison, one
 *           increment per arm and a per-arm array view among them) measured
 *           between 183 and 200 matched instructions with the arm unmoved, so
 *           the residual is scheduling, not expression.
 */
void drawShopSlotListThenStepSlotCursorAndPickOrSwapSavedSelection(void) {
  s32 direction;
  u8 count;
  u8 kind;
  u8 slot;
  u8 index;

  drawShopSlotListWithSelectionCursors();
  if (D_801E6254 == 0xFF) {
    func_8014F800(0x1A, 0x14, 0, 0xFF, func_801502D0(0x60));
  } else {
    func_8016620C(0, 5, (u32)(D_80144968 + 164 * D_80181B10[D_801E6254]));
    func_8014F800(0x1A, 0x14, 0, 0xFF, func_801502D0(0x61));
  }

  direction = func_8014E640(D_80145AA8 & 0xF000);
  count = func_801BDB7C(0);
  kind = D_80148656[2];
  slot = D_80148656[3];

  if (direction & 0xA000) {
    if (D_801E6250 != 0) {
      D_80148656[2] = kind ^ 1;
    }
  }

  if (D_80148658 == 0) {
    if (count - 1 < D_80148659) {
      D_80148659 = count - 1;
    }
  } else {
    if (D_801E6250 - 1 < D_80148659) {
      D_80148659 = D_801E6250 - 1;
    }
  }

  if (direction & 0x1000) {
    u8* cur;

    cur = &D_80148656[3];
    index = *cur - 1;
    *cur = index;
    if ((s8)index < 0) {
      if (D_80148658 == 0) {
        index = count - 1;
      } else {
        index = D_801E6250 - 1;
      }
      *cur = index;
    }
  } else if (direction & 0x4000) {
    u8* cur;

    cur = &D_80148656[3];
    index = *cur + 1;
    *cur = index;
    if (D_80148658 == 0) {
      if ((s8)index > count - 1) {
        D_80148659 = 0;
      }
    } else {
      if ((s8)index > D_801E6250 - 1) {
        D_80148659 = 0;
      }
    }
  }

  if (kind != D_80148656[2] || slot != D_80148656[3]) {
    func_8015DF18(0x100);
  }

  if (D_80145AA8 & D_80145AC2) {
    if (PSX_REF(s8, (u32)&D_8014865A) == 0x7F) {
      func_8015DF18(0x103);
      PSX_REF(s8, (u32)&D_8014865A) = D_80148656[2];
      D_8014865D = D_80148656[3];
    } else {
      D_801E6254 = swapSelectionIds();
      func_8015DF18(D_801E6254 == 0xFF ? 0x103 : 0x107);
      D_8014865A = 0x7F;
      D_8014865D = 0x7F;
    }
  } else if (D_80145AA8 & D_80145AC4) {
    func_8015DF18(0x106);
    if (PSX_REF(s8, (u32)&D_8014865A) != 0x7F) {
      PSX_REF(s8, (u32)&D_8014865A) = 0x7F;
      D_8014865D = 0x7F;
    } else {
      D_80148652 = 0;
      D_80148651 += 1;
      func_8014ECAC(2);
    }
  }
}
