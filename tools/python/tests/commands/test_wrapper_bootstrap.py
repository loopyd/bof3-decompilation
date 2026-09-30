"""Contracts for bin/python-env, the retained adapters and bin/harness dispatch."""

from __future__ import annotations

import os
import sys
from pathlib import Path

from harness.common.commands import tokens as canonical_tokens
from subprocess_fixture import clean_env, run_command

ROOT = Path(__file__).resolve().parents[4]

#: Retained build/toolchain adapters invoked by absolute path or by name.
RETAINED_ADAPTERS = {
    "ar",
    "as",
    "cc",
    "ld",
    "maspsx",
    "nm",
    "objcopy",
    "objdump",
    "ranlib",
    "strip",
}
#: Sourced shared Python bootstrap (not a command).
SOURCED = {"python-env"}
#: The canonical command tree entry point.
ENTRY = {"harness"}
BIN_INVENTORY = RETAINED_ADAPTERS | SOURCED | ENTRY

SETUP_HINT = "run `just setup` first"
VENV_HINT = "run `just venv` first"
#: logical tool id -> (fixed project interpreter, missing status, hint).
#: Mirrors the retired per-tool entry points' `python_env` arguments; the
#: canonical argv comes from harness.common.commands.
PYTHON_POLICY = {
    "analysis-readiness": (False, 2, ""),
    "asm-diff": (False, 2, SETUP_HINT),
    "build": (False, 2, SETUP_HINT),
    "byte-match": (False, 2, SETUP_HINT),
    "combiner": (False, 2, ""),
    "companion-check": (False, 2, VENV_HINT),
    "compiler-variants": (True, 2, SETUP_HINT),
    "data-scan": (False, 2, ""),
    "decomp-audit": (False, 2, ""),
    "decomp-diagnose": (False, 2, ""),
    "decomp-status": (False, 2, SETUP_HINT),
    "emi-target": (False, 2, ""),
    "flag-search": (False, 2, ""),
    "index": (False, 2, ""),
    "m2c": (False, 2, SETUP_HINT),
    "m2ctx": (False, 2, SETUP_HINT),
    "macro-audit": (False, 2, ""),
    "naming-audit": (False, 2, ""),
    "naming-evidence-run": (False, 2, VENV_HINT),
    "next-lift": (False, 2, SETUP_HINT),
    "permute": (True, 1, "run `just venv` from {root}"),
    "plans": (False, 2, ""),
    "promote": (False, 2, SETUP_HINT),
    "psyq-calls": (False, 2, ""),
    "psyq-import": (False, 2, ""),
    "psyq-proposal": (False, 2, ""),
    "psyq-scan": (False, 2, ""),
    "rev-query": (False, 2, VENV_HINT),
    "rizin": (False, 2, SETUP_HINT),
    "rz-project": (False, 2, VENV_HINT),
    "scratchpad": (False, 2, ""),
    "spimdisasm": (False, 2, SETUP_HINT),
    "splat": (False, 2, SETUP_HINT),
    "str-media": (False, 2, ""),
    "symbols": (True, 2, SETUP_HINT),
    "type-audit": (False, 2, ""),
}


def _entry_repo(tmp_path: Path) -> Path:
    repo = tmp_path / "repo"
    (repo / "bin").mkdir(parents=True)
    (repo / "bin/python-env").write_bytes((ROOT / "bin/python-env").read_bytes())
    entry = repo / "bin/harness"
    entry.write_bytes((ROOT / "bin/harness").read_bytes())
    entry.chmod(0o755)
    return repo


def test_python_helper_exports_project_paths_and_safe_path() -> None:
    env = clean_env()
    env.pop("PYTHONPATH", None)
    result = run_command(
        "sh",
        "-c",
        f'ROOT="{ROOT}"; . "{ROOT}/bin/python-env"; python_env "{ROOT}/.venv/bin/python" 2 ""; '
        'printf "%s\\n%s\\n%s\\n" "$PYTHON" "$PYTHONPATH" "$PYTHONSAFEPATH"',
        env=env,
    )

    assert result.returncode == 0
    assert result.stdout.splitlines() == [
        str(ROOT / ".venv/bin/python"),
        str(ROOT / "tools/python"),
        "1",
    ]


def test_python_helper_replaces_poisoned_ambient_pythonpath(tmp_path: Path) -> None:
    poison = tmp_path / "poison"
    poison.mkdir()
    result = run_command(
        "sh",
        "-c",
        f'ROOT="{ROOT}"; . "{ROOT}/bin/python-env"; python_env "{ROOT}/.venv/bin/python" 2 ""; printf "%s\\n" "$PYTHONPATH"',
        env=clean_env() | {"PYTHONPATH": str(poison)},
    )
    assert result.returncode == 0
    assert result.stdout == f"{ROOT / 'tools/python'}\n"


def test_yaml_harness_and_maspsx_poison_cannot_shadow_trusted_modules(
    tmp_path: Path,
) -> None:
    poison = tmp_path / "poison"
    poison.mkdir()
    for name in ("yaml.py", "harness.py", "maspsx.py"):
        (poison / name).write_text("raise RuntimeError('ambient poison loaded')\n")
    env = clean_env() | {"PYTHONPATH": str(poison)}
    for name in ("harness", "maspsx"):
        result = run_command(str(ROOT / "bin" / name), "--help", env=env, cwd=tmp_path)
        assert result.returncode == 0, (name, result.stderr)
        assert "ambient poison loaded" not in result.stderr


def test_python_helper_private_arguments_ignore_ambient_configuration(
    tmp_path: Path,
) -> None:
    missing = tmp_path / "chosen-missing"
    env = clean_env() | {
        "PYTHON_ENV_PYTHON": str(ROOT / ".venv/bin/python"),
        "PYTHON_ENV_EXIT": "0",
        "PYTHON_ENV_HINT": "attacker hint",
    }
    result = run_command(
        "sh",
        "-c",
        f'. "{ROOT}/bin/python-env"; python_env "{missing}" 9 "trusted hint"',
        env=env,
    )

    assert (result.returncode, result.stdout, result.stderr) == (
        9,
        "",
        f"missing project Python environment: {missing}\ntrusted hint\n",
    )


def test_shared_entry_forwards_argv_stdout_stderr_and_exit(
    tmp_path: Path,
) -> None:
    python = tmp_path / "python"
    python.write_text(
        "#!/bin/sh\n"
        "printf 'argv:%s\\n' \"$*\"\n"
        "printf 'stderr:%s\\n' \"$3\" >&2\n"
        "exit 7\n",
        encoding="utf-8",
    )
    python.chmod(0o755)
    result = run_command(
        str(ROOT / "bin/harness"),
        "emi",
        "target",
        "one",
        "two words",
        env=clean_env() | {"PSX_PYTHON": str(python)},
        cwd=tmp_path,
    )

    assert result.returncode == 7
    assert result.stdout == ("argv:-m harness.cli emi target one two words\n")
    assert result.stderr == "stderr:emi\n"


def test_python_commands_preserve_missing_interpreter_contracts(
    tmp_path: Path,
) -> None:
    repo = _entry_repo(tmp_path)
    missing_override = tmp_path / "override-missing"
    for tool, (fixed, status, hint) in PYTHON_POLICY.items():
        env = clean_env() | {"PSX_PYTHON": str(missing_override)}
        result = run_command(
            "sh",
            str(repo / "bin/harness"),
            *canonical_tokens(tool),
            "--help",
            env=env,
            cwd=tmp_path,
        )
        missing = repo / ".venv/bin/python" if fixed else missing_override
        stderr = f"missing project Python environment: {missing}\n"
        if hint:
            stderr += hint.format(root=repo) + "\n"
        assert (result.returncode, result.stdout, result.stderr) == (
            status,
            "",
            stderr,
        ), tool


def test_inherited_private_overrides_cannot_change_command_failures(
    tmp_path: Path,
) -> None:
    repo = _entry_repo(tmp_path)
    attack = {
        "PYTHON_ENV_PYTHON": str(ROOT / ".venv/bin/python"),
        "PYTHON_ENV_EXIT": "0",
        "PYTHON_ENV_HINT": "attacker hint",
    }
    for tool, (_fixed, status, _hint) in PYTHON_POLICY.items():
        result = run_command(
            "sh",
            str(repo / "bin/harness"),
            *canonical_tokens(tool),
            env=clean_env() | attack,
            cwd=tmp_path,
        )
        assert result.returncode == status, tool
        assert "attacker hint" not in result.stderr, tool
        assert str(ROOT / ".venv/bin/python") not in result.stderr, tool


def _index_mode(name: str) -> str:
    result = run_command("git", "ls-files", "-s", "--", f"bin/{name}")
    assert result.returncode == 0, result.stderr
    if not result.stdout:
        return "100755" if os.access(ROOT / "bin" / name, os.X_OK) else "100644"
    return result.stdout.split("\t", 1)[0].split()[0]


def test_bin_inventory_is_only_the_entry_sourced_helper_and_adapters() -> None:
    result = run_command("git", "ls-files", "bin")
    assert result.returncode == 0, result.stderr
    deleted = run_command("git", "ls-files", "--deleted", "bin")
    assert deleted.returncode == 0, deleted.stderr
    removed = set(deleted.stdout.splitlines())
    tracked = sorted(
        line for line in result.stdout.splitlines() if line and line not in removed
    )
    tracked = sorted(
        tracked
        + [
            f"bin/{name}"
            for name in BIN_INVENTORY
            if (ROOT / "bin" / name).is_file() and f"bin/{name}" not in tracked
        ]
    )
    assert tracked == sorted(f"bin/{name}" for name in BIN_INVENTORY)
    # Superseded entry points are gone, not aliased.
    for name in ("build", "asm-diff", "symbols", "naming-audit"):
        assert not (ROOT / "bin" / name).exists(), name


def test_bin_index_modes_match_dispositions() -> None:
    for name in BIN_INVENTORY:
        mode = _index_mode(name)
        if name in SOURCED:
            assert mode == "100644", f"bin/{name} must not be executable in the index"
        else:
            assert mode == "100755", f"bin/{name} must be executable in the index"


def test_python_env_is_sourced_only_not_directly_executable() -> None:
    assert _index_mode("python-env") == "100644"
    result = run_command(
        "git",
        "grep",
        "-n",
        "bin/python-env",
        "--",
        "bin",
        ":!bin/python-env",
    )
    assert result.returncode == 0, result.stderr
    for line in result.stdout.splitlines():
        body = line.split(":", 2)[2]
        assert body.lstrip().startswith(". ") or "source " in body, (
            f"non-source reference to bin/python-env: {line}"
        )


def test_entry_help_outside_cwd_blocks_caller_packages(tmp_path: Path) -> None:
    for package, message in (
        ("harness", "CALLER CWD HARNESS IMPORTED"),
        ("maspsx", "CALLER CWD MASPSX IMPORTED"),
    ):
        fake = tmp_path / package
        fake.mkdir(exist_ok=True)
        (fake / "__init__.py").write_text(
            f"raise SystemExit('{message}')\n", encoding="utf-8"
        )
    result = run_command(
        str(ROOT / "bin/harness"), "--help", env=clean_env(), cwd=tmp_path
    )
    assert (result.returncode, result.stderr) == (0, "")
    assert result.stdout
    assert "CALLER CWD HARNESS IMPORTED" not in result.stderr
    assert "CALLER CWD MASPSX IMPORTED" not in result.stderr


def test_safe_path_blocks_caller_cwd_python_packages(tmp_path: Path) -> None:
    for package, message in (
        ("harness", "CALLER CWD HARNESS IMPORTED"),
        ("maspsx", "CALLER CWD MASPSX IMPORTED"),
    ):
        fake = tmp_path / package
        fake.mkdir()
        (fake / "__init__.py").write_text(
            f"raise SystemExit('{message}')\n", encoding="utf-8"
        )
    result = run_command(
        str(ROOT / "bin/harness"),
        "agent",
        "context",
        "worker",
        env=clean_env(),
        cwd=tmp_path,
    )
    assert result.returncode == 0, result.stderr
    assert "CALLER CWD HARNESS IMPORTED" not in result.stderr


def test_representative_dispatch_classes_forward_exact_argv(tmp_path: Path) -> None:
    python = tmp_path / "python"
    python.write_text("#!/bin/sh\nprintf '%s\\n' \"$@\"\n", encoding="utf-8")
    python.chmod(0o755)
    cases = {
        ("emi", "target"): [
            "-m",
            "harness.cli",
            "emi",
            "target",
            "one",
            "two words",
        ],
        ("lift", "asm-diff"): [
            "-m",
            "harness.cli",
            "lift",
            "asm-diff",
            "one",
            "two words",
        ],
    }
    for prefix, expected in cases.items():
        result = run_command(
            str(ROOT / "bin/harness"),
            *prefix,
            "one",
            "two words",
            env=clean_env() | {"PSX_PYTHON": str(python)},
            cwd=tmp_path,
        )
        assert (result.returncode, result.stdout.splitlines(), result.stderr) == (
            0,
            expected,
            "",
        ), prefix

    maspsx = run_command(
        str(ROOT / "bin/maspsx"),
        "one",
        "two words",
        env=clean_env() | {"PSX_PYTHON": str(python)},
        cwd=tmp_path,
    )
    assert (maspsx.returncode, maspsx.stdout.splitlines(), maspsx.stderr) == (
        0,
        [
            "-m",
            "harness.commands.tool",
            "maspsx",
            "--",
            "one",
            "two words",
        ],
        "",
    )


def test_agent_context_system_fallback_and_isolation_modes(tmp_path: Path) -> None:
    repo = tmp_path / "repo"
    (repo / "bin").mkdir(parents=True)
    (repo / "bin/python-env").write_bytes((ROOT / "bin/python-env").read_bytes())
    entry = repo / "bin/harness"
    entry.write_bytes((ROOT / "bin/harness").read_bytes())
    entry.chmod(0o755)
    (repo / "tools").mkdir()
    (repo / "tools/python").symlink_to(ROOT / "tools/python", target_is_directory=True)
    python_dir = tmp_path / "path"
    python_dir.mkdir()
    (python_dir / "python3").symlink_to(sys.executable)
    env = clean_env()
    env.pop("PSX_PYTHON", None)
    env["PATH"] = f"{python_dir}:{env['PATH']}"
    result = run_command(
        str(repo / "bin/harness"), "agent", "context", "worker", env=env, cwd=tmp_path
    )
    assert result.returncode == 0, result.stderr
    assert "context prefill contract" in result.stdout

    log = tmp_path / "argv"
    fake = tmp_path / "fake-python"
    fake.write_text(
        "#!/bin/sh\n"
        'if [ "$2" = -c ]; then printf agent-context-python; exit 0; fi\n'
        f"printf '%s\\n' \"$@\" > '{log}'\n",
        encoding="utf-8",
    )
    fake.chmod(0o755)
    for args, expected in (
        (("worker",), ["-S", "-m", "harness.cli", "agent", "context", "worker"]),
        (
            ("worker", "--mode=compatibility"),
            [
                "-m",
                "harness.cli",
                "agent",
                "context",
                "worker",
                "--mode=compatibility",
            ],
        ),
    ):
        result = run_command(
            str(ROOT / "bin/harness"),
            "agent",
            "context",
            *args,
            env=clean_env() | {"PSX_PYTHON": str(fake)},
            cwd=tmp_path,
        )
        assert result.returncode == 0, result.stderr
        assert log.read_text(encoding="utf-8").splitlines() == expected


def test_harness_command_tree_help_and_unknown_domain() -> None:
    help_result = run_command(str(ROOT / "bin/harness"), "--help")
    example_result = run_command(str(ROOT / "bin/harness"), "--example")
    rejected_result = run_command(str(ROOT / "bin/harness"), "bogus")

    assert help_result.returncode == 0
    assert "usage: bin/harness <domain> <action>" in help_result.stdout
    assert "lift" in help_result.stdout
    assert "naming" in help_result.stdout
    assert example_result.returncode == 0
    assert example_result.stdout.splitlines()
    assert rejected_result.returncode == 2
    assert rejected_result.stdout == ""
    assert "unknown domain" in rejected_result.stderr
