from pathlib import Path

import pytest

from harness.domain.policy import validate_matching_source, validate_matching_text

ROOT = Path(__file__).resolve().parents[4]


def test_caller_clobbers_are_forbidden() -> None:
    validate_matching_source(ROOT, ROOT / "include/base/compiler.h")
    for register in ("a0", "a1", "a2", "a3", "v0", "v1", "t0", "t9"):
        for spelling in (
            f"CLOBBER_CALLER_REG({register});",
            f"CLOBBER_{register.upper()}();",
            f'__asm__ __volatile__("" : : : "{register}");',
            f'__asm__ __volatile__("nop" : : : "{register}");',
        ):
            with pytest.raises(ValueError, match="forbidden"):
                validate_matching_text(spelling)
    with pytest.raises(ValueError, match="forbidden"):
        validate_matching_text("barrier();")


def test_register_pins_are_forbidden(tmp_path: Path) -> None:
    for spelling in (
        'REGISTER_PIN(int, value, "v0");',
        'register int value asm("v0");',
        'register int value __asm__("$2");',
        'register int value asm("$" "2");',
        "#define REGISTER_PIN(type, name, reg) register type name",
        "#define REGISTER_PIN(type, name, reg)",
        "#define ALIAS __asm__",
    ):
        with pytest.raises(ValueError, match="forbidden"):
            validate_matching_text(spelling)
    validate_matching_text('/* REGISTER_PIN */ const char *note = "barrier()";')
    validate_matching_source(ROOT, ROOT / "include/bof3/symbols.h")
    source = tmp_path / "src/lift.c"
    template = tmp_path / "src/shared/panel.inc"
    template.parent.mkdir(parents=True)
    source.write_text('#include "shared/panel.inc"\n')
    template.write_text('REGISTER_PIN(int, value, "v0");\n')
    with pytest.raises(ValueError, match="forbidden"):
        validate_matching_source(tmp_path, source)
