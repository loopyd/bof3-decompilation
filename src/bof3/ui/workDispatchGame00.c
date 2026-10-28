#include "bof3/ui/game00_internal.h"

/* All arguments are identifiers. Each dispatcher reads one work byte and calls
 * one handler from its target-owned table. */
#define DISPATCH_WORK_HANDLER(name, table, field) \
  void NO_SIBLING_CALLS name(void) { \
    table[g_game_work->field](); \
  }

/*
 * @behavior Dispatches the handler table (0x801C80A4) indexed by the
 * work-area byte (work+0x06) through a framed jalr call.
 * @source 0x8019A53C
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_8019A53C, D_801C80A4, unk_06)

/*
 * @behavior Dispatches the handler table (0x801C80BC) indexed by the
 * work-area byte (work+0x01) through a framed jalr call.
 * @source 0x8019A804
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_8019A804, D_801C80BC, unk_01)

/*
 * @behavior Dispatches the handler table (0x801C80F8) indexed by the
 * work-area byte (work+0x01) through a framed jalr call.
 * @source 0x8019ADA4
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_8019ADA4, D_801C80F8, unk_01)

/*
 * @behavior Dispatches the handler table (0x801C8120) indexed by the
 * work-area byte (work+0x01) through a framed jalr call.
 * @source 0x8019B10C
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_8019B10C, D_801C8120, unk_01)

/*
 * @behavior Dispatches the handler table (0x801C812C) indexed by the
 * work-area byte (work+0x01) through a framed jalr call.
 * @source 0x8019B364
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_8019B364, D_801C812C, unk_01)

/*
 * @behavior Dispatches the handler table (0x801C8144) indexed by the
 * work-area byte (work+0x01) through a framed jalr call.
 * @source 0x8019C620
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_8019C620, D_801C8144, unk_01)

/*
 * @behavior Dispatches the handler table (0x801C8154) indexed by the
 * work-area byte (work+0x01) through a framed jalr call.
 * @source 0x8019CB94
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_8019CB94, D_801C8154, unk_01)

/*
 * @behavior Dispatches the handler table (0x801C8164) indexed by the
 * work-area byte (work+0x01) through a framed jalr call.
 * @source 0x8019DDCC
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_8019DDCC, D_801C8164, unk_01)

/*
 * @behavior Dispatches the handler table (0x801C817C) indexed by the
 * work-area byte (work+0x02) through a framed jalr call.
 * @source 0x8019DE10
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_8019DE10, D_801C817C, flags_02)

/*
 * @behavior Dispatches the handler table (0x801C8198) indexed by the
 * work-area byte (work+0x02) through a framed jalr call.
 * @source 0x8019E264
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_8019E264, D_801C8198, flags_02)

/*
 * @behavior Dispatches the handler table (0x801C81A0) indexed by the
 * work-area byte (work+0x02) through a framed jalr call.
 * @source 0x8019E424
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_8019E424, D_801C81A0, flags_02)

/*
 * @behavior Dispatches the handler table (0x801C81B0) indexed by the
 * work-area byte (work+0x02) through a framed jalr call.
 * @source 0x8019E780
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_8019E780, D_801C81B0, flags_02)

/*
 * @behavior Dispatches the handler table (0x801C81B8) indexed by the
 * work-area byte (work+0x02) through a framed jalr call.
 * @source 0x8019E918
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_8019E918, D_801C81B8, flags_02)

/*
 * @behavior Dispatches the handler table (0x801C81C0) indexed by the
 * work-area byte (work+0x01) through a framed jalr call.
 * @source 0x8019EAF4
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_8019EAF4, D_801C81C0, unk_01)

/*
 * @behavior Dispatches the handler table (0x801C81CC) indexed by the
 * work-area byte (work+0x01) through a framed jalr call.
 * @source 0x8019F2CC
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_8019F2CC, D_801C81CC, unk_01)

/*
 * @behavior Dispatches the handler table (0x801C81DC) indexed by the
 * work-area byte (work+0x01) through a framed jalr call.
 * @source 0x8019F98C
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_8019F98C, D_801C81DC, unk_01)

/*
 * @behavior Dispatches the handler table (0x801CD130) indexed by the
 * work-area byte (work+0x02) through a framed jalr call.
 * @source 0x801B3260
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_801B3260, D_801CD130, flags_02)

/*
 * @behavior Dispatches the handler table (0x801CD140) indexed by the
 * work-area byte (work+0x03) through a framed jalr call.
 * @source 0x801B32D0
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_801B32D0, D_801CD140, pad_03)

/*
 * @behavior Dispatches the handler table (0x801CD154) indexed by the
 * work-area byte (work+0x03) through a framed jalr call.
 * @source 0x801B3540
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_801B3540, D_801CD154, pad_03)

/*
 * @behavior Dispatches the handler table (0x801CD2B0) indexed by the
 * work-area byte (work+0x02) through a framed jalr call.
 * @source 0x801B69C0
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_801B69C0, D_801CD2B0, flags_02)

/*
 * @behavior Dispatches the handler table (0x801CD2EC) indexed by the
 * work-area byte (work+0x03) through a framed jalr call.
 * @source 0x801B726C
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_801B726C, D_801CD2EC, pad_03)

/*
 * @behavior Dispatches the handler table (0x801CD2F4) indexed by the
 * work-area byte (work+0x03) through a framed jalr call.
 * @source 0x801B73B0
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_801B73B0, D_801CD2F4, pad_03)

/*
 * @behavior Dispatches the handler table (0x801CD310) indexed by the
 * work-area byte (work+0x04) through a framed jalr call.
 * @source 0x801B790C
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_801B790C, D_801CD310, field_04)

/*
 * @behavior Dispatches the handler table (0x801CD330) indexed by the
 * work-area byte (work+0x04) through a framed jalr call.
 * @source 0x801B7D00
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_801B7D00, D_801CD330, field_04)

/*
 * @behavior Dispatches the handler table (0x801CD37C) indexed by the
 * work-area byte (work+0x03) through a framed jalr call.
 * @source 0x801B91CC
 * @status exact
 * @match 100.00
 * @residual none
 */
DISPATCH_WORK_HANDLER(func_801B91CC, D_801CD37C, pad_03)
