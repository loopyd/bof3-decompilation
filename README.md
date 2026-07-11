# rebof3-simple

BOF3 is modeled as independently loaded binaries: the main EXE, `LOGO.EXE`,
and reviewed EMI payloads. EMI archives are containers; an entry only becomes
a decompilation target after explicit review.

## Quick start

Place a user-owned US disc image in `disks/`, then run:

```sh
just setup
bin/rebof3 status
bin/rebof3 next
```

To inspect and reverse a confirmed target:

```sh
bin/rebof3 inspect "$TARGET"
bin/rebof3 lift "$TARGET@$ADDRESS"
bin/rebof3 diff "$FUNCTION_SOURCE"
```

Generated files live in `out/`; user-owned disc media lives in `disks/`.
The durable layout is `config/splat/`, `config/symbols/`, `src/exe/`, and
`src/emi/`. See [setup](docs/setup.md), [reverse engineering](docs/reverse-engineering.md), [matching](docs/matching.md), and [troubleshooting](docs/troubleshooting.md).
