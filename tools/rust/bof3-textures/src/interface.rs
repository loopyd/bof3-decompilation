use crate::{Error, Result, catalog::Resource};

struct Atlas {
    source: &'static str,
    entry: usize,
    sha256: &'static str,
    categories: &'static [&'static str],
    content: &'static str,
    consumer: &'static str,
    palette_entry: usize,
    palette_sha256: &'static str,
    selection: &'static str,
}

const ATLASES: &[Atlas] = &[
    Atlas {
        source: "BIN/ETC/FIRST.EMI",
        entry: 3,
        sha256: "d72c7f620b52d93f75a979fb9ae9892aa91d687a4f71eac7605b00795c58493d",
        categories: &["font", "interface"],
        content: "Latin and Japanese glyphs, symbols, icons and interface labels",
        consumer: "SLUS_004.22@0x80150098:8x8-glyphs; SLUS_004.22@0x8014FC00:retail-page-0x2f",
        palette_entry: 8,
        palette_sha256: "166ead38de405cf28c1cfc35a7c4a2b9b4a69ae95a128b970cf4b9df8b4567c7",
        selection: "FIRST entry 8 stages VRAM row 480, words 0..255. The original 8x8 renderer selects CLUT 0x7800 | (A2 & 63), so this source covers only selectors 0..15. Retail page 0x2f matches the atlas at (960, 0), 4 bits per pixel. Actual selector, upload order, overwrites and rendering are unresolved.",
    },
    Atlas {
        source: "BIN/ETC/DEMO.EMI",
        entry: 3,
        sha256: "17e0b3058b76fbdd0d0527293464d3148c8b24dbbf77b41922019b7f92bb410a",
        categories: &["system-screen"],
        content: "BREATH OF FIRE logo raster",
        consumer: "BIN/ETC/GAME.EMI#1@0x801D150C:records-4-5; retail-page-0xb9",
        palette_entry: 7,
        palette_sha256: "25389de257006aa7e940702803b621bae0bb0d6c4d46921eb80c4e8d765e1e06",
        selection: "GAME entry 1 records 4/5 select CLUT row 484, first 256 words of DEMO entry 7, and retail page 0xb9 matches this 8-bit atlas at (576, 256). Slot 0x25f requests the physical DEMO archive. Load success, upload order, overwrites and rendering are unresolved.",
    },
    Atlas {
        source: "BIN/ETC/DEMO.EMI",
        entry: 4,
        sha256: "5587f56d745abffaf77494349b0b902120eaf5cd3dbf521b6999f2e02c3b471d",
        categories: &["system-screen"],
        content: "III logo and Japanese subtitle raster",
        consumer: "BIN/ETC/GAME.EMI#1@0x801D16DC:records-2-3; retail-page-0xbb",
        palette_entry: 7,
        palette_sha256: "25389de257006aa7e940702803b621bae0bb0d6c4d46921eb80c4e8d765e1e06",
        selection: "GAME entry 1 records 2/3 select CLUT row 485, DEMO entry 7 byte range 512..1024, and retail page 0xbb matches this 8-bit atlas at (704, 256). Slot 0x25f requests the physical DEMO archive. Load success, upload order, overwrites and rendering are unresolved.",
    },
    Atlas {
        source: "BIN/ETC/DEMO.EMI",
        entry: 5,
        sha256: "0f2750a7c89914c121a759bdb791220d639e2b2105aa5461e35784e8841c1a32",
        categories: &["interface", "system-screen"],
        content: "PRESS START BUTTON and copyright raster labels",
        consumer: "BIN/ETC/GAME.EMI#1@0x801D12CC:records-7-8-9-19; retail-page-0xbd",
        palette_entry: 7,
        palette_sha256: "25389de257006aa7e940702803b621bae0bb0d6c4d46921eb80c4e8d765e1e06",
        selection: "GAME entry 1 records 7/8/9/19 select CLUT row 486, DEMO entry 7 byte range 1024..1536, and retail page 0xbd matches this 8-bit atlas at (832, 256). Slot 0x25f requests the physical DEMO archive. Load success, upload order, overwrites and rendering are unresolved.",
    },
];

fn push_unique(values: &mut Vec<String>, value: &str) {
    if !values.iter().any(|known| known == value) {
        values.push(value.into());
    }
}

fn check_hash(resource: &Resource, expected: &str) -> Result<()> {
    if resource.sha256 != expected {
        return Err(Error::Invalid(format!(
            "{} differs from its qualified interface mapping",
            resource.id
        )));
    }
    Ok(())
}

pub(crate) fn annotate(resources: &mut [Resource]) -> Result<()> {
    for atlas in ATLASES {
        let locate = |entry| {
            resources.iter().position(|resource| {
                resource.source == atlas.source && resource.entry == Some(entry)
            })
        };
        let Some(image_index) = locate(atlas.entry) else {
            continue;
        };
        let palette_index = locate(atlas.palette_entry).ok_or_else(|| {
            Error::Invalid(format!(
                "{} lacks interface palette entry {}",
                atlas.source, atlas.palette_entry
            ))
        })?;
        check_hash(&resources[image_index], atlas.sha256)?;
        check_hash(&resources[palette_index], atlas.palette_sha256)?;
        let palette_id = resources[palette_index].id.clone();
        let image_id = resources[image_index].id.clone();
        let image = &mut resources[image_index];
        for category in atlas.categories {
            push_unique(&mut image.categories, category);
        }
        push_unique(&mut image.evidence, "qualified-original-interface-content");
        push_unique(&mut image.evidence, atlas.consumer);
        push_unique(&mut image.uses, atlas.content);
        if let Some(dependency) = image
            .dependencies
            .iter_mut()
            .find(|dependency| dependency.kind == "palette")
        {
            dependency.target = Some(palette_id);
            dependency.status = "partial".into();
            dependency.reason = atlas.selection.into();
        }
        push_unique(
            &mut image.findings,
            "recognized raster content can contain text; role classification does not certify non-text or runtime rendering",
        );
        let palette = &mut resources[palette_index];
        push_unique(&mut palette.uses, &image_id);
        push_unique(&mut palette.evidence, atlas.consumer);
        push_unique(
            &mut palette.findings,
            "listed atlas uses are conditional static associations; individual selectors, upload order and runtime palette freshness remain unresolved",
        );
    }
    Ok(())
}
