# psx-rizin skill

A manually invoked Agent Skill for evidence-driven Sony PlayStation 1 reverse engineering with Rizin and emulator-assisted runtime analysis.

## Install

### Portable project-local layout

Copy the whole `psx-rizin` directory to:

```text
.agents/skills/psx-rizin/
```

Both Codex-compatible Agent Skills clients and current OpenCode can discover this layout. Keep the entire directory together; the skill references bundled scripts and documents by relative path.

### Codex

Copy to a user skill directory:

```bash
mkdir -p "${CODEX_HOME:-$HOME/.codex}/skills"
cp -a psx-rizin "${CODEX_HOME:-$HOME/.codex}/skills/psx-rizin"
```

Restart Codex after installation. See `config/INSTALL_CODEX.md`.

### OpenCode

Project-local:

```bash
mkdir -p .opencode/skills
cp -a psx-rizin .opencode/skills/psx-rizin
```

Or global:

```bash
mkdir -p ~/.config/opencode/skills
cp -a psx-rizin ~/.config/opencode/skills/psx-rizin
```

See `config/INSTALL_OPENCODE.md` and `config/opencode.json.example`.

## Invoke explicitly

```text
$psx-rizin inventory path/to/disc.cue
$psx-rizin inspect-exe path/to/SLUS_123.45
$psx-rizin analyze path/to/overlay.bin 0x80180000
$psx-rizin function path/to/game.exe 0x8002a120
$psx-rizin trace attract-mode-01
```

The frontmatter and instructions intentionally say not to auto-trigger the skill.

## Direct utility use

> This skill's `bin/psx-rizin` and `bin/lift` dispatchers are not wired in the
> BOF3 repository. Use the repo's entrypoints instead, and the generic
> `scripts/*.py` helpers for Rizin-specific work.

```bash
bin/rz-project analyze TARGET            # target-qualified Rizin analysis
bin/asm-diff TARGET@0xADDRESS            # instruction-level diff
bin/byte-match TARGET@0xADDRESS          # byte-equality acceptance
bin/permute TARGET@0xADDRESS --time-limit 300
python3 scripts/psx_exe.py GAME.EXE      # generic PS-X EXE inspection
python3 scripts/scan_mips.py OVERLAY.BIN --base 0x80180000
```

## Scope and legal note

This package contains original workflow text, scripts, and links. It does not include a PlayStation BIOS, game data, PsyQ SDK binaries, or mirrored proprietary manuals. Analyze only material you are authorized to possess.

## Validation status

The bundled Python helpers and shell wrappers were syntax-checked and exercised with synthetic PS-X EXE, symbol-import, raw-MIPS scan, and replay-matrix fixtures. Rizin was not installed in the build environment, so commands that invoke Rizin/rz-ghidra were not end-to-end executed here; they defensively check for the required binary and the reference material instructs users to confirm command help against the installed release.
