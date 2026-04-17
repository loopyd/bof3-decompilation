from __future__ import annotations

from ....models.address_resolution import AddressResolution


def build_query_plan(resolution: AddressResolution) -> list[dict[str, object]]:
    address = resolution.requested_address
    address_text = None if address is None else f"0x{address:08x}"
    selector = (
        resolution.primary_program_selector or resolution.requested_program_selector
    )
    if resolution.xref_strategy == "direct_program_xrefs" and address_text:
        return [
            {
                "tool": "get-decompilation",
                "programPath": selector,
                "functionNameOrAddress": address_text,
                "reason": "primary direct function/context probe",
            },
            {
                "tool": "find-cross-references",
                "programPath": selector,
                "location": address_text,
                "reason": "exact inbound refs for resolved program",
                "direction": "to",
            },
        ]
    if (
        resolution.xref_strategy == "containing_function_then_exact_refs"
        and address_text
    ):
        plan: list[dict[str, object]] = []
        if resolution.containing_function_entry is not None:
            plan.append(
                {
                    "tool": "get-decompilation",
                    "programPath": selector,
                    "functionNameOrAddress": f"0x{resolution.containing_function_entry:08x}",
                    "reason": "inspect containing function for internal-label target",
                }
            )
        plan.append(
            {
                "tool": "find-cross-references",
                "programPath": selector,
                "location": address_text,
                "reason": "inspect exact refs for requested internal-label address",
                "direction": "both",
            }
        )
        return plan
    if resolution.xref_strategy == "ranked_overlay_candidates" and address_text:
        plan: list[dict[str, object]] = []
        for candidate_selector in resolution.candidate_program_selectors:
            plan.append(
                {
                    "tool": "get-memory-blocks",
                    "programPath": candidate_selector,
                    "reason": "classify candidate memory layout before neighborhood probes",
                }
            )
            plan.extend(
                [
                    {
                        "tool": "read-memory",
                        "programPath": candidate_selector,
                        "addressOrSymbol": f"0x{max(address - 0x20, 0):08x}",
                        "length": 32,
                        "format": "hex",
                        "reason": "inspect leading neighborhood bytes for shared-region anchors",
                    },
                    {
                        "tool": "read-memory",
                        "programPath": candidate_selector,
                        "addressOrSymbol": address_text,
                        "length": 32,
                        "format": "hex",
                        "reason": "inspect exact neighborhood bytes for shared-region anchors",
                    },
                    {
                        "tool": "read-memory",
                        "programPath": candidate_selector,
                        "addressOrSymbol": f"0x{address + 0x20:08x}",
                        "length": 32,
                        "format": "hex",
                        "reason": "inspect trailing neighborhood bytes for shared-region anchors",
                    },
                    {
                        "tool": "get-data",
                        "programPath": candidate_selector,
                        "addressOrSymbol": address_text,
                        "reason": "inspect typed data at requested shared-region address",
                    },
                ]
            )
            for neighbor in (address - 4, address + 4):
                if neighbor >= 0:
                    plan.append(
                        {
                            "tool": "get-data",
                            "programPath": candidate_selector,
                            "addressOrSymbol": f"0x{neighbor:08x}",
                            "reason": "inspect typed neighboring word for shared-region anchors",
                        }
                    )
            plan.append(
                {
                    "tool": "get-decompilation",
                    "programPath": candidate_selector,
                    "functionNameOrAddress": address_text,
                    "reason": "inspect shared-region candidate context around requested address",
                }
            )
            plan.append(
                {
                    "tool": "find-cross-references",
                    "programPath": candidate_selector,
                    "location": address_text,
                    "direction": "both",
                    "includeFlow": True,
                    "includeData": True,
                    "includeContext": True,
                    "contextLines": 2,
                    "reason": "probe ranked overlay candidate for shared-region address",
                }
            )
            plan.append(
                {
                    "tool": "get-referencers-decompiled",
                    "programPath": candidate_selector,
                    "addressOrSymbol": address_text,
                    "maxReferencers": 5,
                    "reason": "inspect decompiled referencers for shared-region address",
                }
            )
        return plan
    if resolution.xref_strategy == "runtime_neighborhood" and address_text and selector:
        return [
            {
                "tool": "get-memory-blocks",
                "programPath": selector,
                "reason": "classify runtime candidate memory layout before neighborhood probes",
            },
            {
                "tool": "read-memory",
                "programPath": selector,
                "addressOrSymbol": f"0x{max(address - 0x20, 0):08x}",
                "length": 32,
                "format": "hex",
                "reason": "inspect leading runtime neighborhood bytes",
            },
            {
                "tool": "read-memory",
                "programPath": selector,
                "addressOrSymbol": address_text,
                "length": 32,
                "format": "hex",
                "reason": "inspect exact runtime neighborhood bytes",
            },
            {
                "tool": "read-memory",
                "programPath": selector,
                "addressOrSymbol": f"0x{address + 0x20:08x}",
                "length": 32,
                "format": "hex",
                "reason": "inspect trailing runtime neighborhood bytes",
            },
            {
                "tool": "get-data",
                "programPath": selector,
                "addressOrSymbol": address_text,
                "reason": "inspect typed data at runtime candidate address",
            },
            {
                "tool": "find-cross-references",
                "programPath": selector,
                "location": address_text,
                "direction": "both",
                "includeFlow": True,
                "includeData": True,
                "includeContext": True,
                "contextLines": 2,
                "reason": "collect runtime candidate refs for requested address",
            },
            {
                "tool": "get-referencers-decompiled",
                "programPath": selector,
                "addressOrSymbol": address_text,
                "maxReferencers": 5,
                "reason": "inspect decompiled referencers for runtime candidate address",
            },
        ]
    return []
