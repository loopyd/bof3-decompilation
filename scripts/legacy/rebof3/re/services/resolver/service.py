from __future__ import annotations

from ....models.address_resolution import AddressResolution
from ..service import Service
from .plan import build_query_plan
from .runtime import resolve_address_context
from .type_normalization import normalize_type_spec, rewrite_function_pointer_signature
from .type_references import contains_unsupported_signature_shape, referenced_type_names


class ResolverService(Service):
    service_name = "resolver"

    def resolve_context(self, **kwargs) -> AddressResolution:
        return resolve_address_context(**kwargs)

    def build_query_plan(
        self, resolution: AddressResolution
    ) -> list[dict[str, object]]:
        return build_query_plan(resolution)

    def normalize_type_spec(self, type_spec: object, *, kind: str):
        return normalize_type_spec(type_spec, kind=kind)

    def contains_unsupported_signature_shape(self, type_spec: str) -> bool:
        return contains_unsupported_signature_shape(type_spec)

    def rewrite_function_pointer_signature(self, type_spec: object):
        return rewrite_function_pointer_signature(type_spec)

    def referenced_type_names(self, type_spec: object, *, kind: str | None = None):
        return referenced_type_names(type_spec, kind=kind)


DEFAULT_RESOLVER_SERVICE = ResolverService()
