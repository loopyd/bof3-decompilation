# Docs

This tree holds the human documentation for the repo.

This repo is still settling after the tooling/layout migration. The maintained
surfaces are `bin/` plus `tools/python/`; `scripts/legacy/` is compatibility-only.

Start here:

1. `docs/SETUP.md`
2. `docs/REPO_LAYOUT.md`
3. `docs/TROUBLESHOOTING.md`
4. `docs/specs/status.md`

Use `docs/specs/` for stable reverse-engineering knowledge. Keep workflow guidance in the top-level docs files above instead of mixing it into the specs tree.

Some specs still cite legacy generated paths as historical evidence. When
describing the active repo layout or workflows, prefer the top-level docs and
current paths under `out/`.

## Source Of Truth

- `docs/specs/`
  - stable human-maintained reverse-engineering knowledge
- `out/`
  - generated extraction, inventory, planning, and review artifacts
- `bof3/`
  - recovered or reimplemented PSX-first source
- `bin/`
  - maintained command surface
- `tools/python/`
  - repo-owned maintained CLI and setup implementation
- `scripts/legacy/`
  - compatibility-only legacy automation
- `third_party/`
  - vendored external tools
