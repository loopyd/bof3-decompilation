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

The signature submodule is used only by the limited adapter:

```sh
git submodule update --init
bin/harness psyq scan --all
bin/harness psyq calls --all
```

It supplies object-signature evidence for PsyQ versions 3.6–4.7. PsyQ 4.7
headers remain the build-facing declaration baseline.

## Historical GCC variants

When a validated historical GCC variant appears, add it to
`config/compiler/variants.json`. The `bin/compiler-variants` CLI manages the
lifecycle:

```sh
bin/compiler-variants list                    # show catalog entries
bin/compiler-variants resolve                 # print resolved ID (or 'none')
bin/compiler-variants install <id>            # download and install a variant
bin/compiler-variants verify <id>             # verify installed variant
bin/compiler-variants path <id>               # print verified GCC path for CMake
bin/compiler-variants env                     # export environment overrides
bin/compiler-variants sha256                  # verify downloaded archives
```

The empty catalog (current state) means the canonical `gcc-2.7.2-psx` toolchain
is used without modification. A candidate entry remains untracked until its
SHA-256 matches the downloaded archive.
