"""Hermetic contract tests for the ``bin/cc`` GCC → maspsx → assembler path."""

from __future__ import annotations

import os
import shutil
import subprocess
from pathlib import Path

import pytest


_ROOT = Path(__file__).resolve().parents[3]
_CC = _ROOT / "bin" / "cc"
_LIVE_SELECTOR = "emi/world00/area030/04@0x801DAE3C"


def _stub(path: Path, label: str, body: str) -> Path:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        f'#!/bin/sh\nset -eu\nprintf \'%s\\n\' "{label}:$*" >> "$STUB_LOG"\n{body}\n'
    )
    path.chmod(0o755)
    return path


def _pipeline(tmp_path: Path, *, explicit_gcc: bool, expand_div: bool) -> list[str]:
    """Run a copied driver so its canonical default is a fixture-local stub."""
    root = tmp_path / "repo"
    (root / "bin").mkdir(parents=True)
    shutil.copy2(_CC, root / "bin" / "cc")
    log = root / "pipeline.log"
    gcc_body = """
out=""
previous=""
for arg in "$@"; do
    [ "$previous" = "-o" ] && out="$arg"
    previous="$arg"
done
cat > "$out" <<'EOF'
.text
.globl f
f:
    jr $ra
    nop
EOF
"""
    _stub(root / "toolchains" / "gcc-2.7.2-psx" / "gcc", "canonical-gcc", gcc_body)
    _stub(root / "third_party" / "maspsx" / "maspsx.py", "maspsx", "cat")
    assembler = _stub(
        root / "stubs" / "as",
        "assembler",
        """
out=""
previous=""
for arg in "$@"; do
    [ "$previous" = "-o" ] && out="$arg"
    previous="$arg"
done
: > "$out"
""",
    )
    explicit = _stub(root / "stubs" / "gcc", "explicit-gcc", gcc_body)
    python = _stub(
        root / "stubs" / "python",
        "python",
        '[ "$1" = -P ]\nshift\nexec sh "$@"',
    )

    source = root / "source.c"
    output = root / "source.o"
    source.write_text("int f(void) { return 0; }\n")
    env = {
        **os.environ,
        "STUB_LOG": str(log),
        "PSX_AS": str(assembler),
        "MASPSX_PYTHON": str(python),
    }
    if explicit_gcc:
        env["PSX_GCC"] = str(explicit)
    args = ["sh", str(root / "bin" / "cc"), "-c", "-Wa,-G0,-EL,-mips1"]
    if expand_div:
        args.append("-Wa,--expand-div")
    args.extend([str(source), "-o", str(output)])
    result = subprocess.run(args, cwd=root, env=env, text=True, capture_output=True)
    assert result.returncode == 0, result.stderr
    assert output.is_file()
    return log.read_text().splitlines()


@pytest.mark.parametrize(
    ("explicit_gcc", "gcc_label"), [(False, "canonical-gcc:"), (True, "explicit-gcc:")]
)
def test_gcc_selection_still_uses_maspsx_and_assembler(
    tmp_path: Path, explicit_gcc: bool, gcc_label: str
) -> None:
    lines = _pipeline(tmp_path, explicit_gcc=explicit_gcc, expand_div=False)
    gcc = next(line for line in lines if line.startswith(gcc_label))
    maspsx = next(line for line in lines if line.startswith("maspsx:"))
    assembler = next(line for line in lines if line.startswith("assembler:"))

    assert " -S " in f" {gcc} "
    assert "--aspsx-version=2.56" in maspsx
    assembler_args = assembler.removeprefix("assembler:").split()
    assert "-G0" in assembler_args
    assert "-EL" in assembler_args
    assert "-mips1" in assembler_args


def test_expand_div_is_maspsx_only(tmp_path: Path) -> None:
    lines = _pipeline(tmp_path, explicit_gcc=False, expand_div=True)
    maspsx = next(line for line in lines if line.startswith("maspsx:"))
    assembler = next(line for line in lines if line.startswith("assembler:"))

    assert "--expand-div" in maspsx
    assert "--expand-div" not in assembler


def test_live_maspsx_rejects_inherited_caller_python_paths(tmp_path: Path) -> None:
    caller = tmp_path / "caller"
    caller.mkdir()
    for package in ("maspsx", "harness"):
        fake = caller / package
        fake.mkdir()
        (fake / "__init__.py").write_text(
            f'raise SystemExit("CALLER CWD {package.upper()} IMPORTED")\n'
        )

    log = tmp_path / "pipeline.log"
    gcc = _stub(
        tmp_path / "gcc",
        "gcc",
        """
out=""
previous=""
for arg in "$@"; do
    [ "$previous" = "-o" ] && out="$arg"
    previous="$arg"
done
cat > "$out" <<'EOF'
.text
.globl f
f:
    jr $ra
    nop
EOF
""",
    )
    assembler = _stub(
        tmp_path / "as",
        "assembler",
        """
out=""
previous=""
for arg in "$@"; do
    [ "$previous" = "-o" ] && out="$arg"
    previous="$arg"
done
: > "$out"
""",
    )
    source = tmp_path / "source.c"
    output = tmp_path / "source.o"
    source.write_text("int f(void) { return 0; }\n")
    env = {
        **os.environ,
        "STUB_LOG": str(log),
        "PSX_GCC": str(gcc),
        "PSX_AS": str(assembler),
        "MASPSX_PYTHON": shutil.which("python3") or "python3",
        "PYTHONPATH": str(caller),
        "PYTHONSAFEPATH": "inherited-caller-value",
    }

    result = subprocess.run(
        [str(_CC), "-c", str(source), "-o", str(output)],
        cwd=caller,
        env=env,
        text=True,
        capture_output=True,
    )

    assert result.returncode == 0, result.stderr
    assert output.is_file()
    assert "CALLER CWD" not in result.stdout + result.stderr


def test_live_asm_diff_rejects_inherited_caller_harness(tmp_path: Path) -> None:
    caller = tmp_path / "caller"
    fake = caller / "harness"
    fake.mkdir(parents=True)
    (fake / "__init__.py").write_text(
        'raise SystemExit("CALLER CWD HARNESS IMPORTED")\n'
    )
    result = subprocess.run(
        [str(_ROOT / "bin" / "asm-diff"), "--json", _LIVE_SELECTOR],
        cwd=caller,
        env={
            **os.environ,
            "PYTHONPATH": str(caller),
            "PYTHONSAFEPATH": "inherited-caller-value",
        },
        text=True,
        capture_output=True,
    )

    assert result.returncode in (0, 1), result.stderr
    assert "CALLER CWD HARNESS IMPORTED" not in result.stdout + result.stderr
