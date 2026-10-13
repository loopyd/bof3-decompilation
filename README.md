# Breath of Fire III decompilation

This project reconstructs the PlayStation version of *Breath of Fire III* as
readable C code. The goal is to rebuild the game's code and data so they match
the original game byte for byte, while documenting how the game works.

The work targets the US release. It covers the main executable and the modules
the game loads while running, including code for battles, menus, world areas,
audio and other systems. Reconstruction is ongoing.

## How it works

The original game binaries are the reference. We recover C89 source, compile it
with GCC 2.7.2-PSX and maspsx, and compare the result with the original bytes.
Emulator observations and analysis of the original code help establish what
each function does and which names and types are justified.

The game loads separate modules into memory, sometimes reusing the same
addresses. Each function therefore belongs to a specific game image as well as
an address. The [matching guide](docs/agents/matching.md) explains how to work on
a function and verify it against the right image.

## What you'll find

- Recovered game source in [src/bof3/](src/bof3/), with headers in
  [include/bof3/](include/bof3/) and image layouts and symbols in
  [config/targets/](config/targets/).
- Notes on the game's file formats, data and behavior in [docs/specs/](docs/specs/).
- Tools for code analysis, decompilation, compilation and byte comparison,
  together with tools for inspecting and editing game text and audio.
- [PCSX-Redux Agentic](third_party/pcsx-redux-agentic/README.md), our Linux
  PlayStation emulator fork, for debugging the running game and capturing
  memory, save states, screenshots and audio.

## Prerequisites

Use Linux on x86_64, as required by the managed historical compiler binaries.
You also need your own licensed US game disc image as a complete CUE/BIN set.
The first setup needs internet access to download toolchains and dependencies.

<details>
<summary>Build tools and native dependencies</summary>

- Install Git, `just`, `uv`, Python 3.12 or newer, and Rust with Cargo.
- Install a host C compiler and a C++23 compiler, Make, CMake 3.20 or newer,
  Meson 0.58.0 or newer, and Ninja 1.8.2 or newer.
- The PSYQo toolchain build needs `wget`, `bzip2`, `xz`, `bison` and `flex`.
- The RetDec build needs `autoconf`, `automake`, `libtoolize`, `pkg-config`,
  and the OpenSSL and zlib development packages.
- Install `7z` for the PsyQ archives and compressed game inputs.
- Install the native development packages listed in the
  [emulator build guide](third_party/pcsx-redux-agentic/README.md#build-locally).

</details>

Emulator missions also need a PlayStation BIOS. The
[emulator integration guide](docs/agents/pcsx-redux-agentic.md) covers its inputs
and setup.

## Getting started

Place your CUE/BIN files under `inputs/external/`. Setup also accepts
`BreathOfFireIIIv1.1.7z` there if you have not extracted the disc image. See
[local game inputs](inputs/README.md) for details.

From the repository root, run:

```sh
just setup
just doctor
```

Setup initializes submodules, installs the managed toolchains, builds local
tools and prepares the game images. Doctor checks the existing installation.
The [toolchain guide](toolchains/README.md) describes the compilers and SDKs;
the [tool usage guide](docs/agents/tool-usage.md#ordered-workflow) covers the
workflow and setup of individual components.

To explore the available commands and inspect matching status, run:

```sh
bin/harness --help
bin/harness lift status
```

## Learn more and contribute

Start with the [documentation index](docs/INDEX.md). The
[project context](docs/agents/project-context.md) explains how source files and
game images relate. The [matching playbook](docs/agents/matching-playbook.md)
helps diagnose differences between reconstructed code and the original.

Read [CONTRIBUTING.md](CONTRIBUTING.md) before submitting changes. Contributions
can include matching C code, evidence for names and types, game-format research,
documentation and improvements to the analysis tools. Keep game images and
generated build files out of commits.
