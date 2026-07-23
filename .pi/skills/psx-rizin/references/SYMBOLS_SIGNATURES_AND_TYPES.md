# Symbols, signatures, and type recovery

## Source priority

1. Exact symbols from the analyzed build (`SYM`, `MAP`, ELF, CPE/debug data)
2. Exact symbolized object/library signature from the same SDK/compiler version
3. Exact byte or relocation-aware match in a sibling revision
4. Highly constrained structural match
5. Semantic inference from behavior

Every imported name needs:

- original spelling
- normalized alias, if any
- source file/hash
- source address space
- match method
- confidence
- collision/ambiguity notes

## PsyQ artifacts

Search for and preserve:

- `.SYM`: address/name and sometimes source/line information depending on producer
- `.MAP`: linker map with sections/symbols
- `.CPE`: development executable/debug format
- `.OBJ`/`.LIB`: PsyQ object/library containers
- ELF/debug conversions
- source path/assert strings embedded in release binaries

Useful community parsers/loaders include `psx_mnd_sym` and the Ghidra PSX loader. Do not assume Rizin natively understands every PsyQ container; convert reviewed outputs into explicit Rizin flags/functions/types.

## Signature workflow

### Build a controlled library corpus

For each PsyQ release or third-party library:

1. retain archive hash and version
2. enumerate object members
3. preserve symbol names and relocations
4. generate signatures from symbolized code
5. test against known binaries
6. record false positives and functions too short to identify safely

### Apply signatures conservatively

- exact long functions with distinctive constants: higher confidence
- tiny wrappers/thunks: ambiguous unless call context agrees
- functions changed by link-time relaxation, assembler macros, or compiler flags: use structural evidence
- cross-version SDK functions: version-tag every candidate

Use Rizin FLIRT support/`rz-sign` for Rizin-native workflows. Use `lab313ru/psx_psyq_signatures` and `ghidra_psx_ldr` as an independent signature engine.

## Regional and revision matching

Normalize address-independent instruction data before comparing builds:

- mask relocated absolute addresses
- account for different link bases
- compare control-flow graph and constants
- compare call neighborhoods
- compare strings/data references

A symbol in a debug/demo build can name a retail function only when the function correspondence is proven. Record the source revision in the name ledger.

## Type recovery

### Functions

Infer a prototype from:

- all caller definitions of `a0`–`a3`
- stack arguments
- callee reads before overwrite
- return-value consumers
- signedness in comparisons/division/loads
- pointer dereferences and field offsets
- runtime values across scenarios
- known library/API prototype

Represent uncertainty explicitly, for example:

```text
int? func_80012340(Context *? a0, int32_t mode, void *buffer, uint32_t size, ...?)
```

Do not force argument count from one caller.

### Structures

Start from an offset ledger. Require repeated coherent accesses before defining a field. Separate:

- pointer field versus embedded object
- scalar versus bitfield/flags
- array base versus one field
- signed versus unsigned narrow loads
- volatile/MMIO versus normal memory
- union/state-dependent interpretation

### Enums and flags

Switch cases and bit tests are evidence for constrained values. Name values based on behavior only after replay scenarios exercise them.

## Rizin import format

Reviewed-symbol import uses a CSV with:

```text
name,address,kind,size,comment,source,confidence
```

or JSON objects with the same fields. It produces a `.rz` script with flags and optional function definitions/renames. Review the generated script before loading it.

Recommended `kind` values:

```text
function,data,string,label,overlay_entry,callback,bios,psyq
```

Keep imported scripts in the case's `symbols/generated/` directory and never overwrite the source symbol file.
