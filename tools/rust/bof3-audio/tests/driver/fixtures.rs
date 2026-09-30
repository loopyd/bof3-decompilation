use bof3_audio::machine::{bank, executable::Executable, firmware::Image};
use emi_ex_v2::image::ArchiveImage;
use std::{fs, path::PathBuf};

pub fn prepare(name: &str) -> (bank::Prepared, ArchiveImage) {
    let archive = ArchiveImage::from_bytes(
        fs::read(
            PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap())
                .join(format!("BIN/BATTLE/{name}.EMI")),
        )
        .unwrap(),
    )
    .unwrap();
    prepare_image(archive)
}

pub fn prepare_image(archive: ArchiveImage) -> (bank::Prepared, ArchiveImage) {
    prepare_entries(archive, 0, 2)
}

pub fn prepare_entries(
    archive: ArchiveImage,
    header: usize,
    body: usize,
) -> (bank::Prepared, ArchiveImage) {
    let exe =
        Executable::from_bytes(fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let bios =
        Image::from_bytes(fs::read(std::env::var_os("BOF3_AUDIO_BIOS").unwrap()).unwrap()).unwrap();
    let prepared = bank::prepare(
        &exe,
        bios,
        &archive,
        &bank::Options {
            header_entry: header,
            body_entry: body,
            sequence_entry: None,
            layout: None,
        },
        &mut |_, _| Ok(()),
    )
    .unwrap();
    (prepared, archive)
}
