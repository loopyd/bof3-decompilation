# Symbols, signatures, and type recovery

## Source priority

1. Exact symbols from the analyzed build (`SYM`, `MAP`, ELF, CPE/debug data)
2. Exact symbolized object/library signature from the same SDK/compiler version
3. Exact byte or relocation-aware match in a sibling revision
4. Highly constrained structural match
5. Semantic inference from behavior

Every imported name: original spelling · normalized alias (if any) · source file/hash · source address space · match method · confidence · collision/ambiguity notes.

## PsyQ artifacts

Search/preserve: `.SYM` (address/name, sometimes source/line) · `.MAP` (linker map) · `.CPE` (dev executable/debug format) · `.OBJ`/`.LIB` · ELF/debug conversions · source path/assert strings embedded in release binaries. Community parsers: `psx_mnd_sym`, Ghidra PSX loader. Don't assume Rizin natively understands every PsyQ container; convert reviewed outputs into explicit Rizin flags/functions/types.

## Signature workflow

Controlled library corpus, per PsyQ release or third-party library:

1. retain archive hash + version
2. enumerate object members
3. preserve symbol names + relocations
4. generate signatures from symbolized code
5. test against known binaries
6. record false positives + functions too short to identify safely

Apply conservatively: exact long functions with distinctive constants = higher confidence · tiny wrappers/thunks ambiguous unless call context agrees · functions changed by link-time relaxation/assembler macros/compiler flags → structural evidence · cross-version SDK → version-tag every candidate. Rizin-native: FLIRT/`rz-sign` (check `F?`/`rz-sign -h`). Independent engines: `lab313ru/psx_psyq_signatures`, `ghidra_psx_ldr`.

## Regional / revision matching

Normalize address-independent data before comparing builds: mask relocated absolutes · account for different link bases · compare control-flow graph + constants · call neighborhoods · strings/data references. A debug/demo-build symbol names a retail function only when correspondence is proven; record the source revision in the name ledger.

## Type recovery

**Functions** — infer prototype from: caller definitions of a0–a3 · stack arguments · callee reads before overwrite · return-value consumers · signedness in comparisons/division/loads · pointer dereferences + field offsets · runtime values across scenarios · known library/API prototype. Represent uncertainty explicitly:

```text
int? func_80012340(Context *? a0, int32_t mode, void *buffer, uint32_t size, ...?)
```

Never force argument count from one caller.

**Structures** — start from an offset ledger; require repeated coherent accesses before defining a field. Separate: pointer field vs embedded object · scalar vs bitfield/flags · array base vs one field · signed vs unsigned narrow loads · volatile/MMIO vs normal memory · union/state-dependent interpretation.

**Enums/flags** — switch cases + bit tests are evidence for constrained values; name based on behavior only after replay scenarios exercise them.

## Rizin import format

CSV:

```text
name,address,kind,size,comment,source,confidence
```

or JSON objects with the same fields. Produces a `.rz` script with flags + optional function definitions/renames. Review the generated script before loading it.

Recommended `kind` values: `function,data,string,label,overlay_entry,callback,bios,psyq`.

Keep imported scripts in the case's `symbols/generated/`; never overwrite the source symbol file.
