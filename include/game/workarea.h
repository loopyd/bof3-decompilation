/* Work-area reset template for EMI code.
 * Calls func_80196070, which clears the scratchpad game work-area header
 * fields flags_00, unk_01, flags_02 and pad_03[0..1], then returns.
 *
 * Usage:
 *   #include "internal.h"
 *   #include "game/workarea.h"
 *   WORKAREA_RESET(func_801F2D1C)
 */

#ifndef GAME_WORKAREA_H
#define GAME_WORKAREA_H

#define WORKAREA_RESET(func)                                                   \
  void func(void) {                                                            \
    func_80196070();                                                           \
  }

#endif
