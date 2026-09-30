//! Bank WAV/XML extraction with complete EMI preservation and staged publication.

use crate::{
    archive::MediaImage, catalog::loader, catalog::model::Asset, catalog::model::AssetData,
    catalog::model::Catalog, codec::adpcm, digest::sha256_hex, document::xml, interchange::wave,
    machine::executable::Executable, publication::require_absent, publication::write_new,
    publication::Publication, Result,
};
use serde::Serialize;
use std::{collections::BTreeSet, fmt::Write as _, fs, path::PathBuf};

pub struct Options {
    pub disc_root: Option<PathBuf>,
    pub archives: Vec<PathBuf>,
    pub executable: PathBuf,
    pub output: PathBuf,
    pub id: Option<String>,
    pub reference_rate: u32,
}

#[derive(Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub output: String,
    pub banks: usize,
    pub samples: usize,
    pub pcm_frames: usize,
    pub approximate_loops: usize,
    pub reference_rate: u32,
    pub assumptions: Vec<&'static str>,
}

pub fn banks(options: &Options) -> Result<Report> {
    // A custom reference rate represents a register context, not resampling or
    // an intrinsic VAB sample rate. Keep the exact SPU register representable.
    reference_pitch(options.reference_rate)?;
    require_absent(&options.output)?;
    let mut catalog = Catalog::read(options.disc_root.as_deref(), &options.archives)?;
    let executable = Executable::from_bytes(fs::read(&options.executable)?)?;
    let mapping = loader::resolve(&mut catalog, &executable)?;
    let selected = if let Some(id) = &options.id {
        vec![catalog.select("audio", Some("bank"), id)?]
    } else {
        catalog
            .assets
            .iter()
            .filter(|asset| asset.kind() == "bank")
            .collect()
    };
    if selected.is_empty() {
        return Err("extract: no banks found in selected sources".into());
    }
    let transaction = Publication::new(&options.output)?;
    let mut root = String::from("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<audio schema=\"bof3.audio-extraction/v1\" mode=\"audio\" kind=\"bank\">\n");
    let mut report = Report {
        schema: "bof3.audio-extract-report/v1", output: options.output.display().to_string(),
        banks: 0, samples: 0, pcm_frames: 0, approximate_loops: 0,
        reference_rate: options.reference_rate,
        assumptions: vec![
            "WAV rate is an export reference context; VAB samples have no universal playback rate",
            "WAV unity note 60 is an export convention; original tone center/shift and key ranges are recorded separately",
            "fixed PCM loops with pcm_repeat_is_stable=false approximate continuing ADPCM predictor history",
            "gameplay note/voice context and SFX/vocal classification remain unresolved",
            "XML includes full original EMI bytes; bank PCM/loop packing is supported within existing allocations; music extraction remains incomplete",
        ],
    };
    let mut paths = BTreeSet::new();
    for asset in selected {
        let source = catalog
            .sources
            .iter()
            .find(|source| source.source == asset.source)
            .ok_or("bank source missing from catalog")?;
        let path = options.disc_root.as_ref().map_or_else(
            || PathBuf::from(&asset.source),
            |root| root.join(&asset.source),
        );
        let media = MediaImage::read(&path)?;
        let hash = sha256_hex(media.bytes());
        if source.sha256.as_deref() != Some(&hash) {
            return Err(format!("{}: source changed after inventory", asset.source).into());
        }
        let MediaImage::Emi(image) = media else {
            return Err("bank source is not EMI".into());
        };
        let entry = asset.entry.ok_or("bank header entry missing")?;
        let folder = format!("banks/{}/bank-{entry}", sha256_hex(asset.source.as_bytes()));
        if !paths.insert(folder.clone()) {
            return Err("duplicate bank output path".into());
        }
        let directory = transaction.staging.join(&folder);
        fs::create_dir_all(&directory)?;
        let AssetData::Bank {
            content,
            metadata,
            header_sha256,
            ..
        } = &asset.data
        else {
            unreachable!()
        };
        let content = content.as_ref().ok_or_else(|| format!("{}: extraction requires exactly one verified, sufficiently sized bank body; inspect map diagnostics", asset.id))?;
        let body_entry = source
            .entries
            .iter()
            .find(|entry| entry.id == content.body_entry)
            .ok_or("resolved bank body entry missing")?;
        let body = image.entry(body_entry.entry)?;
        if sha256_hex(image.entry(entry)?) != *header_sha256
            || sha256_hex(body) != content.body_sha256
        {
            return Err("bank payload changed after mapping".into());
        }
        let mut manifest = String::from("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        writeln!(manifest, "<bank schema=\"bof3.audio-bank/v1\" id=\"{}\" source=\"{}\" header_entry=\"{entry}\" body_entry=\"{}\" game_bank_id=\"{}\" content_type=\"unresolved\">", xml::attribute(&asset.id)?, xml::attribute(&asset.source)?, body_entry.entry, asset.game_bank_id.ok_or("resolved game bank ID missing")?)?;
        writeln!(manifest, "<runtime profile=\"{}\" executable_sha256=\"{}\" pitch_entry=\"{}\" pitch_table=\"{}\" playback_context=\"unresolved\"/>", mapping.profile.id, mapping.profile.exe_sha256, mapping.profile.pitch_entry, mapping.profile.pitch_table)?;
        writeln!(manifest, "<export reference_rate=\"{}\" reference_pitch_register=\"{}\" midi_unity_note=\"60\" channels=\"1\" rate_context=\"export_reference_only\"/>", options.reference_rate, reference_pitch(options.reference_rate)?)?;
        let association = mapping
            .associations
            .iter()
            .find(|a| a.bank == asset.id)
            .ok_or("bank loader association missing")?;
        writeln!(
            manifest,
            "<loader load_slot=\"{}\" active_layout=\"unresolved\">",
            association.load_slot
        )?;
        for layout in &mapping.layouts {
            let slot = layout
                .slots
                .get(usize::try_from(association.load_slot)?)
                .ok_or("bank load slot exceeds verified layout")?;
            writeln!(
                manifest,
                "<possible_layout selector=\"{}\"{}/>",
                layout.selector,
                xml::attributes(slot, &[])?
            )?;
        }
        for auxiliary in &association.auxiliary_entries {
            writeln!(
                manifest,
                "<auxiliary id=\"{}\"/>",
                xml::attribute(auxiliary)?
            )?;
        }
        for song in &association.songs {
            let song_asset = catalog
                .assets
                .iter()
                .find(|a| a.id == *song)
                .ok_or("associated song missing")?;
            writeln!(manifest, "<song id=\"{}\">", xml::attribute(song)?)?;
            for cue in &song_asset.game_song_ids {
                writeln!(manifest, "<game_song id=\"{cue}\"/>")?;
            }
            manifest.push_str("</song>\n");
        }
        manifest.push_str("</loader>\n");
        writeln!(
            manifest,
            "<metadata{}>",
            xml::attributes(metadata, &["programs", "samples", "diagnostics"])?
        )?;
        for program in &metadata.programs {
            writeln!(
                manifest,
                "<program{}>",
                xml::attributes(program, &["tones"])?
            )?;
            for tone in &program.tones {
                writeln!(manifest, "<tone{}/>", xml::attributes(tone, &[])?)?;
            }
            manifest.push_str("</program>\n");
        }
        for diagnostic in &metadata.diagnostics {
            writeln!(
                manifest,
                "<diagnostic message=\"{}\"/>",
                xml::attribute(diagnostic)?
            )?;
        }
        manifest.push_str("</metadata>\n<samples>\n");
        for sample in &metadata.samples {
            let end = sample
                .body_offset
                .checked_add(sample.encoded_bytes)
                .ok_or("sample range overflow")?;
            let encoded = body
                .get(sample.body_offset..end)
                .ok_or("sample exceeds body")?;
            let decoded = adpcm::decode_sample(encoded)
                .map_err(|e| format!("{} sample {}: {e}", asset.id, sample.sample_id))?;
            let mut wav = wave::Wave::new(1, options.reference_rate, decoded.pcm)?;
            let mut sampler = wave::Sampler::new(options.reference_rate, 60)?;
            if let Some(loop_) = decoded.sample_loop {
                sampler.loops.push(wave::SampleLoop::forward(
                    u32::try_from(loop_.start_frame)?,
                    u32::try_from(loop_.end_frame_exclusive)?,
                )?);
                if !loop_.pcm_repeat_is_stable {
                    report.approximate_loops += 1;
                }
            }
            wav.sampler = Some(sampler);
            let wav_bytes = wav.to_bytes()?;
            let name = format!("sample-{:03}.wav", sample.sample_id);
            write_new(&directory.join(&name), &wav_bytes)?;
            let written = fs::read(directory.join(&name))?;
            let check = wave::Wave::from_bytes(&written)?;
            if written != wav_bytes || check.pcm != wav.pcm || check.to_bytes()? != wav_bytes {
                return Err(format!("{}: WAV read-back verification failed", asset.id).into());
            }
            let termination = serde_json::to_value(decoded.termination)?;
            writeln!(manifest, "<sample{} path=\"{name}\" encoded_sha256=\"{}\" wav_sha256=\"{}\" frames=\"{}\" decoded_bytes=\"{}\" trailing_bytes=\"{}\" termination=\"{}\">", xml::attributes(sample, &[])?, sha256_hex(encoded), sha256_hex(&wav_bytes), wav.frames(), decoded.decoded_bytes, decoded.trailing_bytes, termination.as_str().ok_or("termination must be a string")?)?;
            if let Some(loop_) = decoded.sample_loop {
                writeln!(manifest, "<sample_loop{}/>", xml::attributes(&loop_, &[])?)?;
            }
            manifest.push_str("</sample>\n");
            report.samples += 1;
            report.pcm_frames += wav.frames();
        }
        manifest.push_str("</samples>\n");
        // Full bytes are intentionally local to every bank folder so an isolated
        // folder can later reconstruct its source, including unrelated content.
        writeln!(manifest, "<preservation encoding=\"hex\" scope=\"entire_source_archive\" bytes=\"{}\" sha256=\"{hash}\">{}</preservation>\n</bank>", image.bytes().len(), xml::hex(image.bytes()))?;
        write_new(&directory.join("bank.xml"), manifest.as_bytes())?;
        if fs::read(directory.join("bank.xml"))? != manifest.as_bytes() {
            return Err("bank XML read-back verification failed".into());
        }
        write_root_bank(&mut root, asset, &folder, &hash)?;
        report.banks += 1;
    }
    root.push_str("</audio>\n");
    write_new(&transaction.staging.join("audio.xml"), root.as_bytes())?;
    if fs::read(transaction.staging.join("audio.xml"))? != root.as_bytes() {
        return Err("root XML read-back verification failed".into());
    }
    transaction.publish()?;
    Ok(report)
}

fn write_root_bank(root: &mut String, asset: &Asset, folder: &str, hash: &str) -> Result<()> {
    writeln!(
        root,
        "<bank id=\"{}\" path=\"{folder}/bank.xml\" source_sha256=\"{hash}\"/>",
        xml::attribute(&asset.id)?
    )?;
    Ok(())
}

pub(crate) fn reference_pitch(rate: u32) -> Result<u16> {
    let numerator = u64::from(rate) * 4096;
    if rate == 0 || numerator % 44100 != 0 || numerator / 44100 > 0x3fff {
        return Err("reference rate must correspond exactly to a nonzero 14-bit SPU pitch register (e.g. 22050 or 44100 Hz)".into());
    }
    Ok((numerator / 44100) as u16)
}
