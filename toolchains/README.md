# Toolchains

Local SDKs and compilers live here. Generated/staged toolchains are untracked;
the pinned `psx_psyq_signatures/` Git submodule is the sole tracked exception.

`just setup` downloads the official PsyQ 4.7 Runtime Library ZIP for headers
and `.LIB` archives, then stages it under `psyq/4.7/`. It also downloads a
converted per-object form because reviewed target manifests name individual
`.o` members for signature evidence. Both forms are local build infrastructure,
remain separate from the signature database, and do not prove that the shipped
game used that SDK.

`third_party/rizin/` is a pinned Rizin source submodule. `just setup` builds and
stages it under `toolchains/rizin/`; use `bin/harness analysis rizin` rather than a host
installation. `just doctor` verifies its MIPS and JSON-analysis capabilities
before snapshots are used.

Splat, spimdisasm, decomp-permuter, and asm-differ are pinned source submodules
under `third_party/`. `just setup` installs their Python dependencies, including
Splat's MIPS extras, into `.venv`. Use `bin/harness source splat`,
`bin/harness analysis spimdisasm`, `bin/harness lift permute`, and
`bin/harness lift asm-diff` to run the managed versions.

The emulator source and embedded Lua live in `third_party/pcsx-redux-agentic/`.
Its local SDL source, build, and installation live under
`toolchains/pcsx-redux-agentic/`. See the
[emulator guide](../docs/agents/pcsx-redux.md) for setup and invocation.

The signature submodule is used only by the permanent, narrow
`bin/harness psyq {scan|calls|proposal}` evidence adapter. Do not add other
top-level command families; symbol-map mutation remains under `bin/harness source symbols`.

```sh
git submodule update --init
bin/harness psyq scan --all
bin/harness psyq calls --all
```

It supplies object-signature evidence for PsyQ versions 3.6–4.7. PsyQ 4.7
headers remain the build-facing declaration baseline.

## Historical GCC variants

PSX-RetDec reconstruction uses the canonical GCC 2.7.2-PSX and maspsx
profile, with ASPSX 2.56 behavior. Historical compilers remain available
through the catalog for explicitly selected comparisons.

`config/compiler/variants.json` owns the reviewed compiler catalog. Variants
remain comparison candidates until an exact, target-qualified selection is
recorded. Use the CLI to discover available IDs and manage installed versions:

```sh
bin/harness build variants list                    # show catalog entries
bin/harness build variants install <id>            # download and install a variant
bin/harness build variants verify <id>             # verify installed variant
bin/harness build variants path <id>               # print verified GCC path for CMake
```

`config/compiler/variants.json` is reviewed, tracked metadata — the single
source of truth for compiler IDs, archive digests, and executable paths.
GCC archives are cached (digest-verified) under
`inputs/external/private-assets/toolchains/gcc/`; installed variants live in
ignored local state under `toolchains/gcc-variants/` and the canonical
compiler under `toolchains/gcc-2.7.2-psx/`. Unrelated toolchain downloads
(PSn00b, Rizin) stay in `toolchains/downloads/`.

An archive is accepted only when its SHA-256 matches the reviewed catalog
entry. The shared GCC archive lifecycle downloads into a cache-local
temporary file, validates the digest before atomically publishing the cache
entry, extracts to a fresh sibling staging directory, verifies the staged
`gcc --version` identity, and only then atomically replaces the install; a
failed network, digest, extraction, or identity check preserves a prior
verified install. `bin/harness build variants path <id>` and compilation database
generation verify the selected installation without downloading or repairing it.
A missing installation requires `bin/harness build variants install <id>` with
specific installation authorization. `just setup` primes the canonical compiler
plus every host-compatible entry
in the `config/compiler/variants.json` catalog; a host-incompatible
candidate is skipped with its ID and host reported, an invalid catalog fails
setup closed, and a catalog with no candidates installs nothing. Setup never
sets `PSX_GCC`, adds an object override, or changes the default compilation
selection. A corrupt or malformed existing install fails closed
rather than falling back to canonical or host GCC.

Function metadata can select `@compiler` or `@gcc`; those tags take precedence
over legacy `BOF3_OBJCOMPILER_<sanitized-source-path>` entries in
`config/compiler/object-flags.cmake`. With neither selection, the canonical
compiler remains selected. See [compiler settings](../docs/agents/compiler-variants.md#function-compiler-settings)
for the complete resolution and evidence contract.
