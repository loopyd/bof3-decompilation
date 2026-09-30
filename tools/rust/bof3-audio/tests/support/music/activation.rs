use super::{assignment_music, extract, options, tone_controls, Directory};
use bof3_audio::document::manifest;
use emi_ex_v2::image::ArchiveImage;
use std::fs;

#[test]
#[ignore = "requires BOF3_AUDIO_EXE; muted layer shared by two songs"]
fn shared_song_unmute_conflicts_then_publishes_header_only() {
    let d = Directory::new();
    let (_, source) = assignment_music::fixture();
    let image = ArchiveImage::from_bytes(source).unwrap();
    let mut header = image.entry(0).unwrap().to_vec();
    header[0x822] = 0;
    let image = image.replace_entries(&[(0, &header)]).unwrap();
    extract(&d.0, image.bytes());
    let mut opts = options(&d.0, "conflicting-unmute");
    let music = manifest::read(&opts.input.join("music.xml")).unwrap();
    let fonts: Vec<_> = music
        .children
        .iter()
        .map(|n| {
            manifest::relative_file(&opts.input, &opts.input, n.attribute("path").unwrap())
                .unwrap()
                .parent()
                .unwrap()
                .join("bank.sf2")
        })
        .collect();
    assert_eq!(fonts.len(), 2);
    let mut changed = fs::read(&fonts[0]).unwrap();
    // First source sample is nonlooping; original muted playback used synthetic silence.
    tone_controls::map(&mut changed, 0, true, |op, value| match op {
        53 | 54 => 0,
        _ => value,
    });
    fs::write(&fonts[0], &changed).unwrap();
    let error = bof3_audio::music::packing::songs(&opts)
        .unwrap_err()
        .to_string();
    assert!(
        error.contains("selected songs sharing a bank must agree"),
        "{error}"
    );
    assert!(!opts.output.exists());
    fs::write(&fonts[1], &changed).unwrap();
    opts.output.set_file_name("agreed-unmute");
    let report = bof3_audio::music::packing::songs(&opts).unwrap();
    assert_eq!(report.archives[0].changed_entries, [0]);
    assert!(report.songs.iter().all(|s| s.assignments.tones.len() == 1
        && s.assignments.tones[0].unmuted
        && s.tone_controls.tones[0].changed));
    let rebuilt =
        ArchiveImage::from_bytes(fs::read(opts.output.join(&report.archives[0].path)).unwrap())
            .unwrap();
    for i in 1..image.entries().len() {
        assert_eq!(image.entry(i).unwrap(), rebuilt.entry(i).unwrap());
    }
    assert!(rebuilt.entry(0).unwrap()[0x822] > 0);
    for (i, (&a, &b)) in header.iter().zip(rebuilt.entry(0).unwrap()).enumerate() {
        if i != 0x822 && i != 0x823 {
            assert_eq!(a, b, "byte {i:x}");
        }
    }
}
