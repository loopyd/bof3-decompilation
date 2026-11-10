//! Diagnose note-on zero-step sample values; no SF2 acceptance follows automatically.
use bof3_audio::{
    archive::MediaImage,
    catalog::model::AssetData,
    catalog::model::Catalog,
    codec::adpcm::History,
    machine::image::executable::Executable,
    machine::spu_sample::{Model, Player, Signal},
    machine::spu_transfer::RAM_BYTES,
    soundfont::bank::Gain,
    soundfont::gain,
};
use std::{collections::BTreeMap, fs, path::PathBuf};

fn main() -> bof3_audio::Result<()> {
    let args: Vec<_> = std::env::args_os().skip(1).collect();
    if args.len() != 2 {
        return Err("usage: zero_pitch_probe US_EXE CORPUS_ROOT".into());
    }
    let exe = Executable::from_bytes(fs::read(&args[0])?)?;
    let mut reference = bof3_audio::machine::reference::tuning::from_executable(&exe)?;
    let root = PathBuf::from(&args[1]);
    let mut catalog = Catalog::read(Some(&root), &[])?;
    bof3_audio::catalog::loader::resolve(
        &mut catalog,
        &bof3_audio::machine::reference::mapping::read(&exe).unwrap(),
    )?;
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
                player.write_pitch(0);
                player.key_on(0x40);
                let initial = player.tick(&ram, None, Signal::Ordinary { unmuted: true })?;
                for frame_index in 0..64 {
                    let frame = player.tick(&ram, None, Signal::Ordinary { unmuted: true })?;
                    let expected_position = match model {
                        Model::Published => 0,
                        Model::EmulatorReference => 0x20000 + frame_index + 2,
                    };
                    if frame.sample != initial.sample
                        || frame.fetched_address.is_some()
                        || frame.loop_end
                        || frame.mute
                        || frame.stopped
                        || player.counter() != expected_position
                    {
                        return Err("zero pitch: unexpected model progression or output".into());
                    }
                }
                values.push(initial.sample);
            }
            let gain = Gain::from_bank(metadata, program, tone, gain::Model::SpecificationScale)?;
            *samples.entry(values[0]).or_insert(0usize) += keys.len();
            tones.push(
                serde_json::json!({"bank":asset.id,"archive_sha256":archive_source.sha256,"program":program.program,"tone":tone.index,
                "keys":keys,"sample":resolution.sample_id,"first_decoded_sample":first,
                "published_held_sample":values[0],"native_first_sample":values[1],"gain_silent":gain.is_silent()}),
            );
        }
    }
    println!(
        "{}",
        serde_json::to_string_pretty(&serde_json::json!({
            "schema":"bof3.zero-pitch-probe/v2","executable_sha256":reference.profile().exe_sha256,
            "published_held_values_by_tone_key":samples,"tones":tones,"unresolved":unresolved,
            "limitations":["Isolated key-on reader state without pitch changes, modulation or noise.",
                "Native-reference output covers 65 frames with a one-unit 16.16 increment, not a frozen position.",
                "No independent hardware PCM validation or SF2 representability acceptance."]
        }))?
    );
    Ok(())
}
