from .base import PythonSubmoduleToolchain


class SplatToolchain(PythonSubmoduleToolchain):
    label = "splat"
    submodule = "third_party/splat"
    install_target = "third_party/splat[mips]"

    @property
    def executable(self) -> Path:
        return self.root / ".venv" / "bin" / "splat"
