from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True, slots=True)
class DoctorRequest:
    json_output: bool = False
    strict: bool = False
    quiet: bool = False
