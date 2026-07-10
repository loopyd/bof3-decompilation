---
type: Evidence boundary
title: Evidence status
description: Stable BOF3 reverse-engineering constraints and evidence boundaries.
tags: [evidence, bof3, reverse-engineering]
---

# Evidence status

> Stable constraints, not a task tracker.

- BOF3 has two PS-X executables: `SLUS_004.22` and `LOGO.EXE`.
- EMI is a container format. An archive entry can be code, data, graphics, or
  audio; its TOC type alone does not prove that it is executable.
- Exact payload duplicates are meaningful evidence, but only equal payloads at
  the same runtime address can share a build target automatically.
- The local corpus and all quantitative counts are generated from user-owned
  input in `out/catalog/emi.json`; they are not tracked repo facts.

Open questions and module-specific conclusions belong in their owning format,
runtime, or content note. Use Git history or issue tracking for work status.
