from ..domain import FUNCTION_ID_HELP, load_target_manifests, parse_function_id
from ..toolchain.gcc_variants import EmptyCatalog, lookup_variant

    # Resolve optional compiler variant before search.
    compiler_id = args.compiler
    if compiler_id is not None:
        variant = lookup_variant(layout, compiler_id)
        if isinstance(variant, EmptyCatalog):
            raise ValueError(
                f"compiler variant {compiler_id!r} not available (empty catalog)"
            )
        variant.verify(layout)

        compiler_id=compiler_id,
    parser.add_argument("function", help=FUNCTION_ID_HELP)
    parser.add_argument(
        "--compiler", type=str, help="catalog ID for a historical GCC variant"
    )
