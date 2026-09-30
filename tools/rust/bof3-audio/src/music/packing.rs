//! Source-validated song-folder packing with bounded MIDI and SF2 bank edits.
use crate::{
    archive::packing, archive::packing::ArchiveReport, archive::packing::Pending,
    archive::MediaImage, catalog::loader, catalog::model::Catalog, digest::sha256_hex,
    document::manifest, interchange::midi::Midi, machine::executable::Executable,
    machine::profile::Profile, pack::Options, publication::require_absent, publication::write_new,
    publication::Publication, sequence::editing, sequence::timeline::Limits,
    voice::tuning::Reference, Result,
};
use emi_ex_v2::image::ArchiveImage;
use serde::Serialize;
use serde_json::json;
use std::{
    collections::{BTreeMap, BTreeSet},
    fs,
};

#[derive(Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub output: String,
    pub archives: Vec<ArchiveReport>,
    pub songs: Vec<SongReport>,
    pub limitations: Vec<&'static str>,
}

#[derive(Debug, Serialize)]
pub struct SongReport {
    pub id: String,
    pub bank: String,
    pub sequences: Vec<editing::Report>,
    pub soundfont_byte_equal: bool,
    pub soundfont: crate::soundfont::packing::sample::Report,
    pub tone_controls: crate::soundfont::packing::tone::Report,
    pub pitch_controls: crate::soundfont::packing::pitch::Report,
    pub assignments: crate::soundfont::packing::assignment::Report,
    pub populated_programs: Vec<crate::soundfont::packing::program::Population>,
}

pub fn songs(options: &Options) -> Result<Report> {
    require_absent(&options.output)?;
    let root = options.input.canonicalize()?;
    if !root.is_dir() {
        return Err("music pack: --input must be a song folder or music root".into());
    }
    let documents = crate::music::manifest::documents(&root)?;
    let executable = Executable::from_bytes(fs::read(&options.executable)?)?;
    Profile::identify(&executable)?;
    let mut reference = Reference::from_executable(&executable)?;
    let mut envelopes = BTreeMap::new();
    let mut pending = BTreeMap::<String, Pending>::new();
    let mut ids = BTreeSet::new();
    let mut report = Report {
        schema: "bof3.music-pack-report/v1", output: options.output.display().to_string(),
        archives: Vec::new(), songs: Vec::new(),
        limitations: vec![
            "Existing tone instruments support PCM, shared-consistent loops, gain/pan, sample assignments and verified note-on tuning edits; independent new instrument/sample structures and envelope edits remain unsupported.",
            "MIDI edits preserve event order, channels and loops; changed timing must agree at repeated delta consumptions. Resized deltas relocate later SEP records; unsupported edits are rejected.",
            "Empty programs may gain 1..=16 existing tone instruments with matching program controls and percussion aliases. Ordered VAB tone blocks may grow within original EMI and game layout capacity. All 128 MIDI programs are selectable; empty slots remain silent. Songs sharing a bank must agree on rebuilt header/body bytes.",
            "The export's explicit synthesis approximations remain; complete original-game timing and independent audio fidelity are unverified.",
            "Descriptive limitation notes are not reconstruction inputs; authoritative binding, timeline and identity metadata are validated.",
            "Only selected song folders are reconstructed; disc image rebuilding is outside scope.",
        ],
    };
    for (path, link) in documents {
        let node = manifest::read(&path)?;
        let bytes =
            manifest::preservation(node.child("preservation")?, Some("entire_source_archive"))?;
        if let Some(link) = link {
            link.scalars(
                &json!({"id":node.attribute("id")?, "bank":node.attribute("bank")?,
                "source_sha256":sha256_hex(&bytes)}),
                &[],
                &["path"],
            )?;
        }
        let image = ArchiveImage::from_bytes(bytes)?;
        let source = node.attribute("source")?;
        let mut catalog = Catalog::from_media(source, MediaImage::Emi(image.clone()))?;
        let mapping = loader::resolve(&mut catalog, &executable)?;
        let song = catalog.select("music", Some("song"), node.attribute("id")?)?;
        if !ids.insert(song.id.clone()) {
            return Err(format!("music pack: duplicate song identity {}", song.id).into());
        }
        let loops = node
            .child("export")?
            .number::<u32>("infinite_loop_traversals")?;
        if loops == 0 {
            return Err("music pack: infinite loop traversals must be positive".into());
        }
        let export = node.child("export")?;
        for (name, expected) in [
            ("initial_programs", "channel_index"),
            ("empty_programs", "silent"),
        ] {
            if export.attributes.get(name).map(String::as_str) != Some(expected) {
                return Err(format!("music pack: current export policy requires {name}=\"{expected}\"; re-extract with the current implementation").into());
            }
        }
        let baseline = crate::music::document::prepare_selection(
            &catalog,
            song,
            &image,
            &mapping,
            &mut reference,
            &mut envelopes,
            crate::music::document::Selection {
                loops,
                sequence: None,
            },
        )?;
        crate::music::manifest::validate(
            &node,
            &manifest::parse(&baseline.document, Default::default())?,
            "song",
        )?;
        let base = path.parent().ok_or("music pack: manifest parent missing")?;
        let font_path = manifest::relative_file(
            &root,
            base,
            node.child("bank")?.child("soundfont")?.attribute("path")?,
        )?;
        let font = crate::soundfont::reader::Font::from_bytes(fs::read(font_path)?)?;
        let original_font = crate::soundfont::reader::Font::from_bytes(
            baseline
                .files
                .get("bank.sf2")
                .ok_or("music pack: regenerated SF2 missing")?
                .clone(),
        )?;
        let body_entry = baseline.binding.identity.body_entry;
        let header_entry = baseline.binding.identity.header_entry;
        let packed_font = crate::soundfont::packing::sample::pack_bank(
            &baseline.binding,
            image.entry(header_entry)?,
            image.entry(body_entry)?,
            &original_font,
            &font,
            Some(&mut reference),
        )
        .map_err(|e| format!("music pack: {} bank {}: {e}", song.id, baseline.report.bank))?;
        let mut paths = BTreeSet::new();
        let mut edited = Vec::new();
        for sequence in &node.child("sequences")?.children {
            let midi_path = manifest::relative_file(&root, base, sequence.attribute("path")?)?;
            if !paths.insert(midi_path.clone()) {
                return Err("music pack: duplicate MIDI path".into());
            }
            let midi = Midi::from_bytes(&fs::read(midi_path)?)?;
            // All 128 VAB program IDs are selectable, including silent slots.
            // The sequence inverse validates authored events and generated setup.
            let index = sequence.number::<usize>("sequence_index")?;
            edited.push((index, midi));
        }
        let selections: Vec<_> = edited.iter().map(|(index, midi)| (*index, midi)).collect();
        let index = song.entry.ok_or("music pack: song entry missing")?;
        let rebuilt = editing::rebuild_entry(
            image.entry(index)?,
            &selections,
            &editing::Options {
                limits: Limits {
                    infinite_traversals: loops,
                    ..Default::default()
                },
                initialize_channels: true,
            },
        )
        .map_err(|e| format!("music pack: {}: {e}", song.id))?;
        if !packed_font.populated_programs.is_empty() {
            let slot_index = image.entries()[header_entry].ram_ptr as usize;
            let dma_bytes = packed_font.body.len().div_ceil(64) * 64;
            let fits = mapping.layouts.iter().any(|layout| {
                layout.slots.get(slot_index).is_some_and(|slot| {
                    let boundary = layout
                        .slots
                        .iter()
                        .map(|s| s.spu_base)
                        .filter(|&base| base > slot.spu_base)
                        .min()
                        .unwrap_or(512 * 1024);
                    packed_font.header.len() <= slot.header_capacity as usize
                        && rebuilt.bytes.len() <= slot.sequence_capacity as usize
                        && slot.spu_base as usize + dma_bytes <= boundary as usize
                })
            });
            if !fits {
                return Err("music pack: populated programs exceed available game VH/SEP/SPU layout capacity".into());
            }
        }
        let output = pending.entry(source.into()).or_insert_with(|| Pending {
            image: image.clone(),
            entries: BTreeMap::new(),
        });
        if output.image.bytes() != image.bytes() {
            return Err(
                format!("music pack: conflicting preserved archives for source {source}").into(),
            );
        }
        for (entry, bytes) in [
            (index, rebuilt.bytes),
            (header_entry, packed_font.header),
            (body_entry, packed_font.body),
        ] {
            if let Some(existing) = output.entries.insert(entry, bytes.clone()) {
                if existing != bytes {
                    return Err(format!("music pack: conflicting edits for source {source} entry {entry}; selected songs sharing a bank must agree on rebuilt header/sample data").into());
                }
            }
        }
        report.songs.push(SongReport {
            id: song.id.clone(),
            bank: baseline.report.bank,
            sequences: rebuilt.reports,
            soundfont_byte_equal: packed_font.report.soundfont_byte_equal,
            soundfont: packed_font.report,
            tone_controls: packed_font.tone_controls,
            pitch_controls: packed_font.pitch_controls,
            assignments: packed_font.assignments,
            populated_programs: packed_font.populated_programs,
        });
    }
    let transaction = Publication::new(&options.output)?;
    report.archives = packing::stage(&transaction.staging, pending)?;
    let bytes = serde_json::to_vec_pretty(&report)?;
    write_new(&transaction.staging.join("pack.json"), &bytes)?;
    if fs::read(transaction.staging.join("pack.json"))? != bytes {
        return Err("music pack: report read-back verification failed".into());
    }
    transaction.publish()?;
    Ok(report)
}
