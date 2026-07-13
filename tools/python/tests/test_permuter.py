from __future__ import annotations

import pytest

from harness.workflows.permuter import (
    _original_target_assembly,
    _repair_psyq_register_parameters,
)


def test_repairs_psyq_fp_parameter_without_touching_asm_strings() -> None:
    source = 'CdlFILE *CdSearchFile(CdlFILE *$30, char *name);\nasm("mtc2 $12,$30");\n'

    repaired = _repair_psyq_register_parameters(source)

    assert "CdlFILE *fp" in repaired
    assert 'asm("mtc2 $12,$30")' in repaired


def test_original_target_assembly_uses_authoritative_little_endian_words() -> None:
    assembly = _original_target_assembly("func_801625e4", bytes.fromhex("e8ffbd27"))

    assert ".globl func_801625e4" in assembly
    assert ".word 0x27bdffe8" in assembly


def test_original_target_assembly_rejects_partial_instruction() -> None:
    with pytest.raises(ValueError, match="word-aligned"):
        _original_target_assembly("func_801625e4", b"abc")
