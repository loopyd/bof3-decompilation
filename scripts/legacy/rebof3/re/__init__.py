from __future__ import annotations

import importlib
import sys
from types import ModuleType


_MODULE_ALIASES = {
    "bootstrap": ".services.bootstrap",
    "doctor": ".services.doctor",
    "ghidra_decomp": ".services.ghidra",
    "ghidra_helpers": ".services.ghidra",
    "metadata": ".services.metadata",
}


def _load_module(name: str) -> ModuleType:
    module = importlib.import_module(_MODULE_ALIASES[name], __name__)
    sys.modules[f"{__name__}.{name}"] = module
    return module


def __getattr__(name: str) -> ModuleType:
    if name in _MODULE_ALIASES:
        return _load_module(name)
    raise AttributeError(f"module {__name__!r} has no attribute {name!r}")


def __dir__() -> list[str]:
    return sorted(list(globals()) + list(_MODULE_ALIASES))


__all__ = sorted(_MODULE_ALIASES)
