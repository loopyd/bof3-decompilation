# Build & Verify

- BOF3 binaries are independently loaded targets, not one link unit.
- Keep every source file owned by its executable or promoted EMI target.
- Build metadata belongs in CMake; Python must not construct `CMakeFiles/...` paths.
- Generated build and comparison artifacts belong under `build/` and `out/`.

## Verification

```bash
bin/rebof3 diff <source>
just build
just check
bin/rebof3 doctor --strict
```
