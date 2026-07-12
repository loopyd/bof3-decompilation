# bof3-harness

BOF3 is modeled as independently loaded binaries: the main EXE, `LOGO.EXE`,
and reviewed EMI payloads. EMI archives are containers; an entry only becomes
a decompilation target after explicit review.

## Quick start

Place a user-owned US disc image in `disks/`, then run:

```sh
just setup
bin/harness target list
bin/harness index build
```

To inspect and reverse a confirmed target:

```sh
bin/harness show "$TARGET"
bin/harness lift "$TARGET" "$ADDRESS"
bin/harness diff "$TARGET@$ADDRESS"
```

Generated files live in `out/`; user-owned disc media lives in `disks/`.
The durable layout is `config/splat/`, `config/symbols/`, `src/exe/`, and
`src/emi/`. See [setup](docs/setup.md), [reverse engineering](docs/reverse-engineering.md), [matching](docs/matching.md), and [troubleshooting](docs/troubleshooting.md).
