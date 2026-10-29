# SPDX-License-Identifier: MIT
# Owned SPU initialization and its installed DMA callback.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Both initialization modes, live callbacks, timeout paths and delays remain.
# The FIFO caller supplies readable halfwords, rounding odd byte counts upward.
.set noreorder
.set noat
.section .text
.balign 4

# exe/slus_004_22@0x80168374..0x80168608
.globl audio_spu_initialize_common
.ent audio_spu_initialize_common
audio_spu_initialize_common:
    addiu $sp, $sp, -0x20
    sw $s0, 0x10($sp)
    addu $s0, $a0, $zero
    lui $a0, %hi(audio_spu_priority_pointer)
    lw $a0, %lo(audio_spu_priority_pointer)($a0)
    sw $ra, 0x18($sp)
    sw $s1, 0x14($sp)
    lw $v0, 0($a0)
    lui $v1, 0xb
    or $v0, $v0, $v1
    sw $v0, 0($a0)
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    lui $at, %hi(audio_spu_state_3acc)
    sw $zero, %lo(audio_spu_state_3acc)($at)
    lui $at, %hi(audio_spu_state_3ad0)
    sw $zero, %lo(audio_spu_state_3ad0)($at)
    lui $at, %hi(audio_spu_transfer_address)
    sh $zero, %lo(audio_spu_transfer_address)($at)
    sh $zero, 0x180($v0)
    sh $zero, 0x182($v0)
    sh $zero, 0x1aa($v0)
    jal audio_spu_delay
    nop
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    sh $zero, 0x180($v0)
    sh $zero, 0x182($v0)
    lhu $v0, 0x1ae($v0)
    nop
    andi $v0, $v0, 0x7ff
    beqz $v0, .Lspu_8016844c
    addu $v1, $zero, $zero
    addiu $v1, $v1, 1
.Lspu_80168400:
    sltiu $v0, $v1, 0xf01
    bnez $v0, .Lspu_8016842c
    nop
    lui $a0, %hi(audio_spu_timeout_format)
    addiu $a0, $a0, %lo(audio_spu_timeout_format)
    lui $a1, %hi(audio_spu_wait_reset)
    addiu $a1, $a1, %lo(audio_spu_wait_reset)
    jal audio_spu_service_printf
    nop
    j .Lspu_80168450
    addu $a0, $zero, $zero
.Lspu_8016842c:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $v0, 0x1ae($v0)
    nop
    andi $v0, $v0, 0x7ff
    bnez $v0, .Lspu_80168400
    addiu $v1, $v1, 1
.Lspu_8016844c:
    addu $a0, $zero, $zero
.Lspu_80168450:
    lui $a1, %hi(audio_spu_register_shadow)
    addiu $a1, $a1, %lo(audio_spu_register_shadow)
    addiu $v0, $zero, 2
    lui $at, %hi(audio_spu_state_3ad4)
    sw $v0, %lo(audio_spu_state_3ad4)($at)
    addiu $v0, $zero, 3
    lui $at, %hi(audio_spu_transfer_shift)
    sw $v0, %lo(audio_spu_transfer_shift)($at)
    addiu $v0, $zero, 8
    lui $at, %hi(audio_spu_state_3adc)
    sw $v0, %lo(audio_spu_state_3adc)($at)
    addiu $v0, $zero, 7
    lui $at, %hi(audio_spu_state_3ae0)
    sw $v0, %lo(audio_spu_state_3ae0)($at)
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    addiu $v1, $zero, 4
    sh $v1, 0x1ac($v0)
    ori $v1, $zero, 0xffff
    sh $zero, 0x184($v0)
    sh $zero, 0x186($v0)
    sh $v1, 0x18c($v0)
    sh $v1, 0x18e($v0)
    sh $zero, 0x198($v0)
    sh $zero, 0x19a($v0)
.Lspu_801684b4:
    sh $zero, 0($a1)
    addiu $a0, $a0, 1
    slti $v0, $a0, 0xa
    bnez $v0, .Lspu_801684b4
    addiu $a1, $a1, 2
    bnez $s0, .Lspu_801685c4
    addu $v0, $zero, $zero
    lui $a0, %hi(audio_spu_reset_pattern)
    addiu $a0, $a0, %lo(audio_spu_reset_pattern)
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    addiu $v1, $zero, 0x200
    lui $at, %hi(audio_spu_transfer_address)
    sh $v1, %lo(audio_spu_transfer_address)($at)
    sh $zero, 0x190($v0)
    sh $zero, 0x192($v0)
    sh $zero, 0x194($v0)
    sh $zero, 0x196($v0)
    sh $zero, 0x1b0($v0)
    sh $zero, 0x1b2($v0)
    sh $zero, 0x1b4($v0)
    sh $zero, 0x1b6($v0)
    jal audio_spu_write_fifo
    addiu $a1, $zero, 0x10
    addu $a0, $zero, $zero
    addiu $a2, $zero, 0x3fff
    addiu $a1, $zero, 0x200
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
.Lspu_80168528:
    nop
    sh $zero, 0($v1)
    sh $zero, 2($v1)
    sh $a2, 4($v1)
    sh $a1, 6($v1)
    sh $zero, 8($v1)
    sh $zero, 0xa($v1)
    addiu $a0, $a0, 1
    slti $v0, $a0, 0x18
    bnez $v0, .Lspu_80168528
    addiu $v1, $v1, 0x10
    ori $s1, $zero, 0xffff
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    addiu $s0, $zero, 0xff
    sh $s1, 0x188($v0)
    sh $s0, 0x18a($v0)
    jal audio_spu_delay
    nop
    jal audio_spu_delay
    nop
    jal audio_spu_delay
    nop
    jal audio_spu_delay
    nop
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    sh $s1, 0x18c($v0)
    sh $s0, 0x18e($v0)
    jal audio_spu_delay
    nop
    jal audio_spu_delay
    nop
    jal audio_spu_delay
    nop
    jal audio_spu_delay
    nop
    addu $v0, $zero, $zero
.Lspu_801685c4:
    lui $a0, %hi(audio_spu_registers_pointer)
    lw $a0, %lo(audio_spu_registers_pointer)($a0)
    addiu $v1, $zero, 1
    lui $at, %hi(audio_spu_state_3ae4)
    sw $v1, %lo(audio_spu_state_3ae4)($at)
    ori $v1, $zero, 0xc000
    sh $v1, 0x1aa($a0)
    lui $at, %hi(audio_spu_transfer_callback)
    sw $zero, %lo(audio_spu_transfer_callback)($at)
    lui $at, %hi(audio_spu_state_3aec)
    sw $zero, %lo(audio_spu_state_3aec)($at)
    lw $ra, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x20
    jr $ra
    nop
.end audio_spu_initialize_common

# exe/slus_004_22@0x80168608..0x801687D4
.globl audio_spu_write_fifo
.ent audio_spu_write_fifo
audio_spu_write_fifo:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    lui $v1, %hi(audio_spu_transfer_address)
    lhu $v1, %lo(audio_spu_transfer_address)($v1)
    addiu $sp, $sp, -0x30
    sw $s1, 0x1c($sp)
    addu $s1, $a1, $zero
    sw $ra, 0x28($sp)
    sw $s3, 0x24($sp)
    sw $s2, 0x20($sp)
    sw $s0, 0x18($sp)
    lhu $a1, 0x1ae($v0)
    addu $s2, $a0, $zero
    sh $v1, 0x1a6($v0)
    jal audio_spu_delay
    andi $s3, $a1, 0x7ff
    beqz $s1, .Lspu_80168738
    sltiu $v0, $s1, 0x41
.Lspu_80168650:
    beqz $v0, .Lspu_8016865c
    addiu $s0, $zero, 0x40
    addu $s0, $s1, $zero
.Lspu_8016865c:
    blez $s0, .Lspu_80168688
    addu $v1, $zero, $zero
    lui $a0, %hi(audio_spu_registers_pointer)
    lw $a0, %lo(audio_spu_registers_pointer)($a0)
.Lspu_8016866c:
    lhu $v0, 0($s2)
    addiu $s2, $s2, 2
    addiu $v1, $v1, 2
    sh $v0, 0x1a8($a0)
    slt $v0, $v1, $s0
    bnez $v0, .Lspu_8016866c
    nop
.Lspu_80168688:
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    nop
    lhu $a0, 0x1aa($v1)
    nop
    andi $v0, $a0, 0xffcf
    ori $a0, $v0, 0x10
    sh $a0, 0x1aa($v1)
    jal audio_spu_delay
    nop
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $v0, 0x1ae($v0)
    nop
    andi $v0, $v0, 0x400
    beqz $v0, .Lspu_80168720
    addu $v1, $zero, $zero
    addiu $v1, $v1, 1
.Lspu_801686d4:
    sltiu $v0, $v1, 0xf01
    bnez $v0, .Lspu_80168700
    nop
    lui $a0, %hi(audio_spu_timeout_format)
    addiu $a0, $a0, %lo(audio_spu_timeout_format)
    lui $a1, %hi(audio_spu_wait_ready)
    addiu $a1, $a1, %lo(audio_spu_wait_ready)
    jal audio_spu_service_printf
    nop
    j .Lspu_80168720
    nop
.Lspu_80168700:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $v0, 0x1ae($v0)
    nop
    andi $v0, $v0, 0x400
    bnez $v0, .Lspu_801686d4
    addiu $v1, $v1, 1
.Lspu_80168720:
    jal audio_spu_delay
    subu $s1, $s1, $s0
    jal audio_spu_delay
    nop
    bnez $s1, .Lspu_80168650
    sltiu $v0, $s1, 0x41
.Lspu_80168738:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $a0, 0x1aa($v0)
    addu $v1, $zero, $zero
    andi $a0, $a0, 0xffcf
    sh $a0, 0x1aa($v0)
    lhu $v0, 0x1ae($v0)
    andi $a1, $s3, 0xffff
    andi $v0, $v0, 0x7ff
    beq $v0, $a1, .Lspu_801687b4
    addiu $v1, $v1, 1
.Lspu_80168768:
    sltiu $v0, $v1, 0xf01
    bnez $v0, .Lspu_80168794
    nop
    lui $a0, %hi(audio_spu_timeout_format)
    addiu $a0, $a0, %lo(audio_spu_timeout_format)
    lui $a1, %hi(audio_spu_wait_fifo)
    addiu $a1, $a1, %lo(audio_spu_wait_fifo)
    jal audio_spu_service_printf
    nop
    j .Lspu_801687b4
    nop
.Lspu_80168794:
    lui $v0, %hi(audio_spu_registers_pointer)
    lw $v0, %lo(audio_spu_registers_pointer)($v0)
    nop
    lhu $v0, 0x1ae($v0)
    nop
    andi $v0, $v0, 0x7ff
    bne $v0, $a1, .Lspu_80168768
    addiu $v1, $v1, 1
.Lspu_801687b4:
    lw $ra, 0x28($sp)
    lw $s3, 0x24($sp)
    lw $s2, 0x20($sp)
    lw $s1, 0x1c($sp)
    lw $s0, 0x18($sp)
    addiu $sp, $sp, 0x30
    jr $ra
    nop
.end audio_spu_write_fifo

# exe/slus_004_22@0x801687D4..0x801688A4
.globl audio_spu_dma_callback
.ent audio_spu_dma_callback
audio_spu_dma_callback:
    lui $v0, %hi(audio_spu_transfer_direction)
    lw $v0, %lo(audio_spu_transfer_direction)($v0)
    addiu $sp, $sp, -0x18
    bnez $v0, .Lspu_80168800
    sw $ra, 0x10($sp)
    jal audio_spu_delay
    nop
    jal audio_spu_delay
    nop
    jal audio_spu_delay
    nop
.Lspu_80168800:
    lui $a0, %hi(audio_spu_registers_pointer)
    lw $a0, %lo(audio_spu_registers_pointer)($a0)
    nop
    lhu $v0, 0x1aa($a0)
    nop
    andi $v0, $v0, 0xffcf
    sh $v0, 0x1aa($a0)
    lhu $v0, 0x1aa($a0)
    nop
    andi $v0, $v0, 0x30
    beqz $v0, .Lspu_80168858
    addu $v1, $zero, $zero
    addiu $v1, $v1, 1
.Lspu_80168834:
    sltiu $v0, $v1, 0xf01
    beqz $v0, .Lspu_80168858
    nop
    lhu $v0, 0x1aa($a0)
    nop
    andi $v0, $v0, 0x30
    bnez $v0, .Lspu_80168834
    addiu $v1, $v1, 1
    addiu $v1, $v1, -1
.Lspu_80168858:
    lui $v0, %hi(audio_spu_transfer_callback)
    lw $v0, %lo(audio_spu_transfer_callback)($v0)
    nop
    beqz $v0, .Lspu_80168888
    lui $a0, 0xf000
    lui $v0, %hi(audio_spu_transfer_callback)
    lw $v0, %lo(audio_spu_transfer_callback)($v0)
    nop
    jalr $v0
    nop
    j .Lspu_80168894
    nop
.Lspu_80168888:
    ori $a0, $a0, 9
    jal audio_spu_service_deliver_event
    addiu $a1, $zero, 0x20
.Lspu_80168894:
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_spu_dma_callback

# exe/slus_004_22@0x80168CE0..0x80168D28
.globl audio_spu_write_register
.ent audio_spu_write_register
audio_spu_write_register:
    bnez $a2, .Lspu_80168d04
    sll $v0, $a0, 1
    lui $v1, %hi(audio_spu_registers_pointer)
    lw $v1, %lo(audio_spu_registers_pointer)($v1)
    nop
    addu $v0, $v0, $v1
    sh $a1, 0($v0)
    j .Lspu_80168d20
    nop
.Lspu_80168d04:
    lui $a0, %hi(audio_spu_registers_pointer)
    lw $a0, %lo(audio_spu_registers_pointer)($a0)
    lui $v1, %hi(audio_spu_transfer_shift)
    lw $v1, %lo(audio_spu_transfer_shift)($v1)
    addu $v0, $v0, $a0
    srlv $v1, $a1, $v1
    sh $v1, 0($v0)
.Lspu_80168d20:
    jr $ra
    nop
.end audio_spu_write_register

# exe/slus_004_22@0x80168EC0..0x80168F20
.globl audio_spu_delay
.ent audio_spu_delay
audio_spu_delay:
    addiu $sp, $sp, -8
    addiu $v0, $zero, 0xd
    sw $v0, 4($sp)
    sw $zero, 0($sp)
    j .Lspu_80168f00
    nop
.Lspu_80168ed8:
    lw $v1, 4($sp)
    nop
    sll $v0, $v1, 1
    addu $v0, $v0, $v1
    sw $v0, 4($sp)
    lw $v0, 0($sp)
    nop
    addiu $v0, $v0, 1
    sw $v0, 0($sp)
    lw $v0, 0($sp)
.Lspu_80168f00:
    lw $v0, 0($sp)
    nop
    slti $v0, $v0, 0xf0
    bnez $v0, .Lspu_80168ed8
    nop
    addiu $sp, $sp, 8
    jr $ra
    nop
.end audio_spu_delay

# exe/slus_004_22@0x80168F20..0x80169018
.globl audio_spu_initialize
.ent audio_spu_initialize
audio_spu_initialize:
    addiu $sp, $sp, -0x18
    sw $s0, 0x10($sp)
    sw $ra, 0x14($sp)
    jal audio_callbacks_reset
    addu $s0, $a0, $zero
    jal audio_spu_initialize_common
    addu $a0, $s0, $zero
    bnez $s0, .Lspu_80168f60
    ori $a0, $zero, 0xc000
    addiu $v1, $zero, 0x17
    lui $v0, %hi(audio_spu_voice_control_shadow + 46)
    addiu $v0, $v0, %lo(audio_spu_voice_control_shadow + 46)
.Lspu_80168f50:
    sh $a0, 0($v0)
    addiu $v1, $v1, -1
    bgez $v1, .Lspu_80168f50
    addiu $v0, $v0, -2
.Lspu_80168f60:
    jal audio_spu_initialize_dma
    nop
    addiu $a0, $zero, 0xd1
    lui $a1, %hi(audio_spu_initial_reverb_start)
    lw $a1, %lo(audio_spu_initial_reverb_start)($a1)
    lui $at, %hi(audio_spu_state_3b20)
    sw $zero, %lo(audio_spu_state_3b20)($at)
    lui $at, %hi(audio_spu_state_3b24)
    sw $zero, %lo(audio_spu_state_3b24)($at)
    lui $at, %hi(audio_spu_state_3b30)
    sw $zero, %lo(audio_spu_state_3b30)($at)
    lui $at, %hi(audio_spu_state_3b34)
    sh $zero, %lo(audio_spu_state_3b34)($at)
    lui $at, %hi(audio_spu_state_3b36)
    sh $zero, %lo(audio_spu_state_3b36)($at)
    lui $at, %hi(audio_spu_state_3b38)
    sw $zero, %lo(audio_spu_state_3b38)($at)
    lui $at, %hi(audio_spu_state_3b3c)
    sw $zero, %lo(audio_spu_state_3b3c)($at)
    lui $at, %hi(audio_spu_state_3b28)
    sw $a1, %lo(audio_spu_state_3b28)($at)
    jal audio_spu_write_register
    addu $a2, $zero, $zero
    lui $at, %hi(audio_spu_state_3f80)
    sw $zero, %lo(audio_spu_state_3f80)($at)
    lui $at, %hi(audio_spu_state_3f84)
    sw $zero, %lo(audio_spu_state_3f84)($at)
    lui $at, %hi(audio_spu_state_3f88)
    sw $zero, %lo(audio_spu_state_3f88)($at)
    lui $at, %hi(audio_spu_state_3b1c)
    sw $zero, %lo(audio_spu_state_3b1c)($at)
    lui $at, %hi(audio_spu_state_3acc)
    sw $zero, %lo(audio_spu_state_3acc)($at)
    lui $at, %hi(audio_spu_state_3b18)
    sw $zero, %lo(audio_spu_state_3b18)($at)
    lui $at, %hi(audio_spu_state_3b44)
    sw $zero, %lo(audio_spu_state_3b44)($at)
    lui $at, %hi(audio_spu_state_3b40)
    sw $zero, %lo(audio_spu_state_3b40)($at)
    lui $at, %hi(audio_spu_state_3f78)
    sw $zero, %lo(audio_spu_state_3f78)($at)
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_spu_initialize

# exe/slus_004_22@0x80169018..0x80169094
.globl audio_spu_initialize_dma
.ent audio_spu_initialize_dma
audio_spu_initialize_dma:
    lui $v0, %hi(audio_spu_dma_initialized)
    lw $v0, %lo(audio_spu_dma_initialized)($v0)
    addiu $sp, $sp, -0x18
    bnez $v0, .Lspu_80169084
    sw $ra, 0x10($sp)
    addiu $v0, $zero, 1
    lui $at, %hi(audio_spu_dma_initialized)
    sw $v0, %lo(audio_spu_dma_initialized)($at)
    jal audio_spu_service_enter_critical
    nop
    lui $a0, %hi(audio_spu_dma_callback)
    addiu $a0, $a0, %lo(audio_spu_dma_callback)
    jal audio_spu_register_dma
    nop
    lui $a0, 0xf000
    ori $a0, $a0, 9
    addiu $a1, $zero, 0x20
    addiu $a2, $zero, 0x2000
    jal audio_spu_service_open_event
    addu $a3, $zero, $zero
    addu $a0, $v0, $zero
    lui $at, %hi(audio_spu_transfer_event)
    sw $a0, %lo(audio_spu_transfer_event)($at)
    jal audio_spu_service_enable_event
    nop
    jal audio_spu_service_exit_critical
    nop
.Lspu_80169084:
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_spu_initialize_dma

# exe/slus_004_22@0x80169094..0x801690B8
.globl audio_spu_register_dma
.ent audio_spu_register_dma
audio_spu_register_dma:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    addu $a1, $a0, $zero
    jal audio_callbacks_dma
    addiu $a0, $zero, 4
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_spu_register_dma

# Original service wrapper at exe/slus_004_22@0x8017E3E4.
.globl audio_spu_service_printf
audio_spu_service_printf:
    addiu $t2, $zero, 0xa0
    jr $t2
    addiu $t1, $zero, 0x3f
    nop

# Original service wrapper at exe/slus_004_22@0x8017ED2C.
.globl audio_spu_service_deliver_event
audio_spu_service_deliver_event:
    addiu $t2, $zero, 0xb0
    jr $t2
    addiu $t1, $zero, 7
    nop

# Original service wrapper at exe/slus_004_22@0x8017ED3C.
.globl audio_spu_service_open_event
audio_spu_service_open_event:
    addiu $t2, $zero, 0xb0
    jr $t2
    addiu $t1, $zero, 8
    nop

# Original service wrapper at exe/slus_004_22@0x8017ED7C.
.globl audio_spu_service_enable_event
audio_spu_service_enable_event:
    addiu $t2, $zero, 0xb0
    jr $t2
    addiu $t1, $zero, 0xc
    nop

# Original service wrapper at exe/slus_004_22@0x8017EE0C.
.globl audio_spu_service_enter_critical
audio_spu_service_enter_critical:
    addiu $a0, $zero, 1
    syscall
    jr $ra
    nop

# Original service wrapper at exe/slus_004_22@0x8017EE1C.
.globl audio_spu_service_exit_critical
audio_spu_service_exit_critical:
    addiu $a0, $zero, 2
    syscall
    jr $ra
    nop

.section .data
# exe/slus_004_22@0x80183AB0, accessed extent 4 bytes.
.balign 4
.globl audio_spu_registers_pointer
audio_spu_registers_pointer:
    .word 0x1f801c00
# exe/slus_004_22@0x80183AC0, accessed extent 4 bytes.
.balign 4
.globl audio_spu_priority_pointer
audio_spu_priority_pointer:
    .word 0x1f8010f0
# exe/slus_004_22@0x80183AD4, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3ad4
audio_spu_state_3ad4:
    .word 0x00000002
# exe/slus_004_22@0x80183AD8, accessed extent 4 bytes.
.balign 4
.globl audio_spu_transfer_shift
audio_spu_transfer_shift:
    .word 0x00000003
# exe/slus_004_22@0x80183ADC, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3adc
audio_spu_state_3adc:
    .word 0x00000008
# exe/slus_004_22@0x80183AE0, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3ae0
audio_spu_state_3ae0:
    .word 0x00000007
# exe/slus_004_22@0x80183AE4, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3ae4
audio_spu_state_3ae4:
    .word 0x00000001
# exe/slus_004_22@0x80183AF0, accessed extent 16 bytes.
.balign 4
.globl audio_spu_reset_pattern
audio_spu_reset_pattern:
    .fill 16, 1, 7
# exe/slus_004_22@0x80183B48, accessed extent 48 bytes.
.balign 4
.globl audio_spu_voice_control_shadow
audio_spu_voice_control_shadow:
    .fill 24, 2, 0xc000
# exe/slus_004_22@0x80183F8C, ten live reverb work addresses.
.balign 4
.globl audio_spu_initial_reverb_start
audio_spu_initial_reverb_start:
    .word 0x0000fffe
    .word 0x0000fb28, 0x0000fc18, 0x0000f6f8
    .word 0x0000f204, 0x0000ea44, 0x0000e128
    .word 0x0000cff8, 0x0000cff8, 0x0000f880
.globl audio_spu_timeout_format
audio_spu_timeout_format:
    .asciz "SPU:T/O [%s]\n"
.globl audio_spu_wait_reset
audio_spu_wait_reset:
    .asciz "wait (reset)"
.globl audio_spu_wait_ready
audio_spu_wait_ready:
    .asciz "wait (wrdy H -> L)"
.globl audio_spu_wait_fifo
audio_spu_wait_fifo:
    .asciz "wait (dmaf clear/W)"

.section .bss
# exe/slus_004_22@0x80183AC8, accessed extent 2 bytes.
.balign 2
.globl audio_spu_transfer_address
audio_spu_transfer_address:
    .space 2
# exe/slus_004_22@0x80183ACC, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3acc
audio_spu_state_3acc:
    .space 4
# exe/slus_004_22@0x80183AD0, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3ad0
audio_spu_state_3ad0:
    .space 4
# exe/slus_004_22@0x80183AE8, accessed extent 4 bytes.
.balign 4
.globl audio_spu_transfer_callback
audio_spu_transfer_callback:
    .space 4
# exe/slus_004_22@0x80183AEC, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3aec
audio_spu_state_3aec:
    .space 4
# exe/slus_004_22@0x80183B00, accessed extent 4 bytes.
.balign 4
.globl audio_spu_transfer_direction
audio_spu_transfer_direction:
    .space 4
# exe/slus_004_22@0x80183B14, accessed extent 4 bytes.
.balign 4
.globl audio_spu_transfer_event
audio_spu_transfer_event:
    .space 4
# exe/slus_004_22@0x80183B18, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3b18
audio_spu_state_3b18:
    .space 4
# exe/slus_004_22@0x80183B1C, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3b1c
audio_spu_state_3b1c:
    .space 4
# exe/slus_004_22@0x80183B20, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3b20
audio_spu_state_3b20:
    .space 4
# exe/slus_004_22@0x80183B24, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3b24
audio_spu_state_3b24:
    .space 4
# exe/slus_004_22@0x80183B28, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3b28
audio_spu_state_3b28:
    .space 4
# exe/slus_004_22@0x80183B30, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3b30
audio_spu_state_3b30:
    .space 4
# exe/slus_004_22@0x80183B34, accessed extent 2 bytes.
.balign 2
.globl audio_spu_state_3b34
audio_spu_state_3b34:
    .space 2
# exe/slus_004_22@0x80183B36, accessed extent 2 bytes.
.balign 2
.globl audio_spu_state_3b36
audio_spu_state_3b36:
    .space 2
# exe/slus_004_22@0x80183B38, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3b38
audio_spu_state_3b38:
    .space 4
# exe/slus_004_22@0x80183B3C, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3b3c
audio_spu_state_3b3c:
    .space 4
# exe/slus_004_22@0x80183B40, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3b40
audio_spu_state_3b40:
    .space 4
# exe/slus_004_22@0x80183B44, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3b44
audio_spu_state_3b44:
    .space 4
# exe/slus_004_22@0x80183F78, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3f78
audio_spu_state_3f78:
    .space 4
# exe/slus_004_22@0x80183F7C, accessed extent 4 bytes.
.balign 4
.globl audio_spu_dma_initialized
audio_spu_dma_initialized:
    .space 4
# exe/slus_004_22@0x80183F80, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3f80
audio_spu_state_3f80:
    .space 4
# exe/slus_004_22@0x80183F84, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3f84
audio_spu_state_3f84:
    .space 4
# exe/slus_004_22@0x80183F88, accessed extent 4 bytes.
.balign 4
.globl audio_spu_state_3f88
audio_spu_state_3f88:
    .space 4
# exe/slus_004_22@0x8018E230, accessed extent 20 bytes.
.balign 4
.globl audio_spu_register_shadow
audio_spu_register_shadow:
    .space 20
