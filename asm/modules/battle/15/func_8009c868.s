/* Original assembly baseline for src/modules/battle/15/func_8009c868.c.
 * Source: BIN/BATTLE/BATTLE.EMI entry 15, 15.bin, load 0x80096800.
 */

    .set noreorder
    .globl func_8009c868
    .ent func_8009c868
func_8009c868:
    lbu   $v0, 0xe1($a0)
    nop
    srav  $v0, $v0, $a1
    jr    $ra
    andi  $v0, $v0, 0x1
    .end func_8009c868
