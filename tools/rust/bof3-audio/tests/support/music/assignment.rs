use bof3_audio::pack;
use emi_ex_v2::image::ArchiveImage;
use std::{fs, path::PathBuf};

pub fn fixture() -> (ArchiveImage, Vec<u8>) {
    let one = super::music_fixture::synthetic_archive(false);
    let image = ArchiveImage::from_bytes(one).unwrap();
    let mut header = image.entry(0).unwrap().to_vec();
    let mut body = image.entry(1).unwrap().to_vec();
    header[0x16..0x18].copy_from_slice(&2u16.to_le_bytes());
    header[0xc24..0xc26].copy_from_slice(&2u16.to_le_bytes());
    let mut extra = vec![0x11; 16];
    extra[0] = 4;
    extra[1] = 1;
    body.extend(extra);
    let image = image.replace_entries(&[(0, &header), (1, &body)]).unwrap();
    let sep = image.entry(2).unwrap().to_vec();
    let mut original = image.bytes()[..image.bytes().len() - b"archive trailer".len()].to_vec();
    original[..4].copy_from_slice(&5u32.to_le_bytes());
    let table = 16 + 4 * 16;
    original[table..table + 4].copy_from_slice(&(sep.len() as u32).to_le_bytes());
    original[table + 4..table + 8].copy_from_slice(&2u32.to_le_bytes());
    original[table + 12..table + 14].copy_from_slice(&9u16.to_le_bytes());
    original.extend(sep);
    original.resize(original.len().div_ceil(2048) * 2048, 0x5a);
    original.extend(b"archive trailer");
    (image, original)
}
pub fn check(mut options: pack::Options, fonts: &[PathBuf], original_font: &[u8], source: &[u8]) {
    let mut changed = original_font.to_vec();
    super::tone_controls::map(
        &mut changed,
        0,
        true,
        |op, value| if op == 53 { 1 } else { value },
    );
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
    options.output.set_file_name("agreed-assignment");
    let report = bof3_audio::music::packing::songs(&options).unwrap();
    assert_eq!(report.archives[0].changed_entries, [0]);
    assert!(report.songs.iter().all(|s| s.assignments.tones.len() == 1));
    let rebuilt =
        ArchiveImage::from_bytes(fs::read(options.output.join(&report.archives[0].path)).unwrap())
            .unwrap();
    let before = ArchiveImage::from_bytes(source.to_vec()).unwrap();
    for i in 1..before.entries().len() {
        assert_eq!(before.entry(i).unwrap(), rebuilt.entry(i).unwrap());
    }
    assert_eq!(rebuilt.entry(0).unwrap()[0x836], 2);
    for font in fonts {
        fs::write(font, original_font).unwrap();
    }
}
