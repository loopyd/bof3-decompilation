# Manuals and source catalog

This is a comprehensive working catalog for PS1 reverse engineering, not a claim that every private or historically released document is publicly available. Prefer primary project documentation and original hardware/ABI manuals. Public PsyQ/Sony mirrors may contain proprietary/confidential material with unclear redistribution rights; this skill links to them but does not bundle them.

## Source tiers

| tier | use |
|---|---|
| A | official/current tool documentation and original hardware/ABI manuals |
| B | maintained technical specifications and emulator documentation |
| C | public archival mirrors of historical SDK manuals; provenance/legal caution |
| D | active community reverse-engineering tools and real decompilation projects |
| E | forum/blog explanations; use only to discover leads and verify elsewhere |

## Rizin and Cutter — Tier A

- Rizin Handbook: https://book.rizin.re/
- Code analysis, functions, xrefs, jump tables, GP, hints: https://book.rizin.re/src/analysis/code_analysis.html
- Variables/arguments: https://book.rizin.re/src/analysis/variables.html
- Types: https://book.rizin.re/src/analysis/types.html
- Symbols: https://book.rizin.re/src/analysis/symbols.html
- Signatures/FLIRT: https://book.rizin.re/src/analysis/signatures.html
- RzPipe: https://book.rizin.re/src/scripting/rzpipe.html
- Disassembly (`pd` versus `pD`): https://book.rizin.re/src/print_modes/disassembly.html
- Command-line options: https://book.rizin.re/src/first_steps/commandline_options.html
- Reference card: https://book.rizin.re/src/refcard/intro.html
- `rz-bin`: https://book.rizin.re/src/tools/rz-bin/intro.html
- `rz-asm`: https://book.rizin.re/src/tools/rz-asm/intro.html
- Rizin repository/releases: https://github.com/rizinorg/rizin
- rz-ghidra repository and `pdg` commands: https://github.com/rizinorg/rz-ghidra
- Cutter: https://cutter.re/ and https://github.com/rizinorg/cutter

Version note: Rizin and rz-ghidra evolve together. Use an rz-ghidra tag matching the installed Rizin release (tags are named like `rz-X.Y.Z`).

## PS1 hardware/specification — Tier B with provenance warning

- PSX-SPX maintained site: https://psx-spx.consoledev.net/
- Single-page PDF link is available from the PSX-SPX home page.
- Memory map: https://psx-spx.consoledev.net/memorymap/
- I/O map: https://psx-spx.consoledev.net/iomap/
- CPU specifications: https://psx-spx.consoledev.net/cpuspecifications/
- Kernel/BIOS: https://psx-spx.consoledev.net/kernelbios/
- GPU: https://psx-spx.consoledev.net/graphicsprocessingunitgpu/
- GTE: https://psx-spx.consoledev.net/geometrytransformationenginegte/
- DMA: https://psx-spx.consoledev.net/dmachannels/
- CD-ROM: https://psx-spx.consoledev.net/cdromdrive/
- CD file formats: https://psx-spx.consoledev.net/cdromfileformats/
- Controllers/memory cards: https://psx-spx.consoledev.net/controllersandmemorycards/
- SPU: https://psx-spx.consoledev.net/soundprocessingunitspu/
- MDEC: https://psx-spx.consoledev.net/macroblockdecodermdec/
- Original nocash document: https://problemkaputt.de/psx-spx.htm

Important provenance: the maintained PSX-SPX site itself states that republication rights were not formally acquired and that substantial content derives from confidential Sony code/documentation; it is not an independently authored specification. Use it as a highly useful technical reference while keeping legal/provenance claims accurate.

## MIPS CPU and ABI manuals — Tier A/C

- IDT R30xx Family Software Reference Manual (readable mirror): https://usermanual.wiki/Document/r3000manual.723589236/help
- Bitsavers IDT RISC archive: https://www.bitsavers.org/components/idt/risc/
- IDT 79R3051 Hardware User's Manual: https://www.bitsavers.org/components/idt/risc/1991_IDT79R3051_Hardware_Users_Manual.pdf
- IDT R3051/R3052 RISController Hardware User's Manual Rev 1.3: https://www.bitsavers.org/components/idt/risc/1992_IDTR3051_R3052_RISController_Hardware_Users_Manual_Rev1.3_19920821.pdf
- IDT R3051/R3081 Application Guide: https://www.bitsavers.org/components/idt/risc/1992_IDT_R3051_R3081_Application_Guide.pdf
- MIPS System V ABI historical reference: https://refspecs.linuxfoundation.org/elf/mipsabi.pdf

Use the PSX-specific ABI observations from PSX-SPX together with the original MIPS manuals. Generic System V ABI details may differ from PsyQ compiler conventions in edge cases; prove stack/frame behavior in the actual binary.

## PCSX-Redux runtime/debugging — Tier B

- Documentation root: https://pcsx-redux.consoledev.net/
- Debugging introduction: https://pcsx-redux.consoledev.net/Debugging/introduction/
- GDB server: https://pcsx-redux.consoledev.net/Debugging/gdb-server/
- Ghidra connection: https://pcsx-redux.consoledev.net/Debugging/ghidra/
- Lua breakpoints: https://pcsx-redux.consoledev.net/Lua/breakpoints/
- Lua memory/register access: https://pcsx-redux.consoledev.net/Lua/memory-and-registers/
- Handling PSX binaries: https://pcsx-redux.consoledev.net/Lua/binary/
- GPU logger: https://pcsx-redux.consoledev.net/Debugging/gpu-logger/
- Repository: https://github.com/grumpycoders/pcsx-redux

PCSX-Redux is the preferred structured runtime evidence tool in this workflow.

## Replays/TAS and alternative emulator traces — Tier B/D

- BizHawk project/documentation: https://tasvideos.org/Bizhawk
- BizHawk source: https://github.com/TASEmulators/BizHawk
- BizHawk Lua functions: https://tasvideos.org/Bizhawk/LuaFunctions
- BizHawk rerecording/movie guidance: https://tasvideos.org/Bizhawk/Rerecording
- BizHawk PSX notes: https://tasvideos.org/Bizhawk/PSX
- DuckStation repository: https://github.com/stenzek/duckstation
- DuckStation command-line arguments: https://github.com/stenzek/duckstation/wiki/Command-Line-Arguments
- DuckStation logging: https://github.com/stenzek/duckstation/wiki/Logging

Use BizHawk for reproducible input-movie corpus creation and PCSX-Redux for detailed breakpoints/tracing. Use DuckStation CPU logs only in constrained windows.

## Ghidra PSX loaders, symbols, and signatures — Tier D

- Ghidra PSX loader: https://github.com/lab313ru/ghidra_psx_ldr
- PsyQ signature database: https://github.com/lab313ru/psx_psyq_signatures
- PsyQ format research/decompilation: https://github.com/grumpycoders/pcsx-redux/tree/main/src/mips/psyq
- `psx_mnd_sym` symbol parser: https://github.com/mefistotelis/psx_mnd_sym
- Ghidra documentation: https://www.ghidra-sre.org/Documentation.html
- Ghidra BSim documentation: https://ghidra.re/ghidra_docs/GhidraClass/BSim/BSimTutorial.html

Repository names/locations can move. Pin a commit when using signatures in a reproducible case.

## Matching decompilation tools — Tier D

- splat: https://github.com/ethteck/splat
- spimdisasm: https://github.com/Decompollaborate/spimdisasm
- maspsx: https://github.com/mkst/maspsx
- asm-differ: https://github.com/simonlindholm/asm-differ
- decomp-permuter: https://github.com/simonlindholm/decomp-permuter
- objdiff: https://github.com/encounter/objdiff
- decomp.me: https://decomp.me/
- mips2c: https://github.com/matt-kempster/m2c
- mkpsxiso: https://github.com/Lameguy64/mkpsxiso
- jPSXdec: https://github.com/m35/jpsxdec

## Representative PS1 decompilation/reversing projects — Tier D

Use these to study project structure and proven tool combinations, not to copy game-specific conclusions:

- Castlevania: Symphony of the Night matching decompilation: https://github.com/xeeynamo/sotn-decomp
- Chrono Cross matching decompilation: https://github.com/jdperos/chrono-cross-decomp
- Silent Hill matching decompilation: https://github.com/Vatuu/silent-hill-decomp
- Spyro 1 reverse-engineering workflow and symbol map: https://c0mposer.github.io/spyro1-reverse-engineering/html/
- PS1 project/tool index: https://decomp.wiki/platforms/playstation
- Program-reconstruction project index: https://decompilation.wiki/applications/program-reconstruction/

Project ownership and canonical URLs can change. Resolve redirects, pin a commit, and record the repository URL and commit in the case manifest before relying on project-specific tooling.

## Public PsyQ/Sony manual archive — Tier C, do not redistribute

Root indexes:

- Psy-Q archive root: https://psx.arthus.net/sdk/Psy-Q/
- Documentation root: https://psx.arthus.net/sdk/Psy-Q/DOCS/
- Developer references: https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/
- Technical notes: https://psx.arthus.net/sdk/Psy-Q/DOCS/TECHNOTE/
- FAQs: https://psx.arthus.net/sdk/Psy-Q/DOCS/FAQ/
- Training: https://psx.arthus.net/sdk/Psy-Q/DOCS/TRAINING/
- BBS documents: https://psx.arthus.net/sdk/Psy-Q/DOCS/BBS/
- Conference documents: https://psx.arthus.net/sdk/Psy-Q/DOCS/CONF/

### Top-level PsyQ 4.6/4.7 documents

- File formats 4.7: https://psx.arthus.net/sdk/Psy-Q/DOCS/FileFormat47.pdf
- Library overview 4.6: https://psx.arthus.net/sdk/Psy-Q/DOCS/LIBOVR46.PDF
- Library reference 4.6: https://psx.arthus.net/sdk/Psy-Q/DOCS/LIBREF46.PDF
- Library overview 4.7: https://psx.arthus.net/sdk/Psy-Q/DOCS/LibOver47.pdf
- Library reference 4.7: https://psx.arthus.net/sdk/Psy-Q/DOCS/LibRef47.pdf
- XA tutorial: https://psx.arthus.net/sdk/Psy-Q/DOCS/XATUT.pdf

### Developer-reference manuals

Base: `https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/`

- `3dgraph.pdf` — 3D graphics
- `Cdemul.pdf` — CD emulator
- `Cdgen.pdf` — CD generation
- `Dataconv.pdf` — data conversion tools
- `Dtlh2000.pdf` — DTL-H2000
- `Dtlh2500.pdf` — DTL-H2500
- `Filefrmt.pdf` — file formats
- `Hardware.pdf` — hardware summary
- `Inlinref.pdf` — inline/reference material
- `Libovr.pdf` — library overview
- `Libref.pdf` — library reference
- `PDAKern.pdf`, `pdahware.pdf`, `armref.pdf`, `os.pdf` — portable/device-related material in the archive
- `Sound20.pdf` — sound
- `Sprite.pdf` — sprites
- `sdevtc.pdf` — development toolchain
- `tech205.pdf`, `user205.pdf` — development hardware/tool manuals

### Technical notes

Base: `https://psx.arthus.net/sdk/Psy-Q/DOCS/TECHNOTE/`

- `Analog.pdf`
- `CALLBACK.PDF`
- `CDDROP.PDF`
- `CDSWITCH.PDF`
- `CHEKLIST.PDF`
- `DCACHE.PDF`
- `DECICONS.PDF`
- `Ds_servc.pdf`
- `FLASHBAT.PDF`
- `Glblreg.PDF`
- `Guncont.pdf`
- `H25BIOS.PDF`
- `Memcard.PDF`
- `NeGcon.pdf`
- `PSXCONS.PDF`
- `REVC.PDF`
- `SPURAM.PDF`
- `TECHCHK.PDF`
- `cdgenhlp.PDF`
- `epda1.pdf`, `epda2.pdf`
- `joystk.pdf`
- `mdecnote.pdf`
- `mtrc13.pdf`
- `note520.pdf`, `note588.pdf`
- `ordtbl.pdf`
- `palguide.pdf`
- `perfpapr.pdf`
- `pretech.pdf`
- `scee_dev.pdf`, `sceenews.pdf`
- `shft_jis.pdf`
- `sounread.pdf`

### FAQ documents

Base: `https://psx.arthus.net/sdk/Psy-Q/DOCS/FAQ/`

- `art5.pdf` — art
- `cd4.pdf` — CD
- `dev1.pdf` — development
- `doc11.pdf` — documentation
- `emul_faq.pdf` — emulator
- `gpu6.pdf` — GPU
- `gs8.pdf` — graphics system
- `gte7.pdf` — GTE
- `libpdfaq.pdf` — libraries
- `os2.pdf` — OS/kernel
- `psyq10.pdf` — PsyQ
- `sio9.pdf` — serial/controller I/O
- `snd3.pdf` — sound

### Training sets

- Winter 1995: https://psx.arthus.net/sdk/Psy-Q/DOCS/TRAINING/WINTER95/
- Fall 1996: https://psx.arthus.net/sdk/Psy-Q/DOCS/TRAINING/FALL96/
- Summer 1997: https://psx.arthus.net/sdk/Psy-Q/DOCS/TRAINING/Summer97/

Because directory indexes can change, archive the index URL and access date in the case manifest rather than copying every PDF into the skill.

## Additional development/debug tool references

- PSn00bSDK: https://github.com/Lameguy64/PSn00bSDK
- PSn00b Debugger: https://github.com/Lameguy64/PSn00bSDK/tree/master/tools
- no$psx / nocash docs: https://problemkaputt.de/psx.htm
- psx.dev community/documentation: https://www.psx.dev/

## How to use manuals in a case

1. Cite document title/version/page in the evidence note.
2. Distinguish architectural fact from SDK implementation convention.
3. Verify function prototypes against the exact library version/signature.
4. For confidential/proprietary mirrors, link and quote minimally; do not redistribute the document.
5. Prefer runtime proof when a game diverges from the manual or uses custom code.
6. Record `[INFERRED]` for conclusions that combine manual behavior with observed code rather than being stated directly.
