# SPDX-License-Identifier: MIT
# Owned US sound initialization and its explicit sequence/default state.
# Provenance: exe/slus_004_22, SHA-256
# 0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a.
# Preserve complete startup order and selective state writes.
.set noreorder
.set noat
.section .text
.balign 4
# exe/slus_004_22@0x8015CD00..0x8015CE70
.globl audio_sound_initialize
.ent audio_sound_initialize
audio_sound_initialize:
    addiu $sp, $sp, -0x18
    sw $ra, 0x10($sp)
    jal audio_library_reset
    nop
    lui $a0, %hi(audio_sound_sequence_records)
    addiu $a0, $a0, %lo(audio_sound_sequence_records)
    addiu $a1, $zero, 2
    jal audio_sequence_configure
    addiu $a2, $zero, 4
    jal audio_sequence_initialize_tick
    addiu $a0, $zero, 1
    jal audio_sequence_set_reverb_mode
    addiu $a0, $zero, 1
    jal audio_sequence_enable_reverb
    nop
    jal audio_sequence_set_playback_byte
    addu $a0, $zero, $zero
    jal audio_sequence_set_voice_count
    addiu $a0, $zero, 0x10
    addiu $v0, $zero, 1
    lui $at, %hi(audio_sound_channels + 1)
    sb $v0, %lo(audio_sound_channels + 1)($at)
    addiu $v0, $zero, 2
    lui $at, %hi(audio_sound_channels + 2)
    sb $v0, %lo(audio_sound_channels + 2)($at)
    addiu $v0, $zero, 3
    lui $at, %hi(audio_sound_channels + 3)
    sb $v0, %lo(audio_sound_channels + 3)($at)
    addiu $v0, $zero, 4
    lui $at, %hi(audio_sound_channels + 4)
    sb $v0, %lo(audio_sound_channels + 4)($at)
    addiu $v0, $zero, 5
    lui $at, %hi(audio_sound_channels + 5)
    sb $v0, %lo(audio_sound_channels + 5)($at)
    addiu $v0, $zero, 6
    lui $at, %hi(audio_sound_channels + 6)
    sb $v0, %lo(audio_sound_channels + 6)($at)
    addiu $v0, $zero, -1
    lui $at, %hi(audio_sound_channels)
    sb $zero, %lo(audio_sound_channels)($at)
    lui $at, %hi(audio_sound_bank_ids)
    sb $v0, %lo(audio_sound_bank_ids)($at)
    lui $at, %hi(audio_sound_bank_ids + 1)
    sb $v0, %lo(audio_sound_bank_ids + 1)($at)
    lui $at, %hi(audio_sound_bank_ids + 2)
    sb $v0, %lo(audio_sound_bank_ids + 2)($at)
    lui $at, %hi(audio_sound_bank_ids + 3)
    sb $v0, %lo(audio_sound_bank_ids + 3)($at)
    lui $at, %hi(audio_sound_bank_ids + 4)
    sb $v0, %lo(audio_sound_bank_ids + 4)($at)
    lui $at, %hi(audio_sound_bank_ids + 5)
    sb $v0, %lo(audio_sound_bank_ids + 5)($at)
    lui $at, %hi(audio_sound_bank_ids + 6)
    sb $v0, %lo(audio_sound_bank_ids + 6)($at)
    jal audio_sequence_install_handlers
    nop
    lui $a0, %hi(audio_sound_upper_voice_attributes)
    addiu $a0, $a0, %lo(audio_sound_upper_voice_attributes)
    ori $v0, $zero, 0xffff
    addiu $v1, $zero, 3
    lui $at, %hi(audio_sound_lower_voice_attributes)
    sw $v0, %lo(audio_sound_lower_voice_attributes)($at)
    addiu $v0, $zero, 0x7ff
    lui $at, %hi(audio_sound_gains)
    sh $v0, %lo(audio_sound_gains)($at)
    lui $at, %hi(audio_sound_gains + 2)
    sh $v0, %lo(audio_sound_gains + 2)($at)
    lui $at, %hi(audio_sound_lower_voice_attributes + 8)
    sh $v0, %lo(audio_sound_lower_voice_attributes + 8)($at)
    lui $at, %hi(audio_sound_lower_voice_attributes + 10)
    sh $v0, %lo(audio_sound_lower_voice_attributes + 10)($at)
    lui $v0, 0xff
    lui $at, %hi(audio_sound_lower_voice_attributes + 4)
    sw $v1, %lo(audio_sound_lower_voice_attributes + 4)($at)
    sw $v0, 0($a0)
    addiu $v0, $zero, 0x3fff
    lui $at, %hi(audio_sound_upper_voice_attributes + 4)
    sw $v1, %lo(audio_sound_upper_voice_attributes + 4)($at)
    lui $at, %hi(audio_sound_gains + 8)
    sh $v0, %lo(audio_sound_gains + 8)($at)
    lui $at, %hi(audio_sound_gains + 10)
    sh $v0, %lo(audio_sound_gains + 10)($at)
    lui $at, %hi(audio_sound_upper_voice_attributes + 8)
    sh $v0, %lo(audio_sound_upper_voice_attributes + 8)($at)
    lui $at, %hi(audio_sound_upper_voice_attributes + 10)
    sh $v0, %lo(audio_sound_upper_voice_attributes + 10)($at)
    jal audio_spu_set_voice_attributes
    nop
    lw $ra, 0x10($sp)
    addiu $sp, $sp, 0x18
    jr $ra
    nop
.end audio_sound_initialize

.section .bss.audio_sound_gains,"aw",@nobits

# exe/slus_004_22@0x80148A04, 12 bytes.
.balign 4
.globl audio_sound_gains
audio_sound_gains:
    .space 12
    .space 8 # Original gap before the loader pointer arrays.

.section .bss.audio_sound_sequences,"aw",@nobits

# exe/slus_004_22@0x80148A50, 1376 bytes.
.balign 4
.globl audio_sound_sequence_records
audio_sound_sequence_records:
    .space 1376

# exe/slus_004_22@0x80148FB0, 7 bytes.
.balign 4
.globl audio_sound_channels
audio_sound_channels:
    .space 7
    .space 9 # Original gap before the bank-id array.

# exe/slus_004_22@0x80148FC0, 7 bytes.
.balign 4
.globl audio_sound_bank_ids
audio_sound_bank_ids:
    .space 7

.section .bss

# exe/slus_004_22@0x8018B410, 12 bytes.
.balign 4
.globl audio_sound_lower_voice_attributes
audio_sound_lower_voice_attributes:
    .space 12

# exe/slus_004_22@0x8018B450, 12 bytes.
.balign 4
.globl audio_sound_upper_voice_attributes
audio_sound_upper_voice_attributes:
    .space 12
