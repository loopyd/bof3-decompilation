//! Canonical song preparation shared by extraction and manifest validation.
use crate::{
    catalog::model::Asset, catalog::model::AssetData, catalog::model::Catalog, digest::sha256_hex,
    document::manifest, document::xml, interchange::midi::Midi, machine::adsr::Model,
    machine::adsr::Registers, pc_render, sequence::midi as translation, sequence::timeline::Limits,
    soundfont::bank as binding, soundfont::bank::Gain, soundfont::bank::ToneContext,
    soundfont::envelope, soundfont::gain, voice::tuning::Reference, Result,
};
use emi_ex_v2::image::ArchiveImage;
use serde::Serialize;
use std::{collections::BTreeMap, fmt::Write as _, io::Cursor};

pub(crate) const LIMITATIONS: &[&str] = &[
            "explicit inspection approximations: note-on tuning executes the original US routine, but complete game scheduling and PC/PSX audio equivalence are not verified",
            "SF2 gain targets RustySynth 1.3.6 at the recorded unity/center context; envelopes fit the published SPU model; reverb is deliberately dry",
            "MIDI retains source programs and controller/bend values, but standard SF2 dynamic gain/pan and persistent channel bends do not reproduce tone-local and existing-voice-only game behavior",
            "initial programs equal their channel indices, as in the original US SEP initializer; channel volume/expression are full scale and pan is centered; other gameplay overrides remain unmodeled",
            "fixed PCM sample loops may approximate continuing ADPCM history; each independent sequence expands controller loops separately and must be rendered once",
            "original archive bytes and complete binding/timeline evidence are retained in each song.xml; SF2 instrument reconstruction and full music semantic acceptance remain open",
];

#[derive(Debug, Serialize)]
pub struct SongReport {
    pub id: String,
    pub path: String,
    pub bank: String,
    pub source_sha256: String,
    pub soundfont_sha256: String,
    pub sequences: Vec<SequenceReport>,
    #[serde(skip_serializing_if = "Vec::is_empty")]
    pub playback_approximations: Vec<String>,
}
#[derive(Debug, Serialize)]
pub struct SequenceReport {
    pub id: String,
    pub sequence_index: usize,
    pub sequence_id: u16,
    pub path: String,
    pub sha256: String,
    pub game_song_ids: Vec<u16>,
    pub generated_setup_events: usize,
    pub translation: translation::Report,
}

pub(crate) struct Prepared {
    pub document: String,
    pub files: BTreeMap<String, Vec<u8>>,
    pub report: SongReport,
    pub binding: binding::Report,
}

pub(crate) fn prepare(
    catalog: &Catalog,
    song: &Asset,
    image: &ArchiveImage,
    mapping: &crate::catalog::model::Report,
    reference: &mut Reference,
    envelopes: &mut BTreeMap<(u16, u16), envelope::Fit>,
    loops: u32,
) -> Result<Prepared> {
    prepare_selection(
        catalog,
        song,
        image,
        mapping,
        reference,
        envelopes,
        Selection {
            loops,
            sequence: None,
        },
    )
}

pub(crate) struct Selection {
    pub loops: u32,
    pub sequence: Option<usize>,
}

pub(crate) fn prepare_selection(
    catalog: &Catalog,
    song: &Asset,
    image: &ArchiveImage,
    mapping: &crate::catalog::model::Report,
    reference: &mut Reference,
    envelopes: &mut BTreeMap<(u16, u16), envelope::Fit>,
    selection: Selection,
) -> Result<Prepared> {
    let loops = selection.loops;
    let AssetData::Song {
        metadata: sequences,
        bank_candidates,
    } = &song.data
    else {
        unreachable!()
    };
    let [bank_id] = bank_candidates.as_slice() else {
        return Err(format!(
            "{}: music extraction requires exactly one resolved bank",
            song.id
        )
        .into());
    };
    let bank_asset = catalog
        .assets
        .iter()
        .find(|a| a.id == *bank_id)
        .ok_or("music extract: mapped bank missing")?;
    let AssetData::Bank {
        metadata: bank,
        content: Some(content),
        ..
    } = &bank_asset.data
    else {
        return Err(format!("{bank_id}: verified bank body required").into());
    };
    let source = catalog
        .sources
        .iter()
        .find(|s| s.source == song.source)
        .ok_or("music extract: source absent")?;
    let hash = sha256_hex(image.bytes());
    if source.sha256.as_deref() != Some(&hash) {
        return Err(format!("{}: source changed after inventory", song.source).into());
    }
    let header_entry = bank_asset
        .entry
        .ok_or("music extract: bank header entry absent")?;
    let body_entry = source
        .entries
        .iter()
        .find(|e| e.id == content.body_entry)
        .ok_or("music extract: bank body entry absent")?
        .entry;
    let mut contexts = Vec::new();
    for program in &bank.programs {
        for tone in &program.tones {
            let pair = (tone.adsr1, tone.adsr2);
            if let std::collections::btree_map::Entry::Vacant(entry) = envelopes.entry(pair) {
                entry.insert(envelope::fit(
                    Registers {
                        adsr1: pair.0,
                        adsr2: pair.1,
                    },
                    Model::Published,
                    &envelope::Probe::default(),
                )?);
            }
            contexts.push(ToneContext::from_reference(
                reference,
                program.program,
                tone,
                44100,
                Gain::from_bank(bank, program, tone, gain::Model::RustySynth136)?,
                envelopes[&pair].clone(),
            )?);
        }
    }
    let bound=binding::bind(binding::Identity {
            source:song.source.clone(),header_entry,body_entry,game_bank_id:u32::from(bank_asset.game_bank_id.ok_or("music extract: game bank ID unresolved")?),
        },image.entry(header_entry)?,image.entry(body_entry)?,&contexts,&binding::Options {sf2_bank:0,percussion_alias:true,sample_rate:44100,allow_predictor_loop_approximation:true,allow_stopped_pitch_approximation:true,
            reverb:Some(binding::ReverbApproximation {send_tenths_percent:0,provenance:"Explicit dry inspection approximation; game reverb preset/depth are not recovered".into()})
        }).map_err(|e|format!("{} bank {}: {e}",song.id,bank_id))?;
    let font = rustysynth::SoundFont::new(&mut Cursor::new(&bound.bytes))?;
    let entry = song.entry.ok_or("music extract: song entry absent")?;
    let folder = format!("songs/{}/song-{entry}", sha256_hex(song.source.as_bytes()));
    let mut files = BTreeMap::new();
    files.insert("bank.sf2".into(), bound.bytes.clone());
    let sf2_hash = sha256_hex(&bound.bytes);
    let mut document=format!("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<song schema=\"bof3.music-song/v1\" id=\"{}\" source=\"{}\" entry=\"{entry}\" bank=\"{}\">\n",xml::attribute(&song.id)?,xml::attribute(&song.source)?,xml::attribute(bank_id)?);
    writeln!(
        document,
        "<runtime profile=\"{}\" executable_sha256=\"{}\" complete_playback=\"unverified\"/>",
        mapping.profile.id, mapping.profile.exe_sha256
    )?;
    writeln!(document,"<export approximations=\"explicitly_allowed\" sample_rate=\"44100\" gain_model=\"rustysynth_1_3_6\" envelope_model=\"published\" reverb_send=\"0\" infinite_loop_traversals=\"{}\" render_repeats=\"1\" initial_programs=\"channel_index\" empty_programs=\"silent\"/>",loops)?;
    write_bank(&mut document, bank_id, &bound.report, &sf2_hash)?;
    document.push_str("<sequences>\n");
    let payload = image.entry(entry)?;
    let mut exported = Vec::new();
    for sequence in &sequences.sequences {
        if selection
            .sequence
            .is_some_and(|index| index != sequence.sequence_index)
        {
            continue;
        }
        let bytes = payload
            .get(sequence.data_offset..sequence.data_offset + sequence.data_bytes)
            .ok_or("music extract: sequence exceeds entry")?;
        let mut translated = translation::translate(
            sequence,
            bytes,
            &Limits {
                infinite_traversals: loops,
                ..Default::default()
            },
        )
        .map_err(|e| format!("{} sequence {}: {e}", song.id, sequence.sequence_index))?;
        let setup = translation::initialize_channels(&mut translated)?;
        let midi = translated.midi.to_bytes()?;
        if Midi::from_bytes(&midi)?.to_bytes()? != midi {
            return Err("music extract: MIDI round-trip verification failed".into());
        }
        rustysynth::MidiFile::new(&mut Cursor::new(&midi))?;
        pc_render::require_presets(&pc_render::schedule(&translated.midi, 44100)?, &font)?;
        let name = format!("sequence-{:03}.mid", sequence.sequence_index);
        let sequence_asset = sequence_asset(catalog, song, sequence.sequence_index)?;
        let midi_hash = sha256_hex(&midi);
        files.insert(name.clone(), midi);
        writeln!(document,"<sequence{} id=\"{}\" path=\"{name}\" sha256=\"{midi_hash}\" source_sha256=\"{}\" generated_setup_events=\"{setup}\">",xml::attributes(sequence,&[])?,xml::attribute(&sequence_asset.id)?,sha256_hex(bytes))?;
        for cue in &sequence_asset.game_song_ids {
            writeln!(document, "<game_song id=\"{cue}\"/>")?;
        }
        json_evidence(&mut document, "translation", &translated.report)?;
        json_evidence(&mut document, "timeline", &translated.timeline)?;
        document.push_str("</sequence>\n");
        exported.push(SequenceReport {
            id: sequence_asset.id.clone(),
            sequence_index: sequence.sequence_index,
            sequence_id: sequence.sequence_id,
            path: name,
            sha256: midi_hash,
            game_song_ids: sequence_asset.game_song_ids.clone(),
            generated_setup_events: setup,
            translation: translated.report,
        });
    }
    if exported.is_empty() {
        return Err("music preparation: selected sequence is absent".into());
    }
    document.push_str("</sequences>\n");
    json_evidence(&mut document, "limitations", &LIMITATIONS)?;
    writeln!(document,"<preservation encoding=\"hex\" scope=\"entire_source_archive\" bytes=\"{}\" sha256=\"{hash}\">{}</preservation>\n</song>",image.bytes().len(),xml::hex(image.bytes()))?;
    manifest::parse(&document, Default::default())?;
    let report = SongReport {
        id: song.id.clone(),
        path: format!("{folder}/song.xml"),
        bank: bank_id.clone(),
        source_sha256: hash,
        soundfont_sha256: sf2_hash,
        sequences: exported,
        playback_approximations: bound
            .report
            .diagnostics
            .iter()
            .filter(|s| s.contains("stopped-pitch keys") || s.starts_with("Zero-tone VAB programs"))
            .cloned()
            .collect(),
    };
    Ok(Prepared {
        document,
        files,
        report,
        binding: bound.report,
    })
}

fn json_evidence(xml: &mut String, name: &str, value: &impl Serialize) -> Result<()> {
    writeln!(
        xml,
        "<{name} encoding=\"json\">{}</{name}>",
        crate::document::xml::attribute(&serde_json::to_string(value)?)?
    )?;
    Ok(())
}
fn sequence_asset<'a>(catalog: &'a Catalog, song: &Asset, index: usize) -> Result<&'a Asset> {
    catalog.assets.iter().find(|a|matches!(&a.data,AssetData::Sequence {song:parent,metadata} if parent==&song.id && metadata.sequence_index==index)).ok_or_else(||"music extract: sequence identity missing".into())
}

fn write_bank(xml: &mut String, id: &str, report: &binding::Report, hash: &str) -> Result<()> {
    writeln!(xml,"<bank id=\"{}\" header_entry=\"{}\" body_entry=\"{}\" game_bank_id=\"{}\" header_id=\"{}\"><soundfont path=\"bank.sf2\" sha256=\"{hash}\"/>",crate::document::xml::attribute(id)?,report.identity.header_entry,report.identity.body_entry,report.identity.game_bank_id,report.source_metadata.header_id)?;
    for program in &report.programs {
        writeln!(
            xml,
            "<program{}>",
            crate::document::xml::attributes(program, &["percussion_preset"])?
        )?;
        if let Some(preset) = program.percussion_preset {
            writeln!(xml, "<percussion_alias bank=\"128\" preset=\"{preset}\"/>")?;
        }
        for tone in report
            .tones
            .iter()
            .filter(|t| t.source_program == program.source_program)
        {
            writeln!(xml,"<tone index=\"{}\" raw_sample_reference=\"{}\" sample_id=\"{}\" instrument=\"{}\" source_sf2_sample=\"{}\" playback_sf2_sample=\"{}\" silent=\"{}\" zones=\"{}\"/>",tone.source_tone.index,tone.source_tone.sample_reference,tone.sample_resolution.sample_id,tone.sf2_instrument,tone.source_sf2_sample,tone.sf2_sample,tone.silent,tone.zone_count)?;
            for key in &tone.stopped_keys {
                let sample = report
                    .silence_sample
                    .ok_or("music extract: stopped key lacks silence sample")?;
                writeln!(xml, "<stopped_key tone=\"{}\" key=\"{key}\" playback_sf2_sample=\"{sample}\" scope=\"key_on_only\"/>", tone.source_tone.index)?;
            }
        }
        xml.push_str("</program>\n");
    }
    for program in &report.empty_programs {
        let instrument = report
            .empty_program_instrument
            .ok_or("music extract: empty program instrument absent")?;
        let sample = report
            .empty_program_sample
            .ok_or("music extract: empty program sample absent")?;
        writeln!(
            xml,
            "<empty_program{} instrument=\"{instrument}\" playback_sf2_sample=\"{sample}\">",
            crate::document::xml::attributes(program, &["percussion_preset"])?
        )?;
        if let Some(preset) = program.percussion_preset {
            writeln!(xml, "<percussion_alias bank=\"128\" preset=\"{preset}\"/>")?;
        }
        xml.push_str("</empty_program>\n");
    }
    for sample in &report.samples {
        writeln!(xml,"<sample sample_id=\"{}\" body_offset=\"{}\" encoded_bytes=\"{}\" sha256=\"{}\" pcm_frames=\"{}\">",sample.sample_id,sample.body_offset,sample.encoded_bytes,sample.sha256,sample.pcm_frames)?;
        if let Some(index) = sample.sf2_sample {
            writeln!(xml, "<soundfont_sample index=\"{index}\"/>")?;
        }
        xml.push_str("</sample>\n");
    }
    json_evidence(xml, "binding", report)?;
    xml.push_str("</bank>\n");
    Ok(())
}
