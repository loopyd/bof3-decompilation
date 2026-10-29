# SPDX-License-Identifier: MIT
# Owned XA selection, completion, nine-state dispatch, gain and CD positions.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
.set noreorder
.set noat

.section .text
.balign 4
# exe/slus_004_22@0x80163744..0x80163858.
# A0 >> 12 selects one of three streams. The low 12 bits select a cue.
# The caller must bind the complete table walk and indexed LBA before entry.
.globl audio_xa_select_cue
.ent audio_xa_select_cue
audio_xa_select_cue:
    addiu $sp, $sp, -8
    sra $v0, $a0, 12
    beqz $v0, .Lxa_80163758
    addiu $t0, $zero, 8
    addiu $t0, $zero, 16
.Lxa_80163758:
    lui $a1, %hi(audio_xa_state + 0x29)
    addiu $a1, $a1, %lo(audio_xa_state + 0x29)
    sll $v0, $v0, 2
    sb $zero, 0($a1)
    lui $at, %hi(audio_xa_table_pointers)
    addu $at, $at, $v0
    lw $a3, %lo(audio_xa_table_pointers)($at)
    andi $v0, $a0, 0xfff
    blez $v0, .Lxa_801637b8
    addu $v1, $zero, $zero
    addu $a2, $v0, $zero
.Lxa_80163784:
    lhu $v0, 0($a3)
    nop
    andi $v0, $v0, 0x8000
    beqz $v0, .Lxa_801637a8
    nop
    lbu $v0, 0($a1)
    addiu $a3, $a3, 2
    addiu $v0, $v0, 1
    sb $v0, 0($a1)
.Lxa_801637a8:
    addiu $v1, $v1, 1
    slt $v0, $v1, $a2
    bnez $v0, .Lxa_80163784
    addiu $a3, $a3, 2
.Lxa_801637b8:
    lui $a2, %hi(audio_xa_state + 0x28)
    addiu $a2, $a2, %lo(audio_xa_state + 0x28)
    addiu $v0, $zero, 1
    sb $v0, 0($a2)
    sra $v0, $a0, 12
    sll $v0, $v0, 2
    lui $at, %hi(audio_xa_file_slots)
    addu $at, $at, $v0
    lw $v0, %lo(audio_xa_file_slots)($at)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_xa_file_lbas)
    addu $at, $at, $v0
    lw $a0, %lo(audio_xa_file_lbas)($at)
    lui $at, %hi(audio_xa_state + 4)
    sw $a0, %lo(audio_xa_state + 4)($at)
    sw $a0, -0x28($a2)
    lhu $v0, 0($a3)
    nop
    andi $v0, $v0, 0x7fff
    mult $v0, $t0
    lbu $a1, 1($a2)
    mflo $t1
    addu $v1, $t1, $a1
    addu $v1, $a0, $v1
    sw $v1, -0x28($a2)
    lhu $v0, 2($a3)
    nop
    andi $v0, $v0, 0x7fff
    mult $v0, $t0
    addiu $a0, $a0, -0x96
    sw $v1, -0x20($a2)
    mflo $t1
    addu $a1, $t1, $a1
    addu $a0, $a0, $a1
    lui $at, %hi(audio_xa_state + 4)
    sw $a0, %lo(audio_xa_state + 4)($at)
    addiu $sp, $sp, 8
    jr $ra
    nop
.end audio_xa_select_cue

# exe/slus_004_22@0x801636A0..0x80163744.
.globl audio_xa_start
.ent audio_xa_start
audio_xa_start:
    addiu $sp, $sp, -0x18
    sw $s0, 0x10($sp)
    lui $s0, %hi(audio_xa_state + 0xc)
    addiu $s0, $s0, %lo(audio_xa_state + 0xc)
    addiu $v0, $zero, 1
    sw $ra, 0x14($sp)
    sh $zero, 0($s0)
    lui $at, %hi(audio_xa_state + 0x21)
    sb $v0, %lo(audio_xa_state + 0x21)($at)
    addiu $v0, $zero, 2
    lui $at, %hi(audio_xa_state + 0xe)
    sh $a0, %lo(audio_xa_state + 0xe)($at)
    lui $at, %hi(audio_xa_state + 0x23)
    sb $zero, %lo(audio_xa_state + 0x23)($at)
    lui $at, %hi(audio_xa_state + 0x20)
    sb $zero, %lo(audio_xa_state + 0x20)($at)
    lui $at, %hi(audio_xa_state + 0x26)
    sb $zero, %lo(audio_xa_state + 0x26)($at)
    lui $at, %hi(audio_xa_state + 0x14)
    sh $zero, %lo(audio_xa_state + 0x14)($at)
    lui $at, %hi(audio_xa_state + 0x10)
    sh $zero, %lo(audio_xa_state + 0x10)($at)
    lui $at, %hi(audio_xa_state + 0x34)
    sb $v0, %lo(audio_xa_state + 0x34)($at)
    lui $at, %hi(audio_xa_start_clear_byte)
    sb $zero, %lo(audio_xa_start_clear_byte)($at)
    jal audio_xa_select_cue
    nop
    lui $a0, %hi(audio_xa_state)
    lw $a0, %lo(audio_xa_state)($a0)
    jal audio_cd_int_to_position
    addiu $a1, $s0, 0x20
    lui $a0, %hi(audio_xa_completion_callback)
    addiu $a0, $a0, %lo(audio_xa_completion_callback)
    jal audio_cd_set_sync_callback
    nop
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_xa_start

# exe/slus_004_22@0x80163858..0x801638B0.
# Completion is written before the descending, live eight-byte copy.
.globl audio_xa_completion_callback
.ent audio_xa_completion_callback
audio_xa_completion_callback:
    addiu $v0, $zero, 2
    bne $a0, $v0, .Lxa_callback_801638a0
    addiu $v0, $zero, -1
    addiu $v0, $zero, 1
    lui $at, %hi(audio_xa_state + 0x22)
    sb $v0, %lo(audio_xa_state + 0x22)($at)
    addiu $v1, $zero, 7
    addiu $a1, $a1, 7
.Lxa_callback_80163878:
    lbu $v0, 0($a1)
    addiu $a1, $a1, -1
    lui $at, %hi(audio_xa_state + 0x18)
    addu $at, $at, $v1
    sb $v0, %lo(audio_xa_state + 0x18)($at)
    addu $v0, $v1, $zero
    bnez $v0, .Lxa_callback_80163878
    addiu $v1, $v1, -1
    j .Lxa_callback_801638a8
    nop
.Lxa_callback_801638a0:
    lui $at, %hi(audio_xa_state + 0x22)
    sb $v0, %lo(audio_xa_state + 0x22)($at)
.Lxa_callback_801638a8:
    jr $ra
    nop
.end audio_xa_completion_callback

# exe/slus_004_22@0x80175680..0x80175698, CdSyncCallback.
.globl audio_cd_set_sync_callback
.ent audio_cd_set_sync_callback
audio_cd_set_sync_callback:
    lui $v0, %hi(audio_cd_sync_callback)
    lw $v0, %lo(audio_cd_sync_callback)($v0)
    lui $at, %hi(audio_cd_sync_callback)
    sw $a0, %lo(audio_cd_sync_callback)($at)
    jr $ra
    nop
.end audio_cd_set_sync_callback

# exe/slus_004_22@0x80175ADC..0x80175BE0, CdIntToPos.
# A0 is the signed LBA; A1 supplies three writable position bytes.
.globl audio_cd_int_to_position
.ent audio_cd_int_to_position
audio_cd_int_to_position:
    lui $v1, 0x1b4e
    ori $v1, $v1, 0x81b5
    addiu $a0, $a0, 0x96
    mult $a0, $v1
    addu $v0, $a1, $zero
    lui $a1, 0x8888
    ori $a1, $a1, 0x8889
    mfhi $v1
    sra $a3, $v1, 3
    sra $v1, $a0, 0x1f
    subu $a3, $a3, $v1
    mult $a3, $a1
    lui $t1, 0x6666
    ori $t1, $t1, 0x6667
    sll $a1, $a3, 2
    addu $a1, $a1, $a3
    sll $v1, $a1, 4
    mfhi $a2
    subu $v1, $v1, $a1
    subu $a0, $a0, $v1
    mult $a0, $t1
    sra $v1, $a3, 0x1f
    addu $t0, $a2, $a3
    sra $t0, $t0, 5
    subu $t0, $t0, $v1
    sll $v1, $t0, 4
    subu $v1, $v1, $t0
    mfhi $a1
    sll $v1, $v1, 2
    subu $a3, $a3, $v1
    mult $a3, $t1
    sra $v1, $a0, 0x1f
    sra $a1, $a1, 2
    subu $a1, $a1, $v1
    sll $a2, $a1, 4
    sll $v1, $a1, 2
    addu $v1, $v1, $a1
    sll $v1, $v1, 1
    subu $a0, $a0, $v1
    mfhi $t3
    addu $a2, $a2, $a0
    sra $v1, $a3, 0x1f
    mult $t0, $t1
    sb $a2, 2($v0)
    sra $a0, $t3, 2
    subu $a0, $a0, $v1
    sll $a1, $a0, 4
    sll $v1, $a0, 2
    addu $v1, $v1, $a0
    sll $v1, $v1, 1
    subu $a3, $a3, $v1
    addu $a1, $a1, $a3
    sra $v1, $t0, 0x1f
    sb $a1, 1($v0)
    mfhi $t1
    sra $a0, $t1, 2
    subu $a0, $a0, $v1
    sll $a1, $a0, 4
    sll $v1, $a0, 2
    addu $v1, $v1, $a0
    sll $v1, $v1, 1
    subu $t0, $t0, $v1
    addu $a1, $a1, $t0
    jr $ra
    sb $a1, 0($v0)
.end audio_cd_int_to_position

# exe/slus_004_22@0x801639B8..0x801639D0, state-table entry 0.
.globl audio_xa_state_0
.ent audio_xa_state_0
audio_xa_state_0:
    lui $at, %hi(audio_xa_state + 0x10)
    sh $zero, %lo(audio_xa_state + 0x10)($at)
    lui $at, %hi(audio_xa_state + 0x22)
    sb $zero, %lo(audio_xa_state + 0x22)($at)
    jr $ra
    nop
.end audio_xa_state_0

# exe/slus_004_22@0x80163A54..0x80163AF0, state-table entry 2.
.globl audio_xa_state_2
.ent audio_xa_state_2
audio_xa_state_2:
    lui $v0, %hi(audio_xa_state + 0x22)
    lb $v0, %lo(audio_xa_state + 0x22)($v0)
    addiu $sp, $sp, -0x18
    bgez $v0, .Lxa_state2_80163abc
    sw $ra, 0x10($sp)
    lui $v0, %hi(audio_xa_state + 0x14)
    lhu $v0, %lo(audio_xa_state + 0x14)($v0)
    addiu $v1, $zero, 1
    lui $at, %hi(audio_xa_state + 0x21)
    sb $v1, %lo(audio_xa_state + 0x21)($at)
    addiu $v1, $v0, 1
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 6
    lui $at, %hi(audio_xa_state + 0x14)
    sh $v1, %lo(audio_xa_state + 0x14)($at)
    beqz $v0, .Lxa_state2_80163aac
    addiu $v0, $zero, 8
    jal audio_xa_response_has_bit4
    nop
    beqz $v0, .Lxa_state2_80163ae0
    addiu $v0, $zero, 8
.Lxa_state2_80163aac:
    lui $at, %hi(audio_xa_state + 0x21)
    sb $v0, %lo(audio_xa_state + 0x21)($at)
    j .Lxa_state2_80163ae0
    nop
.Lxa_state2_80163abc:
    beqz $v0, .Lxa_state2_80163ae0
    addiu $v0, $zero, 3
    lui $at, %hi(audio_xa_state + 0x21)
    sb $v0, %lo(audio_xa_state + 0x21)($at)
    addiu $v0, $zero, 1
    lui $at, %hi(audio_xa_state + 0x26)
    sb $v0, %lo(audio_xa_state + 0x26)($at)
    lui $at, %hi(audio_xa_state + 0x10)
    sh $zero, %lo(audio_xa_state + 0x10)($at)
.Lxa_state2_80163ae0:
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_xa_state_2

# exe/slus_004_22@0x80163D80..0x80163E3C, state-table entry 7.
.globl audio_xa_state_7
.ent audio_xa_state_7
audio_xa_state_7:
    addiu $sp, $sp, -0x18
    lui $v1, %hi(audio_xa_state + 0x22)
    addiu $v1, $v1, %lo(audio_xa_state + 0x22)
    sw $ra, 0x10($sp)
    lb $v0, 0($v1)
    nop
    bgez $v0, .Lxa_state7_80163df4
    nop
    lui $v0, %hi(audio_xa_state + 0x14)
    lhu $v0, %lo(audio_xa_state + 0x14)($v0)
    addiu $v1, $zero, 6
    lui $at, %hi(audio_xa_state + 0x21)
    sb $v1, %lo(audio_xa_state + 0x21)($at)
    addiu $v1, $v0, 1
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 6
    lui $at, %hi(audio_xa_state + 0x14)
    sh $v1, %lo(audio_xa_state + 0x14)($at)
    beqz $v0, .Lxa_state7_80163de4
    addiu $v0, $zero, 8
    jal audio_xa_response_has_bit4
    nop
    beqz $v0, .Lxa_state7_80163e2c
    addiu $v0, $zero, 8
.Lxa_state7_80163de4:
    lui $at, %hi(audio_xa_state + 0x21)
    sb $v0, %lo(audio_xa_state + 0x21)($at)
    j .Lxa_state7_80163e2c
    nop
.Lxa_state7_80163df4:
    beqz $v0, .Lxa_state7_80163e2c
    addiu $v0, $zero, 2
    lui $at, %hi(audio_xa_state + 0x23)
    sb $v0, %lo(audio_xa_state + 0x23)($at)
    addiu $v0, $zero, 3
    lui $at, %hi(audio_xa_state + 0x21)
    sb $zero, %lo(audio_xa_state + 0x21)($at)
    lui $at, %hi(audio_xa_state + 0x10)
    sh $zero, %lo(audio_xa_state + 0x10)($at)
    sb $zero, 0($v1)
    lui $at, %hi(audio_xa_start_clear_byte)
    sb $v0, %lo(audio_xa_start_clear_byte)($at)
    jal audio_cd_set_sync_callback
    addu $a0, $zero, $zero
.Lxa_state7_80163e2c:
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_xa_state_7

# exe/slus_004_22@0x80163E88..0x80163EA0.
.globl audio_xa_response_has_bit4
.ent audio_xa_response_has_bit4
audio_xa_response_has_bit4:
    lui $v0, %hi(audio_xa_state + 0x18)
    lbu $v0, %lo(audio_xa_state + 0x18)($v0)
    nop
    andi $v0, $v0, 0x10
    jr $ra
    sltu $v0, $zero, $v0
.end audio_xa_response_has_bit4

# exe/slus_004_22@0x80163EA0..0x80163EB0.
.globl audio_xa_is_idle
.ent audio_xa_is_idle
audio_xa_is_idle:
    lui $v0, %hi(audio_xa_state + 0x21)
    lb $v0, %lo(audio_xa_state + 0x21)($v0)
    jr $ra
    sltiu $v0, $v0, 1
.end audio_xa_is_idle

# exe/slus_004_22@0x80175BE0..0x80175C60, CdPosToInt.
# Three live bytes are read at A0 +0, +1, then +2; no BCD validation.
.globl audio_cd_position_to_int
.ent audio_cd_position_to_int
audio_cd_position_to_int:
    lbu $v1, 0($a0)
    lbu $a2, 1($a0)
    srl $a1, $v1, 4
    sll $v0, $a1, 2
    addu $v0, $v0, $a1
    sll $v0, $v0, 1
    andi $v1, $v1, 0xf
    addu $v0, $v0, $v1
    sll $a1, $v0, 4
    subu $a1, $a1, $v0
    sll $a1, $a1, 2
    srl $v1, $a2, 4
    sll $v0, $v1, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 1
    andi $a2, $a2, 0xf
    addu $v0, $v0, $a2
    addu $a1, $a1, $v0
    sll $v1, $a1, 2
    addu $v1, $v1, $a1
    sll $v0, $v1, 4
    lbu $a1, 2($a0)
    subu $v0, $v0, $v1
    srl $a0, $a1, 4
    sll $v1, $a0, 2
    addu $v1, $v1, $a0
    sll $v1, $v1, 1
    andi $a1, $a1, 0xf
    addu $v1, $v1, $a1
    addu $v0, $v0, $v1
    jr $ra
    addiu $v0, $v0, -0x96
.end audio_cd_position_to_int

.section .text
.balign 4
# exe/slus_004_22@0x801638b0..0x801639b8.
.globl audio_xa_tick
.ent audio_xa_tick
audio_xa_tick:
    addiu $sp, $sp, -0x18
    sw $s0, 0x10($sp)
    lui $s0, %hi(audio_xa_state + 0x21)
    addiu $s0, $s0, %lo(audio_xa_state + 0x21)
    sw $ra, 0x14($sp)
    lb $v0, 0($s0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_xa_handlers)
    addu $at, $at, $v0
    lw $v0, %lo(audio_xa_handlers)($at)
    nop
    jalr $v0
    nop
    lui $v0, %hi(audio_xa_state + 0x10)
    lhu $v0, %lo(audio_xa_state + 0x10)($v0)
    lui $v1, %hi(audio_xa_state + 0x20)
    lb $v1, %lo(audio_xa_state + 0x20)($v1)
    addiu $v0, $v0, 1
    lui $at, %hi(audio_xa_state + 0x10)
    sh $v0, %lo(audio_xa_state + 0x10)($at)
    beqz $v1, .Lxa_8016392c
    nop
    lb $v0, 0($s0)
    nop
    slti $v0, $v0, 6
    beqz $v0, .Lxa_8016392c
    addiu $v0, $zero, 8
    lui $at, %hi(audio_xa_state + 0x10)
    sh $zero, %lo(audio_xa_state + 0x10)($at)
    sb $v0, 0($s0)
.Lxa_8016392c:
    lui $v0, %hi(audio_xa_state + 0x10)
    lhu $v0, %lo(audio_xa_state + 0x10)($v0)
    nop
    sltiu $v0, $v0, 0xb5
    bnez $v0, .Lxa_801639a4
    addiu $v1, $zero, 1
    lui $v0, %hi(audio_xa_state + 0x14)
    lhu $v0, %lo(audio_xa_state + 0x14)($v0)
    lui $at, %hi(audio_xa_state + 0x21)
    sb $v1, %lo(audio_xa_state + 0x21)($at)
    addiu $v1, $v0, 1
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 6
    lui $at, %hi(audio_xa_state + 0x14)
    sh $v1, %lo(audio_xa_state + 0x14)($at)
    beqz $v0, .Lxa_80163984
    nop
    jal audio_xa_response_has_bit4
    nop
    beqz $v0, .Lxa_801639a4
    nop
.Lxa_80163984:
    lui $v0, %hi(audio_xa_state + 0x21)
    lb $v0, %lo(audio_xa_state + 0x21)($v0)
    nop
    slti $v0, $v0, 6
    beqz $v0, .Lxa_801639a4
    addiu $v0, $zero, 8
    lui $at, %hi(audio_xa_state + 0x21)
    sb $v0, %lo(audio_xa_state + 0x21)($at)
.Lxa_801639a4:
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_xa_tick

# exe/slus_004_22@0x801639d0..0x80163a54.
.globl audio_xa_state_1
.ent audio_xa_state_1
audio_xa_state_1:
    addiu $sp, $sp, -0x20
    addiu $v0, $zero, 0xc8
    addiu $a0, $zero, 1
    addu $a1, $zero, $zero
    sw $ra, 0x1c($sp)
    sw $s0, 0x18($sp)
    jal audio_cd_sync
    sb $v0, 0x10($sp)
    beqz $v0, .Lxa_80163a40
    addiu $a0, $zero, 0xe
    addiu $a1, $sp, 0x10
    lui $s0, %hi(audio_xa_state + 0x18)
    addiu $s0, $s0, %lo(audio_xa_state + 0x18)
    jal audio_cd_control
    addu $a2, $s0, $zero
    beqz $v0, .Lxa_80163a40
    addiu $a0, $zero, 0x16
    lui $at, %hi(audio_xa_state + 0x22)
    sb $zero, %lo(audio_xa_state + 0x22)($at)
    addiu $a1, $s0, 0x14
    jal audio_cd_control
    addu $a2, $zero, $zero
    beqz $v0, .Lxa_80163a40
    addiu $v0, $zero, 2
    lui $at, %hi(audio_xa_state + 0x21)
    sb $v0, %lo(audio_xa_state + 0x21)($at)
    lui $at, %hi(audio_xa_state + 0x26)
    sb $zero, %lo(audio_xa_state + 0x26)($at)
.Lxa_80163a40:
    lw $ra, 0x1c($sp)
    lw $s0, 0x18($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_xa_state_1

# exe/slus_004_22@0x80163af0..0x80163b74.
.globl audio_xa_state_3
.ent audio_xa_state_3
audio_xa_state_3:
    addiu $sp, $sp, -0x18
    addiu $a0, $zero, 1
    addu $a1, $zero, $zero
    sw $ra, 0x14($sp)
    jal audio_cd_sync
    sw $s0, 0x10($sp)
    beqz $v0, .Lxa_80163b58
    addiu $a0, $zero, 0xd
    lui $s0, %hi(audio_xa_state + 0x28)
    addiu $s0, $s0, %lo(audio_xa_state + 0x28)
    addu $a1, $s0, $zero
    jal audio_cd_control
    addu $a2, $zero, $zero
    beqz $v0, .Lxa_80163b58
    addiu $a0, $zero, 0x1b
    lui $at, %hi(audio_xa_state + 0x22)
    sb $zero, %lo(audio_xa_state + 0x22)($at)
    addiu $a1, $s0, 4
    jal audio_cd_control
    addu $a2, $zero, $zero
    beqz $v0, .Lxa_80163b58
    addiu $v0, $zero, 4
    lui $at, %hi(audio_xa_state + 0x21)
    sb $v0, %lo(audio_xa_state + 0x21)($at)
    lui $at, %hi(audio_xa_state + 0x12)
    sh $zero, %lo(audio_xa_state + 0x12)($at)
.Lxa_80163b58:
    lui $at, %hi(audio_xa_state + 0x10)
    sh $zero, %lo(audio_xa_state + 0x10)($at)
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_xa_state_3

# exe/slus_004_22@0x80163b74..0x80163c58.
.globl audio_xa_state_4
.ent audio_xa_state_4
audio_xa_state_4:
    addiu $sp, $sp, -0x18
    sw $s0, 0x10($sp)
    lui $s0, %hi(audio_xa_state + 0x22)
    addiu $s0, $s0, %lo(audio_xa_state + 0x22)
    sw $ra, 0x14($sp)
    lb $v0, 0($s0)
    nop
    bgez $v0, .Lxa_80163be8
    addiu $v1, $zero, 3
    lui $v0, %hi(audio_xa_state + 0x14)
    lhu $v0, %lo(audio_xa_state + 0x14)($v0)
    lui $at, %hi(audio_xa_state + 0x21)
    sb $v1, %lo(audio_xa_state + 0x21)($at)
    addiu $v1, $v0, 1
    sll $v0, $v0, 0x10
    sra $v0, $v0, 0x10
    slti $v0, $v0, 6
    lui $at, %hi(audio_xa_state + 0x14)
    sh $v1, %lo(audio_xa_state + 0x14)($at)
    beqz $v0, .Lxa_80163bd8
    addiu $v0, $zero, 8
    jal audio_xa_response_has_bit4
    nop
    beqz $v0, .Lxa_80163c44
    addiu $v0, $zero, 8
.Lxa_80163bd8:
    lui $at, %hi(audio_xa_state + 0x21)
    sb $v0, %lo(audio_xa_state + 0x21)($at)
    j .Lxa_80163c44
    nop
.Lxa_80163be8:
    beqz $v0, .Lxa_80163c44
    nop
    jal audio_xa_ramp_gain
    addiu $a0, $zero, 0x80
    lui $v0, %hi(audio_xa_state + 0x12)
    lhu $v0, %lo(audio_xa_state + 0x12)($v0)
    nop
    addiu $v1, $v0, 1
    sltiu $v0, $v0, 6
    lui $at, %hi(audio_xa_state + 0x12)
    sh $v1, %lo(audio_xa_state + 0x12)($at)
    bnez $v0, .Lxa_80163c44
    addiu $v0, $zero, 5
    lui $at, %hi(audio_xa_state + 0x21)
    sb $v0, %lo(audio_xa_state + 0x21)($at)
    addiu $v0, $zero, 1
    lui $at, %hi(audio_xa_state + 0x12)
    sh $zero, %lo(audio_xa_state + 0x12)($at)
    lui $at, %hi(audio_xa_state + 0x10)
    sh $zero, %lo(audio_xa_state + 0x10)($at)
    sb $zero, 0($s0)
    lui $at, %hi(audio_xa_state + 0x23)
    sb $v0, %lo(audio_xa_state + 0x23)($at)
.Lxa_80163c44:
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_xa_state_4

# exe/slus_004_22@0x80163c58..0x80163d20.
.globl audio_xa_state_5
.ent audio_xa_state_5
audio_xa_state_5:
    addiu $sp, $sp, -0x18
    sw $s0, 0x10($sp)
    lui $s0, %hi(audio_xa_state + 0x12)
    addiu $s0, $s0, %lo(audio_xa_state + 0x12)
    sw $ra, 0x14($sp)
    lhu $v0, 0($s0)
    nop
    addiu $v0, $v0, 1
    sh $v0, 0($s0)
    andi $v0, $v0, 1
    beqz $v0, .Lxa_80163d0c
    nop
    lui $v0, %hi(audio_xa_state + 0x22)
    lb $v0, %lo(audio_xa_state + 0x22)($v0)
    nop
    bgtz $v0, .Lxa_80163cb4
    addiu $a0, $zero, 0x10
    lui $at, %hi(audio_xa_state + 0x22)
    sb $zero, %lo(audio_xa_state + 0x22)($at)
    jal audio_cd_control_nonblocking
    addu $a1, $zero, $zero
    j .Lxa_80163d0c
    nop
.Lxa_80163cb4:
    lui $at, %hi(audio_xa_state + 0x22)
    sb $zero, %lo(audio_xa_state + 0x22)($at)
    lui $at, %hi(audio_xa_state + 0x10)
    sh $zero, %lo(audio_xa_state + 0x10)($at)
    jal audio_cd_last_command
    nop
    addiu $v1, $zero, 0x10
    bne $v0, $v1, .Lxa_80163d0c
    nop
    jal audio_cd_position_to_int
    addiu $a0, $s0, 6
    lui $v1, %hi(audio_xa_state + 0x4)
    lw $v1, %lo(audio_xa_state + 0x4)($v1)
    lui $at, %hi(audio_xa_state + 0x8)
    sw $v0, %lo(audio_xa_state + 0x8)($at)
    slt $v0, $v0, $v1
    bnez $v0, .Lxa_80163d0c
    addiu $v0, $zero, 6
    lui $at, %hi(audio_xa_state + 0x21)
    sb $v0, %lo(audio_xa_state + 0x21)($at)
    lui $at, %hi(audio_xa_state + 0x22)
    sb $zero, %lo(audio_xa_state + 0x22)($at)
.Lxa_80163d0c:
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_xa_state_5

# exe/slus_004_22@0x80163d20..0x80163d80.
.globl audio_xa_state_6
.ent audio_xa_state_6
audio_xa_state_6:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    lui $at, %hi(audio_xa_state + 0x22)
    sb $zero, %lo(audio_xa_state + 0x22)($at)
    jal audio_xa_ramp_gain
    addu $a0, $zero, $zero
    addiu $a0, $zero, 1
    jal audio_cd_sync
    addu $a1, $zero, $zero
    beqz $v0, .Lxa_80163d70
    addiu $a0, $zero, 9
    addu $a1, $zero, $zero
    jal audio_cd_control
    addu $a2, $zero, $zero
    beqz $v0, .Lxa_80163d70
    addiu $v0, $zero, 7
    lui $at, %hi(audio_xa_state + 0x21)
    sb $v0, %lo(audio_xa_state + 0x21)($at)
    lui $at, %hi(audio_xa_state + 0x10)
    sh $zero, %lo(audio_xa_state + 0x10)($at)
.Lxa_80163d70:
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_xa_state_6

# exe/slus_004_22@0x80163e3c..0x80163e88.
.globl audio_xa_state_8
.ent audio_xa_state_8
audio_xa_state_8:
    addiu $sp, $sp, -0x18
    sw $s0, 0x10($sp)
    lui $s0, %hi(audio_xa_state + 0x10)
    addiu $s0, $s0, %lo(audio_xa_state + 0x10)
    addiu $a0, $zero, 1
    addiu $a1, $s0, 8
    sw $ra, 0x14($sp)
    jal audio_cd_sync
    sh $zero, 0($s0)
    beqz $v0, .Lxa_80163e74
    addiu $v0, $zero, 6
    lui $at, %hi(audio_xa_state + 0x21)
    sb $v0, %lo(audio_xa_state + 0x21)($at)
    sh $zero, 0($s0)
.Lxa_80163e74:
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_xa_state_8

# exe/slus_004_22@0x80163eb0..0x80163f50.
.globl audio_xa_set_gain
.ent audio_xa_set_gain
audio_xa_set_gain:
    addiu $sp, $sp, -0x18
    lui $v1, %hi(audio_xa_state + 0x24)
    addiu $v1, $v1, %lo(audio_xa_state + 0x24)
    sw $ra, 0x10($sp)
    sb $a0, 0($v1)
    lui $v0, %hi(audio_xa_gain_flag)
    lbu $v0, %lo(audio_xa_gain_flag)($v0)
    nop
    bnez $v0, .Lxa_80163f04
    nop
    lbu $v0, 0($v1)
    lui $at, %hi(audio_xa_state + 0x31)
    sb $zero, %lo(audio_xa_state + 0x31)($at)
    lui $at, %hi(audio_xa_state + 0x33)
    sb $zero, %lo(audio_xa_state + 0x33)($at)
    lui $at, %hi(audio_xa_state + 0x30)
    sb $v0, %lo(audio_xa_state + 0x30)($at)
    lui $at, %hi(audio_xa_state + 0x32)
    sb $v0, %lo(audio_xa_state + 0x32)($at)
    j .Lxa_80163f30
    nop
.Lxa_80163f04:
    lbu $v0, 0($v1)
    nop
    srl $v0, $v0, 1
    lui $at, %hi(audio_xa_state + 0x30)
    sb $v0, %lo(audio_xa_state + 0x30)($at)
    lui $at, %hi(audio_xa_state + 0x31)
    sb $v0, %lo(audio_xa_state + 0x31)($at)
    lui $at, %hi(audio_xa_state + 0x32)
    sb $v0, %lo(audio_xa_state + 0x32)($at)
    lui $at, %hi(audio_xa_state + 0x33)
    sb $v0, %lo(audio_xa_state + 0x33)($at)
.Lxa_80163f30:
    lui $a0, %hi(audio_xa_state + 0x30)
    addiu $a0, $a0, %lo(audio_xa_state + 0x30)
    jal audio_cd_mix
    nop
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_xa_set_gain

# exe/slus_004_22@0x80163f50..0x80164020.
.globl audio_xa_ramp_gain
.ent audio_xa_ramp_gain
audio_xa_ramp_gain:
    addiu $sp, $sp, -0x20
    lui $a1, %hi(audio_xa_state + 0x24)
    addiu $a1, $a1, %lo(audio_xa_state + 0x24)
    sw $ra, 0x18($sp)
    sw $s1, 0x14($sp)
    sw $s0, 0x10($sp)
    lbu $v1, 0($a1)
    andi $a0, $a0, 0xff
    sltu $v0, $v1, $a0
    beqz $v0, .Lxa_80163fb8
    addu $a2, $a1, $zero
    addu $s0, $a1, $zero
    addu $s1, $a0, $zero
.Lxa_80163f84:
    lbu $a0, 0($s0)
    jal audio_xa_set_gain
    nop
    lbu $v0, 0($s0)
    nop
    addiu $v0, $v0, 1
    sb $v0, 0($s0)
    andi $v0, $v0, 0xff
    sltu $v0, $v0, $s1
    beqz $v0, .Lxa_80163ff8
    nop
    j .Lxa_80163f84
    nop
.Lxa_80163fb8:
    sltu $v0, $a0, $v1
    beqz $v0, .Lxa_80163ff8
    nop
    addu $s0, $a2, $zero
    addu $s1, $a0, $zero
.Lxa_80163fcc:
    lbu $a0, 0($s0)
    jal audio_xa_set_gain
    nop
    lbu $v0, 0($s0)
    nop
    addiu $v0, $v0, -1
    sb $v0, 0($s0)
    andi $v0, $v0, 0xff
    sltu $v0, $s1, $v0
    bnez $v0, .Lxa_80163fcc
    nop
.Lxa_80163ff8:
    lui $a0, %hi(audio_xa_state + 0x24)
    lbu $a0, %lo(audio_xa_state + 0x24)($a0)
    jal audio_xa_set_gain
    nop
    lw $ra, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_xa_ramp_gain

.section .data
.balign 4
# Original adjacent arrays at 0x80183230..0x80183248.
.globl audio_xa_file_slots
audio_xa_file_slots:
    .word 681, 434, 682
.globl audio_xa_table_pointers
audio_xa_table_pointers:
    .word 0, 0, 0

# Original nine-entry table at 0x80183274..0x80183298.
.globl audio_xa_handlers
audio_xa_handlers:
    .word audio_xa_state_0, audio_xa_state_1, audio_xa_state_2, audio_xa_state_3, audio_xa_state_4, audio_xa_state_5, audio_xa_state_6, audio_xa_state_7, audio_xa_state_8
.section .bss
.balign 4
# Original zero state at 0x8014680C..0x80146841, rounded to word alignment.
.globl audio_xa_state
audio_xa_state:
    .space 56

.balign 4
# Caller-bound indexed storage corresponding to 0x80182444 slots 0..682.
# Only explicitly supplied entries are configured; zero is not a media binding.
.globl audio_xa_file_lbas
audio_xa_file_lbas:
    .space 2732

# Original zero byte at 0x80146494; START clears it and state 7 writes three.
# Other uses remain unresolved.
.globl audio_xa_start_clear_byte
audio_xa_start_clear_byte:
    .space 1

# Original live zero byte at 0x80144951; its producer remains caller-owned.
.globl audio_xa_gain_flag
audio_xa_gain_flag:
    .space 1
