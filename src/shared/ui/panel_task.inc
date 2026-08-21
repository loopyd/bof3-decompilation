/* Shared compile-time panel function templates; include from target wrappers only. */
#define PANEL_ADVANCE_X(func, limit)                                           \
  void func(void) {                                                            \
    PanelTask* task_root;                                                      \
    u16        next_x;                                                         \
    task_root = D_80148648;                                                    \
    next_x = (u16)(task_root->x + 32u);                                        \
    task_root->x = next_x;                                                     \
    if ((s16)next_x >= (limit) + 1) {                                          \
      task_root->x = (limit);                                                  \
      task_root->state = 0u;                                                   \
    }                                                                          \
  }

#define PANEL_RETREAT_X(func)                                                  \
  void func(void) {                                                            \
    PanelTask* task_root;                                                      \
    s16        next_x;                                                         \
    task_root = D_80148648;                                                    \
    next_x = (s16)((s32)task_root->x - 0x20);                                  \
    task_root->x = (u16)next_x;                                                \
    if (next_x < -0xAA) {                                                      \
      next_x = -0xAA;                                                          \
      task_root->x = (u16)next_x;                                              \
      task_root->state = 0u;                                                   \
    }                                                                          \
  }

#define PANEL_RETREAT_X_CLAMP(func, limit)                                     \
  void func(void) {                                                            \
    PanelTask* task_root;                                                      \
    u16        next_val;                                                       \
    task_root = D_80148648;                                                    \
    next_val = (u16)(task_root->x - 0x20);                                     \
    task_root->x = next_val;                                                   \
    if ((s16)next_val < (limit)) {                                             \
      task_root->x = (limit);                                                  \
      task_root->state = 0;                                                    \
    }                                                                          \
  }

#define PANEL_ADVANCE_FIELD6(func, limit)                                      \
  void func(void) {                                                            \
    PanelTask* task_root;                                                      \
    u16        next_val;                                                       \
    task_root = D_80148648;                                                    \
    next_val = (u16)(*((volatile u16*)((u8*)task_root + 6)) + 0x10);           \
    *((volatile u16*)((u8*)task_root + 6)) = next_val;                         \
    if ((s16)next_val >= (limit) + 1) {                                        \
      *((volatile u16*)((u8*)task_root + 6)) = (limit);                        \
      task_root->state = 0;                                                    \
    }                                                                          \
  }

#define PANEL_RETREAT_FIELD6(func, step, min)                                  \
  void func(void) {                                                            \
    PanelTask* task_root;                                                      \
    s16        next_val;                                                       \
    task_root = D_80148648;                                                    \
    next_val = (s16)(task_root->field_06 - (step));                            \
    task_root->field_06 = (u16)next_val;                                       \
    if (next_val < (min)) {                                                    \
      next_val = (min);                                                        \
      task_root->field_06 = (u16)next_val;                                     \
      task_root->state = 0;                                                    \
    }                                                                          \
  }

#define PANEL_ICON_PRIM(func)                                                  \
  void func(s16 arg0, s16 arg1, s32 arg2, s32 arg3, u16 arg4, u8 arg5) {       \
    IconPrim* icon;                                                            \
    icon = (IconPrim*)g_PrimCursor;                                              \
    SetSprt8((SPRT_8*)icon);                                                   \
    icon->unk_08 = arg0;                                                       \
    icon->unk_0A = arg1;                                                       \
    icon->unk_0C = (s8)(arg2 * 8);                                             \
    icon->unk_0D = (s8)(arg3 * 8);                                             \
    icon->unk_0E = arg4;                                                       \
    icon->unk_06 = arg5;                                                       \
    icon->unk_05 = arg5;                                                       \
    icon->unk_04 = arg5;                                                       \
    SetSemiTrans(icon, 0);                                                     \
    func_8014E5A0(1, 0x10);                                                    \
  }
