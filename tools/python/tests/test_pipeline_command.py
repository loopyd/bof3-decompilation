from __future__ import annotations

import pytest

from rebof3.commands import pipeline as pipeline_command
from rebof3.core import Pipeline, Task
from rebof3.core.process import ProcessError, ProcessResult
from rebof3.pipelines.registry import PipelineRegistry, build_default_registry


def build_fake_registry(events: list[str] | None = None) -> PipelineRegistry:
    events = events if events is not None else []
    registry = PipelineRegistry()
    registry.register(
        "alpha",
        "Alpha pipeline",
        lambda: Pipeline(
            name="alpha",
            description="Alpha pipeline",
            tasks=[
                Task(
                    "first",
                    "First fake task",
                    lambda _: events.append("first"),
                ),
                Task(
                    "second",
                    "Second fake task",
                    lambda _: events.append("second"),
                ),
            ],
        ),
    )
    registry.register(
        "empty",
        "Empty pipeline",
        lambda: Pipeline(
            name="empty",
            description="Empty pipeline",
            tasks=[],
        ),
    )
    return registry


def test_pipeline_list_prints_registered_pipelines(capsys: pytest.CaptureFixture[str]) -> None:
    result = pipeline_command.main(["--list"], registry=build_fake_registry())

    assert result == 0
    assert capsys.readouterr().out.splitlines() == [
        "alpha\tAlpha pipeline",
        "empty\tEmpty pipeline",
    ]


def test_pipeline_plan_prints_tasks_without_running(
    capsys: pytest.CaptureFixture[str],
) -> None:
    events: list[str] = []

    result = pipeline_command.main(
        ["alpha", "--plan"],
        registry=build_fake_registry(events),
    )

    assert result == 0
    assert events == []
    assert capsys.readouterr().out.splitlines() == [
        "alpha: Alpha pipeline",
        "1. first: First fake task",
        "2. second: Second fake task",
    ]


def test_pipeline_run_executes_registered_pipeline() -> None:
    events: list[str] = []

    result = pipeline_command.main(["alpha"], registry=build_fake_registry(events))

    assert result == 0
    assert events == ["first", "second"]


def test_pipeline_run_reports_process_failure_without_traceback(
    capsys: pytest.CaptureFixture[str],
) -> None:
    registry = PipelineRegistry()
    registry.register(
        "fail",
        "Failing pipeline",
        lambda: Pipeline(
            name="fail",
            description="Failing pipeline",
            tasks=[
                Task(
                    "failed-command",
                    "Failed command",
                    lambda _: (_ for _ in ()).throw(
                        ProcessError(
                            ProcessResult(
                                command=("tool",),
                                returncode=7,
                                cwd=None,
                                stdout="",
                                stderr="",
                            )
                        )
                    ),
                )
            ],
        ),
    )

    result = pipeline_command.main(["fail"], registry=registry)

    assert result == 7
    assert "command failed with exit code 7" in capsys.readouterr().err


def test_default_registry_includes_ghidra_bootstrap_pipeline() -> None:
    registry = build_default_registry()

    assert registry.names() == [
        "binary-parity",
        "build-ready",
        "decomp-full-ready",
        "decomp-ready",
        "extract-assets",
        "ghidra-bootstrap",
        "ghidra-ready",
        "harness-ready",
        "inventory-refresh",
        "lift-ready",
        "match-loop",
        "setup-open",
    ]
    assert registry.get("binary-parity").description
    assert registry.get("build-ready").description
    assert registry.get("decomp-full-ready").description
    assert registry.get("decomp-ready").description
    assert registry.get("extract-assets").description
    assert registry.get("ghidra-bootstrap").description
    assert registry.get("ghidra-ready").description
    assert registry.get("harness-ready").description
    assert registry.get("inventory-refresh").description
    assert registry.get("lift-ready").description
    assert registry.get("match-loop").description
    assert registry.get("setup-open").description


def test_pipeline_parser_rejects_unknown_pipeline() -> None:
    with pytest.raises(SystemExit) as excinfo:
        pipeline_command.main(["missing"], registry=build_fake_registry())

    assert excinfo.value.code == 2


def test_pipeline_help_lists_registered_pipelines(
    capsys: pytest.CaptureFixture[str],
) -> None:
    with pytest.raises(SystemExit):
        pipeline_command.main(["--help"], registry=build_fake_registry())

    help_text = capsys.readouterr().out
    assert "pipelines:" in help_text
    assert "alpha" in help_text
    assert "empty" in help_text
