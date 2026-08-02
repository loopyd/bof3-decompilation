from ..domain import (
    FUNCTION_ID_FORMAT,
    FUNCTION_ID_HELP,
    load_target_manifests,
    normalize_target_id,
    parse_function_id,
)
def _targets(
    root: Path, target: str | None, *, manifests: dict | None = None
) -> list[str]:
    pool = manifests if manifests is not None else load_target_manifests(root)
        return sorted(pool)
    if normalized not in pool:
    for target in _targets(root, args.target, manifests=manifests):
        manifest = manifests[target]
        raise ValueError(
            f"--all-qualified cannot be combined with {FUNCTION_ID_FORMAT}"
        )
        raise ValueError(
            f"select at least one {FUNCTION_ID_FORMAT} or pass --all-qualified"
        )
    check = sub.add_parser("check", help="validate target map(s)")
    check.add_argument("target", nargs="?")
    import_psyq.add_argument(
        "selectors", nargs="*", metavar=FUNCTION_ID_FORMAT, help=FUNCTION_ID_HELP
    )
