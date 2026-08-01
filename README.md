# BOF3 reverse-engineering workspace

A reverse-engineering project that recovers independently loaded *Breath of
Fire III* binaries as readable C89, backed by target-qualified evidence and byte
checks.

## License

Project-authored material is dedicated to the public domain under
[CC0 1.0 Universal](LICENSE), to the extent permitted by law. It is provided
as-is, without warranty; use it at your own risk. This dedication does not grant
rights to *Breath of Fire III*, Capcom trademarks, original game assets, or
bundled third-party components, which retain their respective rights and
licenses. Original game assets are not distributed here.

## Start here

```sh
just setup
just doctor
bin/splat TARGET
bin/m2ctx TARGET@0xADDRESS
bin/m2c TARGET@0xADDRESS -o out/candidate.c
# Edit src/<target>/func_XXXXXXXX.c and its target-local evidence.
bin/asm-diff TARGET@0xADDRESS
bin/byte-match TARGET@0xADDRESS
```

Work on an extracted executable image or EMI entry—not the EMI archive.
Original bytes and target manifests are the source of truth.

Want to help? Read [CONTRIBUTING.md](CONTRIBUTING.md). Report a problem or
proposal through the repository's issue forms.

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
