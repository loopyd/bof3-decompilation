# Publish decompilation progress

Install the repository's main-push hook once per clone.

```sh
bin/harness pages install-hook
```

The installer preserves existing hook configuration by refusing to replace it.
The hook needs the repository Python environment, installed Rizin, and local
target binaries. Prepare these with the repository's normal setup before pushing.

Push main normally. The hook builds from the exact commit being pushed in a
temporary worktree, refreshes stale snapshots, rebuilds and validates the reverse
index, and pushes the generated page to `gh-pages`. It does not consume unrelated
working-tree edits. Other branch pushes and main deletions do not publish pages.

The GitHub Actions Pages workflow deploys that branch after GitHub accepts the
matching main commit. Enable GitHub Actions as the Pages publishing source in
repository settings. A failed main push can leave the prepared branch ahead of
main, but that revision does not deploy. Publication failures block the main push.
Resolve the reported error and retry. Concurrent branch updates fail without
force-pushing.

To build a local preview from a committed revision, run these commands from the
repository root.

```sh
bin/harness pages preview --revision HEAD --output out/pages-preview
python3 -m http.server 8000 --directory out/pages-preview
```

Open `http://localhost:8000`. To prepare a matching snapshot after an interrupted
push, use `bin/harness pages publish --revision HEAD --remote origin`, then rerun
the Pages workflow on main. Publishing the branch alone does not deploy it.

The dashboard counts indexed functions within each target. Completed maps to
`exact`; partial maps to `partial`; unlifted includes `unlifted` and `invalid`.
Invalid entries receive no progress credit and have a separate disclosure. These
counts describe indexed declarations, not byte coverage, a fresh native build,
or runtime acceptance. A target without indexed functions has no completion
percentage.

Only HTML, CSS, JavaScript, the logo, status JSON, and `.nojekyll` reach the public
branch. The original logo comes from
`third_party/references/bof3-data-doc/src/DocAssets/Breathlogo.webp`. Breath of Fire
III and its logo belong to Capcom. The page uses local system fonts and has no
runtime font or script dependencies.

To disable the hook, remove this clone's local `core.hooksPath` setting if its value
is `.githooks`. This leaves the existing public page and branch history intact.
