//! Source-qualified uses of banks by the original local effects dispatcher.
//! A reference is not sample classification or accepted execution of a cue.

use super::model::{AssetData, Association, Catalog, Layout, Role};

const DISPATCH: u32 = 0x8015_df18;
const HANDLERS: [u32; 7] = [
    0x8015_e994,
    0x8015_efac,
    0x8015_f5c8,
    0x8015_fbe4,
    0x8016_0200,
    0x8016_081c,
    0x8016_0e38,
];

pub(super) fn resolve(
    catalog: &mut Catalog,
    associations: &[Association],
    layouts: &[Layout],
    target: &'static str,
    load_address: u32,
) {
    for asset in &mut catalog.assets {
        let AssetData::Bank {
            metadata, content, ..
        } = &asset.data
        else {
            continue;
        };
        asset.roles.clear();
        asset.content_type = "unresolved";
        let Some(association) = associations.iter().find(|item| item.bank == asset.id) else {
            continue;
        };
        let [auxiliary] = association.auxiliary_entries.as_slice() else {
            continue;
        };
        let Some(entry) = catalog
            .sources
            .iter()
            .flat_map(|source| &source.entries)
            .find(|entry| &entry.id == auxiliary)
        else {
            continue;
        };
        let Some(records) = &entry.auxiliary_records else {
            continue;
        };
        let slot = usize::from(association.game_bank_id);
        if content.is_none()
            || layouts.is_empty()
            || layouts.iter().any(|layout| {
                let start = layout.slots[slot].auxiliary_address;
                let end = layout
                    .slots
                    .get(slot + 1)
                    .map_or(0x8014_8a04, |next| next.auxiliary_address);
                start.checked_add(entry.bytes).is_none_or(|last| last > end)
            })
        {
            continue;
        }
        let reference_rows: Vec<_> = records
            .iter()
            .enumerate()
            .filter(|(_, record)| {
                record[0] == 0
                    && record[3] & 0x80 == 0
                    && record[3] & 31 < 24
                    && (record[2] >> 4) + ((record[3] >> 5) & 3) < 16
                    && metadata
                        .programs
                        .iter()
                        .any(|program| program.program == record[1] & 0x7f)
            })
            .map(|(row, _)| row)
            .collect();
        if reference_rows.is_empty() {
            continue;
        }
        asset.content_type = "sfx";
        asset.roles.push(Role {
            role: "sfx",
            scope: "local_dispatch_reference",
            auxiliary_entry: auxiliary.clone(),
            reference_rows,
            target,
            dispatcher: DISPATCH,
            dispatcher_file_offset: (DISPATCH - load_address) as usize + 0x800,
            handler: HANDLERS[slot],
            handler_file_offset: (HANDLERS[slot] - load_address) as usize + 0x800,
            playback: "unverified: active layout, loaded state, physical tone/sample selection, histories and independent PCM remain unresolved",
        });
    }
}
