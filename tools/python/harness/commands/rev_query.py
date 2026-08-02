from functools import lru_cache
from ..domain import (
    FUNCTION_ID_FORMAT,
    FUNCTION_ID_HELP,
    load_target_manifests,
    normalize_target_id,
    parse_function_id,
)
from ..layout import parse_splat_layout

@lru_cache(maxsize=None)
def _candidate_context(root: Path, target: str):
    manifest = load_target_manifests(root)[target]
    binary = root / manifest.binary
    if not binary.is_file():
        return manifest, None, None, frozenset()
    return (
        manifest,
        binary.read_bytes(),
        parse_splat_layout(root / manifest.splat, manifest.load_address),
        frozenset(
            symbol.address
            for symbol in load_map(sdk_map_path(root, manifest.psyq_space))
            if not symbol.is_raw
        ),
    )


def _candidate_exclusion(root: Path, row: dict[str, Any]) -> str | None:
    """Reject analyzer-only roots that lack canonical code evidence.

    Reviewed Splat labels and Rizin's function finder are hypotheses. Ranking
    must not offer a raw-data label or an SDK body as a lift candidate.
    """

    address = int(str(row["address"]), 0)
    manifest, image, layout, sdk_addresses = _candidate_context(root, row["target"])
    if image is None:
        return "missing_binary"
    assert layout is not None
    boundary = layout.boundary_starting_at(address)
    if boundary is None or not boundary.is_function:
        return "not_reviewed_code_boundary"
    if boundary.function_name != f"func_{address:08X}":
        return "noncanonical_boundary_name"
    offset = address - manifest.load_address
    size = row["size"]
    payload = image[offset : offset + size]
    if len(payload) != size:
        return "boundary_outside_binary"
    printable = sum(byte == 0 or 0x20 <= byte < 0x7F for byte in payload)
    if len(payload) >= 8 and printable * 4 >= len(payload) * 3:
        return "ascii_or_nul_data"
    words = [
        int.from_bytes(payload[index : index + 4], "little")
        for index in range(0, len(payload) - 3, 4)
    ]
    binary_end = manifest.load_address + len(image)
    if len(words) >= 2 and all(
        manifest.load_address <= word < binary_end for word in words
    ):
        return "in_image_pointer_table"
    if address in sdk_addresses:
        return "shared_sdk_symbol"
    return None


def _priority_rows(
    connection, args: argparse.Namespace, *, root: Path | None = None
) -> list[dict[str, Any]]:
    if root is not None:
        exclusions = [(row, _candidate_exclusion(root, row)) for row in payload]
        if getattr(args, "exclusions", False):
            payload = [
                {**row, "candidate_exclusion": reason}
                for row, reason in exclusions
                if reason is not None
            ]
        else:
            payload = [row for row, reason in exclusions if reason is None]
    if getattr(args, "exclusions", False):
        payload = [
            {
                key: row[key]
                for key in ("id", "target", "address", "candidate_exclusion")
            }
            for row in payload
        ]
        payload.sort(key=lambda row: (row["id"], row["candidate_exclusion"]))
        return payload[: args.limit] if args.limit else payload
            payload = _priority_rows(connection, args, root=_root(args))
            detail = (
                "full"
                if getattr(args, "exclusions", False)
                else resolve_detail(requested=args.detail, json_output=args.json)
            )
    xrefs.add_argument("function", metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP)
    calls = sub.add_parser("calls", help="show calls to or from a function selector")
    calls.add_argument("function", metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP)
                "--exclusions",
                action="store_true",
                help="show candidate rows rejected by canonical-code checks",
            )
    mission.add_argument("function", metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP)
