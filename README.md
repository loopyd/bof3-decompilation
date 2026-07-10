# rebof3-simple

BOF3 is modeled as independently loaded binaries: the main EXE, `LOGO.EXE`,
and reviewed EMI payloads. EMI archives are containers; an entry only becomes
a decompilation target after explicit review.

```sh
just setup
bin/rebof3 status
bin/rebof3 candidates BATTLE
bin/rebof3 promote BIN/BATTLE/BATTLE.EMI#3 --confirm-code
bin/rebof3 next
```

Generated files live in `out/`; user-owned disc media lives in `disks/`.
The durable layout is `config/splat/`, `config/symbols/`, `src/exe/`, and
`src/emi/`. See [setup](docs/setup.md), [reverse-engineering facts](docs/reverse-engineering.md), and [troubleshooting](docs/troubleshooting.md).
