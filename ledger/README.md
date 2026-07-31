# Ledger

This directory holds local, append-only JSONL ledger records and runtime
metadata (e.g. `main.jsonl`) written by tooling. It is machine-owned local
state, not reviewed repository truth.

Only the two placeholders are tracked: `.gitkeep` and this `README.md`.
Every other content is ignored by the local `.gitignore` — including existing
records such as `main.jsonl` — and must never be committed.
