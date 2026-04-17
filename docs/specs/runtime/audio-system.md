# Audio System Reverse Spec

This document tracks the PSYQ audio call patterns used by BOF3's EMI handlers.

It is intentionally separate from `emi-loader.md`:
- `emi-loader.md` describes the full EMI loading pipeline
- this file focuses on audio-specific PSYQ calls and state management

## Status

- Confidence: medium
- Basis:
  - rizin disassembly of SLUS audio handlers (types 6, 7, 8, 9, 10)
  - cross-reference with TOMB5 and PsyDoom LIBSPU implementations
  - EMI manifest analysis of DEMO.EMI and FIRST.EMI

Scope limitation:
- documents PSYQ call patterns from SLUS EMI handlers only
- does NOT disassemble internal libsnd/libspu implementation
- implementation strategy remains open; keep this file focused on the original runtime

## High-Level Model

BOF3 uses the PSYQ audio stack:

```
┌─────────────────────────────────────────┐
│ BOF3 Game Code (SLUS + Overlays)        │
├─────────────────────────────────────────┤
│ libsnd: SsVab*, SsSeq*, SsSep*, SsUt*   │  <- EMI handlers call this layer
├─────────────────────────────────────────┤
│ LIBSPU: Spu*, Spu2*                     │
├─────────────────────────────────────────┤
│ PSX SPU Hardware                        │
└─────────────────────────────────────────┘
```

Key concepts:
- **VAB** (Voice ABstract): VH header + VB body, loaded separately
- **SEQ**: Sequence file referencing VAB programs
- **Bank ID**: TOC `ram_ptr` field used as logical bank selector (0, 1, etc.)

## EMI Audio Type Handlers

The loader dispatches audio payloads through type-specific handlers at `0x80183248`:

| Type | Handler | Purpose |
| ---: | --- | --- |
| `6` | `0x80162790` | VAB header (VH) |
| `7` | `0x80162898` | VAB body (VB) |
| `8` | `0x801629f0` | Audio auxiliary buffer |
| `9` | `0x80162a6c` | Sequence-side or auxiliary payload (shared handler; unresolved) |
| `10` | `0x80162a6c` | Sequence (SEQ) |

### Type 6: VAB Header (VH)

@source: 0x80162790 FUN_80162790

Role:
- loads VAB header into SPU-accessible memory
- initializes the logical bank for subsequent VB/SEQ loads

Observed PSYQ calls:
- `fcn.8016debc` → `SsUtAllKeyOff(vab_id)` - silence existing voices
- `fcn.8016ad2c` → `SsVabOpenHead(header_ptr, vab_id)` - register header

Observed behavior:
- reads bank ID from TOC `ram_ptr`
- copies VH payload to bank-local buffer
- calls `SsVabOpenHead` to register with libsnd

### Type 7: VAB Body (VB)

@source: 0x80162898 FUN_80162898

Role:
- streams VAB body (sample data) to SPU memory
- uses sticky transfer mode for incremental uploads

Observed PSYQ calls:
- `fcn.801690b8` → `SsVabClose(vab_id)` - close prior bank if active
- `fcn.80173818` → `SsVabOpenHeadSticky(vab_id)` - begin sticky transfer
- `fcn.80174354` → `SsVabTransBodyPartly(data, size)` - chunked transfer

Observed behavior:
- reads bank ID from TOC `ram_ptr`
- closes any existing bank at that slot
- initiates sticky transfer mode
- streams VB data in chunks (typically 0x800 bytes per chunk)

### Type 8: Auxiliary Audio Payload

@source: 0x801629f0 FUN_801629f0

Role:
- copies a bank-local auxiliary payload into a second runtime buffer
- uses the same logical-bank remap pattern as the other audio handlers

Observed behavior:
- reads bank ID from TOC `ram_ptr`
- remaps the bank through a secondary runtime table
- reuses the copy path at `0x80162c14`
- exact payload format is still unresolved

### Type 9/10: Sequence-Side Copy Path

@source: 0x80162a6c FUN_80162a6c

Role:
- loads sequence-side payloads through a bank-selected runtime buffer
- type `10` is the current proven `SEQ` path
- type `9` shares the same handler but remains unresolved locally

Observed PSYQ calls:
- `fcn.80162c14` → sequence-side copy helper (internal)

Observed behavior:
- reads bank ID from TOC `ram_ptr`
- remaps the bank through a sequence-side table
- sets a per-bank flag word before copy
- current local type `10` samples begin with `SEQp`

Current interpretation:
- type `10` loads sequence data for the PsyQ music runtime
- type `9` is likely sequence-adjacent or auxiliary audio data, but no concrete shipped local sample is confirmed yet

### Transfer Helper

@source: 0x80163418 FUN_80163418

Role:
- manages chunked SPU transfers
- polls completion status

Observed PSYQ calls:
- `fcn.80174598` → `SsVabTransCompleted(mode)` - check transfer done
- `fcn.80174354` → `SsVabTransBodyPartly(data, size)` - continue transfer

## PSYQ Function Mappings

Resolved from SLUS symbol analysis:

| Address | PSYQ Name | Purpose |
| --- | --- | --- |
| `0x80174598` | `SsVabTransCompleted` | Check if VB transfer finished |
| `0x80174354` | `SsVabTransBodyPartly` | Transfer VB chunk to SPU |
| `0x80173818` | `SsVabOpenHeadSticky` | Open VAB for incremental transfer |
| `0x8016ad2c` | `SsVabOpenHead` | Register VH with libsnd |
| `0x801690b8` | `SsVabClose` | Close and free VAB |
| `0x8016b38c` | `SsSeqOpen` | Register and activate sequence |
| `0x8016debc` | `SsUtAllKeyOff` | Silence all voices for bank |

## SLUS Audio/MIDI Function Recovery (Consolidated)

Scope/provenance:
- historical analyzed undefined-metadata worklist export from an earlier inventory workflow
- canonical metadata inventory store: `processed/inventory/inventory.sqlite`
- deterministic undefined rows are metadata rows whose `type_spec` begins with `undefined`, optionally filtered by `kind` and `program`
- program: `/boot/SLUS_004.22`
- address coverage: `0x801695ac`..`0x801752f0`
- this section keeps integrated findings only; per-slice chronology belongs in ad hoc worklist exports under `tmp/`
- the historical global undefined-metadata helper export still reports `row_count: 3138` for `/boot/SLUS_004.22`
- the current canonical SQLite store may still be missing the equivalent `/boot/SLUS_004.22` rows needed for a fresh focused worklist or export regeneration
- therefore this section records recovered anchors from the analyzed SLUS audio/MIDI window, not the current canonical/actionable undefined-metadata queue for `/boot/SLUS_004.22`

### Confirmed anchors and clusters

| Address | Symbol/Role | Confidence | Notes |
| --- | --- | --- | --- |
| `0x80169c80` | `_SsContDataEntry` (`CC#6`) core handler | high | control-change processing anchor |
| `0x8016a110` | `_SsContDamper` delta-update handler | high | CC64 path |
| `0x8016a1f4` | NRPN handler (`CC_98_OBJ_B4`) | high | guarded function-pointer dispatch |
| `0x8016a73c` | `MIDICC_OBJ_1E0` indirect CC dispatch wrapper | high | specialized CC set dispatcher |
| `0x8016a770` | `MIDICC_OBJ_214` fallback CC helper | high | non-specialized CC fallback |
| `0x8016a77c` | `MIDICC_OBJ_220` `CC0` helper | high | bank-select/control-specific path |
| `0x8016a788` | `MIDICC_OBJ_22C` post-dispatch tail hook | medium | likely epilogue/tail hook |
| `0x8016baa0` | `_SsSndSetReplayMode` | high | replay/state-flag mutator |
| `0x8016bdc8` | `S_SCA_OBJ_9C` (`SpuSetCommonAttr` mode helper) | high | mode/jump-table branch helper |
| `0x8016bdf8` | `S_SCA_OBJ_CC` (`SpuSetCommonAttr` commit helper) | high | shared commit path |
| `0x8016c7bc` | `_SsSndCrescendo` | high | envelope increase path |
| `0x8016ca9c` | `_SsSndDecrescendo` | high | envelope decrease path |
| `0x8016cf1c` | `_SsSeqGetEof` | high | EOF/loop-next transition control |
| `0x8016dc0c` | `_SsSndTempo` | high | tempo-ramp/update path |
| `0x8016ddec` | `_SsSndSetVolData` | high | envelope step-data initialization |
| `0x8016fc90` | `SeAutoPan` | high | auto-pan setup chain anchor |
| `0x80170184` | `SeAutoVol` | high | auto-volume setup chain anchor |
| `0x80170d68` | `_SsVmInit` core reset path (`VM_INIT_OBJ_E4`) | high | voice manager initialization |
| `0x80171154` | `_SsVmKeyOn` orchestration anchor | high | key-on staging/commit path |
| `0x80171684` | `_SsVmKeyOff` anchor | high | match-off scan path |
| `0x80172164` | `_SsVmKeyOffNow` anchor | high | immediate state-clear path |
| `0x80172854` | `_SsVmPBVoice` pitch-bend helper | high | note-to-pitch conversion + voice updates |
| `0x80173da0` | `SsVabOpenHeadWithMode` helper cluster anchor | high | VAB open path with SPU alloc/free helpers |

### Deterministic continuation stabilization (SLUS control-path helpers)

- Recent deterministic undefined-worklist coverage stabilized conservative signatures/roles for `_SsSndPlay`, `_SsSndSetReplayMode`, `SEPINIT_OBJ_*` continuation tails, and `S_SCA_OBJ_*` common-attribute variant/continuation tails within `SLUS_004.22` control paths.
- Confidence split: high for anchor/control-path stabilization; medium-high for helper-role labels in continuation tails.
- Scope guard: these are SLUS control-path helper findings; do not project them to overlay runtime semantics without explicit linkage.

Additional stabilized families from later closure coverage:
- VM/reverb/automation continuation families are now signature-stable at conservative return-type level in SLUS control paths: `S_SRMP_OBJ_*`, `S_CRWA_OBJ_*`, `VM_AUTOP_OBJ_*`, `VM_AUTOV_OBJ_*`, `VM_NOWON_OBJ_*`, plus `UT_KEYV_OBJ_*` tails.
- system-facing wrappers are signature-stable at high confidence for role and return class: `_IsVSync`, `InitPAD2`, `StartPAD2`, `EnablePAD`, `GetIntrMask`, `restartIntr`, `trapIntrVSync`, `setIntrVSync`, `trapIntrDMA`, `setIntrDMA`.

Control-change dispatch classes (`_SsSetControlChange`):
- `CC0` -> `MIDICC_OBJ_220` (direct helper)
- fallback/non-specialized CCs -> `MIDICC_OBJ_214` (direct helper)
- specialized set `{6,7,10,11,64,91,98,99,100,101,121}` -> `MIDICC_OBJ_1E0` or direct `jalr`

NRPN callback table (`UNK_80190388`) status:
- consumer-side indexing is confirmed in `_SsContNrpn1`/`CC_98_OBJ_B4`: `base + row*0x40 + col*4`
- `_SsInit` zero-initializes the table as `0x20 x 0x40` bytes (`32` rows x `16` dword entries)
- dispatch path applies state/non-null guards before `jalr`
- `SEPINIT_OBJ_154` (`0x8016b60c`) is ruled out as a non-zero writer
- no definitive SLUS writer that populates non-zero `UNK_80190388` entries is proven yet
- `DAT_8018db70..DAT_8018db94` is a separate callback-vector path (not this NRPN table)
- overlay table-dispatch analogs exist (for example `ETC/GAME/0.bin`), but runtime linkage to this SLUS table remains unproven

Boundary hygiene note:
- multiple tiny symbols in these regions still look like internal labels/tails and should not be promoted without explicit boundary proof: `0x801695ac`..`0x8016ab84`, `0x8016b490`..`0x8016bc88`, `0x8016be80`..`0x8016ca9c`, `0x8016cc68`..`0x8016ddec`, `0x8016e574`..`0x8016f678`, `0x8016fc90`..`0x80170d68`, `0x80171154`..`0x8017267c`, `0x80172604`..`0x801741b8`

## Global State

Audio-related global addresses observed in handlers:

| Address | Type | Description |
| --- | --- | --- |
| `0x80146458` | `void*` | Current VAB data pointer |
| `0x8014646c` | `int` | Active VAB/SEQ counter |
| `0x80146482` | `u8` | Current bank ID (transfer) |
| `0x80146483` | `u8` | Current bank ID (sticky) |
| `0x801464a0` | `u8[24]` | Queue status array |

Bank state table (per-bank, indexed by `bank_id * 12`):

| Offset | Type | Description |
| --- | --- | --- |
| `+0x6780` | `void*` | VAB data pointer |
| `+0x6788` | `void*` | Sequence data pointer |
| `+0x678c` | `u16` | Transfer chunk size |
| `+0x678e` | `u16` | Sequence state/flags |

## DEMO.EMI Audio Assets

Title screen music assets from `processed/emi_raw/BIN/ETC/DEMO/emi.json`:

| Entry | Type | Name | Size | Bank (`ram_ptr`) |
| ---: | --- | --- | --- | --- |
| `0` | `6` | `0.vh` | 7712 | 0 |
| `1` | `10` | `1.seq` | 2607 | 0 |
| `2` | `7` | `2.vb` | 419312 | 0 |
| `8` | `6` | `8.vh` | 3616 | 1 |
| `9` | `8` | `9.t08` | 44 | 1 |
| `10` | `7` | `10.vb` | 64560 | 1 |

Load order observed:
1. Entry 0: VH → bank 0
2. Entry 2: VB → bank 0 (streamed)
3. Entry 1: SEQ → bank 0 (uses bank 0 instruments)
4. Entry 8: VH → bank 1 (secondary bank)
5. Entry 10: VB → bank 1
6. Entry 9: Aux → bank 1

## External Leads

Useful local references:

| Source | Path | Use |
| --- | --- | --- |
| **vgmtrans** | `third_party/references/vgmtrans/src/main/formats/PS1/Vab.h` | `VAB` structure definitions |
| **vgmtrans** | `third_party/references/vgmtrans/src/main/formats/PS1/PS1Seq.cpp` | `SEQ` parsing reference |
| **psyq_sdk** | `third_party/references/psyq_sdk/psyq_4.5/include/libsnd.h` | `libsnd` API surface |

Non-local leads worth revisiting when implementation work resumes:

- TOMB5 `LIBSPU` code
- PsyDoom `LIBSPU` and SPU mixer code

Use all external projects as leads only, not proof of BOF3 behavior.

## Open Questions

- What exact payload format does type `8` carry?
- Does any shipped local `EMI` archive use type `9`, and if so where?
- What subset of `SEQ` events does BOF3 actually use?
- Are there additional PSYQ audio functions called from overlays (not SLUS)?
- What is the exact SPU memory layout BOF3 expects?
- How does BOF3 handle audio during scene transitions?
- Which runtime writer(s) populate non-zero `UNK_80190388` entries, if any, during active SEP/NRPN playback?
- Resolve `S_SVA` split/tail ambiguity around `0x801695ac`..`0x80169838` before promoting additional boundaries.
- Refine `_SsGetMetaEvent` timing-field semantics around `MIDIMETA_OBJ_1A8`.
