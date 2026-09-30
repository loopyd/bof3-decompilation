use bof3_audio::pack;
use emi_ex_v2::image::ArchiveImage;
use std::{fs, path::PathBuf};
pub fn check(mut options: pack::Options, fonts: &[PathBuf], original_font: &[u8], source: &[u8]) {
    for font in fonts {
        fs::write(font, original_font).unwrap();
    }
    let mut changed = original_font.to_vec();
    super::tone_controls::map(&mut changed, 0, true, |op, amount| {
        if op == 51 {
            (amount as i16 + 1) as u16
        } else {
            amount
        }
    });
    fs::write(&fonts[0], &changed).unwrap();
    let error = bof3_audio::music::packing::songs(&options)
        .unwrap_err()
        .to_string();
    assert!(
        error.contains("selected songs sharing a bank must agree"),
        "{error}"
    );
    assert!(!options.output.exists());
    fs::write(&fonts[1], &changed).unwrap();
    options.output.set_file_name("agreed-pitch");
    let report = bof3_audio::music::packing::songs(&options).unwrap();
    assert_eq!(report.archives[0].changed_entries, [0]);
    assert!(report
        .songs
        .iter()
        .all(|s| s.pitch_controls.tones.len() == 1));
    let rebuilt =
        ArchiveImage::from_bytes(fs::read(options.output.join(&report.archives[0].path)).unwrap())
            .unwrap();
    let before = ArchiveImage::from_bytes(source.to_vec()).unwrap();
    for index in 1..before.entries().len() {
        assert_eq!(before.entry(index).unwrap(), rebuilt.entry(index).unwrap());
    }
    assert_eq!(rebuilt.entry(0).unwrap()[0x824], 59);
    for font in fonts {
        fs::write(font, original_font).unwrap();
    }
}
