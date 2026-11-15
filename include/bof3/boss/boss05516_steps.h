#ifndef BOF3_BOSS_BOSS05516_STEPS_H
#define BOF3_BOSS_BOSS05516_STEPS_H

/* copy is a simple local; source, entry and ready name this overlay's step inputs. */
#define BOSS055_APPLY_TEMPLATE_STEP(copy, source, entry, ready) \
    do { \
        (copy) = (source); \
        func_8014DD3C((entry)); \
        g_battle_work[0x48] = 0; \
        g_battle_work[0x24] = 0; \
        FIELD_REF(s32, g_battle_work, 0x3C) = 0; \
        func_8014D6B8((copy).bytes[D_801463BE]); \
        (ready)(); \
        func_8014D4E0(); \
        refreshWorkFromRecordThenAdvancePositionByModeScaleBossBoss05516_800C3284(); \
        g_battle_work[2]++; \
    } while (0)

#endif
