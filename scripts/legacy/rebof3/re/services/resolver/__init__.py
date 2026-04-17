from __future__ import annotations

from ....models.address_resolution import AddressResolution
from .helpers import MAX_VISIBLE_CANDIDATES
from .plan import build_query_plan
from .runtime import resolve_address_context
from .service import DEFAULT_RESOLVER_SERVICE, ResolverService
from .type_normalization import normalize_type_spec, rewrite_function_pointer_signature
from .type_references import contains_unsupported_signature_shape, referenced_type_names

__all__ = [
    "AddressResolution",
    "contains_unsupported_signature_shape",
    "DEFAULT_RESOLVER_SERVICE",
    "MAX_VISIBLE_CANDIDATES",
    "ResolverService",
    "build_query_plan",
    "normalize_type_spec",
    "referenced_type_names",
    "resolve_address_context",
    "rewrite_function_pointer_signature",
]
