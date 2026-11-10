.section .rodata.assets,"a",@progbits
.include "banks.s"
.balign 4
.globl player_font
player_font: .incbin "font.bin"
.globl player_palette
player_palette: .incbin "palette.bin"
.globl player_logo
player_logo: .incbin "logo.bin"
.section .runtime,"awx",@progbits
.include "runtime.s"
