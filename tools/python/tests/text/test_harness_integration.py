"""The text harness is integration only: registration, forwarding, examples and exit codes.

Behaviour — the readability rule, the payload inventory, the registry, the scan pipeline — is asserted in
the crate's own suite (`cargo test`, including its corpus-backed integration tests). What is checked here is
the harness itself, for **every** action it offers: that it is registered, that it carries an example, that
its flags reach the tool, and that the tool's exit code and output come back unchanged.

The unknown-flag case is used as the uniform probe because it exercises all four properties at once: the
tool refuses with its own clap error, so the harness can return the tool's status only by forwarding the
argument vector, and it can return the tool's text only by passing both streams through untouched.
"""

from __future__ import annotations

import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[4]
HARNESS = ROOT / "bin" / "harness"
BINARY = ROOT / "build" / "tools" / "rust" / "bof3-text" / "release" / "bof3-text"

#: Every action the text domain offers. Each one forwards to exactly one crate mode of the same name.
ACTIONS = (
    "build-index",
    "extract",
    "index",
    "map",
    "pack",
    "payloads",
    "probe",
    "query",
    "readability",
    "scan",
    "search",
    "validate",
    "verify",
    "vocabulary",
    "windows",
)


def _run(*args: str) -> subprocess.CompletedProcess:
    return subprocess.run([str(HARNESS), "text", *args], cwd=ROOT, capture_output=True, text=True)


def _direct(*args: str) -> subprocess.CompletedProcess:
    assert BINARY.is_file(), f"the tool binary is missing at {BINARY}; run `just setup`"
    return subprocess.run([str(BINARY), *args], cwd=ROOT, capture_output=True, text=True)


@pytest.mark.parametrize("action", ACTIONS)
def test_the_action_is_registered_and_carries_an_example(action: str) -> None:
    listed = _run("--help")
    assert listed.returncode == 0, listed.stderr
    assert action in listed.stdout, f"the harness does not list `text {action}`"
    example = _run(action, "--example")
    assert example.returncode == 0, f"`text {action} --example` failed: {example.stderr}"
    assert action in example.stdout, f"the example for `{action}` does not name it"


@pytest.mark.parametrize("action", ACTIONS)
def test_the_action_forwards_its_flags_and_returns_the_tools_status_verbatim(action: str) -> None:
    # The refusal must be the tool's own: same status, same bytes on both streams. A harness that
    # swallowed the flag, or translated the status, or added a line of its own, fails here.
    forwarded = _run(action, "--definitely-not-a-flag")
    direct = _direct(action, "--definitely-not-a-flag")
    assert direct.returncode == 2, f"the tool should refuse an unknown flag, got {direct.returncode}"
    assert forwarded.returncode == direct.returncode, (
        f"`{action}`: the harness returned {forwarded.returncode}, the tool returns {direct.returncode}"
    )
    assert "--definitely-not-a-flag" in forwarded.stderr, (
        f"`{action}`: the tool's refusal did not reach the caller"
    )
    assert forwarded.stderr == direct.stderr, f"`{action}`: stderr was not passed through verbatim"
    assert forwarded.stdout == direct.stdout, f"`{action}`: stdout was not passed through verbatim"


def test_a_successful_action_returns_the_tools_bytes() -> None:
    # A real invocation, not just a refusal: the archive scan and the readability diagnostic must return
    # exactly what the tool printed for the same argv.
    archive = "out/extracted/BIN/WORLD00/AREA000.EMI"
    for args in (
        ("scan", archive, "--json"),
        ("readability", "--text", "the village", "--json"),
        ("query", archive, "--entry", "11", "--grep", "Saved", "--json"),
    ):
        forwarded = _run(*args)
        direct = _direct(*args)
        assert forwarded.returncode == direct.returncode == 0, f"{args}: {forwarded.stderr}"
        assert forwarded.stdout == direct.stdout, f"{args}: the harness did not return the tool's output"
        assert forwarded.stdout.strip(), f"{args}: the action printed nothing"


def test_the_documentation_drift_wiring_runs() -> None:
    drift = subprocess.run(
        [str(HARNESS), "source", "docs"], cwd=ROOT, capture_output=True, text=True
    )
    assert drift.returncode == 0, drift.stderr
    assert "docs drift" in drift.stdout
