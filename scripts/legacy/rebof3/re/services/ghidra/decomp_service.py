from __future__ import annotations

import argparse

from ....cli import logger_from_args
from ....models.ghidra import GhidraDecompRequest
from ..service import Service
from .decomp_runtime import run_decomp_bundle


class GhidraDecompService(Service):
    service_name = "ghidra_decomp"

    def run(self, request: GhidraDecompRequest, *, logger=None):
        return run_decomp_bundle(
            source_text=request.source_text,
            address_text=request.address_text,
            project_dir=request.project_dir,
            project_name=request.project_name,
            program_name=request.program_name,
            artifacts_dir=request.artifacts_dir,
            base_addr=request.base_addr,
            loader_mode=request.loader_mode,
            asm_backend=request.asm_backend,
            emit_spimdisasm=request.emit_spimdisasm,
            no_m2c=request.no_m2c,
            noanalysis=request.noanalysis,
            dry_run=request.dry_run,
            logger=logger,
        )


DEFAULT_GHIDRA_DECOMP_SERVICE = GhidraDecompService()


def _render_bundle_summary(logger, bundle_payload: dict[str, object]) -> None:
    asm_backend = bundle_payload["asm_backend"]
    files = dict(bundle_payload["files"])
    logger.summary(
        " ".join(
            [
                f"artifacts={bundle_payload['artifacts_dir']}",
                f"program={bundle_payload['program_name']}",
                f"asm_backend={asm_backend}",
            ]
        )
    )
    logger.item(f"bundle {files['json']}")
    if files.get("ghidra_asm") is not None:
        logger.item(f"ghidra asm {files['ghidra_asm']}")
    if files.get("asm") is not None:
        logger.item(f"asm {files['asm']}")
    if files.get("m2c_c") is not None:
        logger.item(f"m2c {files['m2c_c']}")

    if files.get("ghidra_c") is not None:
        logger.detail(f"ghidra c {files['ghidra_c']}")
    if files.get("spim_asm") is not None:
        logger.detail(f"spim asm {files['spim_asm']}")
    if files.get("m2c_context_source") is not None:
        logger.detail(f"m2c context source {files['m2c_context_source']}")
    if files.get("m2c_context") is not None:
        logger.detail(f"m2c context {files['m2c_context']}")
    if files.get("m2c_asm") is not None:
        logger.detail(f"m2c asm {files['m2c_asm']}")

    m2c_payload = dict(bundle_payload["m2c"])
    if m2c_payload["attempted"] and m2c_payload["path"] is None:
        logger.detail(f"m2c status {m2c_payload['status']}")
        if m2c_payload.get("stderr"):
            logger.detail(f"m2c stderr {m2c_payload['stderr']}")


def _execute_args(args: argparse.Namespace) -> int:
    logger = logger_from_args(args, "ghidra_decomp")
    returncode, bundle_payload = DEFAULT_GHIDRA_DECOMP_SERVICE.run(
        GhidraDecompRequest(
            source_text=args.input,
            address_text=args.address,
            project_dir=args.project_dir,
            project_name=args.project_name,
            program_name=args.program_name,
            artifacts_dir=args.artifacts_dir,
            base_addr=args.base_addr,
            loader_mode=args.loader_mode,
            asm_backend=args.asm_backend,
            emit_spimdisasm=not args.no_spimdisasm,
            no_m2c=args.no_m2c,
            noanalysis=args.noanalysis,
            dry_run=args.dry_run,
        ),
        logger=logger,
    )
    if args.dry_run and bundle_payload is not None:
        logger.summary(
            " ".join(
                [
                    f"artifacts={bundle_payload['artifacts_dir']}",
                    f"asm_backend={bundle_payload['asm_backend']}",
                    "dry_run=true",
                ]
            )
        )
        for command in bundle_payload["commands"]:
            logger.item(" ".join(command))
        return returncode
    if returncode != 0 or bundle_payload is None:
        return returncode

    _render_bundle_summary(logger, bundle_payload)
    return 0


__all__ = [
    "DEFAULT_GHIDRA_DECOMP_SERVICE",
    "GhidraDecompService",
    "_execute_args",
    "_render_bundle_summary",
]
