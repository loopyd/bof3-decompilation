from __future__ import annotations

from ....inventory.repositories.programs import ProgramRepository


def capture_program_selector(connection, *, program_path: str) -> str:
    return ProgramRepository(connection).resolve_program_selector(
        program_path=program_path
    )
