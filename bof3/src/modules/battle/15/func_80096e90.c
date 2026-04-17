#include "internal.h"

/* does: aborts or backs out of the slot-selection input branch, stages the
 * current message/resource slot, and restores the selection tuple to
 * `(2,0,0)`.
 * @source: 0x80096e90 FUN_80096e90
 */
void func_80096e90(void) {
  battle_queue_frontend_cue(0x106u);
  *(u8*)0x8014839fu = 1u;
  *(u8*)0x8014837bu = 1u;
  battle_stage_message_resource(*(void**)0x801ebf08u);
  *(u8*)0x801462efu = 0u;
  *(u8*)0x801462e1u = 2u;
  *(u8*)0x801462e2u = 0u;
  *(u8*)0x801462e3u = 0u;
  *(u8*)0x801462e4u = 0u;
}
