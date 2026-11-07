#include "bof3/bof3.h"

typedef void (*Boss00113Handler)(void);

/* Overlay entrypoints this installer publishes. The progress hook and the
 * segment setup routine take no argument; the third is the overlay's mode
 * dispatcher, which reads a byte index in $a0, so it is stored through the
 * void slot type. */
void func_800C1FF8(void);
void func_800C2060(void);
void func_800C1E88(u8 index);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss00113Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss00113Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss00113Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1E50
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * the progress hook func_800C1FF8 (tests bit 0x1 of D_80146328; when set it
 * forces 6 into D_80146874 and 0x32 into D_80146875, otherwise it advances the
 * byte counter at D_80146875, then calls func_801DDF28) into 0x801463A4, the
 * segment setup routine func_800C2060 (starts the 0x801EB630 and 0x801EB748
 * records through func_800AC844/func_800AC870, loads each through
 * func_800AC754 into the scratchpad work cell 0x1F800044, clears its +0x2A and
 * +0x48 bytes, and copies the record's x/y halfwords into +0x58/+0x5A after
 * func_8014DD3C and func_8014D6B8) into 0x801463A8, and the mode dispatcher
 * func_800C1E88 (masks a byte index, rejects indices above six and jumps
 * through the seven-entry table at 0x800C1804) into 0x801463AC; the
 * dispatcher's index argument is dropped by the void slot type. No in-payload
 * caller: neither this function's address nor the addresses it stores appear
 * in any word of this payload, so the main program reaches it through its
 * overlay entry point, as it does the sibling installers 0x800C2B60 of
 * BIN/BOSS/BOSS025.EMI#13, 0x800C1F9C and 0x800C2088 of
 * BIN/BOSS/BOSS008.EMI#16 and 0x800C2114 and 0x800C23F8 of
 * BIN/BOSS/BOSS018.EMI#16, which write the same three slots in the same order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss00113_800C1E50(void) {
  D_801463A4 = func_800C1FF8;
  D_801463A8 = func_800C2060;
  D_801463AC = (Boss00113Handler)func_800C1E88;
}
