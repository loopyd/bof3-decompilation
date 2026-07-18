#include "internal.h"

/* @behavior UNKNOWN: verify this recovered game-work loop against target assembly.
 * @source 0x801A0514
 */
#define ENTITY_MAX       30
#define ENTITY_SLOT_SIZE 0x98

extern u8 D_8014933E;
extern void func_8014D290(struct GameWorkArea *arg0);
extern void func_8015A944(struct GameWorkArea *arg0);

void func_801A0514(void) {
    s32 var_s0_15;
    s32 var_v1_19;
    struct GameWorkArea *temp_a0_28;
    u32 var_v0_69;

    var_s0_15 = 0;

    do {
        var_v1_19 = var_s0_15 & 0xFF;

        temp_a0_28 = (struct GameWorkArea *)(
            (u8 *)D_80146888 + (var_v1_19 * ENTITY_SLOT_SIZE)
        );

        SCRATCH_WORK = temp_a0_28;

        if (temp_a0_28->flags_00 & 1) {
            if (temp_a0_28->unk_06 != 9) {
                if (temp_a0_28->pad_1C[8] & 0x10) {
                    temp_a0_28->unk_29 = 4;
                } else {
                    temp_a0_28->unk_29 = D_8014933E;
                }

                goto block_7;
            }

            if (D_8014933E != 4) {
            block_7:
                {
                  struct GameWorkArea *scratch;
                  s32 kind;
                  scratch = SCRATCH_WORK;
                  kind = scratch->unk_06;
                  if (kind == 10) {
                    var_s0_15 += 1;
                    func_8015A944(temp_a0_28);
                    var_v0_69 = var_s0_15 & 0xFF;
                  } else {
                    func_8014D290(temp_a0_28);
                    goto block_10;
                  }
                }
            } else {
                goto block_10;
            }
        } else {
        block_10:
            var_s0_15 += 1;
            var_v0_69 = var_s0_15 & 0xFF;
        }
    } while (var_v0_69 < ENTITY_MAX);
}
