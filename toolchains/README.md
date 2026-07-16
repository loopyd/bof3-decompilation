# toolchains

Local SDKs and compilers live here. Generated/staged toolchains are untracked;
the pinned `psx_psyq_signatures/` Git submodule is the sole tracked exception.

PsyQ 4.7 headers are local build infrastructure. They remain separate from the
signature database and do not prove that the shipped game used that SDK.

The signature submodule is used only by the limited adapter:

```sh
git submodule update --init
bin/harness psyq scan --all
bin/harness psyq calls --all
```

It supplies object-signature evidence for Psy-Q versions 3.6–4.7. PsyQ 4.7
headers remain the build-facing declaration baseline.
