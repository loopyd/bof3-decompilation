use bof3_audio::{digest::sha256_hex, document::manifest, interchange::wave::Wave};
use std::{
    fs,
    path::PathBuf,
    sync::atomic::{AtomicU64, Ordering},
};

struct Directory(PathBuf);
impl Directory {
    fn new() -> Self {
        static NEXT: AtomicU64 = AtomicU64::new(0);
        let path = std::env::temp_dir().join(format!(
            "bof3-manifest-extraction-{}-{}",
            std::process::id(),
            NEXT.fetch_add(1, Ordering::Relaxed)
        ));
        fs::create_dir(&path).unwrap();
        Self(path)
    }
}
impl Drop for Directory {
    fn drop(&mut self) {
        fs::remove_dir_all(&self.0).unwrap();
    }
}

#[test]
fn xa_extraction_paths_preservation_sector_metadata_and_wave_survive_xml_reading() {
    use bof3_audio::{
        xa::extraction::Kind, xa::extraction::Options, xa::Arithmetic, xa::Decoder, xa::Histories,
        xa::Stream,
    };
    let directory = Directory::new();
    let source = directory.0.join("audio & 'é'.STR");
    let mut bytes = Vec::new();
    for channel in [0, 1, 0] {
        let mut sector = vec![0; 2336];
        sector[..8].copy_from_slice(&[2, channel, 0x64, 0, 2, channel, 0x64, 0]);
        for i in 0..18 {
            sector[8 + i * 128..24 + i * 128].fill(0x0c);
        }
        sector[2312..].fill(0x5a); // Opaque tail retained by extraction, not validated as EDC here.
        bytes.extend(sector);
    }
    fs::write(&source, &bytes).unwrap();
    let output = directory.0.join("export");
    bof3_audio::xa::extraction::extract(&Options {
        disc_root: None,
        archives: vec![source.clone()],
        executable: None,
        output: output.clone(),
        id: None,
        kind: Kind::Stream,
        arithmetic: Arithmetic::SplitFloor,
    })
    .unwrap();
    let root = manifest::read(&output.join("audio.xml")).unwrap();
    root.shape(&["schema", "mode", "kind"], &["source", "asset"], false)
        .unwrap();
    let snapshot = manifest::relative_file(
        &output,
        &output,
        root.child("source").unwrap().attribute("path").unwrap(),
    )
    .unwrap();
    let saved = manifest::read(&snapshot).unwrap();
    assert_eq!(saved.attribute("source").unwrap(), source.to_str().unwrap());
    assert!(manifest::preservation(saved.child("preservation").unwrap(), None).unwrap() == bytes);
    let mut total_frames = 0;
    for asset in root.children.iter().filter(|n| n.name == "asset") {
        let file =
            manifest::relative_file(&output, &output, asset.attribute("path").unwrap()).unwrap();
        let node = manifest::read(&file).unwrap();
        assert_eq!(
            node.attribute("id").unwrap(),
            asset.attribute("id").unwrap()
        );
        let base = file.parent().unwrap();
        let source_ref = node.child("source_ref").unwrap();
        assert_eq!(
            manifest::relative_file(&output, base, source_ref.attribute("path").unwrap()).unwrap(),
            snapshot
        );
        let stream = node.child("stream").unwrap();
        let mut decoder = Decoder::new(
            Stream {
                file: stream.number("file").unwrap(),
                channel: stream.number("channel").unwrap(),
                coding: stream.number("coding").unwrap(),
            },
            Arithmetic::SplitFloor,
            Histories::default(),
        )
        .unwrap();
        let mut expected = Vec::new();
        for selected in &node.child("sectors").unwrap().children {
            let index = selected.number::<usize>("index").unwrap();
            assert_eq!(
                selected.number::<usize>("frame_start").unwrap(),
                expected.len()
            );
            expected.extend(
                decoder
                    .decode_sector(&bytes[index * 2336..(index + 1) * 2336])
                    .unwrap(),
            );
        }
        let wave = node.child("wave").unwrap();
        let path = manifest::relative_file(&output, base, wave.attribute("path").unwrap()).unwrap();
        let wav = fs::read(path).unwrap();
        assert_eq!(wave.attribute("sha256").unwrap(), sha256_hex(&wav));
        let parsed = Wave::from_bytes(&wav).unwrap();
        assert_eq!(parsed.frames(), wave.number::<usize>("frames").unwrap());
        assert!(parsed.pcm == expected);
        total_frames += parsed.frames();
    }
    assert_eq!(total_frames, 3 * 4032);
}

#[test]
#[ignore = "requires BOF3_AUDIO_CORPUS and BOF3_AUDIO_EXE; reads freshly extracted original bank XML/WAV"]
fn original_bank_xml_preservation_metadata_and_wav_content_agree_with_source() {
    use bof3_audio::{
        bank::extraction::{self, Options},
        bank::Bank,
        codec::adpcm,
    };
    use emi_ex_v2::image::ArchiveImage;
    let directory = Directory::new();
    let source =
        PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap()).join("BIN/BGM/BGM000.EMI");
    let original = fs::read(&source).unwrap();
    let output = directory.0.join("bank-export");
    let report = extraction::banks(&Options {
        disc_root: None,
        archives: vec![source],
        executable: PathBuf::from(std::env::var_os("BOF3_AUDIO_EXE").unwrap()),
        output: output.clone(),
        id: None,
        reference_rate: 44100,
    })
    .unwrap();
    assert_eq!(report.banks, 1);
    let root = manifest::read(&output.join("audio.xml")).unwrap();
    let link = root.child("bank").unwrap();
    let path = manifest::relative_file(&output, &output, link.attribute("path").unwrap()).unwrap();
    let bank = manifest::read(&path).unwrap();
    assert_eq!(bank.attribute("id").unwrap(), link.attribute("id").unwrap());
    let preserved = manifest::preservation(
        bank.child("preservation").unwrap(),
        Some("entire_source_archive"),
    )
    .unwrap();
    assert!(preserved == original);
    let archive = ArchiveImage::from_bytes(preserved).unwrap();
    let metadata =
        Bank::parse(archive.entry(bank.number("header_entry").unwrap()).unwrap()).unwrap();
    let manifest_metadata = bank.child("metadata").unwrap();
    manifest_metadata
        .scalars(&metadata, &["programs", "samples", "diagnostics"], &[])
        .unwrap();
    for program in manifest_metadata
        .children
        .iter()
        .filter(|n| n.name == "program")
    {
        let original = metadata
            .programs
            .iter()
            .find(|p| p.program == program.number::<u8>("program").unwrap())
            .unwrap();
        program.scalars(original, &["tones"], &[]).unwrap();
        for tone in &program.children {
            tone.scalars(
                &original.tones[tone.number::<usize>("index").unwrap()],
                &[],
                &[],
            )
            .unwrap();
        }
    }
    let body = archive.entry(bank.number("body_entry").unwrap()).unwrap();
    let mut frames = 0;
    for sample in &bank.child("samples").unwrap().children {
        let allocation = &metadata.samples[sample.number::<usize>("sample_id").unwrap() - 1];
        sample
            .scalars(
                allocation,
                &[],
                &[
                    "path",
                    "encoded_sha256",
                    "wav_sha256",
                    "frames",
                    "decoded_bytes",
                    "trailing_bytes",
                    "termination",
                ],
            )
            .unwrap();
        let encoded =
            &body[allocation.body_offset..allocation.body_offset + allocation.encoded_bytes];
        assert_eq!(
            sample.attribute("encoded_sha256").unwrap(),
            sha256_hex(encoded)
        );
        let wav_path = manifest::relative_file(
            &output,
            path.parent().unwrap(),
            sample.attribute("path").unwrap(),
        )
        .unwrap();
        let bytes = fs::read(&wav_path).unwrap();
        assert_eq!(sample.attribute("wav_sha256").unwrap(), sha256_hex(&bytes));
        let wave = Wave::from_bytes(&bytes).unwrap();
        assert!(wave.pcm == adpcm::decode_sample(encoded).unwrap().pcm);
        frames += wave.frames();
    }
    assert_eq!(frames, report.pcm_frames);
    assert_eq!(
        bank.child("samples").unwrap().children.len(),
        report.samples
    );
    eprintln!("Manifest reader: original BGM000 bank, {} samples / {frames} frames; whole EMI preservation and scalar identities agree", report.samples);
}
