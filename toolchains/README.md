# toolchains

Downloaded or staged SDKs and compilers live here.

This is a local tool/runtime staging area, not a repo-owned source tree.
`bin/` plus `tools/python/` remain the maintained workflow surface.

Examples:

- staged PsyQ under `toolchains/psyq/4.7/`
- PSn00b toolchain under `toolchains/psn00b_toolchain/`
- old gcc toolchain under `toolchains/gcc-2.7.2-psx/`
- canonical public ASPSX `psyq4.0` compatibility bundle under `toolchains/aspsx-psyq-binaries/`

Other local staging helpers may appear here, including download caches or
compatibility bundles. Keep durable repo tooling in `tools/python/`, not under
`toolchains/`.
