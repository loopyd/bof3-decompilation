---
name: psx-rizin
description: Reproduce target-qualified Rizin evidence for a BOF3 binary.
---

# PSX Rizin

Use this skill only for one exact `TARGET` at a time. The target manifest,
original bytes, reviewed Splat layout, target map, and tracked replay are the
sources of truth; the generated Rizin project is disposable evidence.

```sh
bin/rz-project rebuild TARGET
bin/rz-project analyze TARGET
bin/rz-project status TARGET
bin/rz-project export TARGET
```

- Never combine executable or overlay mappings in one project, even when their
  addresses overlap.
- `analyze` is bounded. `analyze --deep` creates candidates only.
- `export` prints a deterministic patch. Review it, then use `--write` only
  when the target-local replay update is intended.
- Use `just index` only after all target exports are fresh and complete; query
  it through `bin/rev-query`.
- Keep raw names until semantic evidence is reviewed. Analyzer output does not
  override bytes, manifests, Splat boundaries, maps, or a C match.
