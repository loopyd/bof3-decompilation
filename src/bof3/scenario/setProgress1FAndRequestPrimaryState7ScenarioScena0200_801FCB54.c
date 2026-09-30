#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

/* @source 0x801FCB54
 * @behavior Overlay reset epilogue: it stores 0x1F in the shared scenario
 * progress byte g_ScenarioProgress (0x80146864), clears the three progress bytes
 * that follow it (0x80146865, 0x80146866 and 0x80146867), clears the shared
 * sub-state byte D_80146875 and then requests primary state 7 by storing 7 in
 * the shared primary-state byte D_80146874. It takes no arguments, returns
 * nothing, reads no memory and makes no call, and the store order 0x80146864,
 * 0x80146865, 0x80146866, 0x80146867, 0x80146875, 0x80146874 is the source
 * statement order: each store materialises the 0x8014 page in $at on its own,
 * the constants 0x1F and 7 are materialised in $v0 (the 7 load is scheduled
 * directly after the first store frees $v0, ahead of the four zero stores), the
 * zero stores use $zero directly, and nothing sets $v0 for the caller. No image
 * word names the address; twelve `jal` sites reach it, the epilogue tail of the
 * overlay's record callbacks at 0x801FC02C, 0x801FC0DC, 0x801FC18C, 0x801FC244,
 * 0x801FC2F0, 0x801FC39C, 0x801FC454, 0x801FC500, 0x801FC5B8, 0x801FC670,
 * 0x801FC71C and 0x801FC7CC (176 bytes apart), each of which ends `jal
 * 0x801FCB54`, restores $ra from its own 0x18-byte frame and returns. Its
 * neighbours func_801FCB94 (0x801FCB94) and func_801FCBDC (0x801FCBDC) are
 * separate reviewed boundaries, and g_ScenarioProgress is the shared progress
 * byte the scenario family publishes (0x80146864..0x80146867).
 * @status exact
 * @match 100.00
 * @residual none
 */
void setProgress1FAndRequestPrimaryState7ScenarioScena0200_801FCB54(void) {
  g_ScenarioProgress = 0x1F;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146875 = 0;
  D_80146874 = 7;
}
