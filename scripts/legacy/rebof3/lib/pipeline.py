from __future__ import annotations

from abc import ABC, abstractmethod
from time import perf_counter
from types import MappingProxyType
from typing import Any, Iterable, Mapping

from ..logger import Rebof3Logger, make_logger


# Shared pipeline state is intentionally just a mutable dict.
# The tasks are small and composable, so a plain mapping keeps the wiring simple.
PipelineContext = dict[str, Any]
PipelineOptions = Mapping[str, Any]
PIPELINE_LOGGER_OPTION = "logger"
TASK_LOGGER_OPTION = "task_logger"


def _freeze_options(options: PipelineOptions | None) -> PipelineOptions:
    """Expose options as a read-only mapping inside tasks."""

    if options is None:
        return MappingProxyType({})
    return MappingProxyType(dict(options))


def option_logger(
    options: PipelineOptions | None,
    *,
    fallback_name: str = "pipeline",
) -> Rebof3Logger:
    """Resolve the logger exposed through shared pipeline options."""

    if options is not None:
        for key in (TASK_LOGGER_OPTION, PIPELINE_LOGGER_OPTION):
            logger = options.get(key)
            if isinstance(logger, Rebof3Logger):
                return logger
    return make_logger(fallback_name, quiet=True, verbose=False)


def options_with_logger(
    options: PipelineOptions | None,
    logger: Rebof3Logger,
) -> PipelineOptions:
    """Return a frozen options mapping that carries the shared pipeline logger."""

    next_options = {} if options is None else dict(options)
    next_options[PIPELINE_LOGGER_OPTION] = logger
    return _freeze_options(next_options)


def _task_options(
    options: PipelineOptions,
    logger: Rebof3Logger,
) -> PipelineOptions:
    next_options = dict(options)
    next_options[TASK_LOGGER_OPTION] = logger
    return _freeze_options(next_options)


class PipelineTask(ABC):
    """Minimal task contract used by every pipeline stage."""

    task_name = "task"

    @property
    def name(self) -> str:
        return str(getattr(self, "task_name", self.__class__.__name__.lower()))

    @abstractmethod
    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        raise NotImplementedError


class Pipeline(PipelineTask):
    """A task that runs a fixed sequence of child tasks."""

    task_name = "pipeline"

    def __init__(
        self,
        tasks: Iterable[PipelineTask],
        *,
        task_name: str | None = None,
        options: PipelineOptions | None = None,
    ):
        self.tasks = list(tasks)
        self._task_name = task_name or self.task_name
        self._default_options = _freeze_options(options)

    @property
    def name(self) -> str:
        return self._task_name

    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        # Later options override pipeline defaults, but every child sees the
        # same frozen view for the duration of the run.
        merged_options = dict(self._default_options)
        if options is not None:
            merged_options.update(dict(options))
        current_options = _freeze_options(merged_options)
        run_logger = option_logger(current_options, fallback_name=self.name)
        run_start = perf_counter()
        run_logger.debug(f"start {self.name}")
        current = context
        for task in self.tasks:
            task_logger = run_logger.child(task.name)
            task_options = _task_options(current_options, task_logger)
            task_start = perf_counter()
            task_logger.debug("start")
            try:
                current = task.run(current, options=task_options)
            except Exception as exc:
                task_logger.error(f"failed: {exc}")
                raise
            task_logger.debug(f"done in {perf_counter() - task_start:.2f}s")
        run_logger.debug(f"done {self.name} in {perf_counter() - run_start:.2f}s")
        return current


def pipeline(
    *tasks: PipelineTask,
    task_name: str | None = None,
    options: PipelineOptions | None = None,
) -> Pipeline:
    """Convenience constructor for small declarative pipelines."""

    return Pipeline(tasks, task_name=task_name, options=options)


__all__ = [
    "Pipeline",
    "PipelineContext",
    "PIPELINE_LOGGER_OPTION",
    "PipelineOptions",
    "PipelineTask",
    "TASK_LOGGER_OPTION",
    "option_logger",
    "options_with_logger",
    "pipeline",
]
