# SPDX-License-Identifier: MIT
# Owned entry vectors for the guest services already present in this image.
# The dispatch instruction order follows the US SCPH-5501 RAM vectors and
# dispatchers at 0xA0..0xC0 and 0x5C4..0x600. The relocated A0 table needs a
# full address load. BIOS SHA-256:
# 11052b6499e466bbf0a709b1f9cb6834a9418e66680387912451e971cf8a1fef.
.set noreorder
.set noat
.section .text
.balign 4

.globl audio_a0_dispatch
.ent audio_a0_dispatch
audio_a0_dispatch:
    lui $t0, %hi(audio_a0_services)
    addiu $t0, $t0, %lo(audio_a0_services)
    sll $t1, $t1, 2
    add $t0, $t0, $t1
    lw $t0, 0($t0)
    nop
    jr $t0
    nop
.end audio_a0_dispatch

.globl audio_b0_dispatch
.ent audio_b0_dispatch
audio_b0_dispatch:
    lui $t0, %hi(audio_b0_services)
    addiu $t0, $t0, %lo(audio_b0_services)
    sll $t1, $t1, 2
    add $t0, $t0, $t1
    lw $t0, 0($t0)
    nop
    jr $t0
    nop
.end audio_b0_dispatch

.section .rodata.audio_kernel_vectors,"a"
.balign 4
.globl audio_entry_vectors
audio_entry_vectors:
    lui $t0, %hi(audio_a0_dispatch)
    addiu $t0, $t0, %lo(audio_a0_dispatch)
    jr $t0
    nop
    lui $t0, %hi(audio_b0_dispatch)
    addiu $t0, $t0, %lo(audio_b0_dispatch)
    jr $t0
    nop
.globl audio_entry_vectors_end
audio_entry_vectors_end:

# Admission at A0/B0 is limited to these complete guest bodies. Other services
# still enter the explicit functional kernel model before fetching a vector;
# zero entries are unreachable and do not substitute successful service calls.
.section .data
.balign 4
.globl audio_a0_services
audio_a0_services:
    .space 0x72 * 4
    .word audio_cd_remove
.globl audio_b0_services
audio_b0_services:
    .space 7 * 4
    .word audio_event_deliver
    .space (0x17 - 8) * 4
    .word audio_exception_return
    .word audio_exception_reset
