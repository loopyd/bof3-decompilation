from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]


def test_generic_caller_clobber_keeps_legacy_wrappers_direct() -> None:
    header = (ROOT / "include/base/barrier.h").read_text(encoding="utf-8")
    assert "#define CLOBBER_CALLER_REG(reg) CLOBBER_CALLER_REG_##reg()" in header
    for reg in ("a0", "a1", "a2", "a3", "v0", "v1", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9"):
        assert f'#define CLOBBER_CALLER_REG_{reg}() __asm__ __volatile__("" : : : "{reg}")' in header
    for reg in ("a0", "a1", "a2", "v0", "v1"):
        assert f'#define CLOBBER_{reg.upper()}() __asm__ __volatile__("" : : : "{reg}")' in header
