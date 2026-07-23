# toolchains

Local SDKs and compilers live here. Generated/staged toolchains are untracked;
the pinned `psx_psyq_signatures/` Git submodule is the sole tracked exception.

`just setup` downloads the official PsyQ 4.7 Runtime Library ZIP for headers
and `.LIB` archives, then stages it under `psyq/4.7/`. It also downloads a
converted per-object form because reviewed target manifests name individual
`.o` members for signature evidence. Both forms are local build infrastructure,
remain separate from the signature database, and do not prove that the shipped
game used that SDK.

The signature submodule is used only by the limited adapter:

```sh
git submodule update --init
bin/harness psyq scan --all
bin/harness psyq calls --all
```

It supplies object-signature evidence for Psy-Q versions 3.6–4.7. PsyQ 4.7
headers remain the build-facing declaration baseline.
