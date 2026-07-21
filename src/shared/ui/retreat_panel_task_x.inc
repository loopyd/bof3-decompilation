#if !defined(PANEL_TASK_FUNC)
#error "define PANEL_TASK_FUNC before including this template"
#endif

/* @source varies across EMI code blobs (byte-identical group)
 * @behavior decrements the panel task X coordinate by 32 pixels, clamps it to
 * -170, and clears the panel task state byte when the clamp is reached.
 */
void PANEL_TASK_FUNC(void) {
  PanelTask* task_root;
  s16            next_x;

  task_root = D_80148648;
  next_x = (s16)((s32)task_root->x - 0x20);
  task_root->x = (u16)next_x;
  if (next_x < -0xAA) {
    next_x = -0xAA;
    task_root->x = (u16)next_x;
    task_root->state = 0u;
  }
}

#undef PANEL_TASK_FUNC
