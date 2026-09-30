use super::{extract, music_fixture, options, Directory};
use bof3_audio::{interchange::midi::Midi, sequence::SequenceSet};
use emi_ex_v2::image::ArchiveImage;
use std::fs;

#[test]
#[ignore = "requires BOF3_AUDIO_EXE; MIDI timing with resized SEP publication"]
fn retimed_sequence_relocates_untouched_sequence_and_publishes_verified_archive() {
    let d = Directory::new();
    let source = music_fixture::synthetic_archive(false);
    let path = extract(&d.0, &source);
    let midi_path = path.parent().unwrap().join("sequence-000.mid");
    let mut midi = Midi::from_bytes(&fs::read(&midi_path).unwrap()).unwrap();
    for track in &mut midi.tracks {
        for e in &mut track.events {
            e.tick *= 100;
        }
    }
    fs::write(&midi_path, midi.to_bytes().unwrap()).unwrap();
    let opts = options(&d.0, "retimed");
    let report = bof3_audio::music::packing::songs(&opts).unwrap();
    assert_eq!(report.archives[0].changed_entries, [2]);
    let before = ArchiveImage::from_bytes(source).unwrap();
    let after =
        ArchiveImage::from_bytes(fs::read(opts.output.join(&report.archives[0].path)).unwrap())
            .unwrap();
    for i in [0, 1, 3] {
        assert_eq!(before.entry(i).unwrap(), after.entry(i).unwrap());
    }
    let a = before.entry(2).unwrap();
    let b = after.entry(2).unwrap();
    let old = SequenceSet::parse(a).unwrap();
    let new = SequenceSet::parse(b).unwrap();
    assert!(new.sequences[1].data_offset > old.sequences[1].data_offset);
    assert_eq!(
        &a[old.sequences[1].data_offset - 13..],
        &b[new.sequences[1].data_offset - 13..]
    );
    assert!(!report.songs[0].sequences[0].timing_edits.is_empty());
    assert!(report.songs[0].sequences[1].original_bytes_reused);
    let published = fs::read(opts.output.join(&report.archives[0].path)).unwrap();
    assert!(bof3_audio::music::packing::songs(&opts)
        .unwrap_err()
        .to_string()
        .contains("already exists"));
    assert_eq!(
        fs::read(opts.output.join(&report.archives[0].path)).unwrap(),
        published
    );
}
