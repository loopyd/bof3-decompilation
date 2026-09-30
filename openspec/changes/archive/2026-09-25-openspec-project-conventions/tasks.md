# Tasks

## 1. Author the project configuration

- [x] 1.1 Replace the commented example blocks in `openspec/config.yaml` with an active `context` naming `AGENTS.md` and `docs/INDEX.md` and stating only the hard non-negotiables — verify: `openspec instructions proposal --change openspec-project-conventions --json` returns a non-empty `context`, and no "Context too large" warning appears (the 50 KB cap is not approached).
- [x] 1.2 Add non-empty `rules` entries for `proposal`, `specs`, `design` and `tasks` — verify: `openspec instructions <artifact> --change openspec-project-conventions --json` returns a non-empty `rules` array for all four ids, with no unknown-artifact-id warning.
- [x] 1.3 Add `operations.apply.guidance` and `operations.archive.guidance` — verify: `openspec instructions apply` and `openspec instructions archive` each report non-empty `operationGuidance`.

## 2. Integration Verification

- [x] 2.1 Verify the configuration is accepted as a whole: `openspec doctor` reports the root ok with no configuration warning, and `openspec validate openspec-project-conventions --strict --json` reports `valid: true`.
- [x] 2.2 Verify scope and single ownership held. This change touched only `openspec/` (`git status --short openspec` → `?? openspec/`); `grep -rIn openspec tools/python bin justfile` and `grep -rIn openspec AGENTS.md README.md CONTRIBUTING.md docs` both return no matches, so neither harness behaviour nor documentation is affected. Note: `docs/` was already modified before this change (pre-existing dirty work — its diff contains 0 lines mentioning openspec and predates this change), so that pre-existing state is preserved, not produced.
