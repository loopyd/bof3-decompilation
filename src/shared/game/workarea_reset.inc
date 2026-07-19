#if !defined(WORKAREA_RESET_FUNC)
#error "define WORKAREA_RESET_FUNC before including this template"
#endif

/* Thin tail-call wrapper: resets the scratchpad game work-area header by
 * delegating to func_80196070 (clears flags_00, unk_01, flags_02,
 * pad_03[0..1]). No extra logic. */
void WORKAREA_RESET_FUNC(void) {
  func_80196070();
}

#undef WORKAREA_RESET_FUNC
