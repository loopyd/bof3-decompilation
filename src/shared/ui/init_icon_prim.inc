#if !defined(ICON_PRIM_FUNC)
#error "define ICON_PRIM_FUNC before including this template"
#endif

/* Shared icon/sprite primitive initializer (see each wrapper's @source).
 * Resets the primitive shape at the PsyQ cursor (D_8014598C), writes six caller
 * parameters into its offset fields, clears semi-transparency, and appends the
 * 0x10-byte primitive to OT index 1. */
typedef struct {
    u8  unk_00[4];
    u8  unk_04;
    u8  unk_05;
    u8  unk_06;
    u8  unk_07;
    s16 unk_08;
    s16 unk_0A;
    s8  unk_0C;
    s8  unk_0D;
    u16 unk_0E;
} IconPrim;

void ICON_PRIM_FUNC(s16 arg0, s16 arg1, s32 arg2, s32 arg3, u16 arg4, u8 arg5) {
    IconPrim* icon;

    icon = (IconPrim*)D_8014598C;
    SetSprt8((SPRT_8*)icon);
    icon->unk_08 = arg0;
    icon->unk_0A = arg1;
    icon->unk_0C = (s8)(arg2 * 8);
    icon->unk_0D = (s8)(arg3 * 8);
    icon->unk_0E = arg4;
    icon->unk_06 = arg5;
    icon->unk_05 = arg5;
    icon->unk_04 = arg5;
    SetSemiTrans(icon, 0);
    func_8014E5A0(1, 0x10);
}

#undef ICON_PRIM_FUNC
