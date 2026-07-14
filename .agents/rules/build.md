# Build and verification policy

- BOF3 binaries are independently loaded targets, not one link unit.
- Keep every source file owned by its executable or promoted EMI target.
- Build metadata belongs in CMake; Python must not construct `CMakeFiles/...` paths.
- Generated build and comparison artifacts belong under `build/` and `out/`.

Use the narrowest check while iterating. Before handoff, run all available
repository gates and state any skipped gate with its reason.

```bash
bin/harness diff <source>
just build
just check
bin/harness doctor --strict
```

When a newly lifted function reaches a canonical 100% instruction and byte
match, rerun its diff and prepare only that function plus required boundaries,
declarations, or bindings for a focused commit. Commit or push only when
explicitly authorized.
