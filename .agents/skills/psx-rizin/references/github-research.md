# Read-only upstream research with GitHub CLI

Use GitHub CLI to establish tool versions, compatible plugin tags, source
behavior, releases, and known issues. Upstream source/issues are supporting
evidence; the installed binary's help and local PSX bytes remain authoritative.

## Repository and release inspection

```sh
gh repo view rizinorg/rizin
gh repo view rizinorg/rz-ghidra
gh repo view radareorg/radare2
gh repo view radareorg/r2ghidra

gh release list -R rizinorg/rizin --json tagName,publishedAt,url
gh release list -R rizinorg/rz-ghidra --json tagName,publishedAt,url
gh release view TAG -R rizinorg/rz-ghidra --json tagName,publishedAt,url
```

For rz-ghidra, use the official `rz-X.Y.Z` compatibility tags rather than
matching coincidental release numbers.

## Reproducible shallow source inspection

```sh
gh repo clone rizinorg/rizin /tmp/rizin -- --depth=1
gh repo clone rizinorg/rz-ghidra /tmp/rz-ghidra -- --depth=1
```

Clone only when local source inspection materially helps. Keep upstream clones
outside authored repository paths and do not vendor them incidentally.

## Search source and issues

```sh
gh search code 'QUERY' --repo rizinorg/rizin \
  --json path,sha,url,textMatches --limit 50
gh search issues 'QUERY' --repo rizinorg/rizin --state all \
  --json number,title,state,url,updatedAt --limit 50
```

GitHub CLI code search uses a legacy search endpoint and does not provide full
regex/new web-search parity. Confirm critical results in the exact tag or local
clone.

## API fallback

```sh
gh api repos/rizinorg/rizin/releases --paginate --jq '.[].tag_name'
gh api repos/rizinorg/rz-ghidra/tags --paginate --jq '.[].name'
gh api repos/rizinorg/rz-ghidra/contents/README.md \
  -H 'Accept: application/vnd.github.raw+json'
```

Use `--paginate`, `--slurp`, `--jq`, and explicit Accept headers when needed.
Prefer read-only GET requests. Do not open issues, comment, publish, or mutate
upstream repositories without explicit authorization.

## Record provenance

For a behavior that affects the skill or adapter, record the repository,
tag/commit, file/line or issue/release URL, installed local version, and whether
the conclusion was observed locally or inferred from upstream.

## Official sources

- https://cli.github.com/manual/gh_repo_view
- https://cli.github.com/manual/gh_repo_clone
- https://cli.github.com/manual/gh_release
- https://cli.github.com/manual/gh_release_view
- https://cli.github.com/manual/gh_search_code
- https://cli.github.com/manual/gh_search_issues
- https://cli.github.com/manual/gh_api
- https://cli.github.com/manual/gh_help_formatting
- https://cli.github.com/manual/gh_help_environment
