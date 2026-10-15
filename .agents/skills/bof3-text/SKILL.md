---
name: bof3-text
description: Extract, analyze, search or safely modify BOF3 text with the Rust tool, or refine its source coverage and codecs. Preserve source identities and distinguish verified banks, heuristic leads and runtime evidence.
---

# BOF3 text

Own the Rust tool at `tools/rust/bof3-text` and its `bin/harness text` interface.
Complete US coverage first; record other builds separately.

Read [operations](references/operations.md) before extraction, analysis or edits.
Read [refinement](references/refinement.md) before investigating unknown sources or
changing the tool. Runtime observations use [psx-emulator](../psx-emulator/SKILL.md);
consumer tracing uses [psx-rizin](../psx-rizin/SKILL.md).

Select the source build and requested scope before acting. Retain input hashes and
write edits to new outputs. Report which sources and consumers were verified, the
checks performed, and the gaps that remain. A known-bank round trip proves byte
preservation for that bank; full completion also requires the complete source
inventory and observed game consumers.
