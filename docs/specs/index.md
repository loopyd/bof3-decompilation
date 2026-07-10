# Reverse-engineering specs

Human-maintained BOF3 facts, evidence, and unresolved questions.

## Core concepts

* [Glossary](glossary.md) - Stable terms shared by the specifications.
* [Evidence status](status.md) - Constraints that apply across the knowledge catalog.
* [Recovered memory layouts](recovered-layouts.md) - Locally evidenced working byte layouts for lifts.
* [Formats](formats/index.md) - EMI layout, payload semantics, and generated-artifact ownership.
* [Runtime](runtime/index.md) - Executable roles, loader behavior, and reviewed overlays.
* [Content](content/index.md) - Archive-family and mixed-content case studies.
* [Sources](sources/index.md) - External references, kept distinct from local proof.

Use shipped archive path plus slot for an EMI entry. Add a load address only
when recording a code-module target. Generated corpus state belongs in
`out/catalog/`, not in this catalog.
