"""Bind source images and conservative literal dependency search generations."""

from __future__ import annotations

import copy
import hashlib
import re
from dataclasses import dataclass
from pathlib import Path
from types import MappingProxyType

from harness.common.deadlines import check_deadline
from harness.common.inputs import InputBatch, relative
from harness.common.observation import PathWatch
from harness.domain.includes import parse_literal_includes

_FILE_LIMIT = 64 * 1024 * 1024
_TOTAL_LIMIT = 128 * 1024 * 1024
_INPUT_LIMIT = 16384


@dataclass(frozen=True, slots=True)
class SourceImage:
    """Bind immutable source bytes to their nominal path, kind and expected digest."""

    source: str
    content: bytes
    kind: str
    sha256: str

    def __post_init__(self) -> None:
        check_deadline()
        if relative(self.source) != self.source or "\x00" in self.source:
            raise ValueError("source image requires a canonical repository path")
        if self.kind not in ("current", "retained", "prospective"):
            raise ValueError("unsupported source image kind")
        if type(self.content) is not bytes or len(self.content) > _FILE_LIMIT:
            raise ValueError("source image requires bounded immutable bytes")
        if not isinstance(self.sha256, str) or not re.fullmatch(
            r"[0-9a-f]{64}", self.sha256
        ):
            raise ValueError("source image requires an expected SHA-256")
        if hashlib.sha256(self.content).hexdigest() != self.sha256:
            raise ValueError("source image differs from its expected digest")
        check_deadline()

    def decode_text(self) -> str:
        """Decode this image without reopening its current filesystem spelling."""
        try:
            return self.content.decode("utf-8")
        except UnicodeError as error:
            raise ValueError(f"source image is not UTF-8: {self.source}") from error

    def describe(self) -> dict:
        """Describe seed provenance separately from captured physical inputs."""
        return {
            "source": self.source,
            "kind": self.kind,
            "sha256": self.sha256,
            "size": len(self.content),
        }


class IncludeSnapshot:
    """Keep one bounded search generation and reject later input/lookup mutations."""

    def __init__(
        self, root: Path, source: Path, *, image: SourceImage | None = None
    ) -> None:
        self.root = root
        self._watch: PathWatch | None = None
        self._states: dict[Path, dict | None] = {}
        self._content: dict[Path, bytes] = {}
        self._total = 0
        self._failed = False
        self._active = False
        name = relative(source.relative_to(root).as_posix())
        if image is not None and (
            not isinstance(image, SourceImage) or image.source != name
        ):
            raise ValueError("include source image differs from its nominal path")
        roots = (root / "src", root / "include", root / "toolchains/psyq/4.7/include")
        with InputBatch(root) as batch:
            physical = image is None or image.kind == "current"
            if physical:
                content = self._read(batch, source)
                if content is None:
                    raise ValueError(f"missing include source: {source}")
                if image is not None and image.content != content:
                    raise ValueError("current source image differs from captured bytes")
                image = SourceImage(
                    name, content, "current", self._states[source]["sha256"]
                )
            else:
                self._total += len(image.content)
                if self._total > _TOTAL_LIMIT:
                    raise ValueError("include search exceeds its byte bound")
            self.image = image
            pending = [(source, image.content)]
            scheduled = {source} if physical else set()
            while pending:
                check_deadline()
                path, content = pending.pop()
                try:
                    text = content.decode("utf-8")
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
                        content = self._read(batch, candidate)
                        if content is not None:
                            if candidate not in scheduled:
                                scheduled.add(candidate)
                                pending.append((candidate, content))
                            break
                    else:
                        raise ValueError(
                            f"unresolved literal include: {path}: {filename}"
                        )
        try:
            if self._states:
                self._watch = PathWatch(
                    set(self._states),
                    directories={path.parent for path in self._content},
                )
            self._active = True
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
        if self._failed or not self._active:
            raise ValueError("include snapshot is closed or failed")
        try:
            check_deadline()
            if self._watch is not None:
                self._watch.validate()
        except BaseException:
            self._failed = True
            raise

    def close(self) -> None:
        """Release the watch without allowing this snapshot to be revived."""
        self._active = False
        watch, self._watch = self._watch, None
        if watch is not None:
            watch.close()
