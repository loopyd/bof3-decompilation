# Auto-generated — do not edit
afn func_8014aac8 @ 0x8014aac8
afn func_8014aca0 @ 0x8014aca0
afn func_8014ad28 @ 0x8014ad28
afn func_8014ae08 @ 0x8014ae08
afn func_8014ae9c @ 0x8014ae9c
afn func_8014aee0 @ 0x8014aee0
afn func_8014b020 @ 0x8014b020
afn func_8014b0f0 @ 0x8014b0f0
afn func_8014b17c @ 0x8014b17c
afn func_80161f58 @ 0x80161f58
afn func_80161fdc @ 0x80161fdc
afn func_80162160 @ 0x80162160
afn func_80162178 @ 0x80162178
afn func_801621e8 @ 0x801621e8
afn func_80162230 @ 0x80162230
afn func_80162500 @ 0x80162500
afn func_801625e4 @ 0x801625e4
afn func_80162618 @ 0x80162618
afn func_80162698 @ 0x80162698
afn func_80162790 @ 0x80162790
afn func_80162898 @ 0x80162898
afn func_801629f0 @ 0x801629f0
afn func_80162a6c @ 0x80162a6c
afn func_80162b08 @ 0x80162b08
afn func_80162c14 @ 0x80162c14
afn func_80162cd8 @ 0x80162cd8
afn func_80162d00 @ 0x80162d00
afn func_80162d18 @ 0x80162d18
afn func_8014aa04 @ 0x8014aa04
CC "func_8014aa04" @ 0x8014aa04
afn func_8014afc0 @ 0x8014afc0
CC "func_8014afc0" @ 0x8014afc0
afn func_8014b33c @ 0x8014b33c
CC "func_8014b33c" @ 0x8014b33c
afn func_8014b6b4 @ 0x8014b6b4
CC "func_8014b6b4" @ 0x8014b6b4
afn func_8014b73c @ 0x8014b73c
CC "func_8014b73c" @ 0x8014b73c
afn func_8014b854 @ 0x8014b854
CC "func_8014b854" @ 0x8014b854
afn func_8014e22c @ 0x8014e22c
CC "func_8014e22c" @ 0x8014e22c
afn func_8014e6d0 @ 0x8014e6d0
CC "func_8014e6d0" @ 0x8014e6d0
afn func_8014ea80 @ 0x8014ea80
CC "func_8014ea80" @ 0x8014ea80
afn func_8015cebc @ 0x8015cebc
CC "func_8015cebc" @ 0x8015cebc
afn func_8015d044 @ 0x8015d044
CC "func_8015d044" @ 0x8015d044
afn func_80163010 @ 0x80163010
CC "func_80163010" @ 0x80163010
afn func_80174700 @ 0x80174700
CC "func_80174700" @ 0x80174700
afn func_8017b2d4 @ 0x8017b2d4
CC "func_8017b2d4" @ 0x8017b2d4
afn func_8017b8d4 @ 0x8017b8d4
CC "func_8017b8d4" @ 0x8017b8d4
afn func_8017ba40 @ 0x8017ba40
CC "func_8017ba40" @ 0x8017ba40
afn func_8017bc98 @ 0x8017bc98
CC "func_8017bc98" @ 0x8017bc98
afn func_8017e3d4 @ 0x8017e3d4
CC "func_8017e3d4" @ 0x8017e3d4

fs psyq
# LIBAPI
f psyq._bu_init 1 @ 0x8017dfa0
f psyq.Exec 1 @ 0x8017ecdc
f psyq.OpenEvent 1 @ 0x8017ed3c
f psyq.EnableEvent 1 @ 0x8017ed7c
f psyq.OpenTh 1 @ 0x8017ed9c
f psyq.CloseTh 1 @ 0x8017edac
f psyq.ChangeTh 1 @ 0x8017edbc
f psyq.ChangeClearPAD 1 @ 0x8017eebc
# LIBCARD
f psyq.InitCARD2 1 @ 0x8017e0e4
f psyq._patch_card 1 @ 0x8017e16c
f psyq._patch_card2 1 @ 0x8017e224
# LIBCD
f psyq.CdSync 1 @ 0x80175640
f psyq.CdReady 1 @ 0x80175660
f psyq.CdSyncCallback 1 @ 0x80175680
f psyq.CdReadyCallback 1 @ 0x80175698
f psyq.CdControl 1 @ 0x801756b0
f psyq.CdControlB 1 @ 0x80175914
f psyq.CdGetSector 1 @ 0x80175a78
f psyq.CdIntToPos 1 @ 0x80175adc
f psyq.CdPosToInt 1 @ 0x80175be0
f psyq.CdSearchFile 1 @ 0x80177348
# LIBETC
f psyq.PadStop 1 @ 0x801746e0
f psyq.VSync 1 @ 0x80174700
f psyq.StopCallback 1 @ 0x801749d8
# LIBGPU
f psyq.SetDefDrawEnv 1 @ 0x8017a514
f psyq.SetDefDispEnv 1 @ 0x8017a5e4
f psyq.AddPrims 1 @ 0x8017a88c
f psyq.CatPrim 1 @ 0x8017a8c8
f psyq.SetDispMask 1 @ 0x8017b330
f psyq.DrawSync 1 @ 0x8017b3cc
f psyq.ClearImage 1 @ 0x8017b560
f psyq.ClearOTag 1 @ 0x8017b81c
f psyq.ClearOTagR 1 @ 0x8017b8d4
f psyq.DrawOTag 1 @ 0x8017b9cc
f psyq.PutDrawEnv 1 @ 0x8017ba40
f psyq.PutDispEnv 1 @ 0x8017bc98

fs data
f data.DAT_800e4800 4 @ 0x800e4800
f data.DAT_80140000 4 @ 0x80140000
f data.DAT_80142cc0 4 @ 0x80142cc0
f data.DAT_80142cc4 4 @ 0x80142cc4
f data.DAT_80142ce0 4 @ 0x80142ce0
f data.DAT_80143b40 4 @ 0x80143b40
f data.DAT_80143b44 4 @ 0x80143b44
f data.DAT_80143b48 4 @ 0x80143b48
f data.DAT_80143d40 4 @ 0x80143d40
f data.DAT_80143d44 4 @ 0x80143d44
f data.DAT_80143d48 4 @ 0x80143d48
f data.DAT_80143db8 4 @ 0x80143db8
f data.DAT_80143e68 4 @ 0x80143e68
f data.DAT_80143e6c 4 @ 0x80143e6c
f data.DAT_80143e70 4 @ 0x80143e70
f data.DAT_80143e88 4 @ 0x80143e88
f data.DAT_80143e8c 4 @ 0x80143e8c
f data.DAT_80143ea0 4 @ 0x80143ea0
f data.DAT_80143ea4 4 @ 0x80143ea4
f data.DAT_80143ef8 4 @ 0x80143ef8
f data.DAT_80143efc 4 @ 0x80143efc
f data.DAT_80143f44 4 @ 0x80143f44
f data.DAT_8014598c 4 @ 0x8014598c
f data.DAT_80145990 4 @ 0x80145990
f data.DAT_801459d0 4 @ 0x801459d0
f data.DAT_801459f8 4 @ 0x801459f8
f data.DAT_801459fc 4 @ 0x801459fc
f data.DAT_80145aa4 4 @ 0x80145aa4
f data.DAT_80145ad4 4 @ 0x80145ad4
f data.DAT_80145e14 4 @ 0x80145e14
f data.DAT_80145e18 4 @ 0x80145e18
f data.DAT_80145e1c 4 @ 0x80145e1c
f data.DAT_80145e20 4 @ 0x80145e20
f data.DAT_80145e24 4 @ 0x80145e24
f data.DAT_80145e28 4 @ 0x80145e28
f data.DAT_80145e2c 4 @ 0x80145e2c
f data.DAT_80146450 4 @ 0x80146450
f data.DAT_80146454 4 @ 0x80146454
f data.DAT_80146458 4 @ 0x80146458
f data.DAT_8014645c 4 @ 0x8014645c
f data.DAT_80146460 4 @ 0x80146460
f data.DAT_80146464 4 @ 0x80146464
f data.DAT_80146468 4 @ 0x80146468
f data.DAT_8014646c 4 @ 0x8014646c
f data.DAT_80146478 4 @ 0x80146478
f data.DAT_80146480 4 @ 0x80146480
f data.DAT_80146481 4 @ 0x80146481
f data.DAT_80146482 4 @ 0x80146482
f data.DAT_80146483 4 @ 0x80146483
f data.DAT_80146484 4 @ 0x80146484
f data.DAT_80146485 4 @ 0x80146485
f data.DAT_80146486 4 @ 0x80146486
f data.DAT_80146488 4 @ 0x80146488
f data.DAT_80146489 4 @ 0x80146489
f data.DAT_8014648a 4 @ 0x8014648a
f data.DAT_8014648b 4 @ 0x8014648b
f data.DAT_8014648c 4 @ 0x8014648c
f data.DAT_80146490 4 @ 0x80146490
f data.DAT_80146492 4 @ 0x80146492
f data.DAT_80146494 4 @ 0x80146494
f data.DAT_80146498 4 @ 0x80146498
f data.DAT_801464a0 4 @ 0x801464a0
f data.DAT_801464b8 4 @ 0x801464b8
f data.DAT_80146518 4 @ 0x80146518
f data.DAT_80146678 4 @ 0x80146678
f data.DAT_8014667c 4 @ 0x8014667c
f data.DAT_80146778 4 @ 0x80146778
f data.DAT_8014677c 4 @ 0x8014677c
f data.DAT_80146780 4 @ 0x80146780
f data.DAT_80146788 4 @ 0x80146788
f data.DAT_8014678e 4 @ 0x8014678e
f data.DAT_80146808 4 @ 0x80146808
f data.DAT_8014681a 4 @ 0x8014681a
f data.DAT_80146840 4 @ 0x80146840
f data.DAT_80146844 4 @ 0x80146844
f data.DAT_80146848 4 @ 0x80146848
f data.DAT_80146854 4 @ 0x80146854
f data.DAT_80146858 4 @ 0x80146858
f data.DAT_8014685c 4 @ 0x8014685c
f data.DAT_80148fc0 4 @ 0x80148fc0
f data.DAT_80149990 4 @ 0x80149990
f data.DAT_80149c3c 4 @ 0x80149c3c
f data.DAT_8014b17c 4 @ 0x8014b17c
f data.DAT_8017f470 4 @ 0x8017f470
f data.DAT_8017f4b8 4 @ 0x8017f4b8
f data.DAT_80182444 4 @ 0x80182444
f data.DAT_80183224 4 @ 0x80183224
f data.DAT_80183248 4 @ 0x80183248
f data.DAT_8018b300 4 @ 0x8018b300
f data.DAT_8018b490 4 @ 0x8018b490
f data.DAT_8018b4a0 4 @ 0x8018b4a0
f data.DAT_8018b4a4 4 @ 0x8018b4a4
f data.DAT_8018b4a8 4 @ 0x8018b4a8
f data.DAT_8018b4ac 4 @ 0x8018b4ac

fs functions
