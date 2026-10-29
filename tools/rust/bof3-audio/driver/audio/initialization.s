# SPDX-License-Identifier: MIT
# Owned audio library initialization, voice state and register flushing.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Preserve complete routines, shared state aliases and live callback bindings.
.set noreorder
.set noat
.section .text
.balign 4

# exe/slus_004_22@0x8016B1C4..0x8016B2AC
.globl audio_library_initialize
.ent audio_library_initialize
audio_library_initialize:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    lui $a2, 0x1f80
    ori $a2, $a2, 0x1c00
    addu $a0, $zero, $zero
    lui $a3, %hi(audio_library_register_defaults)
    addiu $a3, $a3, %lo(audio_library_register_defaults)
.Linitialization_8016b1e0:
    addu $a1, $zero, $zero
    addu $v1, $a3, $zero
.Linitialization_8016b1e8:
    lhu $v0, 0($v1)
    addiu $v1, $v1, 2
    addiu $a1, $a1, 1
    sh $v0, 0($a2)
    slti $v0, $a1, 8
    bnez $v0, .Linitialization_8016b1e8
    addiu $a2, $a2, 2
    addiu $a0, $a0, 1
    slti $v0, $a0, 0x18
    bnez $v0, .Linitialization_8016b1e0
    nop
    lui $a2, 0x1f80
    ori $a2, $a2, 0x1d80
    addu $a0, $zero, $zero
    lui $v1, %hi(audio_library_register_defaults + 16)
    addiu $v1, $v1, %lo(audio_library_register_defaults + 16)
.Linitialization_8016b228:
    lhu $v0, 0($v1)
    addiu $v1, $v1, 2
    addiu $a0, $a0, 1
    sh $v0, 0($a2)
    slti $v0, $a0, 0x10
    bnez $v0, .Linitialization_8016b228
    addiu $a2, $a2, 2
    jal audio_sequence_initialize_voices
    addiu $a0, $zero, 0x18
    addu $a1, $zero, $zero
    lui $v1, %hi(audio_sequence_channel_matrix)
    addiu $v1, $v1, %lo(audio_sequence_channel_matrix)
.Linitialization_8016b258:
    addiu $a0, $zero, 0xf
    addiu $v0, $v1, 0x3c
.Linitialization_8016b260:
    sw $zero, 0($v0)
    addiu $a0, $a0, -1
    bgez $a0, .Linitialization_8016b260
    addiu $v0, $v0, -4
    addiu $a1, $a1, 1
    slti $v0, $a1, 0x20
    bnez $v0, .Linitialization_8016b258
    addiu $v1, $v1, 0x40
    addiu $v0, $zero, 0x3c
    lui $at, %hi(audio_sequence_tick_rate)
    sw $v0, %lo(audio_sequence_tick_rate)($at)
    lui $at, %hi(audio_sequence_allocation_mask)
    sw $zero, %lo(audio_sequence_allocation_mask)($at)
    lui $at, %hi(audio_sequence_voice_state_e248)
    sw $zero, %lo(audio_sequence_voice_state_e248)($at)
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_library_initialize

# exe/slus_004_22@0x8016B2AC..0x8016B2DC
.globl audio_library_reset
.ent audio_library_reset
audio_library_reset:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    jal audio_callbacks_reset
    nop
    jal audio_library_initialize_spu
    nop
    jal audio_library_initialize
    nop
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_library_reset

# exe/slus_004_22@0x8016B2DC..0x8016B2FC
.globl audio_library_initialize_spu
.ent audio_library_initialize_spu
audio_library_initialize_spu:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    jal audio_spu_initialize
    addu $a0, $zero, $zero
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_library_initialize_spu

# exe/slus_004_22@0x801706A4..0x80170A58
.globl audio_sequence_flush_voices
.ent audio_sequence_flush_voices
audio_sequence_flush_voices:
    addiu $sp, $sp, -0x38
    sw $s0, 0x20($sp)
    addu $s0, $zero, $zero
    lui $v0, %hi(audio_sequence_envelope_history_index)
    lw $v0, %lo(audio_sequence_envelope_history_index)($v0)
    lui $a0, %hi(audio_sequence_voice_count)
    lbu $a0, %lo(audio_sequence_voice_count)($a0)
    lui $v1, %hi(audio_sequence_envelope_history)
    addiu $v1, $v1, %lo(audio_sequence_envelope_history)
    sw $ra, 0x34($sp)
    sw $s4, 0x30($sp)
    sw $s3, 0x2c($sp)
    sw $s2, 0x28($sp)
    sw $s1, 0x24($sp)
    addiu $v0, $v0, 1
    andi $v0, $v0, 0xf
    lui $at, %hi(audio_sequence_envelope_history_index)
    sw $v0, %lo(audio_sequence_envelope_history_index)($at)
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    blez $a0, .Linitialization_80170750
    sw $zero, 0($v0)
    addu $a2, $v0, $zero
    addiu $t0, $zero, 1
    addu $a3, $a0, $zero
    lui $a1, %hi(audio_sequence_registers_pointer)
    lw $a1, %lo(audio_sequence_registers_pointer)($a1)
    addu $a0, $zero, $zero
.Linitialization_80170714:
    lhu $v0, 0xc($a1)
    lui $at, %hi(audio_sequence_voice_globals + 166)
    addu $at, $at, $a0
    sh $v0, %lo(audio_sequence_voice_globals + 166)($at)
    bnez $v0, .Linitialization_8017073c
    sllv $v1, $t0, $s0
    lw $v0, 0($a2)
    nop
    or $v0, $v0, $v1
    sw $v0, 0($a2)
.Linitialization_8017073c:
    addiu $a1, $a1, 0x10
    addiu $s0, $s0, 1
    slt $v0, $s0, $a3
    bnez $v0, .Linitialization_80170714
    addiu $a0, $a0, 0x34
.Linitialization_80170750:
    lui $v0, %hi(audio_sequence_playback_byte)
    lbu $v0, %lo(audio_sequence_playback_byte)($v0)
    nop
    bnez $v0, .Linitialization_80170804
    addu $s0, $zero, $zero
    addiu $s2, $zero, -1
    lui $v1, %hi(audio_sequence_envelope_history)
    addiu $v1, $v1, %lo(audio_sequence_envelope_history)
.Linitialization_80170770:
    lw $v0, 0($v1)
    addiu $s0, $s0, 1
    and $s2, $s2, $v0
    slti $v0, $s0, 0xf
    bnez $v0, .Linitialization_80170770
    addiu $v1, $v1, 4
    lui $v0, %hi(audio_sequence_voice_count)
    lbu $v0, %lo(audio_sequence_voice_count)($v0)
    nop
    blez $v0, .Linitialization_80170800
    addu $s0, $zero, $zero
    addiu $s4, $zero, 1
    addiu $s3, $zero, 2
    addu $s1, $zero, $zero
.Linitialization_801707a8:
    sllv $v0, $s4, $s0
    and $v0, $s2, $v0
    beqz $v0, .Linitialization_801707e8
    nop
    lui $at, %hi(audio_sequence_voice_globals + 187)
    addu $at, $at, $s1
    lbu $v0, %lo(audio_sequence_voice_globals + 187)($at)
    nop
    bne $v0, $s3, .Linitialization_801707dc
    addu $a0, $zero, $zero
    lui $a1, 0xff
    jal audio_spu_set_reverb_voice_mask
    ori $a1, $a1, 0xffff
.Linitialization_801707dc:
    lui $at, %hi(audio_sequence_voice_globals + 187)
    addu $at, $at, $s1
    sb $zero, %lo(audio_sequence_voice_globals + 187)($at)
.Linitialization_801707e8:
    lui $v0, %hi(audio_sequence_voice_count)
    lbu $v0, %lo(audio_sequence_voice_count)($v0)
    addiu $s0, $s0, 1
    slt $v0, $s0, $v0
    bnez $v0, .Linitialization_801707a8
    addiu $s1, $s1, 0x34
.Linitialization_80170800:
    addu $s0, $zero, $zero
.Linitialization_80170804:
    lui $v0, %hi(audio_sequence_release_mask)
    lhu $v0, %lo(audio_sequence_release_mask)($v0)
    lui $v1, %hi(audio_sequence_voice_globals)
    lhu $v1, %lo(audio_sequence_voice_globals)($v1)
    nor $v0, $zero, $v0
    and $v1, $v1, $v0
    lui $v0, %hi(audio_sequence_envelope_history + 66)
    lhu $v0, %lo(audio_sequence_envelope_history + 66)($v0)
    addu $s1, $zero, $zero
    lui $at, %hi(audio_sequence_voice_globals)
    sh $v1, %lo(audio_sequence_voice_globals)($at)
    lui $v1, %hi(audio_sequence_voice_globals + 2)
    lhu $v1, %lo(audio_sequence_voice_globals + 2)($v1)
    nor $v0, $zero, $v0
    and $v1, $v1, $v0
    lui $at, %hi(audio_sequence_voice_globals + 2)
    sh $v1, %lo(audio_sequence_voice_globals + 2)($at)
.Linitialization_80170848:
    lui $at, %hi(audio_sequence_voice_globals + 188)
    addu $at, $at, $s1
    lh $v0, %lo(audio_sequence_voice_globals + 188)($at)
    nop
    beqz $v0, .Linitialization_80170874
    nop
    lui $v0, %hi(audio_sequence_voice_callback_primary)
    lw $v0, %lo(audio_sequence_voice_callback_primary)($v0)
    nop
    jalr $v0
    addu $a0, $s0, $zero
.Linitialization_80170874:
    lui $at, %hi(audio_sequence_voice_globals + 200)
    addu $at, $at, $s1
    lh $v0, %lo(audio_sequence_voice_globals + 200)($at)
    nop
    beqz $v0, .Linitialization_801708a0
    nop
    lui $v0, %hi(audio_sequence_voice_callback_secondary)
    lw $v0, %lo(audio_sequence_voice_callback_secondary)($v0)
    nop
    jalr $v0
    addu $a0, $s0, $zero
.Linitialization_801708a0:
    addiu $s0, $s0, 1
    slti $v0, $s0, 0x18
    bnez $v0, .Linitialization_80170848
    addiu $s1, $s1, 0x34
    lui $a1, %hi(audio_sequence_voice_dirty)
    addiu $a1, $a1, %lo(audio_sequence_voice_dirty)
    lui $v0, %hi(audio_sequence_voice_register_cache)
    addiu $v0, $v0, %lo(audio_sequence_voice_register_cache)
    addiu $t1, $v0, 0xa
    addiu $t0, $v0, 8
    addu $a0, $zero, $zero
    addiu $a3, $v0, 2
    addu $a2, $v0, $zero
    addiu $t2, $a1, 0x18
.Linitialization_801708d8:
    lbu $v0, 0($a1)
    nop
    andi $v0, $v0, 1
    beqz $v0, .Linitialization_8017090c
    nop
    lui $v0, %hi(audio_sequence_registers_pointer)
    lw $v0, %lo(audio_sequence_registers_pointer)($v0)
    lhu $v1, 0($a2)
    addu $v0, $a0, $v0
    sh $v1, 0($v0)
    lhu $v1, 0($a3)
    nop
    sh $v1, 2($v0)
.Linitialization_8017090c:
    lbu $v0, 0($a1)
    nop
    andi $v0, $v0, 4
    beqz $v0, .Linitialization_8017093c
    nop
    lui $v0, %hi(audio_sequence_registers_pointer)
    lw $v0, %lo(audio_sequence_registers_pointer)($v0)
    lui $at, %hi(audio_sequence_voice_flags + 20)
    addu $at, $at, $a0
    lhu $v1, %lo(audio_sequence_voice_flags + 20)($at)
    addu $v0, $a0, $v0
    sh $v1, 4($v0)
.Linitialization_8017093c:
    lbu $v0, 0($a1)
    nop
    andi $v0, $v0, 8
    beqz $v0, .Linitialization_8017096c
    nop
    lui $v0, %hi(audio_sequence_registers_pointer)
    lw $v0, %lo(audio_sequence_registers_pointer)($v0)
    lui $at, %hi(audio_sequence_voice_flags + 22)
    addu $at, $at, $a0
    lhu $v1, %lo(audio_sequence_voice_flags + 22)($at)
    addu $v0, $a0, $v0
    sh $v1, 6($v0)
.Linitialization_8017096c:
    lbu $v0, 0($a1)
    nop
    andi $v0, $v0, 0x10
    beqz $v0, .Linitialization_801709a0
    nop
    lui $v0, %hi(audio_sequence_registers_pointer)
    lw $v0, %lo(audio_sequence_registers_pointer)($v0)
    lhu $v1, 0($t0)
    addu $v0, $a0, $v0
    sh $v1, 8($v0)
    lhu $v1, 0($t1)
    nop
    sh $v1, 0xa($v0)
.Linitialization_801709a0:
    sb $zero, 0($a1)
    addiu $a1, $a1, 1
    addiu $t1, $t1, 0x10
    addiu $t0, $t0, 0x10
    addiu $a0, $a0, 0x10
    addiu $a3, $a3, 0x10
    slt $v0, $a1, $t2
    bnez $v0, .Linitialization_801708d8
    addiu $a2, $a2, 0x10
    lui $v0, %hi(audio_sequence_registers_pointer)
    lw $v0, %lo(audio_sequence_registers_pointer)($v0)
    lui $v1, %hi(audio_sequence_release_mask)
    lhu $v1, %lo(audio_sequence_release_mask)($v1)
    lui $a0, %hi(audio_sequence_envelope_history + 66)
    lhu $a0, %lo(audio_sequence_envelope_history + 66)($a0)
    lui $a1, %hi(audio_sequence_voice_globals)
    lhu $a1, %lo(audio_sequence_voice_globals)($a1)
    lui $a2, %hi(audio_sequence_voice_globals + 2)
    lhu $a2, %lo(audio_sequence_voice_globals + 2)($a2)
    lui $a3, %hi(audio_sequence_voice_globals + 4)
    lhu $a3, %lo(audio_sequence_voice_globals + 4)($a3)
    lui $t0, %hi(audio_sequence_voice_globals + 6)
    lhu $t0, %lo(audio_sequence_voice_globals + 6)($t0)
    lui $at, %hi(audio_sequence_release_mask)
    sh $zero, %lo(audio_sequence_release_mask)($at)
    lui $at, %hi(audio_sequence_envelope_history + 66)
    sh $zero, %lo(audio_sequence_envelope_history + 66)($at)
    lui $at, %hi(audio_sequence_voice_globals)
    sh $zero, %lo(audio_sequence_voice_globals)($at)
    lui $at, %hi(audio_sequence_voice_globals + 2)
    sh $zero, %lo(audio_sequence_voice_globals + 2)($at)
    sh $v1, 0x18c($v0)
    sh $a0, 0x18e($v0)
    sh $a1, 0x188($v0)
    sh $a2, 0x18a($v0)
    sh $a3, 0x198($v0)
    sh $t0, 0x19a($v0)
    lw $ra, 0x34($sp)
    lw $s4, 0x30($sp)
    lw $s3, 0x2c($sp)
    lw $s2, 0x28($sp)
    lw $s1, 0x24($sp)
    lw $s0, 0x20($sp)
    addiu $sp, $sp, 0x38
    jr $ra
    nop
.end audio_sequence_flush_voices

# exe/slus_004_22@0x80170A58..0x80170A7C
.globl audio_spu_set_reverb_voice_mask
.ent audio_spu_set_reverb_voice_mask
audio_spu_set_reverb_voice_mask:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    addiu $a2, $zero, 0xca
    jal audio_spu_update_voice_mask
    addiu $a3, $zero, 0xcb
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_spu_set_reverb_voice_mask

# exe/slus_004_22@0x80170A7C..0x80170C84
.globl audio_spu_update_voice_mask
.ent audio_spu_update_voice_mask
audio_spu_update_voice_mask:
    addu $t1, $a0, $zero
    lui $v0, %hi(audio_spu_state_3f78)
    lw $v0, %lo(audio_spu_state_3f78)($v0)
    nop
    andi $v0, $v0, 1
    beqz $v0, .Linitialization_80170aa8
    addu $t0, $a1, $zero
    lui $a0, %hi(audio_sequence_buffered_registers)
    addiu $a0, $a0, %lo(audio_sequence_buffered_registers)
    j .Linitialization_80170ab4
    sll $v0, $a3, 1
.Linitialization_80170aa8:
    lui $a0, %hi(audio_spu_registers_pointer)
    lw $a0, %lo(audio_spu_registers_pointer)($a0)
    sll $v0, $a3, 1
.Linitialization_80170ab4:
    addu $v0, $v0, $a0
    lhu $v1, 0($v0)
    sll $v0, $a2, 1
    addu $v0, $v0, $a0
    lhu $v0, 0($v0)
    andi $v1, $v1, 0xff
    sll $v1, $v1, 0x10
    beqz $t1, .Linitialization_80170ba0
    or $t2, $v0, $v1
    addiu $v0, $zero, 1
    bne $t1, $v0, .Linitialization_80170c78
    lui $v0, 0xff
    lui $v0, %hi(audio_spu_state_3f78)
    lw $v0, %lo(audio_spu_state_3f78)($v0)
    nop
    andi $v0, $v0, 1
    beqz $v0, .Linitialization_80170b58
    sll $v1, $a2, 1
    lui $a1, %hi(audio_sequence_buffered_registers)
    addiu $a1, $a1, %lo(audio_sequence_buffered_registers)
    addu $v1, $v1, $a1
    sll $a0, $a3, 1
    lhu $v0, 0($v1)
    addu $a0, $a0, $a1
    or $v0, $v0, $t0
    sh $v0, 0($v1)
    srl $v0, $t0, 0x10
    lhu $v1, 0($a0)
    andi $v0, $v0, 0xff
    or $v1, $v1, $v0
    addiu $v0, $a2, -0xc6
    sra $v0, $v0, 1
    sh $v1, 0($a0)
    lui $v1, %hi(audio_spu_state_3b44)
    lw $v1, %lo(audio_spu_state_3b44)($v1)
    sllv $v0, $t1, $v0
    or $v1, $v1, $v0
    lui $at, %hi(audio_spu_state_3b44)
    sw $v1, %lo(audio_spu_state_3b44)($at)
    j .Linitialization_80170b90
    lui $v0, 0xff
.Linitialization_80170b58:
    lui $a1, %hi(audio_spu_registers_pointer)
    lw $a1, %lo(audio_spu_registers_pointer)($a1)
    sll $a0, $a3, 1
    addu $v1, $v1, $a1
    lhu $v0, 0($v1)
    addu $a0, $a0, $a1
    or $v0, $v0, $t0
    sh $v0, 0($v1)
    srl $v0, $t0, 0x10
    lhu $v1, 0($a0)
    andi $v0, $v0, 0xff
    or $v1, $v1, $v0
    sh $v1, 0($a0)
    lui $v0, 0xff
.Linitialization_80170b90:
    ori $v0, $v0, 0xffff
    and $v0, $t0, $v0
    j .Linitialization_80170c74
    or $t2, $t2, $v0
.Linitialization_80170ba0:
    lui $v0, %hi(audio_spu_state_3f78)
    lw $v0, %lo(audio_spu_state_3f78)($v0)
    nop
    andi $v0, $v0, 1
    beqz $v0, .Linitialization_80170c20
    sll $v1, $a2, 1
    lui $a1, %hi(audio_sequence_buffered_registers)
    addiu $a1, $a1, %lo(audio_sequence_buffered_registers)
    addu $v1, $v1, $a1
    lhu $v0, 0($v1)
    nor $a0, $zero, $t0
    and $v0, $v0, $a0
    sll $a0, $a3, 1
    addu $a0, $a0, $a1
    sh $v0, 0($v1)
    srl $v0, $t0, 0x10
    andi $v0, $v0, 0xff
    lhu $v1, 0($a0)
    nor $v0, $zero, $v0
    and $v1, $v1, $v0
    sh $v1, 0($a0)
    addiu $a0, $a2, -0xc6
    sra $a0, $a0, 1
    addiu $v0, $zero, 1
    lui $v1, %hi(audio_spu_state_3b44)
    lw $v1, %lo(audio_spu_state_3b44)($v1)
    sllv $v0, $v0, $a0
    or $v1, $v1, $v0
    lui $at, %hi(audio_spu_state_3b44)
    sw $v1, %lo(audio_spu_state_3b44)($at)
    j .Linitialization_80170c64
    lui $v0, 0xff
.Linitialization_80170c20:
    lui $a1, %hi(audio_spu_registers_pointer)
    lw $a1, %lo(audio_spu_registers_pointer)($a1)
    nop
    addu $v1, $v1, $a1
    lhu $v0, 0($v1)
    nor $a0, $zero, $t0
    and $v0, $v0, $a0
    sll $a0, $a3, 1
    addu $a0, $a0, $a1
    sh $v0, 0($v1)
    srl $v0, $t0, 0x10
    andi $v0, $v0, 0xff
    lhu $v1, 0($a0)
    nor $v0, $zero, $v0
    and $v1, $v1, $v0
    sh $v1, 0($a0)
    lui $v0, 0xff
.Linitialization_80170c64:
    ori $v0, $v0, 0xffff
    and $v0, $t0, $v0
    nor $v0, $zero, $v0
    and $t2, $t2, $v0
.Linitialization_80170c74:
    lui $v0, 0xff
.Linitialization_80170c78:
    ori $v0, $v0, 0xffff
    jr $ra
    and $v0, $t2, $v0
.end audio_spu_update_voice_mask

# exe/slus_004_22@0x80170C84..0x80170F94
.globl audio_sequence_initialize_voices
.ent audio_sequence_initialize_voices
audio_sequence_initialize_voices:
    addiu $sp, $sp, -0x20
    sw $s1, 0x14($sp)
    addu $s1, $a0, $zero
    addu $a0, $zero, $zero
    sw $ra, 0x18($sp)
    jal audio_spu_set_transfer_mode
    sw $s0, 0x10($sp)
    lui $a1, %hi(audio_sequence_allocation_records)
    addiu $a1, $a1, %lo(audio_sequence_allocation_records)
    lui $at, %hi(audio_sequence_voice_state_e1e0)
    sh $zero, %lo(audio_sequence_voice_state_e1e0)($at)
    jal audio_spu_initialize_allocations
    addiu $a0, $zero, 0x20
    addu $s0, $zero, $zero
    lui $v1, %hi(audio_sequence_voice_register_cache)
    addiu $v1, $v1, %lo(audio_sequence_voice_register_cache)
    andi $v0, $s0, 0xffff
.Linitialization_80170cc8:
    sll $v0, $v0, 1
    addu $v0, $v0, $v1
    sh $zero, 0($v0)
    addiu $s0, $s0, 1
    andi $v0, $s0, 0xffff
    sltiu $v0, $v0, 0xc0
    bnez $v0, .Linitialization_80170cc8
    andi $v0, $s0, 0xffff
    addu $s0, $zero, $zero
    andi $v0, $s0, 0xffff
.Linitialization_80170cf0:
    lui $at, %hi(audio_sequence_voice_dirty)
    addu $at, $at, $v0
    sb $zero, %lo(audio_sequence_voice_dirty)($at)
    addiu $s0, $s0, 1
    andi $v0, $s0, 0xffff
    sltiu $v0, $v0, 0x18
    bnez $v0, .Linitialization_80170cf0
    andi $v0, $s0, 0xffff
    lui $at, %hi(audio_sequence_voice_state_0bd0)
    sh $zero, %lo(audio_sequence_voice_state_0bd0)($at)
    addu $s0, $zero, $zero
    andi $v0, $s0, 0xffff
.Linitialization_80170d20:
    lui $at, %hi(audio_sequence_voice_flags)
    addu $at, $at, $v0
    sb $zero, %lo(audio_sequence_voice_flags)($at)
    addiu $s0, $s0, 1
    andi $v0, $s0, 0xffff
    sltiu $v0, $v0, 0x10
    bnez $v0, .Linitialization_80170d20
    andi $v0, $s0, 0xffff
    andi $a0, $s1, 0xff
    sltiu $v0, $a0, 0x18
    bnez $v0, .Linitialization_80170d60
    addiu $v0, $zero, 0x18
    lui $at, %hi(audio_sequence_voice_count)
    sb $v0, %lo(audio_sequence_voice_count)($at)
    j .Linitialization_80170d68
    nop
.Linitialization_80170d60:
    lui $at, %hi(audio_sequence_voice_count)
    sb $a0, %lo(audio_sequence_voice_count)($at)
.Linitialization_80170d68:
    lui $v0, %hi(audio_sequence_voice_count)
    lbu $v0, %lo(audio_sequence_voice_count)($v0)
    nop
    beqz $v0, .Linitialization_80170f0c
    addu $s0, $zero, $zero
    addiu $s1, $zero, 0xff
    andi $v1, $s0, 0xffff
.Linitialization_80170d84:
    sll $a1, $v1, 3
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addiu $v1, $zero, 0x18
    lui $at, %hi(audio_sequence_voice_globals + 162)
    addu $at, $at, $v0
    sh $v1, %lo(audio_sequence_voice_globals + 162)($at)
    addiu $v1, $zero, -1
    lui $at, %hi(audio_sequence_voice_globals + 174)
    addu $at, $at, $v0
    sh $v1, %lo(audio_sequence_voice_globals + 174)($at)
    addiu $v1, $zero, 0x40
    andi $a1, $a1, 0xffff
    lui $at, %hi(audio_sequence_voice_records)
    addu $at, $at, $v0
    sh $s1, %lo(audio_sequence_voice_records)($at)
    lui $at, %hi(audio_sequence_voice_globals + 187)
    addu $at, $at, $v0
    sb $zero, %lo(audio_sequence_voice_globals + 187)($at)
    lui $at, %hi(audio_sequence_voice_globals + 164)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 164)($at)
    lui $at, %hi(audio_sequence_voice_globals + 166)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 166)($at)
    lui $at, %hi(audio_sequence_voice_globals + 176)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 176)($at)
    lui $at, %hi(audio_sequence_voice_globals + 178)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 178)($at)
    lui $at, %hi(audio_sequence_voice_globals + 180)
    addu $at, $at, $v0
    sh $s1, %lo(audio_sequence_voice_globals + 180)($at)
    lui $at, %hi(audio_sequence_voice_globals + 168)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 168)($at)
    lui $at, %hi(audio_sequence_voice_globals + 170)
    addu $at, $at, $v0
    sb $v1, %lo(audio_sequence_voice_globals + 170)($at)
    lui $at, %hi(audio_sequence_voice_globals + 188)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 188)($at)
    lui $at, %hi(audio_sequence_voice_globals + 190)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 190)($at)
    lui $at, %hi(audio_sequence_voice_globals + 192)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 192)($at)
    lui $at, %hi(audio_sequence_voice_globals + 194)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 194)($at)
    lui $at, %hi(audio_sequence_voice_globals + 200)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 200)($at)
    lui $at, %hi(audio_sequence_voice_globals + 202)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 202)($at)
    lui $at, %hi(audio_sequence_voice_globals + 204)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 204)($at)
    lui $at, %hi(audio_sequence_voice_globals + 206)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 206)($at)
    lui $at, %hi(audio_sequence_voice_globals + 208)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 208)($at)
    lui $at, %hi(audio_sequence_voice_globals + 196)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 196)($at)
    lui $v0, %hi(audio_sequence_registers_pointer)
    lw $v0, %lo(audio_sequence_registers_pointer)($v0)
    sll $a1, $a1, 1
    addu $a1, $a1, $v0
    addiu $v0, $zero, 0x200
    sh $v0, 6($a1)
    addiu $v0, $zero, 0x1000
    sh $v0, 4($a1)
    ori $v0, $zero, 0x80ff
    sh $v0, 8($a1)
    addiu $v0, $zero, 0x4000
    sh $zero, 0($a1)
    sh $zero, 2($a1)
    sh $v0, 0xa($a1)
    lui $at, %hi(audio_sequence_active_voice)
    sh $s0, %lo(audio_sequence_active_voice)($at)
    jal audio_sequence_release_voice
    addiu $a0, $zero, 1
    addiu $s0, $s0, 1
    lui $v1, %hi(audio_sequence_voice_count)
    lbu $v1, %lo(audio_sequence_voice_count)($v1)
    andi $v0, $s0, 0xffff
    sltu $v0, $v0, $v1
    bnez $v0, .Linitialization_80170d84
    andi $v1, $s0, 0xffff
.Linitialization_80170f0c:
    addiu $v0, $zero, 0x3fff
    lui $at, %hi(audio_sequence_voice_globals + 1416)
    sh $v0, %lo(audio_sequence_voice_globals + 1416)($at)
    lui $at, %hi(audio_sequence_voice_globals + 1418)
    sh $v0, %lo(audio_sequence_voice_globals + 1418)($at)
    addiu $v0, $zero, 0x80
    lui $at, %hi(audio_sequence_voice_globals)
    sh $zero, %lo(audio_sequence_voice_globals)($at)
    lui $at, %hi(audio_sequence_voice_globals + 2)
    sh $zero, %lo(audio_sequence_voice_globals + 2)($at)
    lui $at, %hi(audio_sequence_release_mask)
    sh $zero, %lo(audio_sequence_release_mask)($at)
    lui $at, %hi(audio_sequence_voice_globals + 4)
    sh $zero, %lo(audio_sequence_voice_globals + 4)($at)
    lui $at, %hi(audio_sequence_voice_globals + 6)
    sh $zero, %lo(audio_sequence_voice_globals + 6)($at)
    lui $at, %hi(audio_sequence_reverb_attributes)
    sw $zero, %lo(audio_sequence_reverb_attributes)($at)
    lui $at, %hi(audio_sequence_voice_globals + 1412)
    sw $zero, %lo(audio_sequence_voice_globals + 1412)($at)
    lui $at, %hi(audio_sequence_playback_byte)
    sb $zero, %lo(audio_sequence_playback_byte)($at)
    lui $at, %hi(audio_sequence_voice_control_e246)
    sh $zero, %lo(audio_sequence_voice_control_e246)($at)
    lui $at, %hi(audio_sequence_voice_control_e24c)
    sh $v0, %lo(audio_sequence_voice_control_e24c)($at)
    jal audio_sequence_flush_voices
    nop
    lw $ra, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_sequence_initialize_voices

# exe/slus_004_22@0x80170F94..0x80170FE8
.globl audio_spu_initialize_allocations
.ent audio_spu_initialize_allocations
audio_spu_initialize_allocations:
    addu $v0, $a0, $zero
    bgtz $v0, .Linitialization_80170fa8
    lui $v1, 0x4000
    j .Linitialization_80170fe0
    addu $v0, $zero, $zero
.Linitialization_80170fa8:
    lui $a0, %hi(audio_spu_transfer_shift)
    lw $a0, %lo(audio_spu_transfer_shift)($a0)
    ori $v1, $v1, 0x1010
    sw $v1, 0($a1)
    lui $v1, 1
    lui $at, %hi(audio_spu_state_3f88)
    sw $a1, %lo(audio_spu_state_3f88)($at)
    lui $at, %hi(audio_spu_state_3f84)
    sw $zero, %lo(audio_spu_state_3f84)($at)
    lui $at, %hi(audio_spu_state_3f80)
    sw $v0, %lo(audio_spu_state_3f80)($at)
    sllv $v1, $v1, $a0
    addiu $v1, $v1, -0x1010
    sw $v1, 4($a1)
.Linitialization_80170fe0:
    jr $ra
    nop
.end audio_spu_initialize_allocations

# exe/slus_004_22@0x80170FE8..0x80171014
.globl audio_spu_set_transfer_mode
.ent audio_spu_set_transfer_mode
audio_spu_set_transfer_mode:
    addiu $v0, $zero, 1
    bne $a0, $v0, .Linitialization_80171004
    nop
    lui $at, %hi(audio_spu_state_3ae4)
    sw $zero, %lo(audio_spu_state_3ae4)($at)
    j .Linitialization_8017100c
    nop
.Linitialization_80171004:
    lui $at, %hi(audio_spu_state_3ae4)
    sw $v0, %lo(audio_spu_state_3ae4)($at)
.Linitialization_8017100c:
    jr $ra
    nop
.end audio_spu_set_transfer_mode

# exe/slus_004_22@0x8017212C..0x801721FC
.globl audio_sequence_release_voice
.ent audio_sequence_release_voice
audio_sequence_release_voice:
    lui $a0, %hi(audio_sequence_active_voice)
    lhu $a0, %lo(audio_sequence_active_voice)($a0)
    nop
    andi $v1, $a0, 0xffff
    sltiu $v0, $v1, 0x10
    beqz $v0, .Linitialization_80172154
    addiu $v0, $zero, 1
    sllv $a2, $v0, $v1
    j .Linitialization_80172164
    addu $a1, $zero, $zero
.Linitialization_80172154:
    addu $a2, $zero, $zero
    addiu $v1, $v1, -0x10
    sllv $a1, $v0, $v1
    andi $v1, $a0, 0xffff
.Linitialization_80172164:
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    addu $v0, $v0, $v1
    sll $v0, $v0, 2
    lui $at, %hi(audio_sequence_voice_globals + 187)
    addu $at, $at, $v0
    sb $zero, %lo(audio_sequence_voice_globals + 187)($at)
    lui $v1, %hi(audio_sequence_release_mask)
    lhu $v1, %lo(audio_sequence_release_mask)($v1)
    lui $a0, %hi(audio_sequence_envelope_history + 66)
    lhu $a0, %lo(audio_sequence_envelope_history + 66)($a0)
    lui $at, %hi(audio_sequence_voice_globals + 164)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_globals + 164)($at)
    lui $at, %hi(audio_sequence_voice_records)
    addu $at, $at, $v0
    sh $zero, %lo(audio_sequence_voice_records)($at)
    lui $v0, %hi(audio_sequence_voice_globals)
    lhu $v0, %lo(audio_sequence_voice_globals)($v0)
    or $v1, $v1, $a2
    lui $at, %hi(audio_sequence_release_mask)
    sh $v1, %lo(audio_sequence_release_mask)($at)
    nor $v1, $zero, $v1
    and $v0, $v0, $v1
    lui $at, %hi(audio_sequence_voice_globals)
    sh $v0, %lo(audio_sequence_voice_globals)($at)
    lui $v0, %hi(audio_sequence_voice_globals + 2)
    lhu $v0, %lo(audio_sequence_voice_globals + 2)($v0)
    or $a0, $a0, $a1
    lui $at, %hi(audio_sequence_envelope_history + 66)
    sh $a0, %lo(audio_sequence_envelope_history + 66)($at)
    nor $a0, $zero, $a0
    and $v0, $v0, $a0
    lui $at, %hi(audio_sequence_voice_globals + 2)
    sh $v0, %lo(audio_sequence_voice_globals + 2)($at)
    jr $ra
    nop
.end audio_sequence_release_voice

.section .data
.balign 4

# exe/slus_004_22@0x80184410, 48 bytes.
.balign 4
.globl audio_library_register_defaults
audio_library_register_defaults:
    .half 0x0000, 0x0000, 0x1000, 0x3000, 0x00bf, 0x0000, 0x0000, 0x0000
    .half 0x3fff, 0x3fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
    .half 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000

# exe/slus_004_22@0x80184458, 4 bytes.
.balign 4
.globl audio_sequence_registers_pointer
audio_sequence_registers_pointer:
    .word 0x1f801c00

.section .bss
.balign 4

# exe/slus_004_22@0x8018B4C0, 256 bytes.
.balign 4
.globl audio_sequence_allocation_records
audio_sequence_allocation_records:
    .space 256

# exe/slus_004_22@0x8018DB50, 1880 bytes.
.balign 4
.globl audio_sequence_voice_globals
audio_sequence_voice_globals:
    .space 160
.globl audio_sequence_voice_records
audio_sequence_voice_records:
    .space 1208
.globl audio_sequence_buffered_registers
audio_sequence_buffered_registers:
    .space 40
.globl audio_sequence_reverb_attributes
audio_sequence_reverb_attributes:
    .space 24
.globl audio_sequence_voice_dirty
audio_sequence_voice_dirty:
    .space 248
.globl audio_sequence_voice_state_e1e0
audio_sequence_voice_state_e1e0:
    .space 80
.globl audio_spu_register_shadow
audio_spu_register_shadow:
    .space 22
.globl audio_sequence_voice_control_e246
audio_sequence_voice_control_e246:
    .space 2
.globl audio_sequence_voice_state_e248
audio_sequence_voice_state_e248:
    .space 4
.globl audio_sequence_voice_control_e24c
audio_sequence_voice_control_e24c:
    .space 24
.globl audio_sequence_voice_count
audio_sequence_voice_count:
    .space 68

# exe/slus_004_22@0x8018E7D0, 8 bytes.
.balign 4
.globl audio_sequence_allocation_mask
audio_sequence_allocation_mask:
    .space 4
.globl audio_sequence_voice_callback_secondary
audio_sequence_voice_callback_secondary:
    .space 4

# exe/slus_004_22@0x8018E7F2, 2 bytes.
.balign 4
.globl audio_sequence_active_voice
audio_sequence_active_voice:
    .space 2

# exe/slus_004_22@0x8018E7F8, 405 bytes.
.balign 4
.globl audio_sequence_voice_flags
audio_sequence_voice_flags:
    .space 16
.globl audio_sequence_voice_register_cache
audio_sequence_voice_register_cache:
    .space 384
.globl audio_sequence_voice_callback_primary
audio_sequence_voice_callback_primary:
    .space 4
.globl audio_sequence_playback_byte
audio_sequence_playback_byte:
    .space 1

# exe/slus_004_22@0x8018EAF8, 4 bytes.
.balign 4
.globl audio_sequence_envelope_history_index
audio_sequence_envelope_history_index:
    .space 4

# exe/slus_004_22@0x80190BD0, 2 bytes.
.balign 4
.globl audio_sequence_voice_state_0bd0
audio_sequence_voice_state_0bd0:
    .space 2

# exe/slus_004_22@0x80190C18, 68 bytes.
.balign 4
.globl audio_sequence_envelope_history
audio_sequence_envelope_history:
    .space 64
.globl audio_sequence_release_mask
audio_sequence_release_mask:
    .space 4
