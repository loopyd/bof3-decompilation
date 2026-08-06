#include "internal.h"

extern s8                   D_801448EA;
/* @kind: table */
extern GameEntry0StateHandler game_state_handlerTable[];

/* @behavior dispatches through the indexed handler table at game_state_handlerTable
 * using the s8 state selector at D_801448EA.
 * @source 0x801A7BF0
 */
void game_dispatch_state_handler(void) {
  game_state_handlerTable[(s32)D_801448EA]();
}
