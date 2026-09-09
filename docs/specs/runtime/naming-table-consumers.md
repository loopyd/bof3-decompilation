# Naming evidence: fixed-width table consumers

## Battle 15 reviewed capability set

The naming-evidence analyzer recognizes only these target-qualified Battle 15
rows and consumers. Each capability pins the target image digest, reviewed
ranges, original bytes, fixed globals, pointer starts, and consumer function;
there is no automatic sibling or cross-target enrollment.

| Data row | Consumer selector | Access site | Selector offset | Consumer range |
| --- | --- | ---: | ---: | --- |
| `D_80096994` | `emi/battle/battle/15@800AD26C` | `0x800AD278` | `+3` | `0x800AD26C..0x800AD2CC` |
| `D_800969A0` | `emi/battle/battle/15@800AD69C` | `0x800AD6A8` | `+3` | `0x800AD69C..0x800AD6FC` |
| `D_800969AC` | `emi/battle/battle/15@800AD9CC` | `0x800AD9D8` | `+3` | `0x800AD9CC..0x800ADA2C` |
| `D_800969B8` | `emi/battle/battle/15@800ADCC4` | `0x800ADCD0` | `+3` | `0x800ADCC4..0x800ADD24` |
| `D_800969F8` | `emi/battle/battle/15@800B01F0` | `0x800B0204` | `+3` | `0x800B01F0..0x800B0250` |
| `D_80096A08` | `emi/battle/battle/15@800B09CC` | `0x800B09E0` | `+3` | `0x800B09CC..0x800B0A2C` |
| `D_80096A34` | `emi/battle/battle/15@800B138C` | `0x800B13A0` | `+3` | `0x800B138C..0x800B13EC` |

For each row, the reviewed 12-byte data range contains three 32-bit function
pointers. The pinned native consumer copies those pointers to a contiguous
local table, scales an unsigned-byte selector by four, and dispatches through
`jalr` with a `nop` delay slot. This proves a fixed-width dispatch-table
consumer fact only; it does not establish a semantic symbol name.

Fresh collection must still fail closed while any unrelated access evidence is
open or unavailable. Positive capability facts cannot turn such a row into an
`exhausted` or `proposed` conclusion.
