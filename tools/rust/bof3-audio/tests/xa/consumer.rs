//! Validate XA XML, sector identity, PCM, and cue placement independently of production readers.

#[path = "../reference/document.rs"]
mod document;
use bof3_audio::digest::sha256_hex as digest;
use document::*;
use roxmltree::Document;
use std::{
    collections::BTreeMap,
    fs,
    io::Write,
    path::{Path, PathBuf},
    process::{Command, Stdio},
};

pub fn validate(root: &Path, media_root: Option<&Path>, executable: Option<&Path>) {
    let xml = fs::read_to_string(root.join("audio.xml")).unwrap();
    let document = Document::parse(&xml).unwrap();
    let manifest = document.root_element();
    assert_eq!(attr(manifest, "schema"), "bof3.audio-extraction/v1");
    assert_eq!(attr(manifest, "mode"), "audio");
    assert!(matches!(
        attr(manifest, "kind").as_str(),
        "xa_stream" | "xa_cue"
    ));
    let mut sources = BTreeMap::new();
    for reference in children(manifest, "source") {
        let path = relative(root, root, &attr(reference, "path"));
        let xml = fs::read_to_string(&path).unwrap();
        let document = Document::parse(&xml).unwrap();
        let source = document.root_element();
        assert_eq!(attr(source, "schema"), "bof3.xa-source/v1");
        assert_eq!(attr(source, "source"), attr(reference, "id"));
        let preservation = child(source, "preservation");
        let data = hex(preservation.text().unwrap());
        assert_eq!(digest(&data), attr(preservation, "sha256"));
        assert_eq!(digest(&data), attr(reference, "sha256"));
        assert_eq!(data.len(), num(preservation, "bytes"));
        assert_eq!(data.len(), num(source, "sectors") * 2336);
        let original = PathBuf::from(attr(source, "source"));
        let original = media_root.map_or_else(|| original.clone(), |base| base.join(&original));
        assert_eq!(data, fs::read(original).unwrap());
        assert!(sources.insert(path, data).is_none());
    }
    let executable = executable.map(|p| fs::read(p).unwrap());
    for reference in children(manifest, "asset") {
        let path = relative(root, root, &attr(reference, "path"));
        let xml = fs::read_to_string(&path).unwrap();
        let document = Document::parse(&xml).unwrap();
        let asset = document.root_element();
        assert_eq!(attr(asset, "schema"), "bof3.xa-asset/v1");
        assert_eq!(attr(asset, "id"), attr(reference, "id"));
        assert_eq!(attr(asset, "kind"), attr(manifest, "kind"));
        let source = child(asset, "source_ref");
        let data = &sources[&relative(root, path.parent().unwrap(), &attr(source, "path"))];
        assert_eq!(digest(data), attr(source, "sha256"));
        let stream = child(asset, "stream");
        let (file, channel, coding) = (
            num(stream, "file"),
            num(stream, "channel"),
            num(stream, "coding"),
        );
        let channels = if coding & 1 != 0 { 2 } else { 1 };
        let rate = if coding & 4 != 0 { 18900 } else { 37800 };
        let per_sector = if coding & 16 != 0 { 2016 } else { 4032 } / channels;
        assert_eq!(num(stream, "channels"), channels);
        assert_eq!(num(stream, "sample_rate"), rate as usize);
        let decoder = child(asset, "decoder");
        assert_eq!(attr(decoder, "initial_history"), "zero_per_exported_asset");
        assert_eq!(attr(decoder, "arithmetic"), "combined_rounded");
        assert_eq!(attr(decoder, "audible_endpoint"), "unverified");
        let mut indices = Vec::new();
        let mut encoded = Vec::new();
        let mut wrapped = Vec::new();
        for (ordinal, sector) in child(asset, "sectors")
            .children()
            .filter(|n| n.is_element())
            .enumerate()
        {
            let index = num(sector, "index");
            assert!(indices.last().is_none_or(|&n| index > n));
            indices.push(index);
            let raw = &data[index * 2336..(index + 1) * 2336];
            assert_eq!(&raw[..4], &raw[4..8]);
            assert_eq!(
                (
                    usize::from(raw[0]),
                    usize::from(raw[1]),
                    usize::from(raw[3])
                ),
                (file, channel, coding)
            );
            assert_eq!(raw[2] & 0x2e, 0x24);
            assert_eq!(num(sector, "submode"), usize::from(raw[2]));
            assert_eq!(num(sector, "frames"), per_sector);
            assert_eq!(num(sector, "frame_start"), ordinal * per_sector);
            encoded.extend_from_slice(raw);
            wrapped.extend_from_slice(&[
                0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 2, 0, 2,
            ]);
            wrapped.extend_from_slice(raw);
        }
        let wave = child(asset, "wave");
        let wav = fs::read(relative(root, path.parent().unwrap(), &attr(wave, "path"))).unwrap();
        assert_eq!(digest(&wav), attr(wave, "sha256"));
        assert_eq!(digest(&encoded), attr(wave, "selected_sectors_sha256"));
        let chunks = riff(&wav);
        assert_eq!(u16le(&chunks["fmt "], 2) as usize, channels);
        assert_eq!(u32le(&chunks["fmt "], 4), rate);
        let pcm = &chunks["data"];
        assert_eq!(pcm.len(), indices.len() * per_sector * channels * 2);
        assert_eq!(num(wave, "frames"), indices.len() * per_sector);
        assert_eq!(
            coding & 16,
            0,
            "8-bit XA has separate integer-vector validation"
        );
        let mut process = Command::new("ffmpeg")
            .args([
                "-hide_banner",
                "-loglevel",
                "error",
                "-f",
                "psxstr",
                "-i",
                "pipe:0",
                "-map",
                "0:a:0",
                "-f",
                "s16le",
                "-acodec",
                "pcm_s16le",
                "pipe:1",
            ])
            .stdin(Stdio::piped())
            .stdout(Stdio::piped())
            .stderr(Stdio::piped())
            .spawn()
            .unwrap();
        let mut input = process.stdin.take().unwrap();
        let decoded = std::thread::scope(|scope| {
            let writer = scope.spawn(move || input.write_all(&wrapped));
            let result = process.wait_with_output().unwrap();
            writer.join().unwrap().unwrap();
            result
        });
        assert!(
            decoded.status.success(),
            "{}",
            String::from_utf8_lossy(&decoded.stderr)
        );
        assert_eq!(*pcm, decoded.stdout, "{}", attr(asset, "id"));
        let cues = children(asset, "cue");
        for cue in &cues {
            let (first, last, stride) = (
                num(*cue, "sector_start"),
                num(*cue, "last_sector"),
                num(*cue, "sector_stride"),
            );
            let expected: Vec<_> = (first..=last).step_by(stride).collect();
            assert_eq!(expected.len(), num(*cue, "sector_count"));
            assert!(expected.iter().all(|i| indices.contains(i)));
            assert_eq!(
                num(*cue, "frame_start"),
                indices.iter().position(|&i| i == first).unwrap() * per_sector
            );
            assert_eq!(
                num(*cue, "frame_end_exclusive"),
                (indices.iter().position(|&i| i == last).unwrap() + 1) * per_sector
            );
            if attr(asset, "kind") == "xa_cue" {
                assert_eq!(expected, indices);
            }
            if let Some(exe) = &executable {
                let at = num(*cue, "table_address") - 0x80096800 + 0x800;
                let (start, end) = (usize::from(u16le(exe, at)), usize::from(u16le(exe, at + 2)));
                assert_eq!(first, (start & 0x7fff) * stride + channel);
                assert_eq!(last, ((end & 0x7fff) - 1) * stride + channel);
                let slot = num(*cue, "disc_file_slot");
                let base = u32le(exe, 0x80182444 + slot * 4 - 0x80096800 + 0x800) as usize;
                assert_eq!(num(*cue, "runtime_start_lba"), base + first);
                assert_eq!(
                    num(*cue, "runtime_stop_threshold_lba"),
                    base + last + stride - 150
                );
            }
        }
        if attr(asset, "kind") == "xa_cue" {
            assert_eq!(cues.len(), 1);
        }
    }
}
