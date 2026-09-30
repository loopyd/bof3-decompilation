# Compiler pipeline validation

Required when changes can affect invoked compiler/assembler/linker, including:

- `config/compiler/variants.json`, `config/compiler/object-flags.cmake`, and `BOF3_OBJCOMPILER_` selection;
- `bin/cc`, maspsx, `bin/as`, linker adapters/toolchains, and their argument/path handling;
- compiler registry membership, compiler path/version selection, setup/discovery, and wrapper bootstraps that affect invocation.

Managed compiler/toolchain lifecycle remains owned by the toolchain base/registry; setup, doctor, and tool dispatch are clients. Flat compiler/linker adapters remain POSIX commands. Do not duplicate lifecycle ownership while changing pipeline selection.

Run in order:

```sh
python -m pytest tools/python/tests/build/test_bin_cc_pipeline.py -v
python -m pytest tools/python/tests/build/test_asm_link.py -v
```

Then live normal asm-diff and byte-match every affected authored lift. Source-only
lifts exempt from this pipeline-change protocol. Wrapper tests alone never accept
pipeline changes; record affected compiler/flags/path, each live selector result,
and unsupported-host blockers.
