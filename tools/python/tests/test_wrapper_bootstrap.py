"""Contract tests for bin/python-env and converted wrappers."""

from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]

# Post-Phase-4 retained bin/ inventory derived from the plan's baseline table:
# "executable" = retained directly executable wrapper (index mode 100755),
# "sourced" = bin/python-env shared bootstrap (index mode 100644),
# "retired" = scheduled for deletion (mode needs no normalization).
BIN_DISPOSITIONS = {
    "agent-context": "executable",
    "analysis-readiness": "executable",
    "ar": "executable",
    "as": "executable",
    "asm-diff": "executable",
    "bof3-disk": "executable",
    "build": "executable",
    "byte-match": "executable",
    "cc": "executable",
    "companion-check": "executable",
    "compiler-variants": "executable",
    "data-scan": "executable",
    "decomp-status": "executable",
    "emi-ex": "executable",
    "emi-target": "executable",
    "flag-search": "executable",
    "harness": "executable",
    "index": "executable",
    "ld": "executable",
    "m2c": "executable",
    "m2ctx": "executable",
    "macro-audit": "executable",
    "maspsx": "executable",
    "naming-audit": "executable",
    "nm": "executable",
    "objcopy": "executable",
    "objdump": "executable",
    "package-psx-audio": "executable",
    "permute": "executable",
    "promote": "executable",
    "psx-audio": "executable",
    "psyq-import": "executable",
    "python-env": "sourced",
    "ranlib": "executable",
    "rev-query": "executable",
    "rizin": "executable",
    "rz-project": "executable",
    "scratchpad": "executable",
    "spimdisasm": "executable",
    "splat": "executable",
    "str-media": "executable",
    "strip": "executable",
    "symbols": "executable",
    "type-audit": "executable",
}
WRAPPER_MATRIX = {
    "agent-context": ("standalone", "harness.commands.agent_context"),
    "analysis-readiness": ("python-env", "harness.commands.analysis_readiness"),
    "ar": ("flat", "PSX_AR"),
    "as": ("flat", "PSX_AS"),
    "asm-diff": ("python-env", "harness.commands.lift"),
    "bof3-disk": ("native", "PSX_BOF3_DISK"),
    "build": ("python-env", "harness.commands.build"),
    "byte-match": ("python-env", "harness.commands.lift"),
    "cc": ("flat", "GCC -> maspsx -> PSX_AS"),
    "companion-check": ("python-env", "harness.commands.companion_check"),
    "compiler-variants": ("python-env-fixed", "harness.commands.compiler_variants"),
    "data-scan": ("python-env", "harness.commands.data_scan"),
    "decomp-status": ("python-env", "harness.commands.decomp_status"),
    "emi-ex": ("native", "PSX_EMI_EX"),
    "emi-target": ("python-env", "harness.commands.emi_target"),
    "flag-search": ("python-env", "harness.commands.flag_search"),
    "harness": ("python-env", "harness.commands.psyq"),
    "index": ("python-env", "harness.commands.index"),
    "ld": ("flat", "PSX_LD"),
    "m2c": ("python-env", "harness.commands.lift"),
    "m2ctx": ("python-env", "harness.commands.lift"),
    "macro-audit": ("python-env", "harness.commands.macro_audit"),
    "maspsx": ("python-env", "harness.commands.tool"),
    "naming-audit": ("python-env", "harness.commands.naming_audit"),
    "nm": ("flat", "PSX_NM"),
    "objcopy": ("flat", "PSX_OBJCOPY"),
    "objdump": ("flat", "PSX_OBJDUMP"),
    "package-psx-audio": ("shell", "source packager"),
    "permute": ("python-env-fixed", "harness.commands.permute"),
    "promote": ("python-env", "harness.commands.lift"),
    "psx-audio": ("native", "ignored CMake build/bof3-audio"),
    "psyq-import": ("python-env", "harness.commands.psyq_import"),
    "python-env": ("sourced", "shared Python bootstrap"),
    "ranlib": ("flat", "PSX_RANLIB"),
    "rev-query": ("python-env", "harness.commands.rev_query"),
    "rizin": ("python-env", "harness.commands.tool"),
    "rz-project": ("python-env", "harness.commands.rz_project"),
    "scratchpad": ("python-env", "harness.commands.scratchpad"),
    "spimdisasm": ("python-env", "harness.commands.tool"),
    "splat": ("python-env", "harness.commands.splat"),
    "str-media": ("python-env", "harness.commands.str_media"),
    "strip": ("flat", "PSX_STRIP"),
    "symbols": ("python-env-fixed", "harness.commands.symbols"),
    "type-audit": ("python-env", "harness.commands.type_audit"),
}
PYTHON_WRAPPERS = tuple(
    name
    for name, (bootstrap, _) in WRAPPER_MATRIX.items()
    if bootstrap.startswith("python")
)
MISSING_HINTS = {
    "asm-diff": "run `just setup` first",
    "build": "run `just setup` first",
    "byte-match": "run `just setup` first",
    "companion-check": "run `just venv` first",
    "compiler-variants": "run `just setup` first",
    "decomp-status": "run `just setup` first",
    "m2c": "run `just setup` first",
    "m2ctx": "run `just setup` first",
    "maspsx": "run `just setup` first",
    "permute": "run `just venv` from {root}",
    "promote": "run `just setup` first",
    "rev-query": "run `just venv` first",
    "rizin": "run `just setup` first",
    "rz-project": "run `just venv` first",
    "spimdisasm": "run `just setup` first",
    "splat": "run `just setup` first",
    "symbols": "run `just setup` first",
}
FIXED_PYTHON_WRAPPERS = {"compiler-variants", "permute", "symbols"}


def _run(
    *args: str,
    env: dict[str, str] | None = None,
    cwd: Path = ROOT,
) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        args,
        cwd=cwd,
        capture_output=True,
        text=True,
        env=env,
        check=False,
    )


def _clean_env() -> dict[str, str]:
    return {
        key: value
        for key, value in os.environ.items()
        if key
        not in {
            "PSX_PYTHON",
            "PYTHON_ENV_EXIT",
            "PYTHON_ENV_HINT",
            "PYTHON_ENV_PYTHON",
        }
    }


def test_python_helper_exports_project_paths_and_safe_path() -> None:
    env = _clean_env()
    env.pop("PYTHONPATH", None)
    result = _run(
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
    result = _run(
        "sh",
        "-c",
        f'ROOT="{ROOT}"; . "{ROOT}/bin/python-env"; python_env "{ROOT}/.venv/bin/python" 2 ""; printf "%s\\n" "$PYTHONPATH"',
        env=_clean_env() | {"PYTHONPATH": str(poison)},
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
    env = _clean_env() | {"PYTHONPATH": str(poison)}
    for wrapper in ("splat", "harness", "maspsx"):
        result = _run(str(ROOT / "bin" / wrapper), "--help", env=env, cwd=tmp_path)
        assert result.returncode == 0, (wrapper, result.stderr)
        assert "ambient poison loaded" not in result.stderr


def test_python_helper_private_arguments_ignore_ambient_configuration(
    tmp_path: Path,
) -> None:
    missing = tmp_path / "chosen-missing"
    env = _clean_env() | {
        "PYTHON_ENV_PYTHON": str(ROOT / ".venv/bin/python"),
        "PYTHON_ENV_EXIT": "0",
        "PYTHON_ENV_HINT": "attacker hint",
    }
    result = _run(
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


def test_all_python_wrappers_help_from_outside_cwd(tmp_path: Path) -> None:
    assert len(PYTHON_WRAPPERS) == 29
    for name in PYTHON_WRAPPERS:
        result = _run(
            str(ROOT / "bin" / name), "--help", env=_clean_env(), cwd=tmp_path
        )
        assert (result.returncode, result.stderr) == (0, ""), name
        assert result.stdout, name


def test_shared_wrapper_forwards_argv_stdout_stderr_and_exit(
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
    result = _run(
        str(ROOT / "bin/emi-target"),
        "one",
        "two words",
        env=_clean_env() | {"PSX_PYTHON": str(python)},
        cwd=tmp_path,
    )

    assert result.returncode == 7
    assert result.stdout == "argv:-m harness.commands.emi_target one two words\n"
    assert result.stderr == "stderr:one\n"


def test_all_python_wrappers_preserve_missing_interpreter_contracts(
    tmp_path: Path,
) -> None:
    repo = tmp_path / "repo"
    (repo / "bin").mkdir(parents=True)
    (repo / "bin/python-env").write_bytes((ROOT / "bin/python-env").read_bytes())
    missing_override = tmp_path / "override-missing"
    for name in PYTHON_WRAPPERS:
        wrapper = repo / "bin" / name
        wrapper.write_bytes((ROOT / "bin" / name).read_bytes())
        env = _clean_env() | {"PSX_PYTHON": str(missing_override)}
        result = _run("sh", str(wrapper), "--help", env=env, cwd=tmp_path)
        fixed = name in FIXED_PYTHON_WRAPPERS
        missing = repo / ".venv/bin/python" if fixed else missing_override
        status = 1 if name == "permute" else 2
        hint = MISSING_HINTS.get(name, "").format(root=repo)
        stderr = f"missing project Python environment: {missing}\n"
        if hint:
            stderr += f"{hint}\n"
        assert (result.returncode, result.stdout, result.stderr) == (
            status,
            "",
            stderr,
        ), name


def test_inherited_private_overrides_cannot_change_wrapper_failures(
    tmp_path: Path,
) -> None:
    repo = tmp_path / "repo"
    (repo / "bin").mkdir(parents=True)
    (repo / "bin/python-env").write_bytes((ROOT / "bin/python-env").read_bytes())
    attack = {
        "PYTHON_ENV_PYTHON": str(ROOT / ".venv/bin/python"),
        "PYTHON_ENV_EXIT": "0",
        "PYTHON_ENV_HINT": "attacker hint",
    }
    for name in PYTHON_WRAPPERS:
        wrapper = repo / "bin" / name
        wrapper.write_bytes((ROOT / "bin" / name).read_bytes())
        result = _run("sh", str(wrapper), env=_clean_env() | attack, cwd=tmp_path)
        assert result.returncode == (1 if name == "permute" else 2), name
        assert "attacker hint" not in result.stderr, name
        assert str(ROOT / ".venv/bin/python") not in result.stderr, name


def _index_mode(name: str) -> str:
    result = _run("git", "ls-files", "-s", "--", f"bin/{name}")
    assert result.returncode == 0, result.stderr
    return result.stdout.split("\t", 1)[0].split()[0]


def test_bin_inventory_matches_disposition_table() -> None:
    result = _run("git", "ls-files", "bin")
    assert result.returncode == 0, result.stderr
    deleted = _run("git", "ls-files", "--deleted", "bin")
    assert deleted.returncode == 0, deleted.stderr
    removed = set(deleted.stdout.splitlines())
    tracked = sorted(
        line for line in result.stdout.splitlines() if line and line not in removed
    )
    expected = sorted(f"bin/{name}" for name in BIN_DISPOSITIONS)
    assert tracked == expected
    assert set(WRAPPER_MATRIX) == set(BIN_DISPOSITIONS)
    assert len(WRAPPER_MATRIX) == 44


def test_all_shell_wrappers_parse_and_use_the_characterized_bootstrap() -> None:
    for name, (bootstrap, _owner) in WRAPPER_MATRIX.items():
        wrapper = ROOT / "bin" / name
        if bootstrap == "retired" or wrapper.read_bytes().startswith(b"\x7fELF"):
            continue
        result = _run("sh", "-n", str(wrapper))
        assert result.returncode == 0, f"bin/{name}: {result.stderr}"
        text = wrapper.read_text(encoding="utf-8")
        if bootstrap.startswith("python-env"):
            assert '. "$ROOT/bin/python-env"' in text, name
            assert "python_env " in text, name
        else:
            assert '. "$ROOT/bin/python-env"' not in text, name
        if _owner.startswith("harness.commands"):
            assert _owner in text, name


def test_bin_index_modes_match_dispositions() -> None:
    for name, disposition in BIN_DISPOSITIONS.items():
        mode = _index_mode(name)
        if disposition == "executable":
            assert mode == "100755", f"bin/{name} must be executable in the index"
        elif disposition == "sourced":
            assert mode == "100644", f"bin/{name} must not be executable in the index"
        # retired files need no normalization; any tracked mode is accepted


def test_python_env_is_sourced_only_not_directly_executable() -> None:
    # A 100644 index mode checks out non-executable on fresh clones, so no
    # consumer can depend on direct execution; every tracked consumer sources
    # it (`. "$ROOT/bin/python-env"`).
    assert _index_mode("python-env") == "100644"
    result = _run(
        "git",
        "grep",
        "-n",
        "bin/python-env",
        "--",
        ":!bin/python-env",
        ":!tools/python/tests/test_wrapper_bootstrap.py",
    )
    assert result.returncode == 0, result.stderr
    for line in result.stdout.splitlines():
        body = line.split(":", 2)[2]
        assert body.lstrip().startswith(". ") or "source " in body, (
            f"non-source reference to bin/python-env: {line}"
        )


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
    for name in PYTHON_WRAPPERS:
        result = _run(
            str(ROOT / "bin" / name), "--help", env=_clean_env(), cwd=tmp_path
        )
        assert result.returncode == 0, name
        assert "CALLER CWD HARNESS IMPORTED" not in result.stderr, name
        assert "CALLER CWD MASPSX IMPORTED" not in result.stderr, name

    result = _run(
        str(ROOT / "bin/agent-context"), "worker", env=_clean_env(), cwd=tmp_path
    )
    assert result.returncode == 0, result.stderr
    assert "CALLER CWD HARNESS IMPORTED" not in result.stderr


def test_representative_wrapper_classes_forward_exact_argv(tmp_path: Path) -> None:
    python = tmp_path / "python"
    python.write_text("#!/bin/sh\nprintf '%s\\n' \"$@\"\n", encoding="utf-8")
    python.chmod(0o755)
    cases = {
        "emi-target": ["-m", "harness.commands.emi_target", "one", "two words"],
        "asm-diff": [
            "-c",
            "from harness.commands.lift import main; raise SystemExit(main('asm-diff'))",
            "one",
            "two words",
        ],
        "maspsx": [
            "-m",
            "harness.commands.tool",
            "maspsx",
            "--",
            "one",
            "two words",
        ],
    }
    for name, expected in cases.items():
        result = _run(
            str(ROOT / "bin" / name),
            "one",
            "two words",
            env=_clean_env() | {"PSX_PYTHON": str(python)},
            cwd=tmp_path,
        )
        assert (result.returncode, result.stdout.splitlines(), result.stderr) == (
            0,
            expected,
            "",
        ), name


def test_agent_context_system_fallback_and_invocation_modes(tmp_path: Path) -> None:
    repo = tmp_path / "repo"
    (repo / "bin").mkdir(parents=True)
    (repo / "tools").mkdir()
    agent_context = repo / "bin/agent-context"
    agent_context.write_bytes((ROOT / "bin/agent-context").read_bytes())
    agent_context.chmod(0o755)
    (repo / "tools/python").symlink_to(ROOT / "tools/python", target_is_directory=True)
    python_dir = tmp_path / "path"
    python_dir.mkdir()
    (python_dir / "python3").symlink_to(sys.executable)
    env = _clean_env()
    env.pop("PSX_PYTHON", None)
    env["PATH"] = f"{python_dir}:{env['PATH']}"
    result = _run(str(repo / "bin/agent-context"), "worker", env=env, cwd=tmp_path)
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
    for args, expected_prefix in [
        (("worker",), ["-S", "-m", "harness.commands.agent_context"]),
        (("worker", "--mode=compatibility"), ["-m", "harness.commands.agent_context"]),
    ]:
        result = _run(
            str(ROOT / "bin/agent-context"),
            *args,
            env=_clean_env() | {"PSX_PYTHON": str(fake)},
            cwd=tmp_path,
        )
        assert result.returncode == 0, result.stderr
        assert (
            log.read_text(encoding="utf-8").splitlines()[: len(expected_prefix)]
            == expected_prefix
        )


def test_agent_context_exports_safe_path_to_interpreter(tmp_path: Path) -> None:
    fake = tmp_path / "python"
    fake.write_text(
        "#!/bin/sh\n"
        '[ "${PYTHONSAFEPATH:-}" = 1 ] || exit 8\n'
        'if [ "$2" = -c ]; then printf agent-context-python; fi\n',
        encoding="utf-8",
    )
    fake.chmod(0o755)
    result = _run(
        str(ROOT / "bin/agent-context"),
        "worker",
        env=_clean_env() | {"PSX_PYTHON": str(fake)},
        cwd=tmp_path,
    )
    assert result.returncode == 0, result.stderr


def test_harness_is_permanently_narrow_psyq_adapter() -> None:
    wrapper = ROOT / "bin/harness"

    help_result = _run(str(wrapper), "--help")
    example_result = _run(str(wrapper), "--example")
    rejected_result = _run(str(wrapper), "symbols", "check")

    assert help_result.returncode == 0
    assert help_result.stdout == "usage: bin/harness psyq {scan|calls|proposal} --all\n"
    assert example_result.returncode == 0
    assert example_result.stdout.splitlines() == [
        "bin/harness psyq scan --all",
        "bin/harness psyq calls --all",
    ]
    assert rejected_result.returncode == 2
    assert rejected_result.stdout == ""
    assert rejected_result.stderr == (
        "usage: bin/harness psyq {scan|calls|proposal} --all\n"
    )
