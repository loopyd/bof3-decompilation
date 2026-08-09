.set noreorder

.section .text.startup, "ax", @progbits

.globl _start
.globl bof3_boot_main
.ent _start
_start:
	lui	$sp, 0x8020
	ori	$sp, $sp, 0xff00

	lui	$t0, %hi(__bss_start)
	addiu	$t0, $t0, %lo(__bss_start)
	lui	$t1, %hi(__bss_end)
	addiu	$t1, $t1, %lo(__bss_end)

1:
	sltu	$t2, $t0, $t1
	beqz	$t2, 2f
	nop
	sw	$zero, 0($t0)
	addiu	$t0, $t0, 4
	b	1b
	nop

2:
	jal	bof3_boot_main
	nop

3:
	b	3b
	nop
.end _start

.ent bof3_boot_main
bof3_boot_main:
	addiu	$sp, $sp, -16
	sw	$ra, 12($sp)

	jal	runLogoExe
	nop
	move	$t0, $v0

	jal	slot_table_logo_str
	nop
	move	$t1, $v0

	li	$a0, 0x262
	jal	slot_table_find
	nop
	move	$t2, $v0

	move	$t3, $zero

	beqz	$t0, 4f
	nop
	ori	$t3, $t3, 0x1

4:
	beqz	$t1, 5f
	nop
	ori	$t3, $t3, 0x2

5:
	beqz	$t2, 6f
	nop
	ori	$t3, $t3, 0x4

6:
	lui	$t4, %hi(g_bof3_boot_probe)
	sw	$t3, %lo(g_bof3_boot_probe)($t4)

7:
	lw	$t5, %lo(g_bof3_boot_probe)($t4)
	beqz	$t5, 8f
	nop
	b	7b
	nop

8:
	lw	$ra, 12($sp)
	addiu	$sp, $sp, 16
	jr	$ra
	nop
.end bof3_boot_main

.section .bss, "aw", @nobits
.balign 4
g_bof3_boot_probe:
	.space 4
