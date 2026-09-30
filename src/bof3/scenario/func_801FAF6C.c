#include "bof3/scenario/scena00_internal.h"

/**
 * @source 0x801FAF6C
 * @behavior Drives the SCENA00 route/exit state byte D_80146875. State 0
 * remaps that byte through the four-entry route table D_801FCA5C keyed by the
 * front-side byte D_80143F03. States 10, 20 and 30, unless the shared gate
 * byte D_80145E91 reads 2, run func_8015C088, func_801BE1B0 with 3/4/5 and
 * func_8015B580 with the scenario flag word D_8014686C and flag index 7/8/9,
 * advancing to state 0xB/0x15/0x1F. States 11, 21 and 31 wait for scenario
 * progress byte 0xB/5/4, then pass the D_80145EC4/D_80145EC8 pair with 4/5/6 to
 * func_801BFE34 and run func_801C7A54(4/5/6), advancing to state 0xC/0x16/0x20.
 * States 13, 23 and 33 run func_8015C058, clear D_80146875 and D_80146874 and
 * advance the scenario progress byte. State 50 seeds the front selector
 * context through func_8019FA28(0x1F, 0x40000, 0x240000, 1), stores 4 in
 * D_80143F1F and clears D_80146874 and D_80146875. Every other state value
 * below the table bound 0x33 is empty.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801FAF6C(void)
{
  switch (D_80146875) {
  case 0:
    D_80146875 = D_801FCA5C[D_80143F03];
    break;

  case 10:
    func_8015C088();
    func_801BE1B0(3);
    func_8015B580(D_8014686C, 7);
    D_80146875 = 0xb;
    break;

  case 11:
    if (*(u8 *)&g_ScenarioProgress != 0xbu) {
      break;
    }
    func_801BFE34(D_80145EC4, D_80145EC8, 4);
    func_801C7A54(4);
    D_80146875 = 0xc;
    break;

  case 13: {
    u8 *progress = (u8 *)&g_ScenarioProgress;

    *progress += 1;
    func_8015C058();
    D_80146875 = 0;
    D_80146874 = 0;
    break;
  }

  case 20:
    if (D_80145E91[0] == 2) {
      break;
    }
    func_8015C088();
    func_801BE1B0(4);
    func_8015B580(D_8014686C, 8);
    D_80146875 = 0x15;
    break;

  case 21:
    if (*(u8 *)&g_ScenarioProgress != 5u) {
      break;
    }
    func_801BFE34(D_80145EC4, D_80145EC8, 5);
    func_801C7A54(5);
    D_80146875 = 0x16;
    break;

  case 23:
    func_8015C058();
    D_80146875 = 0;
    D_80146874 = 0;
    *(u8 *)&g_ScenarioProgress += 1;
    break;

  case 30:
    if (D_80145E91[0] == 2) {
      break;
    }
    func_8015C088();
    func_801BE1B0(5);
    func_8015B580(D_8014686C, 9);
    D_80146875 = 0x1f;
    break;

  case 31:
    if (*(u8 *)&g_ScenarioProgress != 4u) {
      break;
    }
    func_801BFE34(D_80145EC4, D_80145EC8, 6);
    func_801C7A54(6);
    D_80146875 = 0x20;
    break;

  case 33:
    func_8015C058();
    D_80146875 = 0;
    D_80146874 = 0;
    *(u8 *)&g_ScenarioProgress += 1;
    break;

  case 50:
    func_8019FA28(0x1fu, 0x40000u, 0x240000u, 1u);
    D_80143F1F = 4u;
    D_80146874 = 0;
    D_80146875 = 0;
    break;
  }
}
