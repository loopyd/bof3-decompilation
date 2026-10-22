.section .rodata.assets,"a",@progbits
.balign 64
.globl player_header
player_header: .incbin "bank.vh"
.balign 64
.globl player_body
player_body: .incbin "bank.vb"
.balign 4
.globl player_sequence
player_sequence: .incbin "song.sep"
.balign 4
.globl player_font
player_font: .incbin "font.bin"
.globl player_palette
player_palette: .incbin "palette.bin"
.globl player_logo
player_logo: .incbin "logo.bin"
.section .runtime,"awx",@progbits
.incbin "runtime.bin", 0, 0xd421c
.word __note_call
.incbin "runtime.bin", 0xd4220
