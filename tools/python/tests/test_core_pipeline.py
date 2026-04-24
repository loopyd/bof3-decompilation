from __future__ import annotations

import pytest

from rebof3.core import Pipeline, Task


def test_pipeline_plan_preserves_task_order() -> None:
    pipeline = Pipeline(
        name="demo",
        description="Demo pipeline",
        tasks=[
            Task("first", "First task", lambda context: None),
            Task("second", "Second task", lambda context: None),
            Task("third", "Third task", lambda context: None),
        ],
    )

    assert [task.name for task in pipeline.plan()] == ["first", "second", "third"]


def test_pipeline_run_preserves_task_order() -> None:
    events: list[str] = []
    context = {"target": "demo"}
    pipeline = Pipeline(
        name="demo",
        description="Demo pipeline",
        tasks=[
            Task("first", "First task", lambda ctx: events.append(ctx["target"])),
            Task("second", "Second task", lambda ctx: events.append("second")),
        ],
    )

    result = pipeline.run(context)

    assert events == ["demo", "second"]
    assert result is context


def test_pipeline_run_accepts_context_replacement() -> None:
    pipeline = Pipeline(
        name="demo",
        description="Demo pipeline",
        tasks=[
            Task("replace", "Replace context", lambda _: {"target": "next"}),
            Task("mutate", "Mutate context", lambda ctx: ctx.update({"done": True})),
        ],
    )

    assert pipeline.run({"target": "initial"}) == {"target": "next", "done": True}


def test_pipeline_run_stops_on_failure() -> None:
    events: list[str] = []

    def fail(_: object) -> None:
        events.append("fail")
        raise RuntimeError("boom")

    pipeline = Pipeline(
        name="demo",
        description="Demo pipeline",
        tasks=[
            Task("first", "First task", lambda _: events.append("first")),
            Task("fail", "Failing task", fail),
            Task("after", "Skipped task", lambda _: events.append("after")),
        ],
    )

    with pytest.raises(RuntimeError, match="boom"):
        pipeline.run()

    assert events == ["first", "fail"]
