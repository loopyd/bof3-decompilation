#if !defined(BOF3_PANEL_TASK_FUNCTION) || !defined(BOF3_PANEL_LIMIT)
#error "define BOF3_PANEL_TASK_FUNCTION and BOF3_PANEL_LIMIT before including"
#endif

void BOF3_PANEL_TASK_FUNCTION(void) {
  Bof3PanelTask* task_root;
  u16            next_x;

  task_root = D_80148648;
  next_x = (u16)(task_root->x + 32u);
  task_root->x = next_x;
  if ((s16)next_x >= BOF3_PANEL_LIMIT + 1) {
    task_root->x = BOF3_PANEL_LIMIT;
    task_root->state = 0u;
  }
}

#undef BOF3_PANEL_LIMIT
#undef BOF3_PANEL_TASK_FUNCTION
