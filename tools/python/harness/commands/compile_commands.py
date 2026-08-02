from ..compiler_config import (
    load_object_compilers,
    load_object_flags,
    sanitize_identifier,
)
from ..toolchain.gcc_variants import ensure_variant, lookup_variant
OPTIMIZATION_RE = __import__("re").compile(r"^-O(?:[0-3s]|fast)$")
    cc_driver = root / "bin" / "cc"
        str(cc_driver),
    object_flags = load_object_flags(root)
    object_compilers = load_object_compilers(root)
        key = sanitize_identifier(relative)
        override = object_flags.get(key)
        compiler_id = object_compilers.get(key)
        # Build argument vector
        if compiler_id is None:
            variant_prefix: list[str] = []
        else:
            # Resolve the specific requested compiler ID; a missing install is
            # installed on demand (only this catalog ID is ever downloaded).
            layout = repo_layout(root)
            variant = lookup_variant(layout, compiler_id)
            gcc_path = ensure_variant(layout, variant)
            variant_prefix = [
                "cmake", "-E", "env",
                f"PSX_GCC={gcc_path}",
            ]
            base_args = [*common, "-c", str(source), "-o", str(object_path)]
            base_args = [
        arguments = [*variant_prefix, *base_args]
