# Reviewed SLUS_004.22 EMI loader functions. Keep canonical address names so
# analyzer evidence remains traceable to Splat, source, and matching output.
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

# Semantic aliases are additional flags, never replacements for address-based
# function names. These aliases are reviewed in the owning symbol layer.
fs semantic
f semantic.emi_loader_initialize 1 @ 0x80161f58
f semantic.emi_stream_init_slot 1 @ 0x80161fdc
f semantic.emi_loader_slot_lba 1 @ 0x80162160
f semantic.emi_cd_sync_callback 1 @ 0x801621e8
f semantic.emi_cd_ready_callback 1 @ 0x80162230
f semantic.emi_loader_is_ready 1 @ 0x80162d00

# Official SDK names are target-local flags at reviewed SLUS bindings. Direct
# call xrefs remain analyzer-derived from the original JAL instruction words.
fs psyq
f psyq.VSync 1 @ 0x80174700
f psyq.CdSync 1 @ 0x80175640
f psyq.CdReady 1 @ 0x80175660
f psyq.CdSyncCallback 1 @ 0x80175680
f psyq.CdReadyCallback 1 @ 0x80175698
f psyq.CdGetSector 1 @ 0x80175a78
f psyq.CdIntToPos 1 @ 0x80175adc
f psyq.CdPosToInt 1 @ 0x80175be0
fs functions
