# Third-party tools

Pinned tool repositories live here. Git records their exact revisions.

Treat these as third-party code, not as the home for repo-owned tooling.
Do not patch a fork without documenting the BOF3-specific reason and comparing
it with the current upstream project first.

`references/` contains pinned BOF3 research inputs:

- `bof3-data-doc` — EMI, overlay, texture, and palette notes.
- `vast-violence` — US data-table offsets and record layouts.

They are not installed or executed by the core workflow. Neither reference
currently declares a license, so retain them as submodule pointers and do not
copy their content into this repository.
