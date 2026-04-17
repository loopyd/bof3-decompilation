from __future__ import annotations

import json
from pathlib import Path

from ....models.metadata import (
    MetadataSyncFromRequest,
    MetadataSyncPlan,
    MetadataSyncToRequest,
)
from ..service import Service
from .capture import capture_into_inventory, preflight_capture
from .ghidra_bridge import load_known_type_names, run_ghidra_metadata
from .planning import build_plan


class MetadataSyncService(Service):
    service_name = "metadata_sync"

    def execute_to(
        self, request: MetadataSyncToRequest
    ) -> tuple[int, dict[str, object], MetadataSyncPlan]:
        known_type_names: tuple[str, ...] = ()
        payload: dict[str, object] = {}
        output_path = request.output_path or (
            Path("tmp") / f"metadata_sync_to_{request.mode}.json"
        )
        if request.mode in {"preflight", "apply"}:
            output_path.parent.mkdir(parents=True, exist_ok=True)
            known_types_output = output_path.with_name(
                output_path.stem + ".known_types.json"
            )
            try:
                known_type_names = load_known_type_names(
                    db_path=request.db_path,
                    owner=request.owner,
                    selectors=request.selectors,
                    project_dir=request.project_dir,
                    project_name=request.project_name,
                    output_path=known_types_output,
                    log_path=request.log_path,
                )
            except Exception as exc:  # noqa: BLE001
                payload["known_types_error"] = str(exc)
        plan = build_plan(
            db_path=request.db_path,
            owner=request.owner,
            selectors=request.selectors,
            kind=request.kind,
            mode=request.mode,
            known_type_names=known_type_names,
        )
        payload["direction"] = "to"
        payload["known_type_names"] = list(known_type_names)
        payload["plan"] = plan.as_dict()
        exit_code = 0
        if request.mode in {"preflight", "apply"}:
            returncode, ghidra_result, stdout_text, stderr_text = run_ghidra_metadata(
                plan=plan,
                project_dir=request.project_dir,
                project_name=request.project_name,
                output_path=output_path,
                log_path=request.log_path,
            )
            payload["ghidra_result"] = ghidra_result or {}
            payload["stdout"] = stdout_text
            payload["stderr"] = stderr_text
            blocked = [
                row.as_dict()
                for row in plan.row_plans
                if row.classification.startswith("blocked_")
            ]
            payload["blocked_rows"] = blocked
            failures = 0
            if ghidra_result is not None:
                for row in ghidra_result.get("rows", []):
                    if isinstance(row, dict) and str(row.get("status") or "") != "ok":
                        failures += 1
                for program in ghidra_result.get("programs", []):
                    if not isinstance(program, dict):
                        continue
                    for row in program.get("rows", []):
                        if (
                            isinstance(row, dict)
                            and str(row.get("status") or "") != "ok"
                        ):
                            failures += 1
            payload["failure_count"] = failures
            exit_code = 0 if returncode == 0 and not blocked and failures == 0 else 1
        if request.output_path is not None:
            output_path.parent.mkdir(parents=True, exist_ok=True)
            output_path.write_text(
                json.dumps(payload, indent=2, sort_keys=True),
                encoding="utf-8",
            )
        return exit_code, payload, plan

    def execute_from(self, request: MetadataSyncFromRequest) -> dict[str, object]:
        if request.mode == "report":
            if request.input_path is None:
                raise SystemExit("sync from --mode report requires --input <path>")
            return json.loads(request.input_path.read_text(encoding="utf-8"))
        if request.mode == "preflight":
            return preflight_capture(
                db_path=request.db_path,
                owner=request.owner,
                selectors=request.selectors,
                kind=request.kind,
                project_dir=request.project_dir,
                project_name=request.project_name,
                include_default=request.include_default,
                user_defined_only=request.user_defined_only,
                output_path=request.output_path,
                log_path=request.log_path,
            )
        return capture_into_inventory(
            db_path=request.db_path,
            owner=request.owner,
            selectors=request.selectors,
            kind=request.kind,
            project_dir=request.project_dir,
            project_name=request.project_name,
            include_default=request.include_default,
            user_defined_only=request.user_defined_only,
            output_path=request.output_path,
            log_path=request.log_path,
        )


DEFAULT_METADATA_SYNC_SERVICE = MetadataSyncService()
