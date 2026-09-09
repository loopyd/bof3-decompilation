"""Restricted native Codex configuration and event interpretation."""

from __future__ import annotations

import json
import re
import shutil
from pathlib import Path
from typing import Any

from harness.common.process import run_bounded
from harness.io import unique_object

FEATURES = ("apps", "plugins", "multi_agent", "skill_mcp_dependency_install")


def resolve_command(root: Path, deadline: float) -> tuple[list[str], list[str]]:
    executable = shutil.which("codex")
    if executable is None:
        raise ValueError("installed Codex executable is required; nothing is installed")
    options = [part for feature in FEATURES for part in ("--disable", feature)]
    result = run_bounded(
        root,
        [executable, *options, "mcp", "list", "--json"],
        timeout=30,
        deadline=deadline,
        output_limit=1024 * 1024,
    )
    if result["failure"] or result["exit_code"] != 0:
        raise RuntimeError("Codex capability enumeration failed; diagnostics withheld")
    try:
        servers = json.loads(result["stdout"], object_pairs_hook=unique_object)
        if not isinstance(servers, list) or len(servers) > 128:
            raise ValueError("invalid server inventory")
        names = sorted({server["name"] for server in servers})
        if any(
            not isinstance(name, str) or not re.fullmatch(r"[A-Za-z0-9_-]{1,200}", name)
            for name in names
        ):
            raise ValueError("invalid server name")
    except (ValueError, KeyError, TypeError) as error:
        raise RuntimeError(
            "Codex capability inventory is invalid; contents withheld"
        ) from error
    for name in names:
        options.extend(("-c", f"mcp_servers.{name}.enabled=false"))
    checked = run_bounded(
        root,
        [executable, *options, "mcp", "list", "--json"],
        timeout=30,
        deadline=deadline,
        output_limit=1024 * 1024,
    )
    try:
        inventory = json.loads(checked["stdout"], object_pairs_hook=unique_object)
        valid = (
            checked["failure"] is None
            and checked["exit_code"] == 0
            and isinstance(inventory, list)
            and sorted(server["name"] for server in inventory) == names
            and all(server.get("enabled") is False for server in inventory)
        )
    except (ValueError, TypeError, KeyError):
        valid = False
    if not valid:
        raise RuntimeError("Codex did not confirm the restricted capability inventory")
    command = [
        executable,
        *options,
        "-c",
        'approval_policy="never"',
        "exec",
        "--json",
        "--ephemeral",
        "--sandbox",
        "read-only",
        "--cd",
        str(root),
        "-",
    ]
    return command, names


def summarize_events(text: str) -> dict[str, Any]:
    events = [
        json.loads(line, object_pairs_hook=unique_object)
        for line in text.splitlines()
        if line.strip()
    ]
    if not events or any(not isinstance(event, dict) for event in events):
        raise ValueError("invalid Codex event stream")
    kinds = [event.get("type") for event in events]
    if (
        any(not isinstance(kind, str) or not kind for kind in kinds)
        or kinds[:2] != ["thread.started", "turn.started"]
        or kinds.count("thread.started") != 1
        or kinds.count("turn.started") != 1
        or kinds.count("turn.completed") != 1
        or "turn.failed" in kinds
        or "error" in kinds
        or kinds[-1] != "turn.completed"
    ):
        raise ValueError("Codex did not report one completed turn")
    thread = next(
        event.get("thread_id") for event in events if event["type"] == "thread.started"
    )
    if not isinstance(thread, str) or not thread or len(thread) > 128:
        raise ValueError("invalid emitted Codex thread identity")
    proposals = [
        event["item"].get("text")
        for event in events
        if event.get("type") == "item.completed"
        and isinstance(event.get("item"), dict)
        and event["item"].get("type") == "agent_message"
    ]
    if not proposals or any(
        not isinstance(proposal, str) or not proposal.strip() for proposal in proposals
    ):
        raise ValueError("Codex did not return a review proposal")
    return {"thread_id": thread, "event_count": len(events), "proposal": proposals[-1]}
