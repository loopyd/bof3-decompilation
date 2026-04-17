from __future__ import annotations

import argparse
import json

from .checks import DoctorCheck


def render_checks_payload(
    *,
    json_output: bool,
    strict: bool,
    quiet: bool,
    checks: list[DoctorCheck],
) -> None:
    if json_output:
        payload = {
            "ok": not any(check.required and check.status != "ok" for check in checks)
            and not any(
                strict and not check.required and check.status != "ok"
                for check in checks
            ),
            "checks": [check.as_dict() for check in checks],
        }
        print(json.dumps(payload, indent=2, sort_keys=True))
        return

    for check in checks:
        if quiet and check.status == "ok":
            continue
        scope = "required" if check.required else "optional"
        print(f"[{scope}] {check.group}/{check.name}: {check.status} - {check.detail}")
        if check.hint and check.status != "ok":
            print(f"  hint: {check.hint}")


def render_checks(args: argparse.Namespace, checks: list[DoctorCheck]) -> None:
    render_checks_payload(
        json_output=bool(args.json),
        strict=bool(args.strict),
        quiet=bool(getattr(args, "quiet", False)),
        checks=checks,
    )


__all__ = ["render_checks", "render_checks_payload"]
