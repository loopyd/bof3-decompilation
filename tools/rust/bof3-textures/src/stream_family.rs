//! Original stream requests and conditional archive entry order for sprite banks.
use serde::{Deserialize, Serialize};

use crate::{
    catalog::{Dependency, Resource},
    record::Record,
    sources,
};

/// The default save record selected by an original field-party identifier.
/// This does not identify an age, action, direction or current mutable name.
#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct PartyRecord {
    pub root: u8,
    pub selector_id: u8,
    pub record_index: u8,
    pub default_name: String,
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Request {
    pub family: u8,
    pub index: u8,
    pub slot: u16,
    pub lba: u32,
    pub selector_ids: [u8; 3],
    pub party_records: Option<[PartyRecord; 3]>,
}

const PARTY_RECORDS: [u8; 11] = [0, 1, 2, 3, 4, 5, 6, 0, 1, 0, 7];
const DEFAULT_NAMES: [&str; 8] = [
    "Ryu", "Nina", "Garr", "Teepo", "Rei", "Momo", "Peco", "Whelp",
];

struct Family {
    number: u8,
    prefix: &'static str,
    first_slot: u16,
    lbas: [u32; 19],
}

const SELECTORS: [[u8; 3]; 19] = [
    [0, 1, 2],
    [0, 1, 5],
    [0, 1, 6],
    [0, 2, 5],
    [0, 2, 6],
    [0, 3, 4],
    [0, 5, 6],
    [2, 4, 7],
    [2, 5, 7],
    [2, 6, 7],
    [2, 7, 8],
    [2, 7, 10],
    [3, 4, 9],
    [4, 5, 7],
    [4, 6, 7],
    [4, 7, 8],
    [5, 6, 7],
    [5, 7, 8],
    [6, 7, 8],
];

const FAMILIES: [Family; 3] = [
    Family {
        number: 0,
        prefix: "BIN/PLCHAR/PL",
        first_slot: 618,
        lbas: [
            0xf365, 0xf418, 0xf4b8, 0xf533, 0xf5f9, 0xf69a, 0xf732, 0xf7c0, 0xf883, 0xf947, 0xf9e7,
            0xfaa4, 0xfb3e, 0xfbc3, 0xfc72, 0xfcfc, 0xfda4, 0xfe30, 0xfeda,
        ],
    },
    Family {
        number: 1,
        prefix: "BIN/BPLCHAR/BPLD",
        first_slot: 475,
        lbas: [
            0xc0a0, 0xc136, 0xc1cc, 0xc24e, 0xc2f2, 0xc383, 0xc405, 0xc495, 0xc539, 0xc5e5, 0xc67e,
            0xc726, 0xc7a9, 0xc7ea, 0xc88d, 0xc91d, 0xc9bd, 0xca55, 0xcafd,
        ],
    },
    Family {
        number: 2,
        prefix: "BIN/BPLCHAR/BPLU",
        first_slot: 494,
        lbas: [
            0xcb92, 0xcc27, 0xccbe, 0xcd3f, 0xcde1, 0xce6d, 0xcef1, 0xcf7f, 0xd022, 0xd0cb, 0xd15e,
            0xd203, 0xd284, 0xd328, 0xd3cd, 0xd45c, 0xd4fd, 0xd593, 0xd63a,
        ],
    },
];

fn request(source: &str) -> Option<Request> {
    for family in &FAMILIES {
        for (index, ids) in SELECTORS.iter().enumerate() {
            let path = format!("{}{:X}{:X}{:X}.EMI", family.prefix, ids[0], ids[1], ids[2]);
            if source == path {
                return Some(Request {
                    family: family.number,
                    index: index as u8,
                    slot: family.first_slot + index as u16,
                    lba: family.lbas[index],
                    selector_ids: *ids,
                    party_records: None,
                });
            }
        }
    }
    None
}

pub(crate) fn annotate(resources: &mut [Resource], records: &mut [Record]) {
    let exe = format!("{}:SLUS_004.22#file", sources::RELEASE);
    let game = format!("{}:BIN/ETC/GAME.EMI#entry-000", sources::RELEASE);
    let has_exe = resources.iter().any(|resource| resource.id == exe);
    let has_game = resources.iter().any(|resource| resource.id == game);
    let defaults = [
        format!("{}:BIN/ETC/START.EMI#entry-008", sources::RELEASE),
        format!("{}:BIN/ETC/STATUS.EMI#entry-000", sources::RELEASE),
    ];
    let has_defaults = defaults
        .each_ref()
        .map(|id| resources.iter().any(|resource| &resource.id == id));
    for resource in resources.iter_mut() {
        if resource.load_argument != Some(0x8003_b800) || !matches!(resource.emi_type, Some(0 | 1))
        {
            continue;
        }
        let Some(mut request) = request(&resource.source) else {
            continue;
        };
        resource.evidence.extend([
            "SLUS_004.22@0x8016728C:stream-family-request".into(),
            "SLUS_004.22@0x80162160:slot-lba-lookup".into(),
            "SLUS_004.22@0x80182444:original-disc-lbas".into(),
            "SLUS_004.22@0x80181B34:three-selector-identifiers".into(),
        ]);
        resource.dependencies.push(Dependency {
            kind: "stream-request".into(),
            target: Some(exe.clone()),
            status: if has_exe { "partial" } else { "unresolved" }.into(),
            reason: format!(
                "Original family {} index {} requests slot {} at LBA {}. {} Request completion and final residency are unobserved.",
                request.family, request.index, request.slot, request.lba,
                if has_exe { "The original EXE is captured." } else { "The original EXE is not captured in this input." },
            ),
        });
        resource.dependencies.push(Dependency {
            kind: "root-selection".into(),
            target: Some(game.clone()),
            status: if has_game { "partial" } else { "unresolved" }.into(),
            reason: format!(
                "For this requested index, the original hint selects identifiers {:?}; a matching argument selects root ordinal 0/1/2 at 0x8003b800. {} Current argument, bank residency and the unchecked no-match ordinal 3 remain unresolved.",
                request.selector_ids,
                if has_game { "The GAME entry 0 consumer is captured." } else { "The GAME entry 0 consumer is not captured in this input." },
            ),
        });
        resource.evidence.extend([
            "BIN/ETC/GAME.EMI#0@0x801C1254:root-draw-context-initialization".into(),
            "SLUS_004.22@0x80166C48:selected-root-palette-zero-copy".into(),
            "SLUS_004.22@0x8014E22C:conditional-palette-block-upload".into(),
            "SLUS_004.22@0x8014CE78:draw-clut-selection".into(),
        ]);
        for (kind, target, captured, reason) in [
            (
                "draw-clut",
                &game,
                has_game,
                "For a matched root and explicit initialization slot s in 0..7, GAME sets entity flags=1, page=5, V base=(80*s)&255, selector=0x78+s and mode=2. The draw helper selects CLUT (495<<6)|(2*s). The initializer accepts an unchecked byte slot; this bounded relationship does not establish the gameplay slot domain or a current entity.",
            ),
            (
                "palette-stage",
                &exe,
                has_exe,
                "The initializer copies palette zero of the selected root, exactly 32 words, to RAM 0x80035600+64*s and its +0x4000 mirror, and marks the palette upload pending. For s in 0..7, the conditional (0,480,256,32) upload from 0x80037800 places those words at row 495, columns 32*s..32*s+31. This covers only indices 0..31; selected root, execution, neighboring colors and intervening overwrites remain unresolved.",
            ),
            (
                "image-stage",
                &exe,
                has_exe,
                "For the initialized entity, 0x8014DC08 queues the selected root image at VRAM x=320 words, y=(80*s)&255 unless entity byte zero has mask 0x02 set. The consumer 0x8014E6D0 uses the queued dimensions and codec. The base eight-bit texture page is 0x85 before blend bits and part-specific correction; image selection, completed upload and draw-time residency remain unobserved.",
            ),
        ] {
            resource.dependencies.push(Dependency {
                kind: kind.into(),
                target: Some(target.clone()),
                status: if captured { "partial" } else { "unresolved" }.into(),
                reason: if captured {
                    reason.into()
                } else {
                    format!("{reason} The required consumer source is not captured.")
                },
            });
        }
        resource.dependencies.push(Dependency {
            kind: "resident-base".into(),
            target: None,
            status: "unresolved".into(),
            reason: "The header declares destination 0x8003b800. Type-0 and type-1 writes can share it; transfer order, overwrites and active draw-time bytes are not established by a request.".into(),
        });
        if request.family == 0
            && resource
                .sprite_bank
                .as_ref()
                .is_some_and(|bank| bank.roots == 3)
        {
            request.party_records = Some(std::array::from_fn(|root| {
                let selector_id = request.selector_ids[root];
                let record_index = PARTY_RECORDS[usize::from(selector_id)];
                PartyRecord {
                    root: root as u8,
                    selector_id,
                    record_index,
                    default_name: DEFAULT_NAMES[usize::from(record_index)].into(),
                }
            }));
            resource.categories = vec!["field-character".into()];
            resource.evidence.extend([
                "BIN/ETC/GAME.EMI#0@0x801C1400:field-party-record-copy".into(),
                "SLUS_004.22@0x80181B10:identifier-to-party-record".into(),
                "BIN/ETC/START.EMI#8@0x801D15C0:default-record-initializer".into(),
                "BIN/ETC/STATUS.EMI#0@0x801D15C0:identical-default-record-initializer".into(),
            ]);
            for edge in &mut resource.dependencies {
                if edge.kind == "game-role" {
                    edge.reason = "The field-party record use is qualified. Age and form labels, actions, directions and complete consumer uses remain unresolved.".into();
                }
            }
            resource.dependencies.push(Dependency {
                kind: "party-record".into(),
                target: Some(game.clone()),
                status: if has_game && has_exe { "partial" } else { "unresolved" }.into(),
                reason: "GAME 0x801C14B8 passes an identifier to the root initializer. After the call, 0x801C14E8 reloads it and uses EXE 0x80181B10 to copy 0xA4 bytes from 0x80144968 + record_index*0xA4 to entity+0x74. Linking the two identifier reads requires stable table bytes and preserved saved registers. Current identifiers, scratch pointers, records and execution are unobserved.".into(),
            });
            for (target, captured) in defaults.iter().zip(has_defaults) {
                resource.dependencies.push(Dependency {
                    kind: "default-name".into(),
                    target: Some(target.clone()),
                    status: if captured && has_exe { "partial" } else { "unresolved" }.into(),
                    reason: "The identical START/STATUS initializer copies default records 0..6 from 0x801EB514 at stride 0xA4 to 0x80144968. It separately copies source record 7 from 0x801EB990 to the slot selected by EXE byte 0x80181B1A. The five-byte default names label records, not age variants, actions, mutable names or observed actors.".into(),
                });
            }
            for record in records
                .iter_mut()
                .filter(|record| record.parent == resource.id)
            {
                record.categories = resource.categories.clone();
                record.dependencies.push(Dependency {
                    kind: "party-record".into(),
                    target: Some(resource.id.clone()),
                    status: "conditional".into(),
                    reason: "The parent bank's party_records maps root ordinals to field-party default records. This stored record belongs to that bank; the mapping does not establish its active root, action, direction, execution or appearance.".into(),
                });
            }
        }
        resource.stream_request = Some(request);
    }
    annotate_entry_order(resources, has_exe);
}

fn annotate_entry_order(resources: &mut [Resource], has_exe: bool) {
    let pairs = resources
        .iter()
        .enumerate()
        .filter(|(_, resource)| {
            resource.source.starts_with("BIN/BPLCHAR/")
                && resource.entry == Some(3)
                && resource.emi_type == Some(0)
                && resource.load_argument == Some(0x8003_b800)
                && resource.size == 16384
                && resource.sha256
                    == "4fe7b59af6de3b665b67788cc2f99892ab827efae3a467342b3bb4e3bc8e5bfe"
        })
        .filter_map(|(clear, resource)| {
            let has_audio_records = [(4, 6), (5, 8), (6, 7)].iter().all(|&(entry, kind)| {
                resources.iter().any(|candidate| {
                    candidate.source == resource.source
                        && candidate.entry == Some(entry)
                        && candidate.emi_type == Some(kind)
                })
            });
            if !has_audio_records {
                return None;
            }
            let bank = resources.iter().position(|candidate| {
                candidate.source == resource.source
                    && candidate.entry == Some(7)
                    && candidate.emi_type == Some(1)
                    && candidate.load_argument == Some(0x8003_b800)
                    && candidate
                        .bank
                        .as_ref()
                        .is_some_and(|info| info.decoded_bytes > 16384)
            })?;
            Some((clear, bank))
        })
        .collect::<Vec<_>>();
    for (clear_index, bank_index) in pairs {
        let clear = resources[clear_index].id.clone();
        let bank = resources[bank_index].id.clone();
        for index in [clear_index, bank_index] {
            resources[index].evidence.extend([
                "SLUS_004.22@0x80161FDC:archive-header-first".into(),
                "SLUS_004.22@0x80162474:completed-entry-advance".into(),
                "SLUS_004.22@0x80162B08:conditional-record-selection".into(),
                "SLUS_004.22@0x801625E4:type0-sector-transfer".into(),
                "SLUS_004.22@0x80162C14:entry-copy-destination".into(),
            ]);
        }
        resources[clear_index].uses.push(bank);
        resources[bank_index].dependencies.push(Dependency {
            kind: "entry-order".into(),
            target: Some(clear),
            status: if has_exe { "partial" } else { "unresolved" }.into(),
            reason: format!(
                "On the original normal, unskipped no-error path with mode bit 0x20 clear, entry 3's 16384 zero bytes at RAM 0x8003b800..0x8003f800 finish transferring before entry 7 is selected. The later bank declares the same destination and a larger decoded span. Audio-owned types 6/8/7 intervene. {} Their complete RAM effects, later writes, request completion and draw-time state remain unresolved. This ordering does not establish a decoder dependency on zero history.",
                if has_exe {
                    "The original EXE entry-selection consumer is captured."
                } else {
                    "The required original EXE entry-selection consumer is not captured."
                },
            ),
        });
    }
}
