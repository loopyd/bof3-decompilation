#include "internal.h"

/* @behavior draws the battle local panel with three mode-specific rectangle
 * regions, a cursor for the active battler, and optional preview placement
 * for the current selection state.
 * @source 0x800B0498
 */
void func_800B0498(void) {
  volatile u8* task_root;
  volatile u8* message_slot;
  volatile u8* panel_base;
  s16          root_x;
  s16          root_y;
  u8           root_w;
  u8           root_h;
  u8           cursor_index;
  s16          cursor_x;
  s16          cursor_y;
  s32          draw_x;
  s32          draw_y;
  s8           sel_substate_plus;
  u8           sel_substate;
  u8           panel_count;
  s32          scratchpad_saved;

  task_root = (volatile u8*)REG32(0x80148648u);

  root_x = (s16)REG16((u32)(task_root + 4u));
  root_y = (s16)REG16((u32)(task_root + 6u));
  root_w = REG8((u32)(task_root + 8u));
  root_h = REG8((u32)(task_root + 9u));

  ((void (*)(s16, s16, u8, u8))0x800b06f4u)(root_x, root_y, root_w, root_h);

  task_root = (volatile u8*)REG32(0x80148648u);
  draw_x = (s32)((u16)REG16((u32)(task_root + 4u)) + 6u);
  draw_y = (s32)((u16)REG16((u32)(task_root + 6u)) + 0x21u);
  ((void (*)(s32, s32, s32, s32, s32, s32))0x801ae3f0u)(
      draw_x, draw_y, ((s32)REG8((u32)(task_root + 8u)) * 8) - 0xf,
      ((s32)REG8((u32)(task_root + 9u)) * 8) - 0x27, 0,
      (s32)REG8(0x80144952u));

  task_root = (volatile u8*)REG32(0x80148648u);
  draw_x = (s32)((u16)REG16((u32)(task_root + 4u)) + 0x36u);
  draw_y = (s32)((u16)REG16((u32)(task_root + 6u)) + 6u);
  ((void (*)(s32, s32, s32, s32, s32, s32))0x801ae3f0u)(
      draw_x, draw_y, 0x48u, 0x18u, 0u, (s32)REG8(0x80144952u));

  task_root = (volatile u8*)REG32(0x80148648u);
  draw_x = (s32)((u16)REG16((u32)(task_root + 4u)) + 0x87u);
  draw_y = (s32)((u16)REG16((u32)(task_root + 6u)) + 0xau);
  ((void (*)(s32, s32, s32, s32, s32, s32))0x801ae3f0u)(
      draw_x, draw_y, 0x30u, 0x10u, 0u, (s32)REG8(0x80144952u));

  task_root = (volatile u8*)REG32(0x80148648u);
  message_slot = (volatile u8*)REG32(0x801ebf08u);
  cursor_index = REG8(0x801463b8u);
  panel_count =
      ((u16)REG16((u32)(message_slot + 0x8au)) < cursor_index) ? 7u : 0u;

  ((void (*)(s16, s16, s32, volatile u32*))0x8014ff0cu)(
      (s16)((u16)REG16((u32)(task_root + 4u)) + 0x88u),
      (s16)((u16)REG16((u32)(task_root + 6u)) + 0xcu), (s32)panel_count,
      (volatile u32*)0x800b6d1cu);

  ((void (*)(volatile u16*, volatile void*, u8))0x8017e3f4u)(
      (volatile u16*)0x80145ad4u, (volatile void*)0x80096a04u,
      REG8(0x801463b8u));

  task_root = (volatile u8*)REG32(0x80148648u);
  ((void (*)(s16, s16, s32, volatile void*))0x8014ff0cu)(
      (s16)((u16)REG16((u32)(task_root + 4u)) + 0xa0u),
      (s16)((u16)REG16((u32)(task_root + 6u)) + 0xcu), (s32)panel_count,
      (volatile void*)0x80145ad4u);

  if (REG8(0x801462e3u) == 4u) {
    sel_substate = (u8)REG8(0x801462e6u);
    if ((s8)sel_substate == (s8)-1) {
      task_root = (volatile u8*)REG32(0x80148648u);
      cursor_x = (s16)((u16)REG16((u32)(task_root + 4u)) + 0xeu);
      cursor_y = (s16)((u16)REG16((u32)(task_root + 6u)) + 0xcu);
      ((void (*)(s32, s32, s32, s8))0x801647c4u)((s32)(u16)cursor_x,
                                                 (s32)(u16)cursor_y, 0, (s8)-1);
    } else {
      task_root = (volatile u8*)REG32(0x80148648u);
      cursor_x = (s16)((u16)REG16((u32)(task_root + 4u)) + 0xeu +
                       (u16)(REG8(0x801462e7u) * 0x1eu));
      cursor_y = (s16)((u16)REG16((u32)(task_root + 6u)) + 0x2cu +
                       ((u32)sel_substate << 5u));
      ((void (*)(s32, s32, s32, s8))0x801647c4u)(
          (s32)(u16)cursor_x, (s32)(u16)cursor_y, 0, (s8)sel_substate);
    }
  }

  (void)scratchpad_saved;
  (void)panel_base;
}
