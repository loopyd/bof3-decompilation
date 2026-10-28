//! Qualified raster content, conditional CLUT associations and palette staging.
//! `archive-request` names the original requester; `palette-stage` names the EXE
//! copy routine. Missing captured prerequisites remain unresolved. Static
//! staging does not establish subsequent GPU execution or palette residency.

use crate::{
    Error, Result,
    catalog::{Dependency, Resource},
    sources,
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

struct PaletteStager {
    source: &'static str,
    slot: u16,
    request: &'static str,
    stage: PaletteStage,
}

#[derive(Clone, Copy)]
enum PaletteStage {
    InitialBlock,
    SelectedRows,
    TitleBlock,
}

const PALETTE_STAGERS: &[PaletteStager] = &[
    PaletteStager {
        source: "BIN/ETC/FIRST.EMI",
        slot: 0x261,
        request: "SLUS_004.22@0x8014EAB4:archive-request",
        stage: PaletteStage::InitialBlock,
    },
    PaletteStager {
        source: "BIN/ETC/BATE.EMI",
        slot: 0x257,
        request: "BIN/ETC/GAME.EMI#0@0x801987D8:archive-request",
        stage: PaletteStage::SelectedRows,
    },
    PaletteStager {
        source: "BIN/ETC/SHISU.EMI",
        slot: 0x265,
        request: "BIN/ETC/GAME.EMI#0@0x80198998:archive-request",
        stage: PaletteStage::SelectedRows,
    },
    PaletteStager {
        source: "BIN/ETC/DEMO.EMI",
        slot: 0x25f,
        request: "BIN/ETC/GAME.EMI#1@0x801D0C90:archive-request-and-ready-poll",
        stage: PaletteStage::TitleBlock,
    },
];

const TITLE_PALETTE_ROWS: [(usize, u32, &str); 1] = [(
    7,
    4,
    "25389de257006aa7e940702803b621bae0bb0d6c4d46921eb80c4e8d765e1e06",
)];

const INITIAL_PALETTE_ROWS: [(usize, u32, &str); 4] = [
    (
        8,
        0,
        "166ead38de405cf28c1cfc35a7c4a2b9b4a69ae95a128b970cf4b9df8b4567c7",
    ),
    (
        9,
        1,
        "a8a1701c0555c20d44259c66123fb491efc1df1a87c89e850078fa7948a64fdb",
    ),
    (
        10,
        2,
        "5cfa8f083db270a3af233d0213475a13b96f01c6bac9bfb38b6423e4fe27dec7",
    ),
    (
        13,
        26,
        "54a7ea6d69ff675bf1e07ba1fcd488e44ebcb50c9ad945bf1068abd43791269a",
    ),
];

const SELECTED_PALETTE_ROWS: [(usize, u32, &str); 2] = [
    (
        3,
        1,
        "a8a1701c0555c20d44259c66123fb491efc1df1a87c89e850078fa7948a64fdb",
    ),
    (
        4,
        2,
        "5cfa8f083db270a3af233d0213475a13b96f01c6bac9bfb38b6423e4fe27dec7",
    ),
];

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
            "SLUS_004.22@0x80150098:8x8-glyphs; SLUS_004.22@0x8014F84C:8x12-glyph-selection; SLUS_004.22@0x8014FB80:rectangle-packet; SLUS_004.22@0x8014FC00:retail-page-0x2f",
        ),
        palettes: &[PaletteBinding {
            entry: 8,
            bits: 4,
            first_words: &[
                0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240,
            ],
            sha256: "166ead38de405cf28c1cfc35a7c4a2b9b4a69ae95a128b970cf4b9df8b4567c7",
            selection: "FIRST entry 8 stages VRAM row 480, words 0..255. The 8x8 renderer selects CLUT 0x7800 | (A2 & 63), so this source covers only selectors 0..15 of that path. The 8x12 caller selects the low four flag bits, including palette-control operands. Retail page 0x2f matches the atlas at (960, 0), 4 bits per pixel. Call selection, upload order, intervening overwrites and rendering remain unresolved.",
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
    Atlas {
        source: "BIN/ETC/DEMO.EMI",
        entry: 6,
        sha256: "d933822cecb0f13950af92494d34b61f36c15feee188fe21ca6798e75a711b2a",
        categories: &["system-screen"],
        content: "Four indexed panels selected by the scrolling title-screen consumer",
        consumer: Some(
            "BIN/ETC/GAME.EMI#1@0x801D18F8:four-panel-loop; BIN/ETC/GAME.EMI#1@0x801D17D8:rectangle-consumer; BIN/ETC/GAME.EMI#1@0x801D1C6C:records-11-12-13-14",
        ),
        palettes: &[PaletteBinding {
            entry: 7,
            bits: 8,
            first_words: &[768, 1024, 1280, 1536],
            sha256: "25389de257006aa7e940702803b621bae0bb0d6c4d46921eb80c4e8d765e1e06",
            selection: "GAME entry 1 records 11..14 select CLUT rows 487..490, DEMO entry 7 byte ranges 1536..2048, 2048..2560, 2560..3072 and 3072..3584 respectively. The four-iteration loop selects retail pages 0x85/0x87/0x89/0x8b at VRAM x=320/448/576/704, y=0, 8 bits per pixel. Each record samples UV (0,0), width 256, height 192 within its corresponding quarter of the 1024x256-pixel upload. Whole-atlas previews under each palette are derivatives, not assembled screen frames. Loading, upload order, overwrites, scrolling, fades and rendering remain unobserved.",
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
    annotate_glyph_layout(resources);
    annotate_staging(resources)?;
    Ok(())
}

fn annotate_glyph_layout(resources: &mut [Resource]) {
    let Some(index) = locate(resources, "BIN/ETC/FIRST.EMI", 3) else {
        return;
    };
    let executable = format!("{}:SLUS_004.22#file", sources::RELEASE);
    let captured = resources.iter().any(|resource| resource.id == executable);
    let layout = "SLUS_004.22@0x8014FA58: ordinary-byte branch uses d=byte-48, q=trunc(d/31), r=d-31*q; U=low8(8*r), V=low8(72+12*q+crop_top). Control 0x15 instead uses n=next_byte+91, q=trunc(n/21), r=n-21*q; U=low8(12*r), V=low8(12*q+crop_top), without the ordinary path's +72. Both emit width=8, height=12-crop_reduction and advance X by 8. The 16 two-byte crop records at 0x8017F580 store crop_top then crop_reduction; initial flag bits 4..7 select the record once. Low four palette flag bits select CLUT 0x7800..0x780f; controls 0x05/0x06 change this state without reloading crop. Controls 0x00/0x01/0x05/0x06/0x07/0x15/0xff precede ordinary-byte dispatch; 0xff advances X without emitting. These formulas require nonaliasing source/packet storage and preservation of relevant saved registers across calls. Texture-window state, valid text domains, GPU submission and display remain unresolved.";
    resources[index].dependencies.push(Dependency {
        kind: "glyph-layout".into(),
        target: Some(executable),
        status: if captured { "partial" } else { "unresolved" }.into(),
        reason: if captured {
            layout.into()
        } else {
            format!("{layout} The original executable is outside this input.")
        },
    });
}

fn annotate_staging(resources: &mut [Resource]) -> Result<()> {
    let game = format!("{}:BIN/ETC/GAME.EMI#entry-000", sources::RELEASE);
    let title = format!("{}:BIN/ETC/GAME.EMI#entry-001", sources::RELEASE);
    let stager = format!("{}:SLUS_004.22#file", sources::RELEASE);
    let game_captured = resources.iter().any(|resource| resource.id == game);
    let title_captured = resources.iter().any(|resource| resource.id == title);
    let stager_captured = resources.iter().any(|resource| resource.id == stager);
    for source in PALETTE_STAGERS {
        let (rows, requester, requester_captured, routine) = match source.stage {
            PaletteStage::InitialBlock => (
                INITIAL_PALETTE_ROWS.as_slice(),
                &stager,
                stager_captured,
                "SLUS_004.22@0x8014E284:32-row-palette-block-copy",
            ),
            PaletteStage::SelectedRows => (
                SELECTED_PALETTE_ROWS.as_slice(),
                &game,
                game_captured,
                "SLUS_004.22@0x8014E3A0:256-word-palette-row-copy",
            ),
            PaletteStage::TitleBlock => (
                TITLE_PALETTE_ROWS.as_slice(),
                &title,
                title_captured,
                "SLUS_004.22@0x8014E284:32-row-palette-block-copy",
            ),
        };
        for &(entry, row, sha256) in rows {
            let Some(index) = locate(resources, source.source, entry) else {
                continue;
            };
            let palette = &mut resources[index];
            check_hash(palette, sha256)?;
            let source_ram = 0x8003_3800 + row * 512;
            let stage_ram = 0x8003_7800 + row * 512;
            let row_count = match source.stage {
                PaletteStage::TitleBlock => 8,
                _ => 1,
            };
            if palette.kind != "palette-bank"
                || palette.palette_upload.is_none()
                || palette.load_argument != Some(source_ram)
                || palette.size != u64::from(row_count) * 512
            {
                return Err(Error::Invalid(format!(
                    "{} differs from its qualified palette staging layout",
                    palette.id
                )));
            }
            palette.evidence.extend([
                source.request.into(),
                "SLUS_004.22@0x80162160:slot-lba-lookup".into(),
                routine.into(),
            ]);
            let staging = match source.stage {
                PaletteStage::InitialBlock => {
                    "The original bootstrap clears source rows 28..31, then copies all 32 rows of 256 palette words into staging RAM. Its path follows the ready branch and intervening initialization calls. Other resident source rows remain unresolved."
                }
                PaletteStage::SelectedRows => {
                    "After the ready branch, the original initializer copies rows 1 and 2, each containing 256 palette words, into staging RAM."
                }
                PaletteStage::TitleBlock => {
                    "The original title initializer waits for DEMO to become ready, calls the 32-row palette copy and marks the upload pending. DEMO entry 7 supplies source rows 4..11; other copied source rows are not supplied by this entry."
                }
            };
            for (kind, target, captured, reason) in [
                (
                    "archive-request",
                    requester,
                    requester_captured,
                    format!(
                        "Original requester selects slot {:#x} for {} and polls readiness before palette staging. Actual execution and loading are unobserved.",
                        source.slot, source.source
                    ),
                ),
                (
                    "palette-stage",
                    &stager,
                    stager_captured,
                    format!(
                        "{staging} This archive span declares RAM {source_ram:#x}; the copy maps that address to staging RAM {stage_ram:#x}. The separately qualified conditional upload places its {row_count} rows at VRAM y={}..{}. Other RAM producers, overwrites, GPU execution and active residency are unresolved.",
                        480 + row,
                        480 + row + row_count - 1
                    ),
                ),
            ] {
                palette.dependencies.push(Dependency {
                    kind: kind.into(),
                    target: Some(target.clone()),
                    status: if captured { "partial" } else { "unresolved" }.into(),
                    reason: if captured {
                        reason
                    } else {
                        format!("{reason} The required consumer source is not captured.")
                    },
                });
            }
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
