use bof3_audio::{document::manifest, pack, soundfont::reader::Font};
use std::{fs, path::PathBuf};
struct Directory(PathBuf);
impl Drop for Directory {
    fn drop(&mut self) {
        fs::remove_dir_all(&self.0).unwrap();
    }
}
fn target(bytes: &mut [u8], preset: usize, instrument: u16) {
    let font = Font::from_bytes(bytes.to_vec()).unwrap();
    let table = |id| font.chunks.iter().find(|c| c.id == id).unwrap().data.start;
    let read = |at| u16::from_le_bytes(bytes[at..at + 2].try_into().unwrap()) as usize;
    let bag = read(table(*b"phdr") + preset * 38 + 24);
    let generator = read(table(*b"pbag") + bag * 4);
    let at = table(*b"pgen") + generator * 4;
    assert_eq!(&bytes[at..at + 2], &41u16.to_le_bytes());
    bytes[at + 2..at + 4].copy_from_slice(&instrument.to_le_bytes());
}
#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_CORPUS"]
fn current_policy_population_publication_and_original_runtime_playback() {
    use bof3_audio::{
        bank::Bank,
        machine::{executable::Executable, firmware::Image},
        pc_archive, pc_render, psx_render,
    };
    use emi_ex_v2::image::ArchiveImage;
    let root =
        Directory(std::env::temp_dir().join(format!("bof3-music-programs-{}", std::process::id())));
    fs::create_dir(&root.0).unwrap();
    let exe_path = PathBuf::from(std::env::var_os("BOF3_AUDIO_EXE").unwrap());
    let archive_path =
        PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap()).join("BIN/BGM/BGM053.EMI");
    let original = ArchiveImage::from_bytes(fs::read(&archive_path).unwrap()).unwrap();
    let opts = pack::Options {
        input: root.0.join("export"),
        output: root.0.join("packed"),
        executable: exe_path.clone(),
    };
    bof3_audio::music::extraction::songs(&bof3_audio::music::extraction::Options {
        disc_root: None,
        archives: vec![archive_path],
        executable: exe_path.clone(),
        output: opts.input.clone(),
        id: None,
        loops: 2,
        allow_approximations: true,
    })
    .unwrap();
    let music = manifest::read(&opts.input.join("music.xml")).unwrap();
    let path = manifest::relative_file(
        &opts.input,
        &opts.input,
        music.children[0].attribute("path").unwrap(),
    )
    .unwrap();
    let xml = fs::read_to_string(&path).unwrap();
    for (old, new) in [
        (" initial_programs=\"channel_index\"", ""),
        (
            " initial_programs=\"channel_index\"",
            " initial_programs=\"zero\"",
        ),
        (" empty_programs=\"silent\"", ""),
        (" empty_programs=\"silent\"", " empty_programs=\"omit\""),
    ] {
        assert!(xml.contains(old));
        fs::write(&path, xml.replace(old, new)).unwrap();
        let error = bof3_audio::music::packing::songs(&opts)
            .unwrap_err()
            .to_string();
        assert!(
            error.contains("re-extract with the current implementation"),
            "{error}"
        );
        assert!(!opts.output.exists());
    }
    fs::write(&path, &xml).unwrap();
    let unchanged = pack::Options {
        output: root.0.join("unchanged"),
        input: opts.input.clone(),
        executable: opts.executable.clone(),
    };
    let report = bof3_audio::music::packing::songs(&unchanged).unwrap();
    assert!(report.archives[0].byte_equal);
    assert_eq!(
        fs::read(unchanged.output.join(&report.archives[0].path)).unwrap(),
        original.bytes()
    );
    let font_path = path.parent().unwrap().join("bank.sf2");
    let mut bytes = fs::read(&font_path).unwrap();
    let font = Font::from_bytes(bytes.clone()).unwrap();
    let instrument = font
        .presets
        .iter()
        .find(|p| p.bank == 0 && p.program == 10)
        .unwrap()
        .zones[0]
        .target
        .unwrap() as u16;
    for bank in [0, 128] {
        let preset = font
            .presets
            .iter()
            .position(|p| p.bank == bank && p.program == 1)
            .unwrap();
        target(&mut bytes, preset, instrument);
    }
    fs::write(&font_path, &bytes).unwrap();
    let mut oversized = bytes.clone();
    for bank in [0, 128] {
        let preset = font
            .presets
            .iter()
            .position(|p| p.bank == bank && p.program == 2)
            .unwrap();
        target(&mut oversized, preset, instrument);
    }
    fs::write(&font_path, oversized).unwrap();
    let error = bof3_audio::music::packing::songs(&opts)
        .unwrap_err()
        .to_string();
    assert!(error.contains("sector allocation"), "{error}");
    assert!(!opts.output.exists());
    assert!(!fs::read_dir(&root.0).unwrap().any(|e| e
        .unwrap()
        .file_name()
        .to_string_lossy()
        .starts_with(".bof3-extract-")));
    fs::write(&font_path, bytes).unwrap();
    let report = bof3_audio::music::packing::songs(&opts).unwrap();
    assert_eq!(report.songs[0].populated_programs.len(), 1);
    assert_eq!(report.songs[0].populated_programs[0].program, 1);
    let packed =
        ArchiveImage::from_bytes(fs::read(opts.output.join(&report.archives[0].path)).unwrap())
            .unwrap();
    let bank = Bank::parse(packed.entry(0).unwrap()).unwrap();
    assert_eq!(
        bank.programs.iter().map(|p| p.program).collect::<Vec<_>>(),
        [1, 10]
    );
    assert_eq!(packed.entry(2).unwrap(), original.entry(2).unwrap());
    let events = [0, 0xc0, 1, 0, 0x90, 48, 127, 96, 0x90, 48, 0, 0, 0xff, 0x2f];
    let mut sep = original.entry(1).unwrap()[..19].to_vec();
    sep[8..10].copy_from_slice(&96u16.to_be_bytes());
    sep[10..13].copy_from_slice(&[7, 0xa1, 0x20]);
    sep[15..19].copy_from_slice(&(events.len() as u32).to_be_bytes());
    sep.extend(events);
    sep.resize(original.entry(1).unwrap().len(), 0);
    let executable = Executable::from_bytes(fs::read(exe_path).unwrap()).unwrap();
    let bios = fs::read(std::env::var_os("BOF3_AUDIO_BIOS").unwrap()).unwrap();
    for (archive, audible) in [(&original, false), (&packed, true)] {
        let media = archive.replace_entries(&[(1, &sep)]).unwrap();
        let psx = psx_render::render(
            &executable,
            Image::from_bytes(bios.clone()).unwrap(),
            &media,
            &psx_render::Options {
                sequence: 0,
                layout: None,
                body: psx_render::Body::Duration(22050),
                release_frames: 0,
                safety_frames: 44100,
            },
        )
        .unwrap();
        assert_eq!(psx.wave.pcm.iter().any(|&v| v != 0), audible);
        let pc = pc_archive::render(
            &executable,
            "program-population.emi",
            &media,
            &pc_archive::Options {
                sequence: 0,
                loops: None,
                allow_approximations: true,
                playback: pc_render::Options {
                    duration_frames: Some(22050),
                    release_frames: 0,
                    safety_frames: 44100,
                    ..Default::default()
                },
            },
        )
        .unwrap();
        assert_eq!(pc.wave.pcm.iter().any(|&v| v != 0), audible);
    }
}
