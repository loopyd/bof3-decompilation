"""Conservative current source consumers of explicitly selected identifiers."""

from __future__ import annotations

from pathlib import Path
import re

from harness.common.lexicon import mask_c_text
from harness.domain.claims import manifest_source_paths
from harness.domain.functions import collect_lift_metadata
from harness.domain.includes import local_include_files, parse_literal_includes
from harness.domain.manifests import load_target_manifests

_IDENTIFIER = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")


def identifier_consumers(
    root: Path, target: str, names: set[str], *, include_dependencies: bool = False
) -> tuple[set[str], list[dict]]:
    """Return local selectors and explicit foreign/unresolved coverage findings.

    When requested, header mentions include every translation unit that includes
    that header. References are leads; no cross-target ownership is inferred.
    """
    selectors: set[str] = set()
    findings: list[dict] = []
    text_cache: dict[Path, str] = {}
    token_cache: dict[Path, set[str]] = {}
    owned_names: dict[str, set[str]] = {}

    def own_names(owner: str) -> set[str]:
        """Names the target defines itself; its own definitions are not consumers."""
        if owner not in owned_names:
            path = root / "config" / "targets" / owner / "symbols.txt"
            names_here: set[str] = set()
            if path.is_file():
                for line in path.read_text(encoding="utf-8").splitlines():
                    name, _, address = line.partition("=")
                    if address.strip():
                        names_here.add(name.strip())
            owned_names[owner] = names_here
        return owned_names[owner]

    def text(path: Path) -> str:
        if path not in text_cache:
            text_cache[path] = path.read_text(encoding="utf-8")
            parse_literal_includes(text_cache[path], source=str(path))
            token_cache[path] = set(_IDENTIFIER.findall(mask_c_text(text_cache[path])))
        return text_cache[path]

    for owner, manifest in sorted(load_target_manifests(root).items()):
        for source in manifest_source_paths(root, manifest):
            if source.suffix != ".c":
                continue
            try:
                body = text(source)
                dependencies = (
                    local_include_files(
                        root,
                        [source],
                        include_angle=True,
                        strict=True,
                        require_literal=True,
                        read_source=text,
                    )
                    if include_dependencies
                    else []
                )
                for path in dependencies:
                    text(path)
                matched = {
                    name
                    for path in [source, *dependencies]
                    for name in names & token_cache[path]
                }
                if not matched:
                    continue
                addresses = collect_lift_metadata(body)
                if owner != target:
                    # A duplicated function carries the same name in several
                    # targets; where the other target defines that name itself,
                    # the mention is its own definition rather than our consumer.
                    if matched and matched <= own_names(owner):
                        continue
                    findings.append(
                        {
                            "status": "unavailable",
                            "code": "foreign_consumers",
                            "target": owner,
                            "source": source.relative_to(root).as_posix(),
                            "message": "possible cross-target consumer requires an explicit target check",
                        }
                    )
                elif not addresses and "{" not in mask_c_text(body):
                    continue
                elif not addresses:
                    findings.append(
                        {
                            "status": "unavailable",
                            "code": "unresolved_consumer",
                            "source": source.relative_to(root).as_posix(),
                            "message": "referencing translation unit has no authored function identity",
                        }
                    )
                else:
                    selectors.update(
                        f"{owner}@0x{address:08X}" for address in addresses
                    )
            except (OSError, UnicodeError, ValueError) as error:
                findings.append(
                    {
                        "status": "unavailable",
                        "code": "consumer_inputs",
                        "target": owner,
                        "source": source.relative_to(root).as_posix(),
                        "message": str(error),
                    }
                )
    return selectors, findings
