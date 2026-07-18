# BOF3 reverse-engineering workspace

Recover independently loaded *Breath of Fire III* binaries as readable C89
with target-qualified evidence and byte comparison.

## Quick path

```sh
just setup
just doctor
bin/splat TARGET
bin/m2ctx TARGET@0xADDRESS
bin/m2c TARGET@0xADDRESS -o out/candidate.c
# edit src/<target>/func_XXXXXXXX.c and its local evidence
bin/asm-diff TARGET@0xADDRESS
bin/byte-match TARGET@0xADDRESS
```

An EMI archive is a container, not an analysis target. Original bytes and
target manifests are authoritative.

## Documentation

| Task | Reference |
| --- | --- |
| Follow the complete ordered tool workflow | [Tool usage](docs/usage.md) |
| Install and inspect tools | [Setup and tools](docs/setup.md) |
| Lift and match one function | [Matching](docs/matching.md) |
| Bootstrap EMI and reproduce Rizin evidence | [Rizin and reverse index](docs/reverse-engineering.md) |
| Understand target identity and ownership | [Context](CONTEXT.md) |
| Read reviewed format/runtime/data findings | [Specs](docs/specs/index.md) |
| Avoid known reverse-engineering mistakes | [Lessons](LESSONS.md) |

Run `--help` or `--example` on commands. Run `just check` before handoff.
