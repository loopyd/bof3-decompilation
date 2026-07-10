# Troubleshooting

> Resolve setup and evidence-state problems without changing tracked layout by hand.

| Symptom | Check | Fix |
| --- | --- | --- |
| `just setup` stops before extraction | `bin/rebof3 doctor` | Install the reported host prerequisite or place the US BIN/CUE input in `disks/`. |
| Configuration reports missing PsyQ headers | `test -f toolchains/psyq/4.7/include/libgpu.h` | Run `just psyq`. |
| The compiler exits with `Bad system call` | Confirm the command is running in a restricted sandbox | Run the configure/build command outside that sandbox; this is not a Ninja requirement. |
| `scan` finds no entries | Verify `out/extracted/` exists | Run `just extract`, then rerun `bin/rebof3 scan`. |
| `promote` rejects an entry | Read `out/catalog/emi.json` | Confirm the entry is code or mixed code/data; type `0` alone is insufficient. |
| `lift` rejects an address | Check the entry load address and size in the catalog | Use an address inside the confirmed payload and do not duplicate an existing lift. |
| A Splat config cannot resolve its target | Check its `target_path` and normalized image | Rerun extraction/normalization; do not edit generated files to hide the error. |

For command syntax, use `bin/rebof3 <command> --help`. Remove only generated
`out/` or `build/` content when a clean regeneration is required; never remove
the user-owned source media in `disks/`.
