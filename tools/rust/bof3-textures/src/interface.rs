use crate::{
    Error, Result,
    catalog::{Dependency, Resource},
};

struct PaletteBinding {
    entry: usize,
    sha256: &'static str,
    selection: &'static str,
    bits: u8,
    first_words: &'static [usize],
}

struct Atlas {
    source: &'static str,
    entry: usize,
    sha256: &'static str,
    categories: &'static [&'static str],
    content: &'static str,
    consumer: Option<&'static str>,
    palettes: &'static [PaletteBinding],
}

struct SharedContent {
    sha256: &'static str,
    argument: u32,
    categories: &'static [&'static str],
    content: &'static str,
    reference: &'static str,
}

const PORTRAIT_CONTENT: &str = "Portraits, window frames, labels and interface icons";

const SHARED_CONTENT: &[SharedContent] = &[
    SharedContent {
        sha256: "d6a8ad127418dfeb2c8cb9f186d40941b47010ee75dd40e809473599ddf08c9e",
        argument: 0x1c08_0200,
        categories: &["portrait", "interface"],
        content: PORTRAIT_CONTENT,
        reference: "BIN/ETC/FIRST.EMI#4:identical-original-interface-content",
    },
    SharedContent {
        sha256: "0cf5ef90802f620701919bef9d97f2b0cb86951c0855dda8f27ef5f9a014a841",
        argument: 0x1a08_0200,
        categories: &["interface"],
        content: "Window frames, arrows and interface controls",
        reference: "BIN/ETC/SHISU.EMI#2:identical-original-interface-content",
    },
];

const ATLASES: &[Atlas] = &[
    Atlas {
        source: "BIN/ETC/FIRST.EMI",
        entry: 3,
        sha256: "d72c7f620b52d93f75a979fb9ae9892aa91d687a4f71eac7605b00795c58493d",
        categories: &["font", "interface"],
        content: "Latin and Japanese glyphs, symbols, icons and interface labels",
        consumer: Some(
            "SLUS_004.22@0x80150098:8x8-glyphs; SLUS_004.22@0x8014FC00:retail-page-0x2f",
        ),
        palettes: &[PaletteBinding {
            entry: 8,
            bits: 4,
            first_words: &[
                0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240,
            ],
            sha256: "166ead38de405cf28c1cfc35a7c4a2b9b4a69ae95a128b970cf4b9df8b4567c7",
            selection: "FIRST entry 8 stages VRAM row 480, words 0..255. The original 8x8 renderer selects CLUT 0x7800 | (A2 & 63), so this source covers only selectors 0..15. Retail page 0x2f matches the atlas at (960, 0), 4 bits per pixel. Actual selector, upload order, overwrites and rendering are unresolved.",
        }],
    },
    Atlas {
        source: "BIN/ETC/FIRST.EMI",
        entry: 4,
        sha256: "d6a8ad127418dfeb2c8cb9f186d40941b47010ee75dd40e809473599ddf08c9e",
        categories: &["portrait", "interface"],
        content: PORTRAIT_CONTENT,
        consumer: None,
        palettes: &[],
    },
    Atlas {
        source: "BIN/ETC/FIRST.EMI",
        entry: 5,
        sha256: "08425dbe70f9f7fac389b81efce7d70592581cf6aaf0b54337747368a91919cc",
        categories: &["font"],
        content: "Japanese kanji glyph raster",
        consumer: None,
        palettes: &[],
    },
    Atlas {
        source: "BIN/ETC/FIRST.EMI",
        entry: 7,
        sha256: "2269d4d35e2e206c80c2e4cc0bbb89093a21eb9657ebbbb731feedda78e50009",
        categories: &["interface"],
        content: "Small hearts, status symbols and interface icons",
        consumer: None,
        palettes: &[],
    },
    Atlas {
        source: "BIN/ETC/FIRST.EMI",
        entry: 12,
        sha256: "17e0b3058b76fbdd0d0527293464d3148c8b24dbbf77b41922019b7f92bb410a",
        categories: &["system-screen"],
        content: "BREATH OF FIRE logo raster",
        consumer: None,
        palettes: &[],
    },
    Atlas {
        source: "BIN/ETC/ENDKANJI.EMI",
        entry: 0,
        sha256: "d72c7f620b52d93f75a979fb9ae9892aa91d687a4f71eac7605b00795c58493d",
        categories: &["font", "interface"],
        content: "Latin and Japanese glyphs, symbols, icons and interface labels",
        consumer: None,
        palettes: &[],
    },
    Atlas {
        source: "BIN/ETC/ENDKANJI.EMI",
        entry: 1,
        sha256: "08425dbe70f9f7fac389b81efce7d70592581cf6aaf0b54337747368a91919cc",
        categories: &["font"],
        content: "Japanese kanji glyph raster",
        consumer: None,
        palettes: &[],
    },
    Atlas {
        source: "BIN/ETC/SHISU.EMI",
        entry: 1,
        sha256: "d6a8ad127418dfeb2c8cb9f186d40941b47010ee75dd40e809473599ddf08c9e",
        categories: &["portrait", "interface"],
        content: PORTRAIT_CONTENT,
        consumer: Some(
            "BIN/ETC/SHISU.EMI#0@0x801D44C0:portrait-rectangles; BIN/ETC/SHISU.EMI#0@0x801D9B78:selection-records",
        ),
        palettes: &[
            PaletteBinding {
                entry: 3,
                bits: 4,
                first_words: &[208, 224, 240],
                sha256: "a8a1701c0555c20d44259c66123fb491efc1df1a87c89e850078fa7948a64fdb",
                selection: "SHISU entry 3 stages VRAM row 481. Original consumer 0x801D44C0 reads table 0x801D9B78 + 4*(A2 & 255), emits 40x48 rectangles on retail page 0x1e and builds CLUT ((480 + record[3]) << 6) | (record[2] >> 4). Physical records 6/8/11 select row 481, word offsets 208/224/240. Other atlas regions, the complete valid index domain, upload order and rendering are unresolved.",
            },
            PaletteBinding {
                entry: 4,
                bits: 4,
                first_words: &[0, 16, 32, 176, 192, 208, 224, 240],
                sha256: "5cfa8f083db270a3af233d0213475a13b96f01c6bac9bfb38b6423e4fe27dec7",
                selection: "SHISU entry 4 stages VRAM row 482. Original consumer 0x801D44C0 selects the atlas at (896, 256), 4 bits per pixel, with table UV bytes 0/1 and 40x48 rectangles. Physical records 0/1/2/3/4/5/7/9/10 select row 482, word offsets 176/240/32/192/208/0/16/176/224. These twelve records do not establish the complete valid index domain. Other atlas regions, upload order and rendering are unresolved.",
            },
        ],
    },
    Atlas {
        source: "BIN/ETC/DEMO.EMI",
        entry: 3,
        sha256: "17e0b3058b76fbdd0d0527293464d3148c8b24dbbf77b41922019b7f92bb410a",
        categories: &["system-screen"],
        content: "BREATH OF FIRE logo raster",
        consumer: Some("BIN/ETC/GAME.EMI#1@0x801D150C:records-4-5; retail-page-0xb9"),
        palettes: &[PaletteBinding {
            entry: 7,
            bits: 8,
            first_words: &[0],
            sha256: "25389de257006aa7e940702803b621bae0bb0d6c4d46921eb80c4e8d765e1e06",
            selection: "GAME entry 1 records 4/5 select CLUT row 484, first 256 words of DEMO entry 7, and retail page 0xb9 matches this 8-bit atlas at (576, 256). Slot 0x25f requests the physical DEMO archive. Load success, upload order, overwrites and rendering are unresolved.",
        }],
    },
    Atlas {
        source: "BIN/ETC/DEMO.EMI",
        entry: 4,
        sha256: "5587f56d745abffaf77494349b0b902120eaf5cd3dbf521b6999f2e02c3b471d",
        categories: &["system-screen"],
        content: "III logo and Japanese subtitle raster",
        consumer: Some("BIN/ETC/GAME.EMI#1@0x801D16DC:records-2-3; retail-page-0xbb"),
        palettes: &[PaletteBinding {
            entry: 7,
            bits: 8,
            first_words: &[256],
            sha256: "25389de257006aa7e940702803b621bae0bb0d6c4d46921eb80c4e8d765e1e06",
            selection: "GAME entry 1 records 2/3 select CLUT row 485, DEMO entry 7 byte range 512..1024, and retail page 0xbb matches this 8-bit atlas at (704, 256). Slot 0x25f requests the physical DEMO archive. Load success, upload order, overwrites and rendering are unresolved.",
        }],
    },
    Atlas {
        source: "BIN/ETC/DEMO.EMI",
        entry: 5,
        sha256: "0f2750a7c89914c121a759bdb791220d639e2b2105aa5461e35784e8841c1a32",
        categories: &["interface", "system-screen"],
        content: "PRESS START BUTTON and copyright raster labels",
        consumer: Some("BIN/ETC/GAME.EMI#1@0x801D12CC:records-7-8-9-19; retail-page-0xbd"),
        palettes: &[PaletteBinding {
            entry: 7,
            bits: 8,
            first_words: &[512],
            sha256: "25389de257006aa7e940702803b621bae0bb0d6c4d46921eb80c4e8d765e1e06",
            selection: "GAME entry 1 records 7/8/9/19 select CLUT row 486, DEMO entry 7 byte range 1024..1536, and retail page 0xbd matches this 8-bit atlas at (832, 256). Slot 0x25f requests the physical DEMO archive. Load success, upload order, overwrites and rendering are unresolved.",
        }],
    },
];

pub(crate) fn preview_bindings(resource: &Resource) -> Vec<(usize, u8, usize)> {
    ATLASES
        .iter()
        .filter(|atlas| {
            resource.source == atlas.source
                && resource.entry == Some(atlas.entry)
                && resource.sha256 == atlas.sha256
        })
        .flat_map(|atlas| atlas.palettes)
        .flat_map(|binding| {
            binding
                .first_words
                .iter()
                .map(move |first| (binding.entry, binding.bits, *first))
        })
        .collect()
}

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
    for image in resources.iter_mut() {
        let Some(content) = SHARED_CONTENT
            .iter()
            .find(|content| image.sha256 == content.sha256)
        else {
            continue;
        };
        if image.kind != "texture-upload"
            || image.emi_type != Some(3)
            || image.size != 32768
            || image.load_argument != Some(content.argument)
        {
            return Err(Error::Invalid(format!(
                "{} differs from its qualified shared interface layout",
                image.id
            )));
        }
        annotate_content(image, content.categories, content.content);
        push_unique(&mut image.evidence, content.reference);
        push_unique(
            &mut image.findings,
            "identical original payloads share recognized raster content; palette, consumer and loading evidence remains specific to each source",
        );
    }
    for atlas in ATLASES {
        let Some(image_index) = locate(resources, atlas.source, atlas.entry) else {
            continue;
        };
        check_hash(&resources[image_index], atlas.sha256)?;
        let image = &mut resources[image_index];
        annotate_content(image, atlas.categories, atlas.content);
        if let Some(consumer) = atlas.consumer {
            push_unique(&mut image.evidence, consumer);
        }
        if atlas.palettes.is_empty() {
            push_unique(
                &mut image.findings,
                "content recognition does not establish this source's consumer or palette; equal payloads retain separate identities",
            );
        }
        for (number, binding) in atlas.palettes.iter().enumerate() {
            bind_palette(resources, image_index, atlas, binding, number == 0)?;
        }
    }
    Ok(())
}

fn annotate_content(image: &mut Resource, categories: &[&str], content: &str) {
    for category in categories {
        push_unique(&mut image.categories, category);
    }
    push_unique(&mut image.evidence, "qualified-original-interface-content");
    push_unique(&mut image.uses, content);
    push_unique(
        &mut image.findings,
        "recognized raster content can contain text; role classification does not certify non-text or runtime rendering",
    );
}

fn locate(resources: &[Resource], source: &str, entry: usize) -> Option<usize> {
    resources
        .iter()
        .position(|resource| resource.source == source && resource.entry == Some(entry))
}

fn bind_palette(
    resources: &mut [Resource],
    image_index: usize,
    atlas: &Atlas,
    binding: &PaletteBinding,
    first: bool,
) -> Result<()> {
    let palette_index = locate(resources, atlas.source, binding.entry).ok_or_else(|| {
        Error::Invalid(format!(
            "{} lacks interface palette entry {}",
            atlas.source, binding.entry
        ))
    })?;
    check_hash(&resources[palette_index], binding.sha256)?;
    let palette_id = resources[palette_index].id.clone();
    let image_id = resources[image_index].id.clone();
    let image = &mut resources[image_index];
    let dependency = Dependency {
        kind: "palette".into(),
        target: Some(palette_id),
        status: "partial".into(),
        reason: binding.selection.into(),
    };
    if let Some(existing) = image
        .dependencies
        .iter_mut()
        .find(|edge| first && edge.kind == "palette")
    {
        *existing = dependency;
    } else {
        image.dependencies.push(dependency);
    }
    let palette = &mut resources[palette_index];
    push_unique(&mut palette.uses, &image_id);
    if let Some(consumer) = atlas.consumer {
        push_unique(&mut palette.evidence, consumer);
    }
    push_unique(
        &mut palette.findings,
        "listed atlas uses are conditional static associations; individual selectors, upload order and runtime palette freshness remain unresolved",
    );
    Ok(())
}
