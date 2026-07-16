# EMI-EX v1 parity checklist

Authority: the imported v1 implementation under `third_party/deprecated/emi-ex/`, not its
README alone. A checked item needs a Rust regression test and, for archive
bytes, a v1-produced golden fixture or direct v1 comparison.

## Archive library

- [x] Parse `count`, archive version, `MATH_TBL`, and 16-byte TOC records.
- [x] Derive payload offsets from `0x800`-aligned sizes.
- [x] Preserve `size`, `ram_ptr`, `first4`, and numeric type metadata.
- [x] Extract all entries as `<index>.<extension>`.
- [x] Extract one entry and reject an out-of-range index.
- [ ] Accept the v1 packer's stated range of 1 through 255 explicit entries.
  The current Rust writer additionally rejects a TOC crossing `0x800`; decide
  whether to preserve that safer restriction or reproduce v1 before claiming
  CLI parity.
- [x] Preserve archive version and entry `ram_ptr` when packing from a manifest.
- [x] Produce byte-identical archives for the existing golden fixture.
- [ ] Expose a full-entry read equivalent to v1 `Emi::read`.
- [x] Test empty payloads and payload sizes exactly on and across `0x800`
  boundaries.
- [ ] Test rejection of a TOC larger than the first sector, truncated headers,
  truncated TOC records, overflowing ranges, and truncated payloads.

## Types and folder enumeration

- [x] Implement case-insensitive v1 type guessing: `.img`/`.tim` -> 3,
  `.vh` -> 6, `.vb` -> 7, `.seq`/`.mid` -> 10, `.bin`/`.dat` -> 0, unknown
  extension -> Unknown. A path with no extension resolves to Binary.
- [x] Enumerate only immediate regular files; do not recurse.
- [x] Apply `*` and `?` include/exclude globs to the filename, case-sensitively.
- [x] Sort selected paths lexicographically by their full path.
- [x] Use the requested default type only when guessing returns Unknown.
- [x] During CLI folder packing without `-J`, exclude `emi.json` even if the
  caller did not supply that exclude pattern.
- [ ] Keep `emi.json` when using the library folder API directly; this differs
  intentionally from the CLI behavior.

## Manifest JSON

- [x] Emit version 1 with `archive_version` and entries containing `index`,
  `name`, `type`, `size`, `ram_ptr`, and `first4`.
- [x] Match v1's pretty-printed key order and no-final-newline representation.
- [x] Accept a missing `archive_version` as zero.
- [x] Do not require top-level `version`; v1 only requires an `entries` array.
- [x] Skip non-object entries and entries whose `name` is absent or empty.
- [x] Default absent entry fields to type/size/ram_ptr/first4 zero.
- [x] Reject invalid JSON, a missing/non-array `entries`, or a result with no
  named entries.
- [x] When packing a folder with a manifest, resolve every entry name relative
  to the input folder. The manifest file may live elsewhere.
- [x] Preserve manifest order and use its type and `ram_ptr`; `size` and
  `first4` are descriptive and are recomputed from the named files by v1.

## Extract CLI

- [x] Permit omitted `extract` as the default command.
- [x] Support `-e`, `--extensions`, and `--typed-extensions`.
- [x] Support both a positional index and `-n`/`--index`.
- [x] Default extract-all output to `<archive parent>/<archive stem>`.
- [x] For a single entry, treat an empty output or existing directory as a
  directory; otherwise use `-o` as the exact output filename. Do not infer this
  from whether the path has an extension.
- [x] Support `-d`/`--dry-run` without creating output files or directories.
- [x] Support `-q`/`--quiet` for console progress.
- [ ] Support `-C`/`--no-color` and `-L`/`--log-file` if CLI/log parity remains
  part of the canonical replacement contract.
- [x] Support `--print-manifest` on extract-all only.
- [x] Support optional `-J`/`--manifest-json [path]`: no path writes
  `<output>/emi.json`; an existing directory path appends `emi.json`; an
  explicit file path is used exactly.
- [x] Reject all manifest-output forms when extracting one index.

## Pack CLI

- [x] Require `-o`/`--output`.
- [x] Accept one positional folder, or one or more `-i`/`--input` groups, but
  never mix them.
- [x] A `-t`/`--type` applies as the fallback type to later files in the next
  input group; recognized filename extensions override it.
- [x] Folder mode without `-J` uses sorted enumeration, type guessing, and
  repeated `-I`/`--include` and `-X`/`--exclude` patterns.
- [x] Folder mode with optional `-J [path]` uses `<folder>/emi.json` by default,
  requires the requested manifest to exist, preserves its order, and ignores
  include/exclude filters.
- [x] Default to keeping source files; `-K`/`--no-keep` deletes only successfully
  packed inputs, while `-k`/`--keep-original` restores the default explicitly.
- [x] Dry-run performs validation/planning but writes and deletes nothing.

## Canonical replacement gate

- [ ] Rust tests cover every checked contract above.
- [ ] Extract-all and typed extract-all match v1 for every available shipped EMI.
- [ ] Single-entry output paths and failures match v1 representative cases.
- [ ] Manifest output parses to the same JSON values and matches v1 bytes where
  byte identity is part of the repository workflow.
- [ ] Folder, explicit-input, and manifest-driven packs match v1 bytes for
  representative types, unknown extensions, empty entries, and alignment edges.
- [ ] Extract -> manifest -> pack reproduces each available shipped EMI byte for
  byte, including archive version and `ram_ptr` metadata.
