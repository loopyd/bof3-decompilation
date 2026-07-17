# Install in OpenCode

OpenCode currently searches these relevant layouts:

```text
.opencode/skills/psx-rizin/SKILL.md
~/.config/opencode/skills/psx-rizin/SKILL.md
.agents/skills/psx-rizin/SKILL.md
~/.agents/skills/psx-rizin/SKILL.md
```

Project-local installation:

```bash
mkdir -p .opencode/skills
cp -a psx-rizin .opencode/skills/psx-rizin
```

Allow the native skill tool in `opencode.json` if your permission policy blocks it. See `opencode.json.example`.

Ask OpenCode to load `psx-rizin` only when explicitly invoked:

```text
$psx-rizin analyze ./extracted/SLUS_...
```
