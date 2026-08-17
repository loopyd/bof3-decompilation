# Contributing

Thanks for helping improve this reverse-engineering project.

## Ground rules

- Never submit game assets, extracted binaries, copyrighted source, or large
  binary slices.
- Identify reverse-engineering findings as `TARGET@0xADDRESS`.
- Keep one C source per lifted function under `src/bof3/<subsystem>/`. Lift
  identity and ownership come from explicit manifest claims and
  `@source`/`@behavior` metadata, never from filenames or directory ancestry;
  a filename may remain raw (`func_80143B40.c`) or use a reviewed semantic
  name.
- Run the smallest relevant check. Run `just check` before handoff when you can.
- Follow the sanctioned matching-aid contract: no inline assembly or register
  pinning except the shared helpers — `barrier()`/`CLOBBER_*` for ordering, one
  bounded `REGISTER_PIN` only after the clean-C ladder is exhausted (retained
  only with `MATCHING_AID`, a live byte match, and independent review), and
  `WEAK_SYMBOL_AT` for address binding. Direct numeric register spelling and
  `INCLUDE_ASM` require explicit approval. See
  [Function matching](docs/agents/matching.md) for the matching workflow and
  [AGENTS.md](AGENTS.md) for the complete sanctioned-helper contract.

Read [AGENTS.md](AGENTS.md), [Tool usage](docs/usage.md), and
[Function matching](docs/agents/matching.md) before proposing a lift.

## Pull requests

Keep each pull request focused. State the target/address, byte evidence, files
changed, and checks run. Do not commit generated `out/` files or anything under
`inputs/` except its tracked placeholders.

## Code of Conduct

Participation is governed by the [Code of Conduct](CODE_OF_CONDUCT.md). By
participating, you agree to follow it.
