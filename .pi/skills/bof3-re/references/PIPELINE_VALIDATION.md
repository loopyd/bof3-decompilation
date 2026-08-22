# Compiler pipeline validation

Use this contract for any change capable of changing the invoked compiler, assembler, or linker, including:

- `config/compiler/variants.json`, `config/compiler/object-flags.cmake`, and `BOF3_OBJCOMPILER_` selection;
- `bin/cc`, maspsx, `bin/as`, linker adapters/toolchains, and their argument/path handling;
- compiler registry membership, compiler path/version selection, setup/discovery, and wrapper bootstraps that affect invocation.

Managed compiler/toolchain lifecycle remains owned by the toolchain base/registry; setup, doctor, and tool dispatch are clients. Flat compiler/linker adapters remain POSIX commands. Do not duplicate lifecycle ownership while changing pipeline selection.

Run in order:

```sh
python -m pytest tools/python/tests/test_bin_cc_pipeline.py -v
python -m pytest tools/python/tests/test_asm_link.py -v
```

Then run live normal asm-diff and byte-match checks for every affected authored lift. Source-only lifts are exempt. A pipeline change is not accepted from wrapper tests alone; record the affected compiler/flags/path, each live selector check, and any unsupported-host blocker.
