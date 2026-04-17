# Glossary

This file defines only stable shared terms used across multiple BOF3 specs.

Keep subsystem-local jargon in the owning spec instead of here.

## Terms

### EMI

Capcom archive/container format used throughout the BOF3 disc. An EMI file
contains a header, a TOC, and one or more payload entries that may be code,
graphics, audio, or tables.

### EMI Entry

One TOC-described payload inside an EMI archive. Each entry has a `size`, a
`type`, a `ram_ptr`-style argument, and payload bytes stored after the TOC.

### TOC

The EMI table of contents. In BOF3 it is walked by the main EXE to compute
sector offsets and to decide how each payload is loaded.

### Slot Id

Logical BOF3 loader index used by `SLUS_004.22` before disc I/O begins. A slot
id resolves through the top-level slot table into a disc LBA for one EMI
archive.

### Family

Higher-level BOF3 content class used by runtime selectors before a slot id is
chosen.

### `ram_ptr`

The second TOC word in an EMI entry. In BOF3 this is not always a literal CPU
RAM pointer; its meaning depends on entry type.

### Load Address

The runtime destination implied by an EMI entry's `ram_ptr`. For code-bearing
overlays this is usually the CPU address where the payload is streamed. For
graphics and audio entries it may instead be an encoded descriptor or logical
bank selector.

### Overlay

Code-bearing payload loaded at runtime from an EMI entry into a target RAM
address. In BOF3 these behave like PSX dynamic modules rather than static EXE
code only.

### Secondary Executable

A separate PS-X EXE loaded by `SLUS_004.22` and entered with `Exec`. In BOF3,
`LOGO/LOGO.EXE` is the proven boot-logo branch.

### Overlay Dispatch Table

Overlay-local table near the start of some code-bearing payloads that points at
internal entry routines or state handlers inside that same payload.

### Callable Entrypoint

The first address that higher-level code actually calls after a payload is
loaded. This may differ from the overlay load base when the payload starts with
local vectors, metadata, or a dispatch table.

### Dispatch Root

Scenario- or overlay-local object/table pointer used as the first stable handoff
target after a higher-level loader resolves a content unit.

### Representative Overlay

One chosen code-bearing EMI payload used as the primary reverse-engineering
target for a duplicate cluster. Other exact duplicates should map back to it
instead of being recovered independently.

### TIM

Standard PlayStation image container format used as a baseline reference for
PSX-native pixel, CLUT, and VRAM semantics.

### CLUT

PlayStation color lookup table. In BOF3, many frontend and menu images behave
more like raw indexed pixels plus separate CLUT rows than like one wrapped TIM
file on disc.
