from __future__ import annotations

from .base import SubmoduleToolchain


class M2cToolchain(SubmoduleToolchain):
    label = "m2c"
    submodule = "third_party/m2c"
    command = ("third_party/m2c/m2c.py", "--help")
