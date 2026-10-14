"""Prepare exact text images for consolidation records."""

from __future__ import annotations

import hashlib

from harness.build.preservation import validate_preservation_size, validate_states
from harness.common.inputs import relative


def prepare_images(images: object) -> tuple[dict, dict]:
    if not isinstance(images, dict) or not images:
        raise ValueError("transaction requires exact joint file images")
    validate_preservation_size(images)
    changes, post = {}, {}
    for name, image in images.items():
        if not isinstance(name, str) or relative(name) != name:
            raise ValueError("transaction images require canonical repository paths")
        if image is None:
            changes[name] = post[name] = None
            continue
        if (
            not isinstance(image, dict)
            or set(image) != {"text"}
            or not isinstance(image["text"], str)
        ):
            raise ValueError("transaction images require exactly text")
        changes[name] = image["text"]
        post[name] = {
            "sha256": hashlib.sha256(image["text"].encode("utf-8")).hexdigest(),
        }
    return changes, validate_states(post)
