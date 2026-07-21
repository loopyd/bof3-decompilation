#if !defined(PANEL_TASK_FUNC) || !defined(PANEL_LIMIT)
#error "define PANEL_TASK_FUNC and PANEL_LIMIT before including this template"
#endif

void PANEL_TASK_FUNC(void) {
  PanelTask* task_root;
  u16            next_x;

  task_root = D_80148648;
  next_x = (u16)(task_root->x + 32u);
  task_root->x = next_x;
  if ((s16)next_x >= PANEL_LIMIT + 1) {
    task_root->x = PANEL_LIMIT;
    task_root->state = 0u;
  }
}

#undef PANEL_LIMIT
#undef PANEL_TASK_FUNC
