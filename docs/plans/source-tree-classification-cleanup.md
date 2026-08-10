# BOF3 semantic source-tree migration — completed

Completed by `6fd4e4a1` (`refactor(decomp): centralize metadata-owned lifts`).

## Result

- Authored lifts live under shallow semantic `src/bof3/<subsystem>/` paths.
- Explicit manifest `sources`, `support_sources`, and `headers` claims plus function-level `@source`/`@behavior`, maps, and reviewed Splat boundaries own target identity.
- Manifest `source_dir` is compatibility metadata, not ownership.
- Build, matching, symbols, Splat, status, and PsyQ binding resolve target-qualified claims instead of path ancestry or address-encoded filenames.

## Continuing contract

- Place fresh lifts under an established `src/bof3/<subsystem>/`; use `src/bof3/unknown/` only with explicit classification debt.
- Never infer ownership from directory, address, filename, or duplicate bytes.
- Keep equal addresses in different targets independent; never share target-local addresses merely because bytes or addresses coincide.
- Relocations remain one-target/one-subsystem transactions: atomically update manifest claims, Splat source paths, includes, and path-keyed flags; validate every selector with `asm-diff` and `byte-match`.
- `src/shared/` owns compile-time implementation templates only.
