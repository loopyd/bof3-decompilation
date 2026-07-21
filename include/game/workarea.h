/* Work-area reset template (byte-identical across EMI code blobs).
 * Thin tail-call to func_80196070 — clears the scratchpad game work-area
 * header (flags_00, unk_01, flags_02, pad_03[0..1]).
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
