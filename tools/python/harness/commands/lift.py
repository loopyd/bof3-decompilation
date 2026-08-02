from ..domain import FUNCTION_ID_HELP, FunctionId, parse_function_id
from ..toolchain.m2c import M2cToolchain
    m2c = M2cToolchain(root)
    arguments: list[str] = [
        arguments.extend(("--context", extra))
        arguments.append("--void")
    arguments.append(str(assembly))
    result = m2c.execute(arguments, capture_output=True, text=True)
def _run_match(
    function: FunctionId,
    manifest: TargetManifest,
    source: Path,
    *,
    diagnostics: bool,
) -> dict[str, object]:
    symbols = load_target_symbols(root, function.target.value)
    binding_text = weak_bindings_c(symbols)
    if not bindings.is_file() or bindings.read_text(encoding="utf-8") != binding_text:
        bindings.write_text(binding_text, encoding="utf-8")
            canonical_bindings={
                symbol.canonical_name: symbol.address for symbol in symbols
            },
            section_placements=manifest.section_placements.get(function.address, ()),
            diagnostics=diagnostics,
    if diagnostics:
                outputs = payload["outputs"]
                if not isinstance(outputs, dict):
                    raise ValueError("invalid asm-diff payload outputs")
                projected["diff"] = outputs["diff"]
            outputs = payload["outputs"]
            if not isinstance(outputs, dict):
                raise ValueError("invalid asm-diff payload outputs")
            diff = Path(outputs["diff"])
def _require_lifted_source(function: FunctionId, source: Path) -> None:
            f"lifted source does not exist: {source}; generate and review it with "
            f"bin/m2c {function.target.value}@0x{function.address:08X} -o {source}"
        )
    function, manifest, source = resolve_function(args.function)
    _require_lifted_source(function, source)
        _run_match(function, manifest, source, diagnostics=True),
    function, manifest, source = resolve_function(args.function)
    _require_lifted_source(function, source)
        _run_match(function, manifest, source, diagnostics=False),
        json_output=args.json,
        bytes_only=True,
    function, manifest, source = resolve_function(args.function)
    payload = _run_match(function, manifest, source, diagnostics=True)
        parser.add_argument("function", nargs="?", help=FUNCTION_ID_HELP)
        parser.add_argument("function", nargs="?", help=FUNCTION_ID_HELP)
        parser.add_argument("function", nargs="?", help=FUNCTION_ID_HELP)
        parser.add_argument("function", nargs="?", help=FUNCTION_ID_HELP)
        parser.error(f"{FUNCTION_ID_HELP} is required")
