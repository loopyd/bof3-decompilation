//! Byte identity of resolved VAB header/body payloads, not playback equivalence.

use crate::catalog::model::{Content, Group};
use crate::{catalog::model::AssetData, catalog::model::Association, catalog::model::Catalog};
use std::collections::BTreeMap;

pub(crate) fn resolve(
    catalog: &mut Catalog,
    associations: &[Association],
    unresolved: &mut Vec<String>,
) -> Vec<Group> {
    let entries: BTreeMap<_, _> = catalog
        .sources
        .iter()
        .flat_map(|source| {
            source
                .entries
                .iter()
                .map(|entry| (entry.id.as_str(), entry))
        })
        .collect();
    let associated: BTreeMap<_, _> = associations
        .iter()
        .map(|item| (item.bank.as_str(), item))
        .collect();
    let mut groups = BTreeMap::<(String, String, u32, u32), Vec<String>>::new();
    for asset in &mut catalog.assets {
        let AssetData::Bank {
            header_sha256,
            content,
            metadata,
            ..
        } = &mut asset.data
        else {
            continue;
        };
        *content = None;
        let Some(association) = associated.get(asset.id.as_str()) else {
            continue;
        };
        let [body_id] = association.body_entries.as_slice() else {
            unresolved.push(format!("{}: bank content identity requires one resolved body, found {}; load history is required", asset.id, association.body_entries.len()));
            continue;
        };
        let Some(header) = entries.get(asset.id.as_str()) else {
            continue;
        };
        let Some(body) = entries.get(body_id.as_str()) else {
            continue;
        };
        let Some(body_sha256) = &body.body_sha256 else {
            continue;
        };
        let sample_bytes = metadata
            .samples
            .last()
            .map_or(metadata.body_prefix_bytes, |sample| {
                sample.body_offset + sample.encoded_bytes
            });
        if sample_bytes > body.bytes as usize {
            unresolved.push(format!("{}: declared samples require {sample_bytes} bytes but resolved body {} contains {}", asset.id, body_id, body.bytes));
            continue;
        }
        *content = Some(Content {
            body_entry: body_id.clone(),
            body_sha256: body_sha256.clone(),
            header_bytes: header.bytes,
            body_bytes: body.bytes,
            declared_sample_bytes: sample_bytes,
        });
        groups
            .entry((
                header_sha256.clone(),
                body_sha256.clone(),
                header.bytes,
                body.bytes,
            ))
            .or_default()
            .push(asset.id.clone());
    }
    groups.into_iter().filter_map(|((header_sha256, body_sha256, header_bytes, body_bytes), mut banks)| {
        if banks.len() < 2 {return None}
        banks.sort();
        Some(Group { header_sha256, body_sha256, header_bytes, body_bytes, banks,
            equivalence: "identical VH/VB payload hashes and lengths; archive layout, game identities and playback contexts remain separate" })
    }).collect()
}
