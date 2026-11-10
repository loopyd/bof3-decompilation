# SPDX-License-Identifier: MIT
# DeliverEvent with the original guest frame, live record reads and JALR ABI.
# Startup binds the scanner, mode, callback and final return boundaries. The
# CPU loop validates ownership without changing guest registers or delay slots.
.set noreorder
.set noat
.section .text
.balign 4
.globl audio_event_deliver, audio_event_loop, audio_event_mode
.globl audio_event_callback, audio_event_callback_return
.globl audio_event_return, audio_event_return_delay
.ent audio_event_deliver
audio_event_deliver:
    lui $t6, 0xa000
    lw $t6, 0x124($t6)
    addiu $at, $zero, 28
    divu $zero, $t6, $at
    addiu $sp, $sp, -64
    lui $v1, 0xa000
    lw $v1, 0x120($v1)
    sw $s6, 32($sp)
    sw $s4, 24($sp)
    sw $s5, 28($sp)
    sw $s0, 20($sp)
    addu $s5, $a0, $zero
    addu $s6, $a1, $zero
    sw $ra, 36($sp)
    addu $s0, $v1, $zero
    mflo $t7
    sll $t8, $t7, 3
    subu $t8, $t8, $t7
    sll $t8, $t8, 2
    addu $v0, $v1, $t8
    sltu $at, $v1, $v0
    beq $at, $zero, .Lepilogue
    addu $s4, $v0, $zero
    sw $s1, 48($sp)
    sw $s2, 44($sp)
    sw $s3, 40($sp)
    addiu $s3, $zero, 0x1000
    addiu $s2, $zero, 0x4000
    addiu $s1, $zero, 0x2000
audio_event_loop:
    lw $t9, 4($s0)
    nop
    bne $s1, $t9, audio_event_callback_return
    nop
    lw $t0, 0($s0)
    nop
    bne $s5, $t0, audio_event_callback_return
    nop
    lw $t1, 8($s0)
    nop
    bne $s6, $t1, audio_event_callback_return
    nop
    lw $v0, 12($s0)
    nop
audio_event_mode:
    bne $s1, $v0, .Lcallback_mode
    nop
    beq $zero, $zero, audio_event_callback_return
    sw $s2, 4($s0)
.Lcallback_mode:
    bne $s3, $v0, audio_event_callback_return
    nop
    lw $v0, 16($s0)
    nop
    beq $v0, $zero, audio_event_callback_return
    nop
audio_event_callback:
    jalr $v0
    nop
audio_event_callback_return:
    addiu $s0, $s0, 28
    sltu $at, $s0, $s4
    bne $at, $zero, audio_event_loop
    nop
    lw $s1, 48($sp)
    lw $s2, 44($sp)
    lw $s3, 40($sp)
    nop
.Lepilogue:
    lw $ra, 36($sp)
    lw $s0, 20($sp)
    lw $s4, 24($sp)
    lw $s5, 28($sp)
    lw $s6, 32($sp)
audio_event_return:
    jr $ra
audio_event_return_delay:
    addiu $sp, $sp, 64
.end audio_event_deliver

# WaitEvent retains the selected record in v1 throughout its busy loop. IRQ
# capture and return preserve that guest register without resolving EvCB again.
.balign 4
.globl audio_event_wait
.ent audio_event_wait
audio_event_wait:
    andi $t0, $a0, 0xffff
    sll $t1, $t0, 3
    subu $t1, $t1, $t0
    sll $t1, $t1, 2
    lui $v1, 0xa000
    lw $v1, 0x120($v1)
    nop
    addu $v1, $v1, $t1
    lw $t0, 4($v1)
    addiu $t1, $zero, 0x4000
    beq $t0, $t1, .Lwait_ready
    addiu $v0, $zero, 0
    addiu $t1, $zero, 0x2000
    bne $t0, $t1, .Lwait_return
    nop
.Lwait_busy:
    lw $t0, 4($v1)
    addiu $t1, $zero, 0x4000
    bne $t0, $t1, .Lwait_busy
    nop
.Lwait_ready:
    addiu $t0, $zero, 0x2000
    sw $t0, 4($v1)
    addiu $v0, $zero, 1
.Lwait_return:
    jr $ra
    nop
.end audio_event_wait

# SCPH-5501 event controls, BIOS SHA-256
# 11052b6499e466bbf0a709b1f9cb6834a9418e66680387912451e971cf8a1fef.
# ROM offsets 0x11800..0x11944 and 0x119C8..0x11A88 copy to RAM
# 0x1D00..0x1E44 and 0x1EC8..0x1F88. Only OpenEvent's JAL relocates.
# The separate UnDeliverEvent body is ROM 0x1175C..0x11800,
# RAM 0x1C5C..0x1D00. Entry guards bound descriptors, handles and frames.
.balign 4
.globl audio_event_free
.ent audio_event_free
audio_event_free:
    lui $t7, 0xa000
    lw $t7, 0x124($t7)
    addiu $at, $zero, 28
    divu $zero, $t7, $at
    lui $t6, 0xa000
    lw $t6, 0x120($t6)
    nop
    addu $v1, $t6, $zero
    mflo $t8
    sll $t9, $t8, 3
    subu $t9, $t9, $t8
    sll $t9, $t9, 2
    addu $v0, $t6, $t9
    sltu $at, $t6, $v0
    beq $at, $zero, .Lfree_full
    nop
.Lfree_scan:
    lw $t0, 4($v1)
    nop
    bne $t0, $zero, .Lfree_next
    nop
    lui $t1, 0xa000
    lw $t1, 0x120($t1)
    addiu $at, $zero, 28
    subu $v0, $v1, $t1
    div $zero, $v0, $at
    mflo $v0
    jr $ra
    nop
.Lfree_next:
    addiu $v1, $v1, 28
    sltu $at, $v1, $v0
    bne $at, $zero, .Lfree_scan
    nop
.Lfree_full:
    addiu $v0, $zero, -1
    jr $ra
    nop
.end audio_event_free

.globl audio_event_open
.ent audio_event_open
audio_event_open:
    addiu $sp, $sp, -48
    sw $ra, 36($sp)
    lui $v1, 0xa000
    sw $a0, 48($sp)
    lw $v1, 0x120($v1)
    sw $a1, 52($sp)
    sw $a2, 56($sp)
    sw $a3, 60($sp)
    jal audio_event_free
    sw $v1, 44($sp)
    lw $v1, 44($sp)
    addiu $at, $zero, -1
    bne $v0, $at, .Lopen_slot
    addu $a0, $v0, $zero
    beq $zero, $zero, .Lopen_return
    addiu $v0, $zero, -1
.Lopen_slot:
    sll $t6, $a0, 3
    subu $t6, $t6, $a0
    lw $t7, 48($sp)
    sll $t6, $t6, 2
    addu $v1, $v1, $t6
    sw $t7, 0($v1)
    lw $t8, 52($sp)
    addiu $t1, $zero, 0x1000
    sw $t8, 8($v1)
    lw $t9, 56($sp)
    lui $at, 0xf100
    sw $t9, 12($v1)
    lw $t0, 60($sp)
    sw $t1, 4($v1)
    or $v0, $a0, $at
    sw $t0, 16($v1)
.Lopen_return:
    lw $ra, 36($sp)
    addiu $sp, $sp, 48
    jr $ra
    nop
.end audio_event_open

.globl audio_event_close
.ent audio_event_close
audio_event_close:
    andi $a0, $a0, 0xffff
    sll $t7, $a0, 3
    lui $t6, 0xa000
    lw $t6, 0x120($t6)
    subu $t7, $t7, $a0
    sll $t7, $t7, 2
    addu $t8, $t6, $t7
    sw $zero, 4($t8)
    jr $ra
    addiu $v0, $zero, 1
.end audio_event_close

.globl audio_event_test
.ent audio_event_test
audio_event_test:
    andi $a0, $a0, 0xffff
    sll $t7, $a0, 3
    lui $t6, 0xa000
    lw $t6, 0x120($t6)
    subu $t7, $t7, $a0
    sll $t7, $t7, 2
    addu $v0, $t6, $t7
    lw $t8, 4($v0)
    addiu $at, $zero, 0x4000
    bne $t8, $at, .Ltest_not_ready
    addu $v1, $v0, $zero
    addiu $t9, $zero, 0x2000
    sw $t9, 4($v1)
    jr $ra
    addiu $v0, $zero, 1
.Ltest_not_ready:
    addu $v0, $zero, $zero
    jr $ra
    nop
.end audio_event_test

.globl audio_event_enable
.ent audio_event_enable
audio_event_enable:
    andi $a0, $a0, 0xffff
    sll $t7, $a0, 3
    lui $t6, 0xa000
    lw $t6, 0x120($t6)
    subu $t7, $t7, $a0
    sll $t7, $t7, 2
    addu $v1, $t6, $t7
    lw $t8, 4($v1)
    addu $v0, $v1, $zero
    beq $t8, $zero, .Lenable_return
    nop
    addiu $t9, $zero, 0x2000
    sw $t9, 4($v0)
.Lenable_return:
    jr $ra
    addiu $v0, $zero, 1
.end audio_event_enable

.globl audio_event_disable
.ent audio_event_disable
audio_event_disable:
    andi $a0, $a0, 0xffff
    sll $t7, $a0, 3
    lui $t6, 0xa000
    lw $t6, 0x120($t6)
    subu $t7, $t7, $a0
    sll $t7, $t7, 2
    addu $v1, $t6, $t7
    lw $t8, 4($v1)
    addu $v0, $v1, $zero
    beq $t8, $zero, .Ldisable_return
    nop
    addiu $t9, $zero, 0x1000
    sw $t9, 4($v0)
.Ldisable_return:
    jr $ra
    addiu $v0, $zero, 1
.end audio_event_disable

.globl audio_event_undeliver
.ent audio_event_undeliver
audio_event_undeliver:
    lui $t6, 0xa000
    lw $t6, 0x124($t6)
    addiu $at, $zero, 28
    divu $zero, $t6, $at
    lui $v1, 0xa000
    lw $v1, 0x120($v1)
    nop
    addu $v0, $v1, $zero
    mflo $t7
    sll $t8, $t7, 3
    subu $t8, $t8, $t7
    sll $t8, $t8, 2
    addu $a3, $v1, $t8
    sltu $at, $v1, $a3
    beq $at, $zero, .Lundeliver_return
    addu $a2, $a3, $zero
    addiu $v1, $zero, 0x4000
    addiu $a3, $zero, 0x2000
.Lundeliver_scan:
    lw $t9, 4($v0)
    nop
    bne $v1, $t9, .Lundeliver_next
    nop
    lw $t0, 0($v0)
    nop
    bne $a0, $t0, .Lundeliver_next
    nop
    lw $t1, 8($v0)
    nop
    bne $a1, $t1, .Lundeliver_next
    nop
    lw $t2, 12($v0)
    nop
    bne $a3, $t2, .Lundeliver_next
    nop
    sw $a3, 4($v0)
.Lundeliver_next:
    addiu $v0, $v0, 28
    sltu $at, $v0, $a2
    bne $at, $zero, .Lundeliver_scan
    nop
.Lundeliver_return:
    jr $ra
    nop
.end audio_event_undeliver
