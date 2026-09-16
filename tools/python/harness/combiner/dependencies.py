"""Capture and watch conservative literal include search inputs for comparison."""

from __future__ import annotations

import copy
from pathlib import Path
from types import MappingProxyType

from harness.common.deadlines import check_deadline
from harness.common.inputs import InputBatch
from harness.common.observation import PathWatch
from harness.domain.includes import parse_literal_includes

_FILE_LIMIT = 64 * 1024 * 1024
_TOTAL_LIMIT = 128 * 1024 * 1024
_INPUT_LIMIT = 16384


class IncludeSnapshot:
    """Keep one bounded search generation and reject later input/lookup mutations."""

    def __init__(self, root: Path, source: Path) -> None:
        self.root = root
        self._watch: PathWatch | None = None
        self._states: dict[Path, dict | None] = {}
        self._content: dict[Path, bytes] = {}
        self._total = 0
        self._failed = False
        roots = (root / "src", root / "include", root / "toolchains/psyq/4.7/include")
        with InputBatch(root) as batch:
            if self._read(batch, source) is None:
                raise ValueError(f"missing include source: {source}")
            pending = [source]
            scheduled = {source}
            while pending:
                check_deadline()
                path = pending.pop()
                try:
                    text = self._content[path].decode("utf-8")
                except UnicodeError as error:
                    raise ValueError(f"include source is not UTF-8: {path}") from error
                for filename, quoted in parse_literal_includes(text, source=str(path)):
                    if Path(filename).is_absolute():
                        raise ValueError(
                            f"external include filename: {path}: {filename}"
                        )
                    search = ((path.parent,) if quoted else ()) + roots
                    for directory in search:
                        candidate = directory / filename
                        for mapping in (
                            directory / "header.gcc",
                            candidate.parent / "header.gcc",
                        ):
                            if self._read(batch, mapping) is not None:
                                raise ValueError(
                                    f"unsupported compiler header mapping: {mapping}"
                                )
                        if self._read(batch, candidate) is not None:
                            if candidate not in scheduled:
                                scheduled.add(candidate)
                                pending.append(candidate)
                            break
                    else:
                        raise ValueError(
                            f"unresolved literal include: {path}: {filename}"
                        )
        try:
            self._watch = PathWatch(
                set(self._states), directories={path.parent for path in self._content}
            )
            self.validate()
        except BaseException:
            self.close()
            raise

    def _read(self, batch: InputBatch, path: Path) -> bytes | None:
        check_deadline()
        if path not in self._states:
            if len(self._states) >= _INPUT_LIMIT:
                raise ValueError("include search exceeds its input bound")
            state, content = batch.read(
                path, max_bytes=_FILE_LIMIT, include_metadata=True, single_link=True
            )
            self._states[path] = state
            if content is not None:
                self._total += len(content)
                if self._total > _TOTAL_LIMIT:
                    raise ValueError("include search exceeds its byte bound")
                self._content[path] = content
        return self._content.get(path)

    @property
    def content(self):
        return MappingProxyType(self._content)

    @property
    def paths(self) -> frozenset[Path]:
        return frozenset(self._states)

    def describe_inputs(self) -> dict:
        """Describe captured present files and negative search candidates."""
        return {
            path.relative_to(self.root).as_posix(): copy.deepcopy(state)
            for path, state in sorted(self._states.items())
        }

    def validate(self) -> None:
        """Recheck the original sample without refreshing discovery or content."""
        self.check()
        try:
            with InputBatch(self.root) as batch:
                for path, expected in self._states.items():
                    check_deadline()
                    state, _ = batch.read(
                        path,
                        max_bytes=_FILE_LIMIT,
                        include_metadata=True,
                        single_link=True,
                    )
                    if state != expected:
                        raise ValueError(f"include search input changed: {path}")
            self.check()
        except BaseException:
            self._failed = True
            raise

    def check(self) -> None:
        """Latch events between full endpoint verifications without rereading files."""
        if self._failed or self._watch is None:
            raise ValueError("include snapshot is closed or failed")
        try:
            self._watch.validate()
        except BaseException:
            self._failed = True
            raise

    def close(self) -> None:
        """Release the watch without allowing this snapshot to be revived."""
        watch, self._watch = self._watch, None
        if watch is not None:
            watch.close()
