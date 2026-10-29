# SPDX-License-Identifier: MIT
# Owned XA selection, START, completion callback and SDK position conversion.
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

.section .data
.balign 4
# Original adjacent arrays at 0x80183230..0x80183248.
.globl audio_xa_file_slots
audio_xa_file_slots:
    .word 681, 434, 682
.globl audio_xa_table_pointers
audio_xa_table_pointers:
    .word 0, 0, 0

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

.balign 4
# Original zero callback cell at 0x80185780; shared with future CD dispatch.
.globl audio_cd_sync_callback
audio_cd_sync_callback:
    .space 4

# Original zero byte at 0x80146494; START clears it. Other uses remain unresolved.
.globl audio_xa_start_clear_byte
audio_xa_start_clear_byte:
    .space 1
