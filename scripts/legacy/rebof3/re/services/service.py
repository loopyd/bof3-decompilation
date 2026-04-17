from __future__ import annotations

from abc import ABC


class Service(ABC):
    service_name = "service"

    @property
    def name(self) -> str:
        return self.service_name
