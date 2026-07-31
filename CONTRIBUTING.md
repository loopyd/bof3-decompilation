# Contributing

Thanks for helping improve this clean-room reverse-engineering project.

## Ground rules

- Never submit game assets, extracted binaries, copyrighted source, or large
  binary slices.
- Identify reverse-engineering findings as `TARGET@0xADDRESS`.
- Keep one function in each `func_XXXXXXXX.c` file.
- Run the smallest relevant check. Run `just check` before handoff when you can.
- Do not add inline assembly or register pinning without maintainer approval.

Read [AGENTS.md](AGENTS.md), [Tool usage](docs/usage.md), and
[Matching](docs/matching.md) before proposing a lift.

## Pull requests

Keep each pull request focused. State the target/address, byte evidence, files
changed, and checks run. Do not commit generated `out/` files or anything under
`inputs/` except its tracked placeholders.

## Code of Conduct

Participation is governed by the [Code of Conduct](CODE_OF_CONDUCT.md). By
participating, you agree to follow it.
