//! Checked SoundFont hydra table relationships; unknown synthesis data is retained.
use crate::{
    soundfont::reader::model::Generator, soundfont::reader::model::Instrument,
    soundfont::reader::model::Modulator, soundfont::reader::model::Preset,
    soundfont::reader::model::Sample, soundfont::reader::model::Zone, Result,
};
use std::collections::{BTreeMap, BTreeSet};

pub(crate) const TABLES: [([u8; 4], usize); 9] = [
    (*b"phdr", 38),
    (*b"pbag", 4),
    (*b"pmod", 10),
    (*b"pgen", 4),
    (*b"inst", 22),
    (*b"ibag", 4),
    (*b"imod", 10),
    (*b"igen", 4),
    (*b"shdr", 46),
];

pub(crate) fn read(
    tables: &BTreeMap<[u8; 4], &[u8]>,
    limit: usize,
) -> Result<(Vec<Preset>, Vec<Instrument>, Vec<Sample>)> {
    let mut records = 0usize;
    for (id, width) in TABLES {
        let data = tables
            .get(&id)
            .ok_or_else(|| format!("SF2: missing {} table", label(&id)))?;
        if data.is_empty() || data.len() % width != 0 {
            return Err(format!("SF2: invalid {} record framing", label(&id)).into());
        }
        records = records
            .checked_add(data.len() / width)
            .ok_or("SF2: record count overflow")?;
        if records > limit {
            return Err("SF2: table record limit exceeded".into());
        }
    }
    let phdr = tables[b"phdr"];
    let inst = tables[b"inst"];
    let shdr = tables[b"shdr"];
    if phdr.len() < 76 || inst.len() < 44 || shdr.len() < 92 {
        return Err(
            "SF2: preset, instrument and sample tables require a data and terminal record".into(),
        );
    }
    let preset_zones = zones(
        phdr,
        38,
        24,
        tables[b"pbag"],
        tables[b"pgen"],
        tables[b"pmod"],
        41,
        inst.len() / 22 - 1,
    )?;
    let instrument_zones = zones(
        inst,
        22,
        20,
        tables[b"ibag"],
        tables[b"igen"],
        tables[b"imod"],
        53,
        shdr.len() / 46 - 1,
    )?;
    let mut presets = Vec::new();
    let mut identities = BTreeSet::new();
    for (row, zones) in phdr.as_chunks::<38>().0.iter().zip(preset_zones) {
        let program = u16_at(row, 20);
        let bank = u16_at(row, 22);
        if program > 127 || bank > 128 || !identities.insert((bank, program)) {
            return Err(
                format!("SF2: invalid or duplicate preset bank {bank} program {program}").into(),
            );
        }
        presets.push(Preset {
            name: row[..20].try_into().unwrap(),
            program,
            bank,
            library: u32_at(row, 26),
            genre: u32_at(row, 30),
            morphology: u32_at(row, 34),
            zones,
        });
    }
    let instruments = inst
        .as_chunks::<22>()
        .0
        .iter()
        .zip(instrument_zones)
        .map(|(row, zones)| Instrument {
            name: row[..20].try_into().unwrap(),
            zones,
        })
        .collect();
    let samples = shdr
        .as_chunks::<46>()
        .0
        .iter()
        .take(shdr.len() / 46 - 1)
        .map(|row| Sample {
            name: row[..20].try_into().unwrap(),
            start: u32_at(row, 20),
            end: u32_at(row, 24),
            loop_start: u32_at(row, 28),
            loop_end: u32_at(row, 32),
            rate: u32_at(row, 36),
            root_key: row[40],
            correction_cents: row[41] as i8,
            link: u16_at(row, 42),
            kind: u16_at(row, 44),
        })
        .collect();
    Ok((presets, instruments, samples))
}

#[allow(clippy::too_many_arguments)]
fn zones(
    headers: &[u8],
    width: usize,
    bag_offset: usize,
    bags: &[u8],
    generators: &[u8],
    modulators: &[u8],
    link: u16,
    targets: usize,
) -> Result<Vec<Vec<Zone>>> {
    let bag_count = bags.len() / 4;
    let header_indices: Vec<_> = headers
        .chunks_exact(width)
        .map(|r| usize::from(u16_at(r, bag_offset)))
        .collect();
    indices(&header_indices, bag_count - 1, "header/bag")?;
    let generator_indices: Vec<_> = bags
        .as_chunks::<4>()
        .0
        .iter()
        .map(|r| usize::from(u16_at(r, 0)))
        .collect();
    let modulator_indices: Vec<_> = bags
        .as_chunks::<4>()
        .0
        .iter()
        .map(|r| usize::from(u16_at(r, 2)))
        .collect();
    indices(
        &generator_indices,
        generators.len() / 4 - 1,
        "bag/generator",
    )?;
    indices(
        &modulator_indices,
        modulators.len() / 10 - 1,
        "bag/modulator",
    )?;
    let mut result = Vec::new();
    for (owner, pair) in header_indices.windows(2).enumerate() {
        if pair[0] == pair[1] {
            return Err(format!("SF2: owner {owner} has no zones").into());
        }
        let mut zones = Vec::new();
        for bag in pair[0]..pair[1] {
            let gs = &generators[generator_indices[bag] * 4..generator_indices[bag + 1] * 4];
            let ms = &modulators[modulator_indices[bag] * 10..modulator_indices[bag + 1] * 10];
            let generators: Vec<_> = gs
                .as_chunks::<4>()
                .0
                .iter()
                .map(|r| Generator {
                    operator: u16_at(r, 0),
                    amount: u16_at(r, 2),
                })
                .collect();
            let modulators = ms
                .as_chunks::<10>()
                .0
                .iter()
                .map(|r| Modulator {
                    source: u16_at(r, 0),
                    destination: u16_at(r, 2),
                    amount: u16_at(r, 4) as i16,
                    amount_source: u16_at(r, 6),
                    transform: u16_at(r, 8),
                })
                .collect();
            let target = generators
                .last()
                .filter(|g| g.operator == link)
                .map(|g| usize::from(g.amount));
            if target.is_some_and(|i| i >= targets) {
                return Err(
                    format!("SF2: owner {owner} bag {bag} link exceeds target table").into(),
                );
            }
            if target.is_none() && (bag != pair[0] || pair[1] - pair[0] < 2) {
                return Err(format!(
                    "SF2: owner {owner} bag {bag} lacks terminal link or valid first global zone"
                )
                .into());
            }
            let mut seen = BTreeSet::new();
            for (index, generator) in generators.iter().enumerate() {
                if !seen.insert(generator.operator) {
                    return Err(format!(
                        "SF2: bag {bag} duplicate generator {}",
                        generator.operator
                    )
                    .into());
                }
                if generator.operator == link && index + 1 != generators.len() {
                    return Err(format!("SF2: bag {bag} generator follows terminal link").into());
                }
                if matches!(generator.operator, 43 | 44) {
                    let [low, high] = generator.amount.to_le_bytes();
                    if low > high || high > 127 {
                        return Err(format!("SF2: bag {bag} invalid key/velocity range").into());
                    }
                    if (generator.operator == 43 && index != 0)
                        || (generator.operator == 44
                            && index != 0
                            && !(index == 1 && generators[0].operator == 43))
                    {
                        return Err(
                            format!("SF2: bag {bag} key/velocity range ordering invalid").into(),
                        );
                    }
                }
            }
            zones.push(Zone {
                generators,
                modulators,
                target,
            });
        }
        result.push(zones);
    }
    Ok(result)
}

fn indices(values: &[usize], terminal: usize, context: &str) -> Result<()> {
    if values.first() != Some(&0)
        || values.last() != Some(&terminal)
        || values.windows(2).any(|p| p[0] > p[1])
    {
        return Err(format!(
            "SF2: {context} indices must start at zero, be monotonic and match terminal count"
        )
        .into());
    }
    Ok(())
}
pub(crate) fn u16_at(bytes: &[u8], offset: usize) -> u16 {
    u16::from_le_bytes(bytes[offset..offset + 2].try_into().unwrap())
}
pub(crate) fn u32_at(bytes: &[u8], offset: usize) -> u32 {
    u32::from_le_bytes(bytes[offset..offset + 4].try_into().unwrap())
}
fn label(id: &[u8; 4]) -> String {
    String::from_utf8_lossy(id).into_owned()
}
