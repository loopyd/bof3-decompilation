from __future__ import annotations

from abc import ABC, abstractmethod
from typing import Any


class Installer(ABC):
    installer_name = "installer"

    @property
    def name(self) -> str:
        return self.installer_name

    @abstractmethod
    def install(self, request: Any, *, logger) -> int:
        raise NotImplementedError
