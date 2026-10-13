# Third-party tools

Pinned tool repositories live here. Git records their exact revisions.

Canonical sources are recorded in [.gitmodules](../.gitmodules). Inspect the
checked-out revisions with `git submodule status --recursive`; setup uses these
pins rather than following upstream branch tips.

The maintained forks have their own source and documentation owners:

| Fork | Role | Integration guide |
| --- | --- | --- |
| [pcsx-redux-agentic](pcsx-redux-agentic/README.md) | Linux emulator, embedded Lua missions and native capture APIs | [Emulator harness](../docs/agents/pcsx-redux-agentic.md) |
| [rz-psxdec](rz-psxdec/README.md) | Rizin plugin, overlay mounts and original-only generation | [Native build](../tools/retdec-psx/README.md) |
| [retdec-psx](retdec-psx/README.md) | PS1 decoder, analysis and C output backend | [Reconstruction](../tools/retdec-psx/reconstruction.md) |

Commit fork changes in their repositories, then update this project's gitlinks.
Compare dependency changes with the canonical upstream and document their reason.
BOF3 harness code and game-specific integration stay in this repository.

`references/` contains pinned BOF3 research inputs:

- `bof3-data-doc` — EMI, overlay, texture, and palette notes.
- `vast-violence` — US data-table offsets and record layouts.

They are not installed or executed by the core workflow. Neither reference
currently declares a license, so retain them as submodule pointers and do not
copy their content into this repository.
