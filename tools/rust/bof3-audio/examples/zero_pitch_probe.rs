//! Diagnose note-on zero-step sample values; no SF2 acceptance follows automatically.
use bof3_audio::{
    archive::MediaImage, catalog::model::AssetData, catalog::model::Catalog, codec::adpcm::History,
    machine::executable::Executable, machine::spu_sample::Model, machine::spu_sample::Player,
    machine::spu_transfer::RAM_BYTES, soundfont::bank::Gain, soundfont::gain,
    voice::tuning::Reference,
};
use std::{collections::BTreeMap, fs, path::PathBuf};

fn main() -> bof3_audio::Result<()> {
    let args: Vec<_> = std::env::args_os().skip(1).collect();
    if args.len() != 2 {
        return Err("usage: zero_pitch_probe US_EXE CORPUS_ROOT".into());
    }
    let exe = Executable::from_bytes(fs::read(&args[0])?)?;
    let mut reference = Reference::from_executable(&exe)?;
    let root = PathBuf::from(&args[1]);
    let mut catalog = Catalog::read(Some(&root), &[])?;
    bof3_audio::catalog::loader::resolve(&mut catalog, &exe)?;
    let mut ram = vec![0; RAM_BYTES];
    let mut samples = BTreeMap::new();
    let mut tones = Vec::new();
    let mut unresolved = Vec::new();
    for asset in &catalog.assets {
        let AssetData::Bank {
            metadata, content, ..
        } = &asset.data
        else {
            continue;
        };
        let mut rows = Vec::new();
        for program in &metadata.programs {
            for tone in &program.tones {
                let zeros: Vec<_> = reference
                    .tone(tone, 44100)?
                    .into_iter()
                    .filter(|r| {
                        r.pitch_register == 0
                            && r.pitch_table_index.is_some_and(|i| (0..193).contains(&i))
                    })
                    .map(|r| r.key)
                    .collect();
                if !zeros.is_empty() {
                    rows.push((program, tone, zeros));
                }
            }
        }
        if rows.is_empty() {
            continue;
        }
        let Some(content) = content else {
            return Err("zero pitch: missing bank content".into());
        };
        let archive_source = catalog
            .sources
            .iter()
            .find(|s| s.source == asset.source)
            .ok_or("zero pitch: missing source identity")?;
        let body_index = archive_source
            .entries
            .iter()
            .find(|e| e.id == content.body_entry)
            .ok_or("zero pitch: missing body entry")?
            .entry;
        let MediaImage::Emi(image) = MediaImage::read(&root.join(&asset.source))? else {
            unreachable!()
        };
        if archive_source.sha256.as_deref()
            != Some(bof3_audio::digest::sha256_hex(image.bytes()).as_str())
        {
            return Err("zero pitch: source changed after catalog inspection".into());
        }
        let body = image.entry(body_index)?;
        for (program, tone, keys) in rows {
            let resolution = match bof3_audio::sample::reference::resolve_us_pcm(
                tone.sample_reference,
                metadata.declared_samples,
            ) {
                Ok(value) => value,
                Err(error) => {
                    unresolved.push(
                        serde_json::json!({"bank":asset.id,"archive_sha256":archive_source.sha256,"program":program.program,
                        "tone":tone.index,"keys":keys,"error":error.to_string()}),
                    );
                    continue;
                }
            };
            let source = metadata
                .samples
                .iter()
                .find(|s| s.sample_id == resolution.sample_id)
                .ok_or("zero pitch: missing sample")?;
            let block: &[u8; 16] = body
                .get(source.body_offset..source.body_offset + 16)
                .ok_or("zero pitch: missing first block")?
                .try_into()?;
            let first = History::default().decode_block(block)?[0];
            ram[0x200..0x210].copy_from_slice(block);
            let mut values = Vec::new();
            for model in [Model::Published, Model::EmulatorReference] {
                let mut player = Player::new(model);
                player.key_on(0x40);
                let initial = player.tick(&ram, 0, None)?;
                for _ in 0..64 {
                    let frame = player.tick(&ram, 0, None)?;
                    if frame.sample != initial.sample
                        || frame.fetched_address.is_some()
                        || frame.loop_end
                        || frame.mute
                        || player.counter() != 0
                    {
                        return Err("zero pitch: reader advanced or output changed".into());
                    }
                }
                values.push(initial.sample);
            }
            if values[0] != values[1] {
                return Err("zero pitch: arithmetic models disagree".into());
            }
            let gain = Gain::from_bank(metadata, program, tone, gain::Model::SpecificationScale)?;
            *samples.entry(values[0]).or_insert(0usize) += keys.len();
            tones.push(
                serde_json::json!({"bank":asset.id,"archive_sha256":archive_source.sha256,"program":program.program,"tone":tone.index,
                "keys":keys,"sample":resolution.sample_id,"first_decoded_sample":first,
                "held_sample":values[0],"gain_silent":gain.is_silent()}),
            );
        }
    }
    println!(
        "{}",
        serde_json::to_string_pretty(&serde_json::json!({
            "schema":"bof3.zero-pitch-probe/v1","executable_sha256":reference.profile().exe_sha256,
            "held_values_by_tone_key":samples,"tones":tones,"unresolved":unresolved,
            "limitations":["Isolated key-on reader state without pitch changes, modulation or noise.",
                "No independent hardware PCM validation or SF2 representability acceptance."]
        }))?
    );
    Ok(())
}
