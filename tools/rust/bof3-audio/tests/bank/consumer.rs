//! Validate extracted banks against original archive bytes with separate XML/RIFF parsing.

#[path = "../reference/document.rs"]
mod document;
use bof3_audio::digest::sha256_hex as digest;
use document::*;
use roxmltree::Document;
use std::{
    collections::BTreeSet,
    fs,
    path::{Path, PathBuf},
};

fn relative(root: &Path, base: &Path, name: &str) -> PathBuf {
    assert!(!Path::new(name)
        .components()
        .any(|p| matches!(p, std::path::Component::ParentDir)));
    document::relative(root, base, name)
}

pub fn validate(directory: &Path, source_root: Option<&Path>) {
    let xml = fs::read_to_string(directory.join("audio.xml")).unwrap();
    let document = Document::parse(&xml).unwrap();
    let root = document.root_element();
    assert_eq!(root.attributes().len(), 3);
    assert_eq!(attr(root, "schema"), "bof3.audio-extraction/v1");
    assert_eq!(attr(root, "mode"), "audio");
    assert_eq!(attr(root, "kind"), "bank");
    let (mut banks, mut samples, mut frames, mut approximate) = (0, 0, 0, 0);
    let mut ids = BTreeSet::new();
    let mut records = String::new();
    for link in root.children().filter(|n| n.is_element()) {
        assert!(link.has_tag_name("bank"));
        assert!(ids.insert(attr(link, "id")));
        let manifest = relative(directory, directory, &attr(link, "path"));
        let xml = fs::read_to_string(&manifest).unwrap();
        let document = Document::parse(&xml).unwrap();
        let bank = document.root_element();
        assert_eq!(attr(bank, "schema"), "bof3.audio-bank/v1");
        assert_eq!(attr(bank, "id"), attr(link, "id"));
        let source = PathBuf::from(attr(bank, "source"));
        let source = source_root.map_or_else(|| source.clone(), |base| base.join(&source));
        let preservation = child(bank, "preservation");
        assert_eq!(attr(preservation, "encoding"), "hex");
        assert_eq!(attr(preservation, "scope"), "entire_source_archive");
        let original = hex(preservation.text().unwrap());
        assert_eq!(original, fs::read(source).unwrap());
        assert_eq!(original.len(), num(preservation, "bytes"));
        assert_eq!(digest(&original), attr(preservation, "sha256"));
        assert_eq!(digest(&original), attr(link, "source_sha256"));
        let mut entries = Vec::new();
        let mut offset = 2048;
        for entry in 0..(u32le(&original, 0) & 0xffff) as usize {
            let size = u32le(&original, 16 + 16 * entry) as usize;
            entries.push(&original[offset..offset + size]);
            offset += size.div_ceil(2048) * 2048;
        }
        let vh = entries[num(bank, "header_entry")];
        let vb = entries[num(bank, "body_entry")];
        assert_eq!(&vh[..4], b"pBAV");
        let metadata = child(bank, "metadata");
        assert_eq!(num(metadata, "header_id"), u32le(vh, 8) as usize);
        for program in children(metadata, "program") {
            let p = num(program, "program");
            let raw = &vh[0x20 + p * 16..0x30 + p * 16];
            assert_eq!(
                program.children().filter(|n| n.is_element()).count(),
                usize::from(raw[0])
            );
            for (k, v) in ["volume", "priority", "mode", "pan"]
                .into_iter()
                .zip(&raw[1..5])
            {
                assert_eq!(num(program, k), usize::from(*v));
            }
            for tone in program.children().filter(|n| n.is_element()) {
                let at = 0x820 + num(program, "tone_block") * 512 + num(tone, "index") * 32;
                let raw = &vh[at..at + 32];
                for (k, v) in [
                    "priority",
                    "mode",
                    "volume",
                    "pan",
                    "center",
                    "shift",
                    "key_min",
                    "key_max",
                    "vibrato_width",
                    "vibrato_time",
                    "portamento_width",
                    "portamento_time",
                    "bend_min",
                    "bend_max",
                ]
                .into_iter()
                .zip(&raw[..14])
                {
                    assert_eq!(num(tone, k), usize::from(*v));
                }
                for (i, k) in ["adsr1", "adsr2", "program_reference", "sample_reference"]
                    .into_iter()
                    .enumerate()
                {
                    let value = u16le(raw, 16 + 2 * i);
                    let value = if i < 2 {
                        i64::from(value)
                    } else {
                        i64::from(value as i16)
                    };
                    assert_eq!(attr(tone, k).parse::<i64>().unwrap(), value);
                }
            }
        }
        let export = child(bank, "export");
        let rate = num(export, "reference_rate");
        assert_eq!(rate * 4096, num(export, "reference_pitch_register") * 44100);
        assert_eq!(attr(export, "rate_context"), "export_reference_only");
        assert_eq!(
            attr(child(bank, "runtime"), "playback_context"),
            "unresolved"
        );
        let loader = child(bank, "loader");
        assert_eq!(attr(loader, "active_layout"), "unresolved");
        let layouts = children(loader, "possible_layout");
        assert_eq!(layouts.len(), 3);
        for layout in layouts {
            assert_eq!(attr(layout, "index"), attr(loader, "load_slot"));
            assert_eq!(attr(layout, "vab_id"), attr(bank, "game_bank_id"));
        }
        for sample in child(bank, "samples").children().filter(|n| n.is_element()) {
            let path = relative(directory, manifest.parent().unwrap(), &attr(sample, "path"));
            let data = fs::read(path).unwrap();
            assert_eq!(digest(&data), attr(sample, "wav_sha256"));
            let chunks = riff(&data);
            assert_eq!(u16le(&chunks["fmt "], 2), 1);
            assert_eq!(u32le(&chunks["fmt "], 4) as usize, rate);
            let pcm = &chunks["data"];
            assert_eq!(pcm.len() % 2, 0);
            let count = pcm.len() / 2;
            assert_eq!(count, num(sample, "frames"));
            let sampler = &chunks["smpl"];
            assert_eq!(u32le(sampler, 8) as usize, 1_000_000_000 / rate);
            assert_eq!(u32le(sampler, 12), 60);
            let loops = children(sample, "sample_loop");
            let loop_fields = if loops.is_empty() {
                assert_eq!(u32le(sampler, 28), 0);
                "-|-|-".to_string()
            } else {
                assert_eq!(loops.len(), 1);
                let (start, end) = (
                    num(loops[0], "start_frame"),
                    num(loops[0], "end_frame_exclusive"),
                );
                assert!(start < end && end <= count);
                assert_eq!(u32le(sampler, 28), 1);
                for (i, v) in [0, 0, start, end - 1, 0, 0].into_iter().enumerate() {
                    assert_eq!(u32le(sampler, 36 + i * 4) as usize, v);
                }
                let stable = attr(loops[0], "pcm_repeat_is_stable");
                approximate += usize::from(stable == "false");
                format!("{start}|{end}|{stable}")
            };
            let at = num(sample, "body_offset");
            assert_eq!(
                digest(&vb[at..at + num(sample, "encoded_bytes")]),
                attr(sample, "encoded_sha256")
            );
            records.push_str(&format!(
                "{}/sample={}|{}|{count}|{}|{}|{}|{loop_fields}\n",
                attr(bank, "id"),
                attr(sample, "sample_id"),
                digest(pcm),
                attr(sample, "termination"),
                attr(sample, "decoded_bytes"),
                attr(sample, "trailing_bytes")
            ));
            samples += 1;
            frames += count;
        }
        banks += 1;
    }
    if source_root.is_some() {
        assert_eq!(
            (banks, samples, frames, approximate),
            (1020, 8385, 138296704, 790)
        );
        assert_eq!(
            digest(records.as_bytes()),
            "cf7f115ccb8e468b1e321657f4715ab4ffd5573cc0bbcd37a911b1f644522eae"
        );
    }
}
