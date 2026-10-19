use crate::catalog::{Dependency, Resource};

pub(crate) fn annotate(resource: &mut Resource) {
    let video = match (resource.source.as_str(), resource.sha256.as_str()) {
        (
            "LOGO/CAPCOM30.STR",
            "c496080be2e134dba766e89a52bacbae7616239e1e26fe9d0c1242a92d12e915",
        ) => true,
        (
            "BIN/BMAG_XA/MAGIC00.STR",
            "9916db03238af4469843db0e71891ab6680e8a3a9e755bcb2d96e2dfbf92d3fa",
        )
        | (
            "BIN/SCE_XA/S_XA00.STR",
            "4dd8d034d4524aa907be793da1450f9f85cb67cd837ba412f1e328b719e0a37d",
        )
        | (
            "BIN/SCE_XA/VOICE.STR",
            "f4f9d8ce9a2849501e8cd44dfcfee85aad4bc92cd9ff5113acde43c9362c65cc",
        ) => false,
        _ => return,
    };
    resource.kind = if video {
        "media-owned-stream"
    } else {
        "audio-owned-stream"
    }
    .into();
    resource.storage = "2336-byte-sector-stream".into();
    resource.decode = if video {
        "delegated-to-media-and-audio"
    } else {
        "delegated-to-audio"
    }
    .into();
    resource.evidence_status = "owner-qualified".into();
    resource.evidence.extend([
        "tools/rust/bof3-audio/src/xa/reference.rs:complete-original-disc-extent".into(),
        "tools/rust/bof3-audio/src/archive/mod.rs:XaImage-sector-inventory".into(),
        format!(
            "bin/harness media str inspect {} --json:qualified-original-sector-and-frame-inventory",
            resource.source
        ),
    ]);
    resource.dependencies.push(Dependency {
        kind: "audio-owner".into(),
        target: Some("tools/rust/bof3-audio/src/xa/extraction.rs".into()),
        status: "verified".into(),
        reason: "the XA owner decodes selected audio sectors and preserves the complete source; delegation does not establish every sector's meaning or runtime playback".into(),
    });
    resource.findings = vec![
        "the whole source extent is retained; sector headers and owner delegation do not establish complete byte decoding or residual-sector roles".into(),
    ];
    resource.dependencies.push(Dependency {
        kind: "stream-byte-disposition".into(),
        target: None,
        status: "unresolved".into(),
        reason: "unselected, padding, control and terminal sectors remain in the source; no subrange is excluded from coverage".into(),
    });
    if !video {
        resource.findings.push(
            "the media inspector finds XA sectors and no standard MDEC frame records; this does not prove the absence of other embedded graphics".into(),
        );
        return;
    }
    resource.categories = vec!["system-screen".into(), "unclassified-graphics".into()];
    resource.evidence.extend([
        "LOGO/LOGO.EXE@0x801CEDFC:CAPCOM30-playback-entry".into(),
        "LOGO/LOGO.EXE@0x801CEA98:MDEC-frame-consumer".into(),
    ]);
    resource.dependencies.push(Dependency {
        kind: "video-owner".into(),
        target: Some("tools/python/harness/media/str_media.py".into()),
        status: "verified".into(),
        reason: "the media owner inspects STR frame chunks and provides reversible sector wrapping and desktop conversion; textures preserves the original stream without a second video codec".into(),
    });
    resource.dependencies.push(Dependency {
        kind: "runtime-appearance".into(),
        target: Some("LOGO/LOGO.EXE@0x801CEA98".into()),
        status: "unresolved".into(),
        reason: "the original MDEC consumer establishes video use; playback timing, displayed frames and edited appearance are not accepted by this catalog".into(),
    });
}
