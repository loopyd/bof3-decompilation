# Install in Codex

Codex Agent Skills are directory packages: keep `SKILL.md`, scripts, references, assets, and agent metadata together.

## User installation

```bash
mkdir -p "${CODEX_HOME:-$HOME/.codex}/skills"
cp -a psx-rizin "${CODEX_HOME:-$HOME/.codex}/skills/psx-rizin"
```

Restart Codex so it refreshes its skill index.

Invoke manually:

```text
$psx-rizin <task>
```

The package uses only standard Agent Skills frontmatter. Do not symlink only `SKILL.md`; link or copy the entire skill directory so relative resources remain available.
