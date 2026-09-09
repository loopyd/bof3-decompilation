# Codex configuration and Pi migration

Read [the documentation index](../INDEX.md) first. Project skill definitions and
Python owners are described there and in [HARNESS.md](HARNESS.md).

## Global MCP ownership

MCP server definitions and configured credentials belong in `~/.codex/config.toml`,
not this repository's `.codex` directory. The migration source is
`~/.pi/agent/mcp.json`, overlaid by `.pi/mcp.json` when it defines project servers.
The project file was empty at migration. Preserve unrelated Codex settings and a
private backup before changing global configuration; keep credential-bearing
configuration and backups mode `0600`. Never print tokens, connection strings or
credential-bearing command arguments in reports or commit them to the project.

The migrated servers are `sqlitecloud-mcp-server`, `github-mcp-server`,
`markitdown`, `context7`, `context-mode`, `playwright`, and `vercel`. Command
arguments and literal credentials are preserved. Pi's `directTools` flag has no
Codex configuration equivalent and does not grant approval. Expand Pi environment
references explicitly when translating; do not persist session-temporary PATH
entries. The Pi source remains untouched as recovery/reference material.

Codex successfully parses all seven global definitions. This proves configuration
loading, **not remote connectivity, credential validity or OAuth completion**.
No servers were launched or packages installed for that check. Stdio servers use
their configured environment/arguments; an OAuth status of `unsupported` from the
listing is not a failed API-key validation.

Vercel is configured as streamable HTTP; its OAuth status remains unverified.
Use `codex mcp login vercel` for Codex-owned authentication when needed. Do not
copy opaque Pi keyring records into Codex or substitute an expiring access token
for a complete OAuth lifecycle. Authentication may require user browser approval.
See [official MCP configuration](https://developers.openai.com/codex/mcp/).

## Extension equivalents

Installed Pi extensions are not modified. Use native Codex features before adding
another process or plugin. **Serena is explicitly excluded**, as are replacement
subagent, memory, VCC and compaction extensions. Existing Codex features/settings
are not silently disabled by excluding their Pi counterparts.

| Pi extension | Codex disposition |
| --- | --- |
| `pi-mcp-adapter` | Native MCP configuration; seven definitions migrated globally. |
| `pi-web-access` | Native web tools; existing MarkItDown/Playwright MCP definitions retain document/browser options. This does not claim parity with every provider or video feature. |
| `pi-intercom` | Native `codex agents` and `codex queue --thread ID --message TEXT` cover session discovery/messaging. No Pi broker or subagent bridge is ported. |
| `pi-semantic-edit` | Native `apply_patch` remains the edit mechanism; no fuzzy-match replacement tool or weakened uniqueness rules. |
| `@plannotator/pi-extension` | Publisher-supported [Codex integration](https://github.com/backnotprop/plannotator/blob/main/apps/codex/README.md): review/annotation skills and optional Stop-hook plan review. Not installed or enabled by this migration. Avoid the broad installer that also changes other agents. |
| `pi-lsp` | Non-Serena candidate: [MCP Language Server](https://github.com/isaacphi/mcp-language-server). Its documentation covers clangd, Pyright, TypeScript and Rust; the adapter is not installed or validated here. If adopted, scope workspaces and prefer read-only navigation/diagnostic tools; its rename/edit tools do not bypass BOF3 evidence gates. |
| `pi-session-search` | Use native `codex resume` for session selection; no semantic memory index or automatic import of Pi transcripts. This is not full semantic-search parity. |
| `context-mode` | No additional context/compaction extension or hooks. Its MCP definition was retained under the separate all-server migration request. |
| `pi-subagents`, `@samfp/pi-memory`, `@sting8k/pi-vcc` | Omitted as requested. |
| `pi-archon` | Not ported; its Pi entrypoint is excluded in the current package configuration. |

Plannotator and the LSP adapter are researched alternatives, not installed plugins.
New binaries, dependencies, hooks and global configuration changes require explicit
approval of the specific installation. A capability table or marketplace result is
not installation evidence. Keep project-specific language-server flags and trust
boundaries explicit rather than blindly copying them into global launch arguments.
