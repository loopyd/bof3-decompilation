from __future__ import annotations

import argparse

from ....cli import context_from_args
from ....models.re import DoctorRequest
from ..service import Service
from .checks import build_checks
from .render import render_checks_payload


class DoctorService(Service):
    service_name = "doctor"

    def run(self, request: DoctorRequest, *, logger) -> int:
        checks = build_checks()
        render_checks_payload(
            json_output=request.json_output,
            strict=request.strict,
            quiet=request.quiet,
            checks=checks,
        )

        failures = [
            check for check in checks if check.required and check.status != "ok"
        ]
        optional_failures = [
            check for check in checks if not check.required and check.status != "ok"
        ]
        if request.json_output:
            return 1 if failures or (request.strict and optional_failures) else 0
        if failures or (request.strict and optional_failures):
            logger.error(
                f"doctor found {len(failures)} required issue(s)"
                + (
                    f" and {len(optional_failures)} optional issue(s)"
                    if request.strict and optional_failures
                    else ""
                )
            )
            return 1

        logger.summary("doctor checks passed")
        return 0


DEFAULT_DOCTOR_SERVICE = DoctorService()


def _execute_args(args: argparse.Namespace) -> int:
    context = context_from_args(args, "re_doctor")
    return DEFAULT_DOCTOR_SERVICE.run(
        DoctorRequest(
            json_output=bool(args.json),
            strict=bool(args.strict),
            quiet=context.cli.quiet,
        ),
        logger=context.logger,
    )


__all__ = ["DEFAULT_DOCTOR_SERVICE", "DoctorService", "_execute_args"]
