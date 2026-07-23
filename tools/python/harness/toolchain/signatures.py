from __future__ import annotations

from .base import SubmoduleToolchain


class PsyqSignaturesToolchain(SubmoduleToolchain):
    label = "PsyQ signatures"
    submodule = "toolchains/psx_psyq_signatures"
    command = ("bin/symbols", "--help")

    def verify(self) -> str:
        if not (self.source / ".git").exists() or not any(self.source.iterdir()):
            raise FileNotFoundError(f"missing signature submodule: {self.source}")
        return self.label
