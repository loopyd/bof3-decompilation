# Build & Verify
- `BOF3_SOURCES` in `sources.cmake` = core + common + all module sources → every `.c` has independent `.obj`
- `BOF3_MODULE_*_SOURCES` per module; `cmake/modules/*.cmake` per family
- New PLACEHOLDER modules don't need DECLARED_SOURCES

## Verification
```bash
bin/asm-diff-one <source>   # per-function
make build                   # full
bin/doctor --strict          # env
.venv/bin/python -m pytest -q -p no:cacheprovider tools/python/tests
```
