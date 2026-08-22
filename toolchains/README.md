# toolchains

Local SDKs and compilers live here. Generated/staged toolchains are untracked;
the pinned `psx_psyq_signatures/` Git submodule is the sole tracked exception.

`just setup` downloads the official PsyQ 4.7 Runtime Library ZIP for headers
and `.LIB` archives, then stages it under `psyq/4.7/`. It also downloads a
converted per-object form because reviewed target manifests name individual
`.o` members for signature evidence. Both forms are local build infrastructure,
remain separate from the signature database, and do not prove that the shipped
game used that SDK.

`third_party/rizin/` is a pinned Rizin source submodule. `just setup` builds and
stages it under `toolchains/rizin/`; use `bin/rizin` rather than a host
installation. `just doctor` verifies its MIPS and JSON-analysis capabilities
before snapshots are used.

`third_party/splat` (tag `0.41.1`) and `third_party/spimdisasm` (tag `1.42.2`)
are pinned Python source submodules. `just setup` installs both, including
Splat's MIPS extras, into `.venv`. `third_party/decomp-permuter` is likewise a
pinned submodule; setup installs its required `toml` module into `.venv`.
It similarly installs the pinned `asm-differ` submodule and its declared
dependencies. Use `bin/splat`, `bin/spimdisasm`, `bin/permute`, and
`bin/asm-diff` rather than host installs.

The signature submodule is used only by the permanent, narrow
`bin/harness psyq {scan|calls|proposal}` evidence adapter. Do not add other
top-level command families; symbol-map mutation remains under `bin/symbols`.

```sh
git submodule update --init
bin/harness psyq scan --all
bin/harness psyq calls --all
```

It supplies object-signature evidence for PsyQ versions 3.6–4.7. PsyQ 4.7
headers remain the build-facing declaration baseline.

## Historical GCC variants

`gcc-2.6.3-psx` and `gcc-2.8.0-psx` are verified, opt-in negative candidates
in `config/compiler/variants.json`; no object selects either, so canonical GCC
remains the default. Add another candidate only with the same reviewed
provenance. The `bin/compiler-variants` CLI manages the lifecycle:

```sh
bin/compiler-variants list                    # show catalog entries
bin/compiler-variants install <id>            # download and install a variant
bin/compiler-variants verify <id>             # verify installed variant
bin/compiler-variants path <id>               # print verified GCC path for CMake
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
verified install. `bin/compiler-variants path <id>` and generated
`compile_commands.json` resolve a selected compiler through the same
ensure-installed operation, so a missing install self-heals from the
verified cache. `just setup` primes the canonical compiler plus every host-compatible entry
in the `config/compiler/variants.json` catalog; a host-incompatible
candidate is skipped with its ID and host reported, an invalid catalog fails
setup closed, and a catalog with no candidates installs nothing. Setup never
sets `PSX_GCC`, adds an object override, or changes the default compilation
selection. A corrupt or malformed existing install fails closed
rather than falling back to canonical or host GCC.

A catalog entry never changes the compiler until an exact, target-qualified
object selection is added.
