from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import shutil
import sqlite3
import subprocess
import sys
import time
import tomllib

import pytest

from harness.context import parse_cleanup_request, profile_names, render_context
from harness.context.base import ContextProfile, _add_profile
from harness.domain import parse_function_id


ROOT = Path(__file__).resolve().parents[3]
COMMAND = ROOT / "bin/agent-context"
ROLES = (
    "agents",
    "reverse",
    "review",
    "cleanup",
    "classifier",
    "context-builder",
    "oracle",
    "planner",
    "researcher",
    "reviewer",
    "scout",
    "worker",
)
SELECTOR = "exe/logo@0x801CE758"

# Golden role-only compatibility section lists, frozen from the git HEAD
# legacy .pi/skills/bof3-re/scripts/agent-context.py tables (FULL, ROLE,
# WORKFLOW; script deleted). Kept as data so parity never depends on a
# deleted runtime script.
_HEAD_FULL_SECTIONS = (
    "SOUL.md",
    "AGENTS.md",
    "docs/agents/CODING_STANDARDS.md",
    ".pi/skills/bof3-re/SKILL.md",
    "docs/agents/memory-api.md",
    "docs/agents/matching.md",
    "docs/agents/matching-playbook.md",
    "docs/agents/project-context.md",
    "docs/agents/plan-authoring.md",
    "docs/agents/lessons.md",
)
GOLDEN_COMPAT_SECTIONS = {
    "agents": (
        *_HEAD_FULL_SECTIONS,
        "subagent roster (.pi/agents)",
        "skills (.pi/skills)",
    ),
    "reverse": (
        *_HEAD_FULL_SECTIONS,
        ".pi/skills/bof3-re/references/REVERSE/MISSION_PROTOCOL.md",
        "docs/reference/bof3-eu/README.md",
    ),
    "review": (
        *_HEAD_FULL_SECTIONS,
        ".pi/skills/bof3-re/references/REVIEW/REVIEW_CHECKLIST.md",
        ".pi/skills/bof3-re/references/REVIEW/SHARING_NONMATCHES.md",
    ),
    "cleanup": (),
    "classifier": (),
    "context-builder": ("AGENTS.md", "docs/agents/project-context.md"),
    "oracle": ("AGENTS.md", "docs/agents/plan-authoring.md"),
    "planner": (
        "AGENTS.md",
        "docs/agents/project-context.md",
        "docs/agents/plan-authoring.md",
    ),
    "researcher": ("docs/agents/project-context.md",),
    "reviewer": ("AGENTS.md", "docs/agents/plan-authoring.md"),
    "scout": ("docs/agents/project-context.md",),
    "worker": (
        "AGENTS.md",
        "docs/agents/CODING_STANDARDS.md",
        "docs/agents/project-context.md",
    ),
}


def _run(command: tuple[str, ...], *, cwd: Path = ROOT, env=None):
    return subprocess.run(command, cwd=cwd, env=env, capture_output=True, text=True)


def _command(
    *args: str,
    root: Path = ROOT,
    cwd: Path = ROOT,
    env=None,
    mode: str = "stable",
):
    return _run(
        (str(COMMAND), *args, "--mode", mode, "--root", str(root)),
        cwd=cwd,
        env=env,
    )


def _section_names(text: str) -> list[str]:
    return re.findall(r"^===== (.+) =====$", text, re.MULTILINE)


def _sections(text: str) -> dict[str, str]:
    matches = list(re.finditer(r"^===== (.+) =====\n", text, re.MULTILINE))
    return {
        match.group(1): text[
            match.end() : matches[index + 1].start()
            if index + 1 < len(matches)
            else None
        ].removesuffix("\n")
        for index, match in enumerate(matches)
    }


def _normalized_contract_lines(text: str) -> list[str]:
    return [
        line.rstrip()
        for line in text.splitlines()
        if line.strip() and not re.match(r"^#{1,6}(?:\s|$)", line)
    ]


def _target_selectors() -> dict[str, str]:
    selectors = {}
    for manifest_path in sorted(ROOT.glob("config/targets/**/target.toml")):
        with manifest_path.open("rb") as stream:
            target = str(tomllib.load(stream)["id"])
        symbols = manifest_path.with_name("symbols.txt").read_text(encoding="utf-8")
        address = re.search(r"=\s*(0x[0-9A-Fa-f]+);", symbols)
        assert address is not None, target
        selectors[target] = f"{target}@{address.group(1)}"
    assert len(selectors) == 23
    return selectors


def test_compatibility_agents_role_restores_legacy_optional_selector_parser() -> None:
    no_selector = _command("agents", mode="compatibility")
    selector = _command("agents", SELECTOR, mode="compatibility")
    invalid = _command("agents", "invalid", mode="compatibility")
    multiple = _command("agents", SELECTOR, SELECTOR, mode="compatibility")
    assert no_selector.returncode == selector.returncode == 0
    assert selector.stdout != no_selector.stdout
    assert no_selector.stderr == selector.stderr == ""
    assert invalid.returncode == 2 and "expected TARGET@0xADDRESS" in invalid.stderr
    assert multiple.returncode == 2 and "unrecognized arguments" in multiple.stderr


@pytest.mark.parametrize(
    "role",
    tuple(role for role in ROLES if role not in {"reverse", "review", "cleanup"}),
)
def test_compatibility_role_only_sections_match_head_derived_golden(
    role: str,
) -> None:
    result = _command(role, mode="compatibility")
    assert result.returncode == 0
    assert result.stderr == ""
    assert tuple(_section_names(result.stdout)) == GOLDEN_COMPAT_SECTIONS[role]


@pytest.mark.parametrize(
    "role",
    tuple(role for role in ROLES if role not in {"reverse", "review", "cleanup"}),
)
def test_compatibility_role_output_is_deterministic(role: str) -> None:
    first = _command(role, mode="compatibility")
    second = _command(role, mode="compatibility")
    assert (first.returncode, first.stdout, first.stderr) == (
        second.returncode,
        second.stdout,
        second.stderr,
    )
    names = _section_names(first.stdout)
    assert len(names) == len(set(names))


@pytest.mark.parametrize("role", ("reverse", "review"))
def test_compatibility_selector_output_is_deterministic(role: str) -> None:
    first = _command(role, SELECTOR, mode="compatibility")
    second = _command(role, SELECTOR, mode="compatibility")
    assert first.returncode == second.returncode == 0
    assert first.stdout == second.stdout
    assert first.stderr == second.stderr == ""


def test_historical_cleanup_target_form_matches_compatibility_target_context() -> None:
    historical = _command("cleanup", "--target", "exe/logo", mode="compatibility")
    direct = render_context(ROOT, "cleanup", target="exe/logo", mode="compatibility")
    assert historical.returncode == 0
    assert historical.stderr == ""
    assert historical.stdout == direct


def test_stable_cleanup_target_form_normalizes_to_audit_target() -> None:
    historical = _command("cleanup", "--target", "exe/logo")
    canonical = _command("cleanup", "audit-target", "exe/logo")
    assert (historical.returncode, historical.stdout, historical.stderr) == (
        canonical.returncode,
        canonical.stdout,
        canonical.stderr,
    )


def test_compatibility_cleanup_selector_matches_historical_full_context() -> None:
    result = _command("cleanup", SELECTOR, mode="compatibility")
    expected = render_context(
        ROOT, "cleanup", parse_function_id(SELECTOR), mode="compatibility"
    )
    assert result.returncode == 0
    assert result.stderr == ""
    assert result.stdout == expected


def test_root_first_command_policy_defers_to_every_role_definition() -> None:
    root_policy = " ".join((ROOT / "AGENTS.md").read_text(encoding="utf-8").split())
    assert "bin/agent-context ROLE" in root_policy
    assert "active role definition owns the exact invocation" in root_policy
    for role in ROLES:
        command = f"bin/agent-context {role}"
        if role in {"agents", "classifier"}:
            continue
        agent_name = {
            "reverse": "bof3-reverse",
            "review": "bof3-review",
            "cleanup": "bof3-cleanup",
        }.get(role, role)
        definition = (ROOT / f".pi/agents/{agent_name}.md").read_text(encoding="utf-8")
        assert command in definition, role


def test_bof_role_first_commands_include_required_scope_placeholders() -> None:
    definitions = {
        role: (ROOT / f".pi/agents/bof3-{role}.md").read_text(encoding="utf-8")
        for role in ("reverse", "review", "cleanup")
    }
    assert "bin/agent-context reverse SELECTOR" in definitions["reverse"]
    assert "bin/agent-context review SELECTOR" in definitions["review"]
    assert "bin/agent-context cleanup CANONICAL_REQUEST..." in definitions["cleanup"]


def test_profile_order_and_role_membership() -> None:
    assert profile_names() == ROLES
    assert _section_names(render_context(ROOT, "worker")) == [
        "docs/agents/CODING_STANDARDS.md",
        "context prefill contract",
    ]
    planner = _section_names(render_context(ROOT, "planner"))
    assert planner == [
        "docs/agents/project-context.md",
        "docs/agents/plan-authoring.md",
        "context prefill contract",
    ]
    assert not any(name.startswith("docs/specs/") for name in planner)


def test_profile_order_survives_prior_concrete_import() -> None:
    probe = (
        "import harness.context.worker; "
        "from harness.context import profile_names; "
        "print(','.join(profile_names()))"
    )
    result = _run(
        (sys.executable, "-c", probe),
        env={**os.environ, "PYTHONPATH": "tools/python"},
    )
    assert result.returncode == 0
    assert tuple(result.stdout.strip().split(",")) == ROLES


def test_roster_and_skills_are_sorted() -> None:
    output = render_context(ROOT, "agents")
    roster, skills = output.split("===== skills (.pi/skills) =====\n")
    roster_lines = roster.split("===== subagent roster (.pi/agents) =====\n", 1)[1]
    names = [line.split(":", 1)[0] for line in roster_lines.strip().splitlines()]
    assert names == sorted(names)
    skill_names = skills.strip().splitlines()
    assert skill_names == sorted(skill_names)


def test_workflow_bounds_are_semantic_not_wording_snapshots() -> None:
    for role in ROLES[4:]:
        output = render_context(ROOT, role)
        ceiling = 13_000 if role in {"researcher", "scout"} else 14_000
        assert len(output.encode()) < ceiling, role
        assert output.count("=====") <= 8, role
        assert "===== context prefill contract =====" in output


def test_stable_prefills_are_tracked_bounded_and_save_discovery_calls() -> None:
    cases = {
        "agents": (None, 14_000, 4),
        "reverse": (SELECTOR, 100_000, 24),
        "review": (SELECTOR, 100_000, 24),
        "cleanup": (None, 24_000, 6),
    }
    for role, (selector, byte_limit, section_limit) in cases.items():
        function = parse_function_id(selector) if selector else None
        cleanup = (
            parse_cleanup_request(("audit-target", "exe/logo"))
            if role == "cleanup"
            else None
        )
        output = render_context(ROOT, role, function, cleanup=cleanup)
        names = _section_names(output)
        assert len(output.encode()) <= byte_limit, role
        assert len(names) <= section_limit, role
        assert len(names) >= 2, role
        assert not any(name.startswith("out/") for name in names), role
        assert len(names) - 1 >= 1, role


def test_full_92_case_stable_target_matrix_stays_within_measured_bounds() -> None:
    cases = 0
    for target, selector in _target_selectors().items():
        function = parse_function_id(selector)
        for role, byte_limit, section_limit in (
            ("reverse", 100_000, 24),
            ("review", 100_000, 24),
        ):
            output = render_context(ROOT, role, function)
            assert len(output.encode()) <= byte_limit, (role, selector)
            assert len(_section_names(output)) <= section_limit, (role, selector)
            cases += 1
        cleanup = parse_cleanup_request(("audit-target", target))
        output = render_context(ROOT, "cleanup", cleanup=cleanup)
        assert len(output.encode()) <= 24_000, target
        assert len(_section_names(output)) <= 6, target
        cases += 1
    assert cases == 69


@pytest.mark.parametrize(
    ("selector", "expected"),
    (
        ("emi/etc/game/00@0x8019601C", "u8 findFreeRecord(u8 mode);"),
        (
            "exe/slus_004_22@0x8015DF18",
            "WEAK_SYMBOL_AT(dispatchSoundCue, 0x8015df18);",
        ),
    ),
)
def test_stable_reverse_includes_semantic_declaration_or_binding(
    selector: str, expected: str
) -> None:
    output = render_context(ROOT, "reverse", parse_function_id(selector))
    assert expected in output


@pytest.mark.parametrize("role", ("reverse", "review"))
def test_stable_reverse_review_contains_complete_contract_docs_with_digest(
    role: str,
) -> None:
    required = [
        "SOUL.md",
        "AGENTS.md",
        "docs/agents/memory-api.md",
        "docs/agents/matching.md",
        "docs/agents/matching-playbook.md",
        "docs/agents/lessons.md",
    ]
    if role == "reverse":
        required.append("docs/reference/bof3-eu/README.md")
    first = render_context(ROOT, role, parse_function_id(SELECTOR))
    second = render_context(ROOT, role, parse_function_id(SELECTOR))
    sections = _sections(first)
    assert required == [name for name in sections if name in required]
    assert all(_section_names(first).count(path) == 1 for path in required)
    source_digests = {}
    section_digests = {}
    for relative in required:
        source = (ROOT / relative).read_text(encoding="utf-8")
        section = sections[relative]
        assert section == source
        assert _normalized_contract_lines(section) == _normalized_contract_lines(source)
        source_digests[relative] = hashlib.sha256(source.encode()).hexdigest()
        section_digests[relative] = hashlib.sha256(section.encode()).hexdigest()
    assert section_digests == source_digests
    assert (
        hashlib.sha256(first.encode()).digest()
        == hashlib.sha256(second.encode()).digest()
    )


def test_stable_profiles_only_emit_required_inherited_contracts() -> None:
    for role in ("reverse", "review"):
        assert (
            _section_names(
                render_context(ROOT, role, parse_function_id(SELECTOR))
            ).count("AGENTS.md")
            == 1
        )
    for role in set(ROLES) - {"classifier", "reverse", "review", "cleanup"}:
        assert "AGENTS.md" not in _section_names(render_context(ROOT, role))
    cleanup = parse_cleanup_request(("audit-target", "exe/logo"))
    assert "AGENTS.md" not in _section_names(
        render_context(ROOT, "cleanup", cleanup=cleanup)
    )
    assert _section_names(render_context(ROOT, "reviewer")) == [
        "context prefill contract"
    ]


def test_compatibility_mode_retains_generated_selector_evidence() -> None:
    function = parse_function_id(SELECTOR)
    stable = render_context(ROOT, "reverse", function)
    compatibility = render_context(ROOT, "reverse", function, mode="compatibility")
    assert not any(name.startswith("out/") for name in _section_names(stable))
    assert "context prefill contract" not in _section_names(compatibility)
    assert len(stable.encode()) < len(compatibility.encode())


def test_stable_workflow_selector_is_rejected() -> None:
    result = _command("worker", SELECTOR)
    assert result.returncode == 2
    assert "does not accept a function selector" in result.stderr


@pytest.mark.parametrize(
    "role",
    tuple(
        role for role in ROLES if role not in {"agents", "reverse", "review", "cleanup"}
    ),
)
def test_compatibility_non_bof_roles_match_historical_optional_selector(
    role: str,
) -> None:
    baseline = _command(role, mode="compatibility")
    selector = _command(role, SELECTOR, mode="compatibility")
    invalid = _command(role, "arbitrary", mode="compatibility")
    extra = _command(role, SELECTOR, "extra", mode="compatibility")
    assert selector.returncode == baseline.returncode == 0
    assert selector.stdout == baseline.stdout
    assert selector.stderr == baseline.stderr == ""
    assert invalid.returncode == 2 and "expected TARGET@0xADDRESS" in invalid.stderr
    assert extra.returncode == 2 and "unrecognized arguments: extra" in extra.stderr


def test_cleanup_symbol_arrow_cli_transport_is_shell_safe() -> None:
    result = _command(
        "cleanup", "symbol", "exe/logo", "old_name", "--rename-to", "new_name"
    )
    assert result.returncode == 0, result.stderr
    request = _sections(result.stdout)["cleanup request"]
    assert '"arguments": [\n    "old_name",\n    "new_name"' in request


def test_full_selector_and_target_modes_have_characterized_ceilings() -> None:
    cases = (
        ("agents", None, None, 73_000, 12),
        ("reverse", SELECTOR, None, 83_000, 24),
        ("review", SELECTOR, None, 81_000, 24),
    )
    for role, selector, target, ceiling, sections in cases:
        function = parse_function_id(selector) if selector else None
        output = render_context(ROOT, role, function, target, mode="compatibility")
        assert len(output.encode()) < ceiling, role
        assert len(_section_names(output)) <= sections, role


def _required_repo(root: Path) -> None:
    for relative in (
        "SOUL.md",
        "AGENTS.md",
        "docs/agents/CODING_STANDARDS.md",
        ".pi/skills/bof3-re/SKILL.md",
        "docs/agents/memory-api.md",
        "docs/agents/matching.md",
        "docs/agents/matching-playbook.md",
        "docs/agents/project-context.md",
        "docs/agents/plan-authoring.md",
        "docs/agents/lessons.md",
        ".pi/skills/bof3-re/references/REVERSE/MISSION_PROTOCOL.md",
        "docs/reference/bof3-eu/README.md",
    ):
        path = root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(f"{relative}\n", encoding="utf-8")


def _target_repo(
    root: Path, *, corrupt_index: bool = False, empty_index: bool = False
) -> None:
    _required_repo(root)
    config = root / "config/targets/exe/test"
    config.mkdir(parents=True)
    source = root / "src/test/nested/lift.c"
    header = root / "src/test/nested/internal.h"
    binding = root / "src/test/bindings/symbols.c"
    binary = root / "out/test.bin"
    source.parent.mkdir(parents=True)
    header.parent.mkdir(parents=True, exist_ok=True)
    binding.parent.mkdir(parents=True)
    binary.parent.mkdir(parents=True)
    (root / "out/splat/exe/test/asm").mkdir(parents=True)
    source.write_text(
        "/* @source 0x80100000\n * @behavior test\n */\nvoid reviewedFunction(void) {}\n"
    )
    header.write_text(
        "typedef int Test;\n/* Absolute-address globals */\nextern int D_80100020;\n"
    )
    binding.write_text("/* binding */\n")
    binary.write_bytes(b"\0" * 0x100)
    (config / "target.toml").write_text(
        "schema='harness.target/v2'\nid='exe/test'\nkind='executable'\n"
        "source_dir='src/test'\nbinary='out/test.bin'\nload_address=0x80100000\n"
        "splat='config/targets/exe/test/splat.yaml'\n"
        "sources=['src/test/nested/lift.c']\n"
        "support_sources=['src/test/bindings/symbols.c']\n"
        "headers=['src/test/nested/internal.h']\n"
    )
    (config / "symbols.txt").write_text(
        "reviewedFunction = 0x80100000;\nD_80100020 = 0x80100020;\n"
    )
    (config / "splat.yaml").write_text(
        "name: test\noptions:\n  asm_path: out/splat/exe/test/asm\n"
        "segments:\n  - name: code\n    type: code\n    start: 0\n"
        "    vram: 0x80100000\n    subsegments:\n"
        "      - [0, c, reviewedFunction, '@source: src/test/nested/lift.c']\n"
    )
    (root / "out/splat/exe/test/asm/reviewedFunction.s").write_text(
        "lui v0, %hi(D_80100020)\n"
    )
    index = root / "out/index/reverse.sqlite"
    index.parent.mkdir(parents=True)
    if corrupt_index:
        index.write_text("not sqlite", encoding="utf-8")
    else:
        connection = sqlite3.connect(index)
        connection.execute(
            "CREATE TABLE data_references(function_id TEXT, target_id TEXT, address INTEGER, symbol TEXT)"
        )
        if not empty_index:
            connection.execute(
                "INSERT INTO data_references VALUES (?, ?, ?, NULL)",
                ("exe/test@80100000", "exe/test", 0x80100020),
            )
        connection.commit()
        connection.close()


@pytest.mark.parametrize("corrupt", (False, True))
def test_minimal_target_fixture_and_optional_index(
    corrupt: bool, tmp_path: Path
) -> None:
    _target_repo(tmp_path, corrupt_index=corrupt)
    output = render_context(
        tmp_path,
        "reverse",
        parse_function_id("exe/test@0x80100000"),
        mode="compatibility",
    )
    names = _section_names(output)
    assert "config/targets/exe/test/target.toml" in names
    assert "config/targets/exe/test/symbols.txt" in names
    assert "src/test/nested/internal.h" in names
    assert "src/test/bindings/symbols.c" in names
    assert "src/test/nested/lift.c" in names
    assert "out/splat/exe/test/asm/reviewedFunction.s" in names
    assert ("data-scan: exe/test" in names) is (not corrupt)


def test_valid_empty_index_keeps_compatibility_data_scan_section(
    tmp_path: Path,
) -> None:
    _target_repo(tmp_path, empty_index=True)
    current = _command(
        "reverse",
        "exe/test@0x80100000",
        root=tmp_path,
        mode="compatibility",
    )
    assert current.returncode == 0
    assert current.stderr == ""
    assert "data-scan: exe/test" in _section_names(current.stdout)


def test_missing_required_file_fails_but_optional_artifacts_do_not(
    tmp_path: Path,
) -> None:
    _required_repo(tmp_path)
    (tmp_path / "docs/agents/CODING_STANDARDS.md").unlink()
    result = _command("worker", root=tmp_path)
    assert result.returncode == 2
    assert result.stdout == ""
    assert "missing required context: docs/agents/CODING_STANDARDS.md" in result.stderr


@pytest.mark.parametrize(
    ("args", "message"),
    (
        (("nope",), "invalid choice"),
        (("reverse", "broken"), "expected TARGET@0xADDRESS"),
        (("reverse", "extra", "scope"), "reverse requires a function selector"),
        (("reverse",), "reverse requires a function selector"),
        (("review",), "review requires a function selector"),
        (("cleanup",), "cleanup requires one canonical request"),
        (("cleanup", "audit-target", "exe/not-real", "extra"), "exactly one TARGET"),
    ),
)
def test_cli_rejects_invalid_requests(args: tuple[str, ...], message: str) -> None:
    result = _command(*args)
    assert result.returncode == 2
    assert message in result.stderr


@pytest.mark.parametrize("role", ("worker", "agents"))
def test_compatibility_selector_behavior_for_non_bof_roles(role: str) -> None:
    result = _command(role, SELECTOR, mode="compatibility")
    assert result.returncode == 0
    assert result.stderr == ""


def test_cli_errors_preserve_semantics_after_program_rename() -> None:
    for args, message in (
        (("cleanup", "audit-target", "exe/logo", "extra"), "exactly one TARGET"),
        (("cleanup", "not-a-selector"), "unknown cleanup mode"),
    ):
        current = _command(*args, mode="compatibility")
        assert current.returncode == 2
        assert current.stdout == ""
        assert message in current.stderr
        if current.stderr.startswith("usage:"):
            assert current.stderr.startswith("usage: agent-context ")


def test_duplicate_registration_is_rejected_in_isolated_registry() -> None:
    registry: dict[str, ContextProfile] = {}
    profile = ContextProfile("test", (), False, False, None, 1, 1, lambda request: ())
    _add_profile(registry, profile)
    with pytest.raises(ValueError, match="duplicate agent context profile"):
        _add_profile(registry, profile)


def test_unknown_api_profile_is_rejected() -> None:
    with pytest.raises(ValueError, match="unknown agent context profile"):
        render_context(ROOT, "not-a-profile")


def test_wrapper_runs_outside_repo_and_honors_python_override(tmp_path: Path) -> None:
    external = _command("worker", cwd=tmp_path)
    assert external.returncode == 0
    assert _section_names(external.stdout)[0] == "docs/agents/CODING_STANDARDS.md"
    env = os.environ.copy()
    env["PSX_PYTHON"] = sys.executable
    override = _command("classifier", cwd=tmp_path, env=env)
    assert override.returncode == 0
    for executable in (str(tmp_path / "missing-python"), "/bin/true"):
        env["PSX_PYTHON"] = executable
        invalid = _command("classifier", cwd=tmp_path, env=env)
        assert invalid.returncode == 2
        assert "invalid Python interpreter" in invalid.stderr


def test_stable_selectors_run_with_isolated_system_python() -> None:
    python = Path("/usr/bin/python3")
    if not python.is_file():
        pytest.skip("isolated system Python is unavailable")
    env = {
        "HOME": os.environ.get("HOME", ""),
        "PATH": "/usr/bin:/bin",
        "PYTHONNOUSERSITE": "1",
        "PSX_PYTHON": str(python),
    }
    for selector, expected in (
        ("emi/etc/game/00@0x8019601C", "u8 findFreeRecord(u8 mode);"),
        (
            "exe/slus_004_22@0x8015DF18",
            "WEAK_SYMBOL_AT(dispatchSoundCue, 0x8015df18);",
        ),
    ):
        result = _run((str(COMMAND), "reverse", selector), env=env)
        assert result.returncode == 0
        assert result.stderr == ""
        assert expected in result.stdout


@pytest.mark.parametrize("mode", (None, "stable", "compatibility"))
@pytest.mark.parametrize("python", (sys.executable, "/usr/bin/python3"))
def test_wrapper_ignores_ambient_stdlib_and_harness_modules(
    tmp_path: Path, mode: str | None, python: str
) -> None:
    if not Path(python).is_file():
        pytest.skip("requested Python mode is unavailable")
    poison = tmp_path / "poison"
    poison.mkdir()
    (poison / "argparse.py").write_text(
        "raise RuntimeError('ambient argparse loaded')\n", encoding="utf-8"
    )
    (poison / "harness.py").write_text(
        "raise RuntimeError('ambient harness loaded')\n", encoding="utf-8"
    )
    env = os.environ.copy()
    env["PSX_PYTHON"] = python
    env["PYTHONPATH"] = str(poison)
    args = [str(COMMAND), "worker"]
    if mode:
        args.extend(("--mode", mode))
    result = _run(tuple(args), cwd=tmp_path, env=env)
    assert result.returncode == 0, result.stderr
    assert "ambient" not in result.stderr


def test_wrapper_equals_mode_matches_split_mode_in_isolated_env() -> None:
    python = Path("/usr/bin/python3")
    if not python.is_file():
        pytest.skip("isolated system Python is unavailable")
    env = {
        "HOME": os.environ.get("HOME", ""),
        "PATH": "/usr/bin:/bin",
        "PYTHONNOUSERSITE": "1",
        "PSX_PYTHON": str(python),
    }
    args = ("worker", "--root", str(ROOT))
    split = _run((str(COMMAND), *args, "--mode", "compatibility"), env=env)
    equals = _run((str(COMMAND), "--mode=compatibility", *args), env=env)
    assert split.returncode == equals.returncode == 0
    assert (split.stdout, split.stderr) == (equals.stdout, equals.stderr)


def test_wrapper_falls_back_to_system_python_without_project_venv(
    tmp_path: Path,
) -> None:
    env = {
        "HOME": str(tmp_path),
        "PATH": os.environ["PATH"],
        "PYTHONNOUSERSITE": "1",
        "PSX_PYTHON": shutil.which("python3") or "python3",
    }
    result = _run((str(COMMAND), "classifier"), cwd=tmp_path, env=env)
    assert result.returncode == 0
    assert result.stderr == ""
    assert "context prefill contract" in result.stdout


def test_render_is_read_only_and_bounded() -> None:
    before = set(ROOT.glob("out/**"))
    started = time.monotonic()
    first = render_context(ROOT, "worker")
    second = render_context(ROOT, "worker")
    assert first == second
    assert time.monotonic() - started < 1.0
    assert set(ROOT.glob("out/**")) == before
