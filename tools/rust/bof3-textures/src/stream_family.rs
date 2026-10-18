//! Original stream requests for the qualified three-root bank source families.
use serde::{Deserialize, Serialize};

use crate::{
    catalog::{Dependency, Resource},
    sources,
};

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Request {
    pub family: u8,
    pub index: u8,
    pub slot: u16,
    pub lba: u32,
    pub selector_ids: [u8; 3],
}

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
                });
            }
        }
    }
    None
}

pub(crate) fn annotate(resources: &mut [Resource]) {
    let exe = format!("{}:SLUS_004.22#file", sources::RELEASE);
    let game = format!("{}:BIN/ETC/GAME.EMI#entry-000", sources::RELEASE);
    let has_exe = resources.iter().any(|resource| resource.id == exe);
    let has_game = resources.iter().any(|resource| resource.id == game);
    for resource in resources {
        if resource.load_argument != Some(0x8003_b800) || !matches!(resource.emi_type, Some(0 | 1))
        {
            continue;
        }
        let Some(request) = request(&resource.source) else {
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
        resource.dependencies.push(Dependency {
            kind: "resident-base".into(),
            target: None,
            status: "unresolved".into(),
            reason: "The header declares destination 0x8003b800. Type-0 and type-1 writes can share it; transfer order, overwrites and active draw-time bytes are not established by a request.".into(),
        });
        resource.stream_request = Some(request);
    }
}
