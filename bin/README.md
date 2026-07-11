# Commands

`rebof3` is the BOF3 command surface:

```sh
rebof3 scan
rebof3 status [target]
rebof3 candidates [family]
rebof3 promote <archive#slot> --confirm-code
rebof3 next [target]
rebof3 lift <target@address>
rebof3 diff <source>
rebof3 ghidra sync
rebof3 assets list
rebof3 disk verify|rebuild
```

The remaining scripts are narrow tool adapters used by setup and extraction:
disc/EMI operations, Ghidra invocation, toolchain setup, and CMake build.
They are not alternate orchestration workflows.
