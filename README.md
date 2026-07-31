# BOF3 reverse-engineering workspace

Recover independently loaded *Breath of Fire III* binaries as readable C89
with target-qualified evidence and byte comparison.

## License

The original project material in this repository is licensed under the
[GNU General Public License, version 3](LICENSE). Third-party components retain
their own licenses; original game assets are not distributed here.

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
| Lift and match one function | [Matching](docs/matching.md) |
| Resolve asm-diff symptoms | [Matching playbook](docs/matching-playbook.md) |
| Memory macros and qualifiers | [Memory API](docs/memory-api.md) |
| Understand target identity and ownership | [Context](CONTEXT.md) |
| Read reviewed format/runtime/data findings | [Specs](docs/specs/) |
| Avoid known reverse-engineering mistakes | [Lessons](LESSONS.md) |

Run `--help` or `--example` on commands. Run `just check` before handoff.
