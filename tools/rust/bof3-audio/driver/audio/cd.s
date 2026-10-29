# SPDX-License-Identifier: MIT
# Owned CD SDK command, response, interrupt and initialization code.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
.set noreorder
.set noat
.section .text
.balign 4
# exe/slus_004_22@0x80175514..0x80175524.
.globl audio_cd_last_command
.ent audio_cd_last_command
audio_cd_last_command:
    lui $v0, %hi(audio_cd_sdk_state + 0xa9)
    lbu $v0, %lo(audio_cd_sdk_state + 0xa9)($v0)
    jr $ra
    nop
.end audio_cd_last_command

# exe/slus_004_22@0x80175524..0x80175534.
.globl audio_cd_response
.ent audio_cd_response
audio_cd_response:
    lui $v0, %hi(audio_cd_sdk_state + 0xa4)
    addiu $v0, $v0, %lo(audio_cd_sdk_state + 0xa4)
    jr $ra
    nop
.end audio_cd_response

# exe/slus_004_22@0x80175534..0x801755a0.
.globl audio_cd_initialize
.ent audio_cd_initialize
audio_cd_initialize:
    addiu $sp, $sp, -0x18
    sw $s0, 0x10($sp)
    addu $s0, $a0, $zero
    addiu $v0, $zero, 2
    bne $s0, $v0, .Lcd_8017555c
    sw $ra, 0x14($sp)
    jal audio_cd_register_irq
    nop
    j .Lcd_8017558c
    addiu $v0, $zero, 1
.Lcd_8017555c:
    jal audio_cd_initialize_core
    nop
    bnez $v0, .Lcd_8017558c
    addu $v0, $zero, $zero
    addiu $v0, $zero, 1
    bne $s0, $v0, .Lcd_8017558c
    nop
    jal audio_cd_spu_initialize
    nop
    bnez $v0, .Lcd_8017558c
    addu $v0, $zero, $zero
    addiu $v0, $zero, 1
.Lcd_8017558c:
    lw $ra, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_cd_initialize

# exe/slus_004_22@0x801755a0..0x801755c0.
.globl audio_cd_reset
.ent audio_cd_reset
audio_cd_reset:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    jal audio_cd_recover
    nop
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_cd_reset

# exe/slus_004_22@0x801755c0..0x801755d8.
.globl audio_cd_set_debug
.ent audio_cd_set_debug
audio_cd_set_debug:
    lui $v0, %hi(audio_cd_sdk_state + 0x94)
    lw $v0, %lo(audio_cd_sdk_state + 0x94)($v0)
    lui $at, %hi(audio_cd_sdk_state + 0x94)
    sw $a0, %lo(audio_cd_sdk_state + 0x94)($at)
    jr $ra
    nop
.end audio_cd_set_debug

# exe/slus_004_22@0x801755d8..0x8017560c.
.globl audio_cd_command_name
.ent audio_cd_command_name
audio_cd_command_name:
    andi $a0, $a0, 0xff
    sltiu $v0, $a0, 0x1c
    beqz $v0, .Lcd_801755fc
    sll $v0, $a0, 2
    lui $at, %hi(audio_cd_sdk_state + 0xac)
    addu $at, $at, $v0
    lw $v0, %lo(audio_cd_sdk_state + 0xac)($at)
    j .Lcd_80175604
    nop
.Lcd_801755fc:
    lui $v0, %hi(.Lcd_string_8014a19c)
    addiu $v0, $v0, %lo(.Lcd_string_8014a19c)
.Lcd_80175604:
    jr $ra
    nop
.end audio_cd_command_name

# exe/slus_004_22@0x8017560c..0x80175640.
.globl audio_cd_status_name
.ent audio_cd_status_name
audio_cd_status_name:
    andi $a0, $a0, 0xff
    sltiu $v0, $a0, 7
    beqz $v0, .Lcd_80175630
    sll $v0, $a0, 2
    lui $at, %hi(audio_cd_sdk_state + 0x12c)
    addu $at, $at, $v0
    lw $v0, %lo(audio_cd_sdk_state + 0x12c)($at)
    j .Lcd_80175638
    nop
.Lcd_80175630:
    lui $v0, %hi(.Lcd_string_8014a19c)
    addiu $v0, $v0, %lo(.Lcd_string_8014a19c)
.Lcd_80175638:
    jr $ra
    nop
.end audio_cd_status_name

# exe/slus_004_22@0x80175640..0x80175660.
.globl audio_cd_sync
.ent audio_cd_sync
audio_cd_sync:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    jal audio_cd_sync_core
    nop
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_cd_sync

# exe/slus_004_22@0x80175660..0x80175680.
.globl audio_cd_ready
.ent audio_cd_ready
audio_cd_ready:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    jal audio_cd_ready_core
    nop
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_cd_ready

# exe/slus_004_22@0x80175698..0x801756b0.
.globl audio_cd_set_ready_callback
.ent audio_cd_set_ready_callback
audio_cd_set_ready_callback:
    lui $v0, %hi(audio_cd_sdk_state + 0x8c)
    lw $v0, %lo(audio_cd_sdk_state + 0x8c)($v0)
    lui $at, %hi(audio_cd_sdk_state + 0x8c)
    sw $a0, %lo(audio_cd_sdk_state + 0x8c)($at)
    jr $ra
    nop
.end audio_cd_set_ready_callback

# exe/slus_004_22@0x801756b0..0x801757e8.
.globl audio_cd_control
.ent audio_cd_control
audio_cd_control:
    addiu $sp, $sp, -0x38
    sw $s1, 0x14($sp)
    addu $s1, $a1, $zero
    sw $s2, 0x18($sp)
    addu $s2, $a2, $zero
    sw $s4, 0x20($sp)
    addu $s4, $a0, $zero
    sw $s0, 0x10($sp)
    addiu $s0, $zero, 3
    sw $s3, 0x1c($sp)
    andi $s3, $s4, 0xff
    lui $v1, %hi(audio_cd_sdk_state)
    addiu $v1, $v1, %lo(audio_cd_sdk_state)
    sw $s5, 0x24($sp)
    lui $s5, %hi(audio_cd_sdk_state + 0x88)
    lw $s5, %lo(audio_cd_sdk_state + 0x88)($s5)
    sll $v0, $s3, 2
    sw $s6, 0x28($sp)
    addu $s6, $v0, $v1
    sw $s7, 0x2c($sp)
    addu $s7, $zero, $zero
    sw $ra, 0x30($sp)
.Lcd_80175708:
    lui $at, %hi(audio_cd_sdk_state + 0x88)
    sw $zero, %lo(audio_cd_sdk_state + 0x88)($at)
    addiu $v0, $zero, 1
    beq $s3, $v0, .Lcd_80175744
    nop
    lui $v0, %hi(audio_cd_sdk_state + 0x98)
    lbu $v0, %lo(audio_cd_sdk_state + 0x98)($v0)
    nop
    andi $v0, $v0, 0x10
    beqz $v0, .Lcd_80175744
    addiu $a0, $zero, 1
    addu $a1, $zero, $zero
    addu $a2, $zero, $zero
    jal audio_cd_command_core
    addu $a3, $zero, $zero
.Lcd_80175744:
    beqz $s1, .Lcd_80175774
    nop
    lw $v0, 0($s6)
    nop
    beqz $v0, .Lcd_80175774
    addiu $a0, $zero, 2
    addu $a1, $s1, $zero
    addu $a2, $s2, $zero
    jal audio_cd_command_core
    addu $a3, $zero, $zero
    bnez $v0, .Lcd_80175798
    nop
.Lcd_80175774:
    lui $at, %hi(audio_cd_sdk_state + 0x88)
    sw $s5, %lo(audio_cd_sdk_state + 0x88)($at)
    andi $a0, $s4, 0xff
    addu $a1, $s1, $zero
    addu $a2, $s2, $zero
    jal audio_cd_command_core
    addu $a3, $zero, $zero
    beqz $v0, .Lcd_801757b8
    addiu $v0, $s7, 1
.Lcd_80175798:
    addiu $s0, $s0, -1
    addiu $v0, $zero, -1
    bne $s0, $v0, .Lcd_80175708
    nop
    lui $at, %hi(audio_cd_sdk_state + 0x88)
    sw $s5, %lo(audio_cd_sdk_state + 0x88)($at)
    addiu $s7, $zero, -1
    addiu $v0, $s7, 1
.Lcd_801757b8:
    lw $ra, 0x30($sp)
    lw $s7, 0x2c($sp)
    lw $s6, 0x28($sp)
    lw $s5, 0x24($sp)
    lw $s4, 0x20($sp)
    lw $s3, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x38
    jr $ra
    nop
.end audio_cd_control

# exe/slus_004_22@0x801757e8..0x80175914.
.globl audio_cd_control_nonblocking
.ent audio_cd_control_nonblocking
audio_cd_control_nonblocking:
    addiu $sp, $sp, -0x30
    sw $s1, 0x14($sp)
    addu $s1, $a1, $zero
    sw $s3, 0x1c($sp)
    addu $s3, $a0, $zero
    sw $s0, 0x10($sp)
    addiu $s0, $zero, 3
    sw $s2, 0x18($sp)
    andi $s2, $s3, 0xff
    lui $v1, %hi(audio_cd_sdk_state)
    addiu $v1, $v1, %lo(audio_cd_sdk_state)
    sw $s4, 0x20($sp)
    lui $s4, %hi(audio_cd_sdk_state + 0x88)
    lw $s4, %lo(audio_cd_sdk_state + 0x88)($s4)
    sll $v0, $s2, 2
    sw $s5, 0x24($sp)
    addu $s5, $v0, $v1
    sw $s6, 0x28($sp)
    addu $s6, $zero, $zero
    sw $ra, 0x2c($sp)
.Lcd_80175838:
    lui $at, %hi(audio_cd_sdk_state + 0x88)
    sw $zero, %lo(audio_cd_sdk_state + 0x88)($at)
    addiu $v0, $zero, 1
    beq $s2, $v0, .Lcd_80175874
    nop
    lui $v0, %hi(audio_cd_sdk_state + 0x98)
    lbu $v0, %lo(audio_cd_sdk_state + 0x98)($v0)
    nop
    andi $v0, $v0, 0x10
    beqz $v0, .Lcd_80175874
    addiu $a0, $zero, 1
    addu $a1, $zero, $zero
    addu $a2, $zero, $zero
    jal audio_cd_command_core
    addu $a3, $zero, $zero
.Lcd_80175874:
    beqz $s1, .Lcd_801758a4
    nop
    lw $v0, 0($s5)
    nop
    beqz $v0, .Lcd_801758a4
    addiu $a0, $zero, 2
    addu $a1, $s1, $zero
    addu $a2, $zero, $zero
    jal audio_cd_command_core
    addu $a3, $zero, $zero
    bnez $v0, .Lcd_801758c8
    nop
.Lcd_801758a4:
    lui $at, %hi(audio_cd_sdk_state + 0x88)
    sw $s4, %lo(audio_cd_sdk_state + 0x88)($at)
    andi $a0, $s3, 0xff
    addu $a1, $s1, $zero
    addu $a2, $zero, $zero
    jal audio_cd_command_core
    addiu $a3, $zero, 1
    beqz $v0, .Lcd_801758e8
    addiu $v0, $s6, 1
.Lcd_801758c8:
    addiu $s0, $s0, -1
    addiu $v0, $zero, -1
    bne $s0, $v0, .Lcd_80175838
    nop
    lui $at, %hi(audio_cd_sdk_state + 0x88)
    sw $s4, %lo(audio_cd_sdk_state + 0x88)($at)
    addiu $s6, $zero, -1
    addiu $v0, $s6, 1
.Lcd_801758e8:
    lw $ra, 0x2c($sp)
    lw $s6, 0x28($sp)
    lw $s5, 0x24($sp)
    lw $s4, 0x20($sp)
    lw $s3, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x30
    jr $ra
    nop
.end audio_cd_control_nonblocking

# exe/slus_004_22@0x80175914..0x80175a58.
.globl audio_cd_control_blocking
.ent audio_cd_control_blocking
audio_cd_control_blocking:
    addiu $sp, $sp, -0x30
    sw $s1, 0x14($sp)
    addu $s1, $a1, $zero
    sw $s2, 0x18($sp)
    addu $s2, $a2, $zero
    sw $s4, 0x20($sp)
    addu $s4, $a0, $zero
    sw $s0, 0x10($sp)
    addiu $s0, $zero, 3
    sw $s3, 0x1c($sp)
    andi $s3, $s4, 0xff
    lui $v1, %hi(audio_cd_sdk_state)
    addiu $v1, $v1, %lo(audio_cd_sdk_state)
    sw $s5, 0x24($sp)
    lui $s5, %hi(audio_cd_sdk_state + 0x88)
    lw $s5, %lo(audio_cd_sdk_state + 0x88)($s5)
    sll $v0, $s3, 2
    sw $s6, 0x28($sp)
    addu $s6, $v0, $v1
    sw $ra, 0x2c($sp)
.Lcd_80175964:
    lui $at, %hi(audio_cd_sdk_state + 0x88)
    sw $zero, %lo(audio_cd_sdk_state + 0x88)($at)
    addiu $v0, $zero, 1
    beq $s3, $v0, .Lcd_801759a0
    nop
    lui $v0, %hi(audio_cd_sdk_state + 0x98)
    lbu $v0, %lo(audio_cd_sdk_state + 0x98)($v0)
    nop
    andi $v0, $v0, 0x10
    beqz $v0, .Lcd_801759a0
    addiu $a0, $zero, 1
    addu $a1, $zero, $zero
    addu $a2, $zero, $zero
    jal audio_cd_command_core
    addu $a3, $zero, $zero
.Lcd_801759a0:
    beqz $s1, .Lcd_801759d0
    nop
    lw $v0, 0($s6)
    nop
    beqz $v0, .Lcd_801759d0
    addiu $a0, $zero, 2
    addu $a1, $s1, $zero
    addu $a2, $s2, $zero
    jal audio_cd_command_core
    addu $a3, $zero, $zero
    bnez $v0, .Lcd_801759f4
    nop
.Lcd_801759d0:
    lui $at, %hi(audio_cd_sdk_state + 0x88)
    sw $s5, %lo(audio_cd_sdk_state + 0x88)($at)
    andi $a0, $s4, 0xff
    addu $a1, $s1, $zero
    addu $a2, $s2, $zero
    jal audio_cd_command_core
    addu $a3, $zero, $zero
    beqz $v0, .Lcd_80175a0c
    addu $v0, $zero, $zero
.Lcd_801759f4:
    addiu $s0, $s0, -1
    addiu $v0, $zero, -1
    bne $s0, $v0, .Lcd_80175964
    nop
    lui $at, %hi(audio_cd_sdk_state + 0x88)
    sw $s5, %lo(audio_cd_sdk_state + 0x88)($at)
.Lcd_80175a0c:
    bnez $v0, .Lcd_80175a28
    addu $a0, $zero, $zero
    jal audio_cd_sync_core
    addu $a1, $s2, $zero
    xori $v0, $v0, 2
    j .Lcd_80175a2c
    sltiu $v0, $v0, 1
.Lcd_80175a28:
    addu $v0, $zero, $zero
.Lcd_80175a2c:
    lw $ra, 0x2c($sp)
    lw $s6, 0x28($sp)
    lw $s5, 0x24($sp)
    lw $s4, 0x20($sp)
    lw $s3, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x30
    jr $ra
    nop
.end audio_cd_control_blocking

# exe/slus_004_22@0x80175a58..0x80175a78.
.globl audio_cd_mix
.ent audio_cd_mix
audio_cd_mix:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    jal audio_cd_mix_core
    nop
    lw $ra, 0x10($sp)
    addiu $v0, $zero, 1
    jr $ra
    addiu $sp, $sp, 0x18
.end audio_cd_mix

# exe/slus_004_22@0x80175c60..0x801761e8.
.globl audio_cd_irq_drain
.ent audio_cd_irq_drain
audio_cd_irq_drain:
    addiu $sp, $sp, -0x30
    lui $v1, %hi(audio_cd_sdk_state + 0x34c)
    lw $v1, %lo(audio_cd_sdk_state + 0x34c)($v1)
    addiu $v0, $zero, 1
    sw $ra, 0x28($sp)
    sw $s1, 0x24($sp)
    sw $s0, 0x20($sp)
    sb $v0, 0($v1)
    lui $a0, %hi(audio_cd_sdk_state + 0x358)
    lw $a0, %lo(audio_cd_sdk_state + 0x358)($a0)
    nop
    lbu $v0, 0($a0)
    nop
    andi $v0, $v0, 7
    sb $v0, 0x10($sp)
    lbu $v0, 0x10($sp)
    nop
    beqz $v0, .Lcd_801761cc
    addu $s1, $zero, $zero
    j .Lcd_80175cc4
    nop
.Lcd_80175cb4:
    lbu $v0, 0($a0)
    nop
    andi $v0, $v0, 7
    sb $v0, 0x10($sp)
.Lcd_80175cc4:
    lbu $v0, 0($a0)
    lbu $v1, 0x10($sp)
    andi $v0, $v0, 7
    bne $v1, $v0, .Lcd_80175cb4
    addu $s0, $zero, $zero
    addiu $v1, $sp, 0x18
.Lcd_80175cdc:
    lui $v0, %hi(audio_cd_sdk_state + 0x34c)
    lw $v0, %lo(audio_cd_sdk_state + 0x34c)($v0)
    nop
    lbu $v0, 0($v0)
    nop
    andi $v0, $v0, 0x20
    beqz $v0, .Lcd_80175d24
    slti $v0, $s0, 8
    lui $v0, %hi(audio_cd_sdk_state + 0x350)
    lw $v0, %lo(audio_cd_sdk_state + 0x350)($v0)
    nop
    lbu $v0, 0($v0)
    addiu $s0, $s0, 1
    sb $v0, 0($v1)
    slti $v0, $s0, 8
    bnez $v0, .Lcd_80175cdc
    addiu $v1, $v1, 1
    slti $v0, $s0, 8
.Lcd_80175d24:
    beqz $v0, .Lcd_80175d48
    addiu $v0, $sp, 0x18
    addu $v1, $s0, $v0
    addiu $a0, $sp, 0x20
.Lcd_80175d34:
    sb $zero, 0($v1)
    addiu $v1, $v1, 1
    slt $v0, $v1, $a0
    bnez $v0, .Lcd_80175d34
    nop
.Lcd_80175d48:
    lui $v1, %hi(audio_cd_sdk_state + 0x34c)
    lw $v1, %lo(audio_cd_sdk_state + 0x34c)($v1)
    addiu $v0, $zero, 1
    sb $v0, 0($v1)
    lui $v0, %hi(audio_cd_sdk_state + 0x358)
    lw $v0, %lo(audio_cd_sdk_state + 0x358)($v0)
    addiu $v1, $zero, 7
    sb $v1, 0($v0)
    lui $v0, %hi(audio_cd_sdk_state + 0x354)
    lw $v0, %lo(audio_cd_sdk_state + 0x354)($v0)
    nop
    sb $v1, 0($v0)
    lbu $v0, 0x10($sp)
    addiu $v1, $zero, 3
    andi $v0, $v0, 0xff
    bne $v0, $v1, .Lcd_80175db4
    nop
    lui $v0, %hi(audio_cd_sdk_state + 0xa9)
    lbu $v0, %lo(audio_cd_sdk_state + 0xa9)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cd_sdk_state + 0x24c)
    addu $at, $at, $v0
    lw $v0, %lo(audio_cd_sdk_state + 0x24c)($at)
    nop
    beqz $v0, .Lcd_80175e18
    nop
.Lcd_80175db4:
    lui $v0, %hi(audio_cd_sdk_state + 0x98)
    lw $v0, %lo(audio_cd_sdk_state + 0x98)($v0)
    nop
    andi $v0, $v0, 0x10
    bnez $v0, .Lcd_80175df8
    nop
    lbu $v0, 0x18($sp)
    nop
    andi $v0, $v0, 0x10
    beqz $v0, .Lcd_80175df8
    nop
    lui $v0, %hi(audio_cd_sdk_state + 0xa0)
    lw $v0, %lo(audio_cd_sdk_state + 0xa0)($v0)
    nop
    addiu $v0, $v0, 1
    lui $at, %hi(audio_cd_sdk_state + 0xa0)
    sw $v0, %lo(audio_cd_sdk_state + 0xa0)($at)
.Lcd_80175df8:
    lbu $v0, 0x18($sp)
    lbu $v1, 0x19($sp)
    andi $v0, $v0, 0xff
    andi $s1, $v0, 0x1d
    lui $at, %hi(audio_cd_sdk_state + 0x98)
    sw $v0, %lo(audio_cd_sdk_state + 0x98)($at)
    lui $at, %hi(audio_cd_sdk_state + 0x9c)
    sw $v1, %lo(audio_cd_sdk_state + 0x9c)($at)
.Lcd_80175e18:
    lbu $v0, 0x10($sp)
    addiu $v1, $zero, 5
    andi $v0, $v0, 0xff
    bne $v0, $v1, .Lcd_80175e9c
    nop
    lui $v0, %hi(audio_cd_sdk_state + 0x94)
    lw $v0, %lo(audio_cd_sdk_state + 0x94)($v0)
    nop
    blez $v0, .Lcd_80175e9c
    nop
    lui $a0, %hi(.Lcd_string_8014a30c)
    addiu $a0, $a0, %lo(.Lcd_string_8014a30c)
    jal audio_spu_service_printf
    nop
    lui $v0, %hi(audio_cd_sdk_state + 0x94)
    lw $v0, %lo(audio_cd_sdk_state + 0x94)($v0)
    nop
    blez $v0, .Lcd_80175e9c
    nop
    lui $v0, %hi(audio_cd_sdk_state + 0xa9)
    lbu $v0, %lo(audio_cd_sdk_state + 0xa9)($v0)
    lui $a2, %hi(audio_cd_sdk_state + 0x98)
    lw $a2, %lo(audio_cd_sdk_state + 0x98)($a2)
    lui $a3, %hi(audio_cd_sdk_state + 0x9c)
    lw $a3, %lo(audio_cd_sdk_state + 0x9c)($a3)
    sll $v0, $v0, 2
    lui $at, %hi(audio_cd_sdk_state + 0xac)
    addu $at, $at, $v0
    lw $a1, %lo(audio_cd_sdk_state + 0xac)($at)
    lui $a0, %hi(.Lcd_string_8014a318)
    addiu $a0, $a0, %lo(.Lcd_string_8014a318)
    jal audio_spu_service_printf
    nop
.Lcd_80175e9c:
    lbu $v0, 0x10($sp)
    nop
    addiu $v1, $v0, -1
    sltiu $v0, $v1, 5
    beqz $v0, .Lcd_801761a8
    sll $v0, $v1, 2
    lui $at, %hi(audio_cd_irq_handlers)
    addu $at, $at, $v0
    lw $v0, %lo(audio_cd_irq_handlers)($at)
    nop
    jr $v0
    nop
.Lcd_80175ecc:
    beqz $s1, .Lcd_80175f18
    addiu $v0, $zero, 5
    lui $v1, %hi(audio_cd_sdk_state + 0x364)
    addiu $v1, $v1, %lo(audio_cd_sdk_state + 0x364)
    sb $v0, 0($v1)
    lui $v1, %hi(audio_cd_results)
    addiu $v1, $v1, %lo(audio_cd_results)
    beqz $v1, .Lcd_80176010
    addiu $a1, $sp, 0x18
    addiu $a0, $zero, 7
    addiu $a2, $zero, -1
.Lcd_80175ef8:
    lbu $v0, 0($a1)
    addiu $a1, $a1, 1
    addiu $a0, $a0, -1
    sb $v0, 0($v1)
    bne $a0, $a2, .Lcd_80175ef8
    addiu $v1, $v1, 1
    j .Lcd_801761d0
    addiu $v0, $zero, 2
.Lcd_80175f18:
    lui $v0, %hi(audio_cd_sdk_state + 0xa9)
    lbu $v0, %lo(audio_cd_sdk_state + 0xa9)($v0)
    nop
    sll $v0, $v0, 2
    lui $at, %hi(audio_cd_sdk_state + 0x14c)
    addu $at, $at, $v0
    lw $v0, %lo(audio_cd_sdk_state + 0x14c)($at)
    nop
    beqz $v0, .Lcd_80175f84
    addiu $v0, $zero, 3
    lui $v1, %hi(audio_cd_sdk_state + 0x364)
    addiu $v1, $v1, %lo(audio_cd_sdk_state + 0x364)
    sb $v0, 0($v1)
    lui $v1, %hi(audio_cd_results)
    addiu $v1, $v1, %lo(audio_cd_results)
    beqz $v1, .Lcd_80175f7c
    addiu $a1, $sp, 0x18
    addiu $a0, $zero, 7
    addiu $a2, $zero, -1
.Lcd_80175f64:
    lbu $v0, 0($a1)
    addiu $a1, $a1, 1
    addiu $a0, $a0, -1
    sb $v0, 0($v1)
    bne $a0, $a2, .Lcd_80175f64
    addiu $v1, $v1, 1
.Lcd_80175f7c:
    j .Lcd_801761d0
    addiu $v0, $zero, 1
.Lcd_80175f84:
    lui $v1, %hi(audio_cd_sdk_state + 0x364)
    addiu $v1, $v1, %lo(audio_cd_sdk_state + 0x364)
    addiu $v0, $zero, 2
    sb $v0, 0($v1)
    lui $v1, %hi(audio_cd_results)
    addiu $v1, $v1, %lo(audio_cd_results)
    beqz $v1, .Lcd_80176010
    addiu $a1, $sp, 0x18
    addiu $a0, $zero, 7
    addiu $a2, $zero, -1
.Lcd_80175fac:
    lbu $v0, 0($a1)
    addiu $a1, $a1, 1
    addiu $a0, $a0, -1
    sb $v0, 0($v1)
    bne $a0, $a2, .Lcd_80175fac
    addiu $v1, $v1, 1
    j .Lcd_801761d0
    addiu $v0, $zero, 2
.Lcd_80175fcc:
    beqz $s1, .Lcd_80175fd8
    addiu $v0, $zero, 2
    addiu $v0, $zero, 5
.Lcd_80175fd8:
    lui $at, %hi(audio_cd_sdk_state + 0x364)
    sb $v0, %lo(audio_cd_sdk_state + 0x364)($at)
    lui $v1, %hi(audio_cd_results)
    addiu $v1, $v1, %lo(audio_cd_results)
    beqz $v1, .Lcd_80176010
    addiu $a1, $sp, 0x18
    addiu $a0, $zero, 7
    addiu $a2, $zero, -1
.Lcd_80175ff8:
    lbu $v0, 0($a1)
    addiu $a1, $a1, 1
    addiu $a0, $a0, -1
    sb $v0, 0($v1)
    bne $a0, $a2, .Lcd_80175ff8
    addiu $v1, $v1, 1
.Lcd_80176010:
    j .Lcd_801761d0
    addiu $v0, $zero, 2
.Lcd_80176018:
    beqz $s1, .Lcd_8017603c
    addiu $v0, $zero, 1
    addiu $v0, $zero, 1
    bne $s0, $v0, .Lcd_80176030
    nop
    addu $s1, $zero, $zero
.Lcd_80176030:
    beqz $s1, .Lcd_8017603c
    addiu $v0, $zero, 1
    addiu $v0, $zero, 5
.Lcd_8017603c:
    lui $at, %hi(audio_cd_sdk_state + 0x365)
    sb $v0, %lo(audio_cd_sdk_state + 0x365)($at)
    lui $v1, %hi(audio_cd_results + 0x8)
    addiu $v1, $v1, %lo(audio_cd_results + 0x8)
    beqz $v1, .Lcd_80176074
    addiu $a1, $sp, 0x18
    addiu $a0, $zero, 7
    addiu $a2, $zero, -1
.Lcd_8017605c:
    lbu $v0, 0($a1)
    addiu $a1, $a1, 1
    addiu $a0, $a0, -1
    sb $v0, 0($v1)
    bne $a0, $a2, .Lcd_8017605c
    addiu $v1, $v1, 1
.Lcd_80176074:
    lui $v0, %hi(audio_cd_sdk_state + 0x34c)
    lw $v0, %lo(audio_cd_sdk_state + 0x34c)($v0)
    nop
    sb $zero, 0($v0)
    lui $v1, %hi(audio_cd_sdk_state + 0x358)
    lw $v1, %lo(audio_cd_sdk_state + 0x358)($v1)
    addiu $v0, $zero, 4
    sb $zero, 0($v1)
    j .Lcd_801761d0
    nop
.Lcd_8017609c:
    lui $a0, %hi(audio_cd_results + 0x10)
    addiu $a0, $a0, %lo(audio_cd_results + 0x10)
    addiu $v0, $zero, 4
    lui $at, %hi(audio_cd_sdk_state + 0x366)
    sb $v0, %lo(audio_cd_sdk_state + 0x366)($at)
    lui $v0, %hi(audio_cd_sdk_state + 0x366)
    lbu $v0, %lo(audio_cd_sdk_state + 0x366)($v0)
    addiu $a1, $sp, 0x18
    lui $at, %hi(audio_cd_sdk_state + 0x365)
    sb $v0, %lo(audio_cd_sdk_state + 0x365)($at)
    beqz $a0, .Lcd_801760e8
    addiu $v1, $zero, 7
    addiu $a2, $zero, -1
.Lcd_801760d0:
    lbu $v0, 0($a1)
    addiu $a1, $a1, 1
    addiu $v1, $v1, -1
    sb $v0, 0($a0)
    bne $v1, $a2, .Lcd_801760d0
    addiu $a0, $a0, 1
.Lcd_801760e8:
    lui $v1, %hi(audio_cd_results + 0x8)
    addiu $v1, $v1, %lo(audio_cd_results + 0x8)
    beqz $v1, .Lcd_80176118
    addiu $a1, $sp, 0x18
    addiu $a0, $zero, 7
    addiu $a2, $zero, -1
.Lcd_80176100:
    lbu $v0, 0($a1)
    addiu $a1, $a1, 1
    addiu $a0, $a0, -1
    sb $v0, 0($v1)
    bne $a0, $a2, .Lcd_80176100
    addiu $v1, $v1, 1
.Lcd_80176118:
    j .Lcd_801761d0
    addiu $v0, $zero, 4
.Lcd_80176120:
    lui $a0, %hi(audio_cd_results)
    addiu $a0, $a0, %lo(audio_cd_results)
    addiu $a1, $sp, 0x18
    addiu $v0, $zero, 5
    lui $at, %hi(audio_cd_sdk_state + 0x365)
    sb $v0, %lo(audio_cd_sdk_state + 0x365)($at)
    lui $v1, %hi(audio_cd_sdk_state + 0x365)
    lbu $v1, %lo(audio_cd_sdk_state + 0x365)($v1)
    lui $v0, %hi(audio_cd_sdk_state + 0x364)
    addiu $v0, $v0, %lo(audio_cd_sdk_state + 0x364)
    sb $v1, 0($v0)
    beqz $a0, .Lcd_80176170
    addiu $v1, $zero, 7
    addiu $a2, $zero, -1
.Lcd_80176158:
    lbu $v0, 0($a1)
    addiu $a1, $a1, 1
    addiu $v1, $v1, -1
    sb $v0, 0($a0)
    bne $v1, $a2, .Lcd_80176158
    addiu $a0, $a0, 1
.Lcd_80176170:
    lui $v1, %hi(audio_cd_results + 0x8)
    addiu $v1, $v1, %lo(audio_cd_results + 0x8)
    beqz $v1, .Lcd_801761a0
    addiu $a1, $sp, 0x18
    addiu $a0, $zero, 7
    addiu $a2, $zero, -1
.Lcd_80176188:
    lbu $v0, 0($a1)
    addiu $a1, $a1, 1
    addiu $a0, $a0, -1
    sb $v0, 0($v1)
    bne $a0, $a2, .Lcd_80176188
    addiu $v1, $v1, 1
.Lcd_801761a0:
    j .Lcd_801761d0
    addiu $v0, $zero, 6
.Lcd_801761a8:
    lui $a0, %hi(.Lcd_string_8014a334)
    addiu $a0, $a0, %lo(.Lcd_string_8014a334)
    jal audio_cd_service_puts
    nop
    lbu $a1, 0x10($sp)
    lui $a0, %hi(.Lcd_string_8014a348)
    addiu $a0, $a0, %lo(.Lcd_string_8014a348)
    jal audio_spu_service_printf
    nop
.Lcd_801761cc:
    addu $v0, $zero, $zero
.Lcd_801761d0:
    lw $ra, 0x28($sp)
    lw $s1, 0x24($sp)
    lw $s0, 0x20($sp)
    addiu $sp, $sp, 0x30
    jr $ra
    nop
.end audio_cd_irq_drain

# exe/slus_004_22@0x801761e8..0x80176468.
.globl audio_cd_sync_core
.ent audio_cd_sync_core
audio_cd_sync_core:
    addiu $sp, $sp, -0x38
    sw $s5, 0x2c($sp)
    addu $s5, $a0, $zero
    sw $s6, 0x30($sp)
    addu $s6, $a1, $zero
    addiu $a0, $zero, -1
    sw $ra, 0x34($sp)
    sw $s4, 0x28($sp)
    sw $s3, 0x24($sp)
    sw $s2, 0x20($sp)
    sw $s1, 0x1c($sp)
    jal audio_vsync
    sw $s0, 0x18($sp)
    lui $s3, %hi(audio_cd_sdk_state + 0x12c)
    addiu $s3, $s3, %lo(audio_cd_sdk_state + 0x12c)
    lui $s2, %hi(audio_cd_sdk_state + 0x364)
    addiu $s2, $s2, %lo(audio_cd_sdk_state + 0x364)
    addiu $s4, $s2, 1
    addiu $v0, $v0, 0x3c0
    lui $at, %hi(audio_cd_results + 0x18)
    sw $v0, %lo(audio_cd_results + 0x18)($at)
    lui $v0, %hi(.Lcd_string_8014a368)
    addiu $v0, $v0, %lo(.Lcd_string_8014a368)
    lui $at, %hi(audio_cd_results + 0x1c)
    sw $zero, %lo(audio_cd_results + 0x1c)($at)
    lui $at, %hi(audio_cd_results + 0x20)
    sw $v0, %lo(audio_cd_results + 0x20)($at)
.Lcd_80176254:
    jal audio_vsync
    addiu $a0, $zero, -1
    lui $v1, %hi(audio_cd_results + 0x18)
    lw $v1, %lo(audio_cd_results + 0x18)($v1)
    nop
    slt $v1, $v1, $v0
    bnez $v1, .Lcd_8017629c
    nop
    lui $v1, %hi(audio_cd_results + 0x1c)
    lw $v1, %lo(audio_cd_results + 0x1c)($v1)
    nop
    addiu $v0, $v1, 1
    lui $at, %hi(audio_cd_results + 0x1c)
    sw $v0, %lo(audio_cd_results + 0x1c)($at)
    lui $v0, 0x3c
    slt $v0, $v0, $v1
    beqz $v0, .Lcd_80176310
    nop
.Lcd_8017629c:
    lui $a0, %hi(.Lcd_string_8014a2e0)
    addiu $a0, $a0, %lo(.Lcd_string_8014a2e0)
    jal audio_cd_service_puts
    nop
    lbu $a0, 0($s2)
    lbu $v0, 1($s2)
    lui $a1, %hi(audio_cd_results + 0x20)
    lw $a1, %lo(audio_cd_results + 0x20)($a1)
    sll $v0, $v0, 2
    addu $v0, $v0, $s3
    sll $a0, $a0, 2
    lw $v1, 0($v0)
    lui $v0, %hi(audio_cd_sdk_state + 0xa9)
    lbu $v0, %lo(audio_cd_sdk_state + 0xa9)($v0)
    addu $a0, $a0, $s3
    sll $v0, $v0, 2
    sw $v1, 0x10($sp)
    lui $at, %hi(audio_cd_sdk_state + 0xac)
    addu $at, $at, $v0
    lw $a2, %lo(audio_cd_sdk_state + 0xac)($at)
    lw $a3, 0($a0)
    lui $a0, %hi(.Lcd_string_8014a2f0)
    addiu $a0, $a0, %lo(.Lcd_string_8014a2f0)
    jal audio_spu_service_printf
    nop
    jal audio_cd_recover
    nop
    j .Lcd_80176314
    addiu $v0, $zero, -1
.Lcd_80176310:
    addu $v0, $zero, $zero
.Lcd_80176314:
    bnez $v0, .Lcd_8017643c
    addiu $v0, $zero, -1
    jal audio_callbacks_active
    nop
    beqz $v0, .Lcd_801763d4
    nop
    lui $v0, %hi(audio_cd_sdk_state + 0x34c)
    lw $v0, %lo(audio_cd_sdk_state + 0x34c)($v0)
    nop
    lbu $v0, 0($v0)
    nop
    andi $s1, $v0, 3
.Lcd_80176344:
    jal audio_cd_irq_drain
    nop
    addu $s0, $v0, $zero
    beqz $s0, .Lcd_801763c4
    andi $v0, $s0, 4
    beqz $v0, .Lcd_8017638c
    andi $v0, $s0, 2
    lui $v0, %hi(audio_cd_sdk_state + 0x8c)
    lw $v0, %lo(audio_cd_sdk_state + 0x8c)($v0)
    nop
    beqz $v0, .Lcd_80176388
    nop
    lbu $a0, 0($s4)
    lui $a1, %hi(audio_cd_results + 0x8)
    addiu $a1, $a1, %lo(audio_cd_results + 0x8)
    jalr $v0
    nop
.Lcd_80176388:
    andi $v0, $s0, 2
.Lcd_8017638c:
    beqz $v0, .Lcd_80176344
    nop
    lui $v0, %hi(audio_cd_sdk_state + 0x88)
    lw $v0, %lo(audio_cd_sdk_state + 0x88)($v0)
    nop
    beqz $v0, .Lcd_80176344
    nop
    lbu $a0, 0($s2)
    lui $a1, %hi(audio_cd_results)
    addiu $a1, $a1, %lo(audio_cd_results)
    jalr $v0
    nop
    j .Lcd_80176344
    nop
.Lcd_801763c4:
    lui $v0, %hi(audio_cd_sdk_state + 0x34c)
    lw $v0, %lo(audio_cd_sdk_state + 0x34c)($v0)
    nop
    sb $s1, 0($v0)
.Lcd_801763d4:
    lbu $v0, 0($s2)
    nop
    andi $a2, $v0, 0xff
    addiu $v0, $zero, 2
    beq $a2, $v0, .Lcd_801763f4
    addiu $v0, $zero, 5
    bne $a2, $v0, .Lcd_80176434
    nop
.Lcd_801763f4:
    addiu $v0, $zero, 2
    sb $v0, 0($s2)
    addu $a1, $s6, $zero
    lui $a0, %hi(audio_cd_results)
    addiu $a0, $a0, %lo(audio_cd_results)
    beqz $a1, .Lcd_8017642c
    addiu $v1, $zero, 7
    addiu $a3, $zero, -1
.Lcd_80176414:
    lbu $v0, 0($a0)
    addiu $a0, $a0, 1
    addiu $v1, $v1, -1
    sb $v0, 0($a1)
    bne $v1, $a3, .Lcd_80176414
    addiu $a1, $a1, 1
.Lcd_8017642c:
    j .Lcd_8017643c
    addu $v0, $a2, $zero
.Lcd_80176434:
    beqz $s5, .Lcd_80176254
    addu $v0, $zero, $zero
.Lcd_8017643c:
    lw $ra, 0x34($sp)
    lw $s6, 0x30($sp)
    lw $s5, 0x2c($sp)
    lw $s4, 0x28($sp)
    lw $s3, 0x24($sp)
    lw $s2, 0x20($sp)
    lw $s1, 0x1c($sp)
    lw $s0, 0x18($sp)
    addiu $sp, $sp, 0x38
    jr $ra
    nop
.end audio_cd_sync_core

# exe/slus_004_22@0x80176734..0x80176b50.
.globl audio_cd_command_core
.ent audio_cd_command_core
audio_cd_command_core:
    lui $v0, %hi(audio_cd_sdk_state + 0x94)
    lw $v0, %lo(audio_cd_sdk_state + 0x94)($v0)
    addiu $sp, $sp, -0x48
    sw $s1, 0x2c($sp)
    addu $s1, $a1, $zero
    sw $s5, 0x3c($sp)
    addu $s5, $a2, $zero
    sw $s2, 0x30($sp)
    addu $s2, $a3, $zero
    sw $s0, 0x28($sp)
    addu $s0, $a0, $zero
    sw $ra, 0x40($sp)
    sw $s4, 0x38($sp)
    slti $v0, $v0, 2
    bnez $v0, .Lcd_80176798
    sw $s3, 0x34($sp)
    andi $v0, $s0, 0xff
    sll $v0, $v0, 2
    lui $at, %hi(audio_cd_sdk_state + 0xac)
    addu $at, $at, $v0
    lw $a1, %lo(audio_cd_sdk_state + 0xac)($at)
    lui $a0, %hi(.Lcd_string_8014a37c)
    addiu $a0, $a0, %lo(.Lcd_string_8014a37c)
    jal audio_spu_service_printf
    nop
.Lcd_80176798:
    andi $v0, $s0, 0xff
    sll $v1, $v0, 2
    lui $at, %hi(audio_cd_sdk_state + 0x2cc)
    addu $at, $at, $v1
    lw $v0, %lo(audio_cd_sdk_state + 0x2cc)($at)
    nop
    beqz $v0, .Lcd_801767f8
    addu $a0, $zero, $zero
    bnez $s1, .Lcd_801767f8
    nop
    lui $v0, %hi(audio_cd_sdk_state + 0x94)
    lw $v0, %lo(audio_cd_sdk_state + 0x94)($v0)
    nop
    blez $v0, .Lcd_80176b28
    addiu $v0, $zero, -2
    lui $at, %hi(audio_cd_sdk_state + 0xac)
    addu $at, $at, $v1
    lw $a1, %lo(audio_cd_sdk_state + 0xac)($at)
    lui $a0, %hi(.Lcd_string_8014a384)
    addiu $a0, $a0, %lo(.Lcd_string_8014a384)
    jal audio_spu_service_printf
    nop
    j .Lcd_80176b28
    addiu $v0, $zero, -2
.Lcd_801767f8:
    jal audio_cd_sync_core
    addu $a1, $zero, $zero
    andi $v1, $s0, 0xff
    addiu $v0, $zero, 2
    bne $v1, $v0, .Lcd_80176840
    addiu $v0, $zero, 0xe
    addu $a0, $zero, $zero
    addu $v1, $s1, $zero
.Lcd_80176818:
    lbu $v0, 0($v1)
    lui $at, %hi(audio_cd_sdk_state + 0xa4)
    addu $at, $at, $a0
    sb $v0, %lo(audio_cd_sdk_state + 0xa4)($at)
    addiu $a0, $a0, 1
    slti $v0, $a0, 4
    bnez $v0, .Lcd_80176818
    addiu $v1, $v1, 1
    andi $v1, $s0, 0xff
    addiu $v0, $zero, 0xe
.Lcd_80176840:
    bne $v1, $v0, .Lcd_80176854
    nop
    lbu $v0, 0($s1)
    lui $at, %hi(audio_cd_sdk_state + 0xa8)
    sb $v0, %lo(audio_cd_sdk_state + 0xa8)($at)
.Lcd_80176854:
    lui $v0, %hi(audio_cd_sdk_state + 0x364)
    addiu $v0, $v0, %lo(audio_cd_sdk_state + 0x364)
    sll $a0, $v1, 2
    sb $zero, 0($v0)
    lui $at, %hi(audio_cd_sdk_state + 0x1cc)
    addu $at, $at, $a0
    lw $v0, %lo(audio_cd_sdk_state + 0x1cc)($at)
    lui $v1, %hi(audio_cd_sdk_state + 0x1cc)
    addiu $v1, $v1, %lo(audio_cd_sdk_state + 0x1cc)
    beqz $v0, .Lcd_80176888
    nop
    lui $at, %hi(audio_cd_sdk_state + 0x365)
    sb $zero, %lo(audio_cd_sdk_state + 0x365)($at)
.Lcd_80176888:
    lui $v0, %hi(audio_cd_sdk_state + 0x34c)
    lw $v0, %lo(audio_cd_sdk_state + 0x34c)($v0)
    nop
    sb $zero, 0($v0)
    addiu $v0, $v1, 0x100
    addu $v1, $a0, $v0
    lw $v0, 0($v1)
    nop
    blez $v0, .Lcd_801768e0
    addu $a0, $zero, $zero
    addu $a2, $v1, $zero
    addu $a1, $s1, $zero
.Lcd_801768b8:
    lui $v1, %hi(audio_cd_sdk_state + 0x354)
    lw $v1, %lo(audio_cd_sdk_state + 0x354)($v1)
    lbu $v0, 0($a1)
    addiu $a1, $a1, 1
    sb $v0, 0($v1)
    lw $v0, 0($a2)
    addiu $a0, $a0, 1
    slt $v0, $a0, $v0
    bnez $v0, .Lcd_801768b8
    nop
.Lcd_801768e0:
    lui $v0, %hi(audio_cd_sdk_state + 0x350)
    lw $v0, %lo(audio_cd_sdk_state + 0x350)($v0)
    lui $at, %hi(audio_cd_sdk_state + 0xa9)
    sb $s0, %lo(audio_cd_sdk_state + 0xa9)($at)
    sb $s0, 0($v0)
    bnez $s2, .Lcd_80176b28
    addu $v0, $zero, $zero
    jal audio_vsync
    addiu $a0, $zero, -1
    addiu $v0, $v0, 0x3c0
    lui $a0, %hi(audio_cd_sdk_state + 0x364)
    addiu $a0, $a0, %lo(audio_cd_sdk_state + 0x364)
    lui $at, %hi(audio_cd_results + 0x18)
    sw $v0, %lo(audio_cd_results + 0x18)($at)
    lui $at, %hi(audio_cd_results + 0x1c)
    sw $zero, %lo(audio_cd_results + 0x1c)($at)
    lbu $v1, 0($a0)
    lui $v0, %hi(.Lcd_string_8014a394)
    addiu $v0, $v0, %lo(.Lcd_string_8014a394)
    lui $at, %hi(audio_cd_results + 0x20)
    sw $v0, %lo(audio_cd_results + 0x20)($at)
    bnez $v1, .Lcd_80176adc
    addu $a2, $s5, $zero
    lui $s3, %hi(audio_cd_sdk_state + 0x12c)
    addiu $s3, $s3, %lo(audio_cd_sdk_state + 0x12c)
    addu $s2, $a0, $zero
    addiu $s4, $s2, 1
.Lcd_8017694c:
    jal audio_vsync
    addiu $a0, $zero, -1
    lui $v1, %hi(audio_cd_results + 0x18)
    lw $v1, %lo(audio_cd_results + 0x18)($v1)
    nop
    slt $v1, $v1, $v0
    bnez $v1, .Lcd_80176994
    nop
    lui $v1, %hi(audio_cd_results + 0x1c)
    lw $v1, %lo(audio_cd_results + 0x1c)($v1)
    nop
    addiu $v0, $v1, 1
    lui $at, %hi(audio_cd_results + 0x1c)
    sw $v0, %lo(audio_cd_results + 0x1c)($at)
    lui $v0, 0x3c
    slt $v0, $v0, $v1
    beqz $v0, .Lcd_80176a08
    nop
.Lcd_80176994:
    lui $a0, %hi(.Lcd_string_8014a2e0)
    addiu $a0, $a0, %lo(.Lcd_string_8014a2e0)
    jal audio_cd_service_puts
    nop
    lbu $a0, 0($s2)
    lbu $v0, 1($s2)
    lui $a1, %hi(audio_cd_results + 0x20)
    lw $a1, %lo(audio_cd_results + 0x20)($a1)
    sll $v0, $v0, 2
    addu $v0, $v0, $s3
    sll $a0, $a0, 2
    lw $v1, 0($v0)
    lui $v0, %hi(audio_cd_sdk_state + 0xa9)
    lbu $v0, %lo(audio_cd_sdk_state + 0xa9)($v0)
    addu $a0, $a0, $s3
    sll $v0, $v0, 2
    sw $v1, 0x10($sp)
    lui $at, %hi(audio_cd_sdk_state + 0xac)
    addu $at, $at, $v0
    lw $a2, %lo(audio_cd_sdk_state + 0xac)($at)
    lw $a3, 0($a0)
    lui $a0, %hi(.Lcd_string_8014a2f0)
    addiu $a0, $a0, %lo(.Lcd_string_8014a2f0)
    jal audio_spu_service_printf
    nop
    jal audio_cd_recover
    nop
    j .Lcd_80176a0c
    addiu $v0, $zero, -1
.Lcd_80176a08:
    addu $v0, $zero, $zero
.Lcd_80176a0c:
    bnez $v0, .Lcd_80176b28
    addiu $v0, $zero, -1
    jal audio_callbacks_active
    nop
    beqz $v0, .Lcd_80176acc
    nop
    lui $v0, %hi(audio_cd_sdk_state + 0x34c)
    lw $v0, %lo(audio_cd_sdk_state + 0x34c)($v0)
    nop
    lbu $v0, 0($v0)
    nop
    andi $s1, $v0, 3
.Lcd_80176a3c:
    jal audio_cd_irq_drain
    nop
    addu $s0, $v0, $zero
    beqz $s0, .Lcd_80176abc
    andi $v0, $s0, 4
    beqz $v0, .Lcd_80176a84
    andi $v0, $s0, 2
    lui $v0, %hi(audio_cd_sdk_state + 0x8c)
    lw $v0, %lo(audio_cd_sdk_state + 0x8c)($v0)
    nop
    beqz $v0, .Lcd_80176a80
    nop
    lbu $a0, 0($s4)
    lui $a1, %hi(audio_cd_results + 0x8)
    addiu $a1, $a1, %lo(audio_cd_results + 0x8)
    jalr $v0
    nop
.Lcd_80176a80:
    andi $v0, $s0, 2
.Lcd_80176a84:
    beqz $v0, .Lcd_80176a3c
    nop
    lui $v0, %hi(audio_cd_sdk_state + 0x88)
    lw $v0, %lo(audio_cd_sdk_state + 0x88)($v0)
    nop
    beqz $v0, .Lcd_80176a3c
    nop
    lbu $a0, 0($s2)
    lui $a1, %hi(audio_cd_results)
    addiu $a1, $a1, %lo(audio_cd_results)
    jalr $v0
    nop
    j .Lcd_80176a3c
    nop
.Lcd_80176abc:
    lui $v0, %hi(audio_cd_sdk_state + 0x34c)
    lw $v0, %lo(audio_cd_sdk_state + 0x34c)($v0)
    nop
    sb $s1, 0($v0)
.Lcd_80176acc:
    lbu $v0, 0($s2)
    nop
    beqz $v0, .Lcd_8017694c
    addu $a2, $s5, $zero
.Lcd_80176adc:
    lui $a0, %hi(audio_cd_results)
    addiu $a0, $a0, %lo(audio_cd_results)
    beqz $a2, .Lcd_80176b0c
    nop
    addiu $v1, $zero, 7
    addiu $a1, $zero, -1
.Lcd_80176af4:
    lbu $v0, 0($a0)
    addiu $a0, $a0, 1
    addiu $v1, $v1, -1
    sb $v0, 0($a2)
    bne $v1, $a1, .Lcd_80176af4
    addiu $a2, $a2, 1
.Lcd_80176b0c:
    lui $v0, %hi(audio_cd_sdk_state + 0x364)
    addiu $v0, $v0, %lo(audio_cd_sdk_state + 0x364)
    lbu $v0, 0($v0)
    nop
    xori $v0, $v0, 5
    sltiu $v0, $v0, 1
    subu $v0, $zero, $v0
.Lcd_80176b28:
    lw $ra, 0x40($sp)
    lw $s5, 0x3c($sp)
    lw $s4, 0x38($sp)
    lw $s3, 0x34($sp)
    lw $s2, 0x30($sp)
    lw $s1, 0x2c($sp)
    lw $s0, 0x28($sp)
    addiu $sp, $sp, 0x48
    jr $ra
    nop
.end audio_cd_command_core

# exe/slus_004_22@0x80176b50..0x80176bd8.
.globl audio_cd_mix_core
.ent audio_cd_mix_core
audio_cd_mix_core:
    lui $v1, %hi(audio_cd_sdk_state + 0x34c)
    lw $v1, %lo(audio_cd_sdk_state + 0x34c)($v1)
    addiu $v0, $zero, 2
    sb $v0, 0($v1)
    lui $v1, %hi(audio_cd_sdk_state + 0x354)
    lw $v1, %lo(audio_cd_sdk_state + 0x354)($v1)
    lbu $v0, 0($a0)
    nop
    sb $v0, 0($v1)
    lui $v1, %hi(audio_cd_sdk_state + 0x358)
    lw $v1, %lo(audio_cd_sdk_state + 0x358)($v1)
    lbu $v0, 1($a0)
    nop
    sb $v0, 0($v1)
    lui $v1, %hi(audio_cd_sdk_state + 0x34c)
    lw $v1, %lo(audio_cd_sdk_state + 0x34c)($v1)
    addiu $v0, $zero, 3
    sb $v0, 0($v1)
    lui $v1, %hi(audio_cd_sdk_state + 0x350)
    lw $v1, %lo(audio_cd_sdk_state + 0x350)($v1)
    lbu $v0, 2($a0)
    nop
    sb $v0, 0($v1)
    lui $v1, %hi(audio_cd_sdk_state + 0x354)
    lw $v1, %lo(audio_cd_sdk_state + 0x354)($v1)
    lbu $v0, 3($a0)
    nop
    sb $v0, 0($v1)
    lui $v1, %hi(audio_cd_sdk_state + 0x358)
    lw $v1, %lo(audio_cd_sdk_state + 0x358)($v1)
    addiu $v0, $zero, 0x20
    sb $v0, 0($v1)
    jr $ra
    addu $v0, $zero, $zero
.end audio_cd_mix_core

# exe/slus_004_22@0x80176bd8..0x80176cb8.
.globl audio_cd_recover
.ent audio_cd_recover
audio_cd_recover:
    lui $v1, %hi(audio_cd_sdk_state + 0x34c)
    lw $v1, %lo(audio_cd_sdk_state + 0x34c)($v1)
    addiu $v0, $zero, 1
    sb $v0, 0($v1)
    lui $v0, %hi(audio_cd_sdk_state + 0x358)
    lw $v0, %lo(audio_cd_sdk_state + 0x358)($v0)
    nop
    lbu $v0, 0($v0)
    nop
    andi $v0, $v0, 7
    beqz $v0, .Lcd_80176c5c
    addiu $a0, $zero, 1
    addiu $v1, $zero, 7
.Lcd_80176c0c:
    lui $v0, %hi(audio_cd_sdk_state + 0x34c)
    lw $v0, %lo(audio_cd_sdk_state + 0x34c)($v0)
    nop
    sb $a0, 0($v0)
    lui $v0, %hi(audio_cd_sdk_state + 0x358)
    lw $v0, %lo(audio_cd_sdk_state + 0x358)($v0)
    nop
    sb $v1, 0($v0)
    lui $v0, %hi(audio_cd_sdk_state + 0x354)
    lw $v0, %lo(audio_cd_sdk_state + 0x354)($v0)
    nop
    sb $v1, 0($v0)
    lui $v0, %hi(audio_cd_sdk_state + 0x358)
    lw $v0, %lo(audio_cd_sdk_state + 0x358)($v0)
    nop
    lbu $v0, 0($v0)
    nop
    andi $v0, $v0, 7
    bnez $v0, .Lcd_80176c0c
    nop
.Lcd_80176c5c:
    lui $at, %hi(audio_cd_sdk_state + 0x366)
    sb $zero, %lo(audio_cd_sdk_state + 0x366)($at)
    lui $v0, %hi(audio_cd_sdk_state + 0x366)
    lbu $v0, %lo(audio_cd_sdk_state + 0x366)($v0)
    lui $a0, %hi(audio_cd_sdk_state + 0x34c)
    lw $a0, %lo(audio_cd_sdk_state + 0x34c)($a0)
    lui $v1, %hi(audio_cd_sdk_state + 0x364)
    addiu $v1, $v1, %lo(audio_cd_sdk_state + 0x364)
    lui $at, %hi(audio_cd_sdk_state + 0x365)
    sb $v0, %lo(audio_cd_sdk_state + 0x365)($at)
    addiu $v0, $zero, 2
    sb $v0, 0($v1)
    sb $zero, 0($a0)
    lui $v0, %hi(audio_cd_sdk_state + 0x358)
    lw $v0, %lo(audio_cd_sdk_state + 0x358)($v0)
    nop
    sb $zero, 0($v0)
    lui $v1, %hi(audio_cd_sdk_state + 0x35c)
    lw $v1, %lo(audio_cd_sdk_state + 0x35c)($v1)
    addiu $v0, $zero, 0x1325
    sw $v0, 0($v1)
    jr $ra
    nop
.end audio_cd_recover

# exe/slus_004_22@0x8017e33c..0x8017e34c.
.globl audio_cd_service_puts
.ent audio_cd_service_puts
audio_cd_service_puts:
    addiu $t2, $zero, 0xb0
    jr $t2
    addiu $t1, $zero, 0x3f
    nop
.end audio_cd_service_puts

# exe/slus_004_22@0x80176468..0x80176734.
.globl audio_cd_ready_core
.ent audio_cd_ready_core
audio_cd_ready_core:
    addiu $sp, $sp, -0x40
    sw $s7, 0x34($sp)
    addu $s7, $a0, $zero
    sw $s4, 0x28($sp)
    addu $s4, $a1, $zero
    addiu $a0, $zero, -1
    sw $ra, 0x38($sp)
    sw $s6, 0x30($sp)
    sw $s5, 0x2c($sp)
    sw $s3, 0x24($sp)
    sw $s2, 0x20($sp)
    sw $s1, 0x1c($sp)
    jal audio_vsync
    sw $s0, 0x18($sp)
    lui $s5, %hi(audio_cd_sdk_state + 0x12c)
    addiu $s5, $s5, %lo(audio_cd_sdk_state + 0x12c)
    lui $s2, %hi(audio_cd_sdk_state + 0x364)
    addiu $s2, $s2, %lo(audio_cd_sdk_state + 0x364)
    addiu $s6, $s2, 1
    addiu $s3, $s2, 2
    addiu $v0, $v0, 0x3c0
    lui $at, %hi(audio_cd_results + 0x18)
    sw $v0, %lo(audio_cd_results + 0x18)($at)
    lui $v0, %hi(.Lcd_string_8014a370)
    addiu $v0, $v0, %lo(.Lcd_string_8014a370)
    lui $at, %hi(audio_cd_results + 0x1c)
    sw $zero, %lo(audio_cd_results + 0x1c)($at)
    lui $at, %hi(audio_cd_results + 0x20)
    sw $v0, %lo(audio_cd_results + 0x20)($at)
.Lcd_801764dc:
    jal audio_vsync
    addiu $a0, $zero, -1
    lui $v1, %hi(audio_cd_results + 0x18)
    lw $v1, %lo(audio_cd_results + 0x18)($v1)
    nop
    slt $v1, $v1, $v0
    bnez $v1, .Lcd_80176524
    nop
    lui $v1, %hi(audio_cd_results + 0x1c)
    lw $v1, %lo(audio_cd_results + 0x1c)($v1)
    nop
    addiu $v0, $v1, 1
    lui $at, %hi(audio_cd_results + 0x1c)
    sw $v0, %lo(audio_cd_results + 0x1c)($at)
    lui $v0, 0x3c
    slt $v0, $v0, $v1
    beqz $v0, .Lcd_80176598
    nop
.Lcd_80176524:
    lui $a0, %hi(.Lcd_string_8014a2e0)
    addiu $a0, $a0, %lo(.Lcd_string_8014a2e0)
    jal audio_cd_service_puts
    nop
    lbu $a0, 0($s2)
    lbu $v0, 1($s2)
    lui $a1, %hi(audio_cd_results + 0x20)
    lw $a1, %lo(audio_cd_results + 0x20)($a1)
    sll $v0, $v0, 2
    addu $v0, $v0, $s5
    sll $a0, $a0, 2
    lw $v1, 0($v0)
    lui $v0, %hi(audio_cd_sdk_state + 0xa9)
    lbu $v0, %lo(audio_cd_sdk_state + 0xa9)($v0)
    addu $a0, $a0, $s5
    sll $v0, $v0, 2
    sw $v1, 0x10($sp)
    lui $at, %hi(audio_cd_sdk_state + 0xac)
    addu $at, $at, $v0
    lw $a2, %lo(audio_cd_sdk_state + 0xac)($at)
    lw $a3, 0($a0)
    lui $a0, %hi(.Lcd_string_8014a2f0)
    addiu $a0, $a0, %lo(.Lcd_string_8014a2f0)
    jal audio_spu_service_printf
    nop
    jal audio_cd_recover
    nop
    j .Lcd_8017659c
    addiu $v0, $zero, -1
.Lcd_80176598:
    addu $v0, $zero, $zero
.Lcd_8017659c:
    bnez $v0, .Lcd_80176704
    addiu $v0, $zero, -1
    jal audio_callbacks_active
    nop
    beqz $v0, .Lcd_8017665c
    nop
    lui $v0, %hi(audio_cd_sdk_state + 0x34c)
    lw $v0, %lo(audio_cd_sdk_state + 0x34c)($v0)
    nop
    lbu $v0, 0($v0)
    nop
    andi $s1, $v0, 3
.Lcd_801765cc:
    jal audio_cd_irq_drain
    nop
    addu $s0, $v0, $zero
    beqz $s0, .Lcd_8017664c
    andi $v0, $s0, 4
    beqz $v0, .Lcd_80176614
    andi $v0, $s0, 2
    lui $v0, %hi(audio_cd_sdk_state + 0x8c)
    lw $v0, %lo(audio_cd_sdk_state + 0x8c)($v0)
    nop
    beqz $v0, .Lcd_80176610
    nop
    lbu $a0, 0($s6)
    lui $a1, %hi(audio_cd_results + 0x8)
    addiu $a1, $a1, %lo(audio_cd_results + 0x8)
    jalr $v0
    nop
.Lcd_80176610:
    andi $v0, $s0, 2
.Lcd_80176614:
    beqz $v0, .Lcd_801765cc
    nop
    lui $v0, %hi(audio_cd_sdk_state + 0x88)
    lw $v0, %lo(audio_cd_sdk_state + 0x88)($v0)
    nop
    beqz $v0, .Lcd_801765cc
    nop
    lbu $a0, 0($s2)
    lui $a1, %hi(audio_cd_results)
    addiu $a1, $a1, %lo(audio_cd_results)
    jalr $v0
    nop
    j .Lcd_801765cc
    nop
.Lcd_8017664c:
    lui $v0, %hi(audio_cd_sdk_state + 0x34c)
    lw $v0, %lo(audio_cd_sdk_state + 0x34c)($v0)
    nop
    sb $s1, 0($v0)
.Lcd_8017665c:
    lbu $v0, 0($s3)
    nop
    andi $a2, $v0, 0xff
    beqz $a2, .Lcd_801766ac
    nop
    sb $zero, 0($s3)
    lui $a0, %hi(audio_cd_results + 0x10)
    addiu $a0, $a0, %lo(audio_cd_results + 0x10)
    beqz $s4, .Lcd_801766f4
    addu $a1, $s4, $zero
    addiu $v1, $zero, 7
    addiu $a3, $zero, -1
.Lcd_8017668c:
    lbu $v0, 0($a0)
    addiu $a0, $a0, 1
    addiu $v1, $v1, -1
    sb $v0, 0($a1)
    bne $v1, $a3, .Lcd_8017668c
    addiu $a1, $a1, 1
    j .Lcd_80176704
    addu $v0, $a2, $zero
.Lcd_801766ac:
    lbu $v0, -1($s3)
    nop
    andi $a2, $v0, 0xff
    beqz $a2, .Lcd_801766fc
    nop
    sb $zero, -1($s3)
    addu $a1, $s4, $zero
    lui $a0, %hi(audio_cd_results + 0x8)
    addiu $a0, $a0, %lo(audio_cd_results + 0x8)
    beqz $a1, .Lcd_801766f4
    addiu $v1, $zero, 7
    addiu $a3, $zero, -1
.Lcd_801766dc:
    lbu $v0, 0($a0)
    addiu $a0, $a0, 1
    addiu $v1, $v1, -1
    sb $v0, 0($a1)
    bne $v1, $a3, .Lcd_801766dc
    addiu $a1, $a1, 1
.Lcd_801766f4:
    j .Lcd_80176704
    addu $v0, $a2, $zero
.Lcd_801766fc:
    beqz $s7, .Lcd_801764dc
    addu $v0, $zero, $zero
.Lcd_80176704:
    lw $ra, 0x38($sp)
    lw $s7, 0x34($sp)
    lw $s6, 0x30($sp)
    lw $s5, 0x2c($sp)
    lw $s4, 0x28($sp)
    lw $s3, 0x24($sp)
    lw $s2, 0x20($sp)
    lw $s1, 0x1c($sp)
    lw $s0, 0x18($sp)
    addiu $sp, $sp, 0x40
    jr $ra
    nop
.end audio_cd_ready_core

# exe/slus_004_22@0x80176cb8..0x80176dac.
.globl audio_cd_spu_initialize
.ent audio_cd_spu_initialize
audio_cd_spu_initialize:
    lui $v1, %hi(audio_cd_sdk_state + 0x360)
    lw $v1, %lo(audio_cd_sdk_state + 0x360)($v1)
    nop
    lhu $v0, 0x1b8($v1)
    nop
    bnez $v0, .Lcd_80176cf4
    addiu $sp, $sp, -8
    lhu $v0, 0x1ba($v1)
    nop
    bnez $v0, .Lcd_80176cf8
    addiu $v0, $zero, 0x3fff
    sh $v0, 0x180($v1)
    sh $v0, 0x182($v1)
    lui $v1, %hi(audio_cd_sdk_state + 0x360)
    lw $v1, %lo(audio_cd_sdk_state + 0x360)($v1)
.Lcd_80176cf4:
    addiu $v0, $zero, 0x3fff
.Lcd_80176cf8:
    sh $v0, 0x1b0($v1)
    sh $v0, 0x1b2($v1)
    ori $v0, $zero, 0xc001
    sh $v0, 0x1aa($v1)
    lui $v1, %hi(audio_cd_sdk_state + 0x34c)
    lw $v1, %lo(audio_cd_sdk_state + 0x34c)($v1)
    addiu $v0, $zero, 0x80
    sb $v0, 2($sp)
    sb $v0, 0($sp)
    addiu $v0, $zero, 2
    sb $zero, 3($sp)
    sb $zero, 1($sp)
    sb $v0, 0($v1)
    lui $v1, %hi(audio_cd_sdk_state + 0x354)
    lw $v1, %lo(audio_cd_sdk_state + 0x354)($v1)
    lbu $v0, 0($sp)
    nop
    sb $v0, 0($v1)
    lui $v1, %hi(audio_cd_sdk_state + 0x358)
    lw $v1, %lo(audio_cd_sdk_state + 0x358)($v1)
    lbu $v0, 1($sp)
    nop
    sb $v0, 0($v1)
    lui $v1, %hi(audio_cd_sdk_state + 0x34c)
    lw $v1, %lo(audio_cd_sdk_state + 0x34c)($v1)
    addiu $v0, $zero, 3
    sb $v0, 0($v1)
    lui $v1, %hi(audio_cd_sdk_state + 0x350)
    lw $v1, %lo(audio_cd_sdk_state + 0x350)($v1)
    lbu $v0, 2($sp)
    nop
    sb $v0, 0($v1)
    lui $v1, %hi(audio_cd_sdk_state + 0x354)
    lw $v1, %lo(audio_cd_sdk_state + 0x354)($v1)
    lbu $v0, 3($sp)
    nop
    sb $v0, 0($v1)
    lui $v1, %hi(audio_cd_sdk_state + 0x358)
    lw $v1, %lo(audio_cd_sdk_state + 0x358)($v1)
    addiu $v0, $zero, 0x20
    sb $v0, 0($v1)
    addu $v0, $zero, $zero
    addiu $sp, $sp, 8
    jr $ra
    nop
.end audio_cd_spu_initialize

# exe/slus_004_22@0x80176dac..0x80176dfc.
.globl audio_cd_register_irq
.ent audio_cd_register_irq
audio_cd_register_irq:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    lui $at, %hi(audio_cd_sdk_state + 0x8c)
    sw $zero, %lo(audio_cd_sdk_state + 0x8c)($at)
    lui $at, %hi(audio_cd_sdk_state + 0x88)
    sw $zero, %lo(audio_cd_sdk_state + 0x88)($at)
    lui $at, %hi(audio_cd_sdk_state + 0x9c)
    sw $zero, %lo(audio_cd_sdk_state + 0x9c)($at)
    lui $at, %hi(audio_cd_sdk_state + 0x98)
    sw $zero, %lo(audio_cd_sdk_state + 0x98)($at)
    jal audio_callbacks_reset
    nop
    lui $a1, %hi(audio_cd_irq_dispatch)
    addiu $a1, $a1, %lo(audio_cd_irq_dispatch)
    jal audio_callbacks_interrupt
    addiu $a0, $zero, 2
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_cd_register_irq

# exe/slus_004_22@0x80176dfc..0x80176fe8.
.globl audio_cd_initialize_core
.ent audio_cd_initialize_core
audio_cd_initialize_core:
    addiu $sp, $sp, -0x18
    lui $a0, %hi(.Lcd_string_8014a3d0)
    addiu $a0, $a0, %lo(.Lcd_string_8014a3d0)
    sw $ra, 0x10($sp)
    jal audio_cd_service_puts
    nop
    lui $a0, %hi(.Lcd_string_8014a3dc)
    addiu $a0, $a0, %lo(.Lcd_string_8014a3dc)
    lui $a1, %hi(audio_cd_sdk_state + 0x368)
    addiu $a1, $a1, %lo(audio_cd_sdk_state + 0x368)
    jal audio_spu_service_printf
    nop
    lui $at, %hi(audio_cd_sdk_state + 0xa9)
    sb $zero, %lo(audio_cd_sdk_state + 0xa9)($at)
    lui $at, %hi(audio_cd_sdk_state + 0xa8)
    sb $zero, %lo(audio_cd_sdk_state + 0xa8)($at)
    lui $at, %hi(audio_cd_sdk_state + 0x8c)
    sw $zero, %lo(audio_cd_sdk_state + 0x8c)($at)
    lui $at, %hi(audio_cd_sdk_state + 0x88)
    sw $zero, %lo(audio_cd_sdk_state + 0x88)($at)
    lui $at, %hi(audio_cd_sdk_state + 0x9c)
    sw $zero, %lo(audio_cd_sdk_state + 0x9c)($at)
    lui $at, %hi(audio_cd_sdk_state + 0x98)
    sw $zero, %lo(audio_cd_sdk_state + 0x98)($at)
    jal audio_callbacks_reset
    nop
    lui $a1, %hi(audio_cd_irq_dispatch)
    addiu $a1, $a1, %lo(audio_cd_irq_dispatch)
    jal audio_callbacks_interrupt
    addiu $a0, $zero, 2
    lui $v1, %hi(audio_cd_sdk_state + 0x34c)
    lw $v1, %lo(audio_cd_sdk_state + 0x34c)($v1)
    addiu $v0, $zero, 1
    sb $v0, 0($v1)
    lui $v0, %hi(audio_cd_sdk_state + 0x358)
    lw $v0, %lo(audio_cd_sdk_state + 0x358)($v0)
    nop
    lbu $v0, 0($v0)
    nop
    andi $v0, $v0, 7
    beqz $v0, .Lcd_80176ef8
    addiu $a0, $zero, 1
    addiu $v1, $zero, 7
.Lcd_80176ea8:
    lui $v0, %hi(audio_cd_sdk_state + 0x34c)
    lw $v0, %lo(audio_cd_sdk_state + 0x34c)($v0)
    nop
    sb $a0, 0($v0)
    lui $v0, %hi(audio_cd_sdk_state + 0x358)
    lw $v0, %lo(audio_cd_sdk_state + 0x358)($v0)
    nop
    sb $v1, 0($v0)
    lui $v0, %hi(audio_cd_sdk_state + 0x354)
    lw $v0, %lo(audio_cd_sdk_state + 0x354)($v0)
    nop
    sb $v1, 0($v0)
    lui $v0, %hi(audio_cd_sdk_state + 0x358)
    lw $v0, %lo(audio_cd_sdk_state + 0x358)($v0)
    nop
    lbu $v0, 0($v0)
    nop
    andi $v0, $v0, 7
    bnez $v0, .Lcd_80176ea8
    nop
.Lcd_80176ef8:
    addiu $a0, $zero, 1
    addu $a1, $zero, $zero
    addu $a2, $zero, $zero
    lui $at, %hi(audio_cd_sdk_state + 0x366)
    sb $zero, %lo(audio_cd_sdk_state + 0x366)($at)
    lui $v0, %hi(audio_cd_sdk_state + 0x366)
    lbu $v0, %lo(audio_cd_sdk_state + 0x366)($v0)
    lui $a3, %hi(audio_cd_sdk_state + 0x34c)
    lw $a3, %lo(audio_cd_sdk_state + 0x34c)($a3)
    lui $v1, %hi(audio_cd_sdk_state + 0x364)
    addiu $v1, $v1, %lo(audio_cd_sdk_state + 0x364)
    lui $at, %hi(audio_cd_sdk_state + 0x365)
    sb $v0, %lo(audio_cd_sdk_state + 0x365)($at)
    addiu $v0, $zero, 2
    sb $v0, 0($v1)
    sb $zero, 0($a3)
    lui $v0, %hi(audio_cd_sdk_state + 0x358)
    lw $v0, %lo(audio_cd_sdk_state + 0x358)($v0)
    addu $a3, $zero, $zero
    sb $zero, 0($v0)
    lui $v1, %hi(audio_cd_sdk_state + 0x35c)
    lw $v1, %lo(audio_cd_sdk_state + 0x35c)($v1)
    addiu $v0, $zero, 0x1325
    sw $v0, 0($v1)
    jal audio_cd_command_core
    nop
    lui $v0, %hi(audio_cd_sdk_state + 0x98)
    lw $v0, %lo(audio_cd_sdk_state + 0x98)($v0)
    nop
    andi $v0, $v0, 0x10
    beqz $v0, .Lcd_80176f88
    addiu $a0, $zero, 1
    addu $a1, $zero, $zero
    addu $a2, $zero, $zero
    jal audio_cd_command_core
    addu $a3, $zero, $zero
.Lcd_80176f88:
    addiu $a0, $zero, 0xa
    addu $a1, $zero, $zero
    addu $a2, $zero, $zero
    jal audio_cd_command_core
    addu $a3, $zero, $zero
    bnez $v0, .Lcd_80176fd8
    addiu $v0, $zero, -1
    addiu $a0, $zero, 0xc
    addu $a1, $zero, $zero
    addu $a2, $zero, $zero
    jal audio_cd_command_core
    addu $a3, $zero, $zero
    bnez $v0, .Lcd_80176fd8
    addiu $v0, $zero, -1
    addu $a0, $zero, $zero
    jal audio_cd_sync_core
    addu $a1, $zero, $zero
    xori $v0, $v0, 2
    sltu $v0, $zero, $v0
    subu $v0, $zero, $v0
.Lcd_80176fd8:
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_cd_initialize_core

# exe/slus_004_22@0x80177264..0x80177348.
.globl audio_cd_irq_dispatch
.ent audio_cd_irq_dispatch
audio_cd_irq_dispatch:
    lui $v0, %hi(audio_cd_sdk_state + 0x34c)
    lw $v0, %lo(audio_cd_sdk_state + 0x34c)($v0)
    addiu $sp, $sp, -0x28
    sw $s1, 0x14($sp)
    lui $s1, %hi(audio_cd_sdk_state + 0x365)
    addiu $s1, $s1, %lo(audio_cd_sdk_state + 0x365)
    sw $ra, 0x20($sp)
    sw $s3, 0x1c($sp)
    sw $s2, 0x18($sp)
    sw $s0, 0x10($sp)
    lbu $v0, 0($v0)
    addiu $s3, $s1, -1
    andi $s2, $v0, 3
.Lcd_80177298:
    jal audio_cd_irq_drain
    nop
    addu $s0, $v0, $zero
    beqz $s0, .Lcd_80177318
    andi $v0, $s0, 4
    beqz $v0, .Lcd_801772e0
    andi $v0, $s0, 2
    lui $v0, %hi(audio_cd_sdk_state + 0x8c)
    lw $v0, %lo(audio_cd_sdk_state + 0x8c)($v0)
    nop
    beqz $v0, .Lcd_801772dc
    nop
    lbu $a0, 0($s1)
    lui $a1, %hi(audio_cd_results + 0x8)
    addiu $a1, $a1, %lo(audio_cd_results + 0x8)
    jalr $v0
    nop
.Lcd_801772dc:
    andi $v0, $s0, 2
.Lcd_801772e0:
    beqz $v0, .Lcd_80177298
    nop
    lui $v0, %hi(audio_cd_sdk_state + 0x88)
    lw $v0, %lo(audio_cd_sdk_state + 0x88)($v0)
    nop
    beqz $v0, .Lcd_80177298
    nop
    lbu $a0, 0($s3)
    lui $a1, %hi(audio_cd_results)
    addiu $a1, $a1, %lo(audio_cd_results)
    jalr $v0
    nop
    j .Lcd_80177298
    nop
.Lcd_80177318:
    lui $v0, %hi(audio_cd_sdk_state + 0x34c)
    lw $v0, %lo(audio_cd_sdk_state + 0x34c)($v0)
    nop
    sb $s2, 0($v0)
    lw $ra, 0x20($sp)
    lw $s3, 0x1c($sp)
    lw $s2, 0x18($sp)
    lw $s1, 0x14($sp)
    lw $s0, 0x10($sp)
    addiu $sp, $sp, 0x28
    jr $ra
    nop
.end audio_cd_irq_dispatch

.section .data
.balign 4
# Original SDK mutable tables and cells: 0x801856F8..0x80185A64.
.globl audio_cd_sdk_state
audio_cd_sdk_state:
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x1
    .word 0x0
    .word 0x0
    .word 0x1
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x1
    .word 0x1
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x1
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0xc047350
    .word 0x3799da
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x2
    .word 0x0
    .word .Lcd_string_8014a298
    .word .Lcd_string_8014a290
    .word .Lcd_string_8014a284
    .word .Lcd_string_8014a27c
    .word .Lcd_string_8014a270
    .word .Lcd_string_8014a264
    .word .Lcd_string_8014a258
    .word .Lcd_string_8014a24c
    .word .Lcd_string_8014a244
    .word .Lcd_string_8014a238
    .word .Lcd_string_8014a22c
    .word .Lcd_string_8014a224
    .word .Lcd_string_8014a218
    .word .Lcd_string_8014a208
    .word .Lcd_string_8014a1fc
    .word .Lcd_string_8014a1f8
    .word .Lcd_string_8014a1ec
    .word .Lcd_string_8014a1e0
    .word .Lcd_string_8014a1f8
    .word .Lcd_string_8014a1d4
    .word .Lcd_string_8014a1c8
    .word .Lcd_string_8014a1bc
    .word .Lcd_string_8014a1b0
    .word .Lcd_string_8014a1f8
    .word .Lcd_string_8014a1f8
    .word .Lcd_string_8014a1f8
    .word .Lcd_string_8014a1f8
    .word .Lcd_string_8014a1a4
    .word .Lcd_string_8014a1f8
    .word .Lcd_string_8014a1f8
    .word .Lcd_string_8014a1f8
    .word .Lcd_string_8014a1f8
    .word .Lcd_string_8014a2d8
    .word .Lcd_string_8014a2cc
    .word .Lcd_string_8014a2c0
    .word .Lcd_string_8014a2b4
    .word .Lcd_string_8014a2ac
    .word .Lcd_string_8014a2a0
    .word .Lcd_string_8014a1f8
    .word .Lcd_string_8014a1f8
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x1
    .word 0x0
    .word 0x0
    .word 0x1
    .word 0x1
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x1
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x1
    .word 0x1
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x1
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x0
    .word 0x0
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x1
    .word 0x0
    .word 0x0
    .word 0x3
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x2
    .word 0x1
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x1
    .word 0x0
    .word 0x1
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x0
    .word 0x1f801800
    .word 0x1f801801
    .word 0x1f801802
    .word 0x1f801803
    .word 0x1f801020
    .word 0x1f801c00
    .word 0x0
    .word audio_cd_sdk_state + 0x364
.globl audio_cd_sync_callback
.set audio_cd_sync_callback, audio_cd_sdk_state + 0x88
.globl audio_cd_ready_callback
.set audio_cd_ready_callback, audio_cd_sdk_state + 0x8c
.globl audio_cd_debug_level
.set audio_cd_debug_level, audio_cd_sdk_state + 0x94
.globl audio_cd_status
.set audio_cd_status, audio_cd_sdk_state + 0x98
.globl audio_cd_response_bytes
.set audio_cd_response_bytes, audio_cd_sdk_state + 0xa4
.globl audio_cd_last_command_byte
.set audio_cd_last_command_byte, audio_cd_sdk_state + 0xa9
.globl audio_cd_completion
.set audio_cd_completion, audio_cd_sdk_state + 0x364
.balign 4
audio_cd_irq_handlers:
    .word .Lcd_80176018
    .word .Lcd_80175fcc
    .word .Lcd_80175ecc
    .word .Lcd_8017609c
    .word .Lcd_80176120
.Lcd_string_8014a19c:
    .asciz "none"
.Lcd_string_8014a1a4:
    .asciz "CdlReadS"
.Lcd_string_8014a1b0:
    .asciz "CdlSeekP"
.Lcd_string_8014a1bc:
    .asciz "CdlSeekL"
.Lcd_string_8014a1c8:
    .asciz "CdlGetTD"
.Lcd_string_8014a1d4:
    .asciz "CdlGetTN"
.Lcd_string_8014a1e0:
    .asciz "CdlGetlocP"
.Lcd_string_8014a1ec:
    .asciz "CdlGetlocL"
.Lcd_string_8014a1f8:
    .asciz "?"
.Lcd_string_8014a1fc:
    .asciz "CdlSetmode"
.Lcd_string_8014a208:
    .asciz "CdlSetfilter"
.Lcd_string_8014a218:
    .asciz "CdlDemute"
.Lcd_string_8014a224:
    .asciz "CdlMute"
.Lcd_string_8014a22c:
    .asciz "CdlReset"
.Lcd_string_8014a238:
    .asciz "CdlPause"
.Lcd_string_8014a244:
    .asciz "CdlStop"
.Lcd_string_8014a24c:
    .asciz "CdlStandby"
.Lcd_string_8014a258:
    .asciz "CdlReadN"
.Lcd_string_8014a264:
    .asciz "CdlBackword"
.Lcd_string_8014a270:
    .asciz "CdlForward"
.Lcd_string_8014a27c:
    .asciz "CdlPlay"
.Lcd_string_8014a284:
    .asciz "CdlSetloc"
.Lcd_string_8014a290:
    .asciz "CdlNop"
.Lcd_string_8014a298:
    .asciz "CdlSync"
.Lcd_string_8014a2a0:
    .asciz "DiskError"
.Lcd_string_8014a2ac:
    .asciz "DataEnd"
.Lcd_string_8014a2b4:
    .asciz "Acknowledge"
.Lcd_string_8014a2c0:
    .asciz "Complete"
.Lcd_string_8014a2cc:
    .asciz "DataReady"
.Lcd_string_8014a2d8:
    .asciz "NoIntr"
.Lcd_string_8014a2e0:
    .asciz "CD timeout: "
.Lcd_string_8014a2f0:
    .asciz "%s:(%s) Sync=%s, Ready=%s\n"
.Lcd_string_8014a30c:
    .asciz "DiskError: "
.Lcd_string_8014a318:
    .asciz "com=%s,code=(%02x:%02x)\n"
.Lcd_string_8014a334:
    .asciz "CDROM: unknown intr"
.Lcd_string_8014a348:
    .asciz "(%d)\n"
.Lcd_string_8014a368:
    .asciz "CD_sync"
.Lcd_string_8014a370:
    .asciz "CD_ready"
.Lcd_string_8014a37c:
    .asciz "%s...\n"
.Lcd_string_8014a384:
    .asciz "%s: no param\n"
.Lcd_string_8014a394:
    .asciz "CD_cw"
.Lcd_string_8014a3d0:
    .asciz "CD_init:"
.Lcd_string_8014a3dc:
    .asciz "addr=%08x\n"
.section .bss
.balign 4
# Original three responses, deadline, spin counter and diagnostic pointer.
.globl audio_cd_results
audio_cd_results:
    .space 36
