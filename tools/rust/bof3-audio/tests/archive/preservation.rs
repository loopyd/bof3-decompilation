use bof3_audio::{
    archive::{MediaImage, XaImage, XA_SECTOR_BYTES},
    verify::{compare_bytes, Report},
};

fn stream() -> Vec<u8> {
    let mut bytes = vec![0xab; XA_SECTOR_BYTES * 3];
    for (index, header) in [[1, 2, 0x64, 0], [1, 7, 0x62, 0], [1, 2, 0xe4, 0]]
        .iter()
        .enumerate()
    {
        let start = index * XA_SECTOR_BYTES;
        bytes[start..start + 4].copy_from_slice(header);
        bytes[start + 4..start + 8].copy_from_slice(header);
    }
    bytes
}

#[test]
fn xa_preserves_interleaving_video_eof_and_edc_bytes() {
    let bytes = stream();
    let image = XaImage::from_bytes(bytes.clone()).unwrap();
    let rebuilt: Vec<_> = (0..image.sectors().len())
        .flat_map(|index| image.sector(index).unwrap().iter().copied())
        .collect();
    assert_eq!(rebuilt, bytes);
    assert!(image.sectors()[0].is_audio());
    assert!(!image.sectors()[1].is_audio());
    assert!(image.sectors()[2].is_audio());
    assert!(image.sector(3).is_err());
}

#[test]
fn xa_rejects_truncation_and_conflicting_subheaders() {
    assert!(XaImage::from_bytes(vec![]).is_err());
    let mut bytes = stream();
    bytes.pop();
    assert!(XaImage::from_bytes(bytes).is_err());
    let mut bytes = stream();
    bytes[XA_SECTOR_BYTES + 4] ^= 1;
    let error = XaImage::from_bytes(bytes).unwrap_err().to_string();
    assert!(error.contains("sector 1"));
}

#[test]
fn byte_comparison_detects_payload_padding_and_length_changes() {
    let original = stream();
    assert!(compare_bytes(&original, &original).equal);
    for offset in [0, 8, XA_SECTOR_BYTES - 1, original.len() - 1] {
        let mut edited = original.clone();
        edited[offset] ^= 1;
        let result = compare_bytes(&original, &edited);
        assert!(!result.equal);
        assert_eq!(result.first_difference, Some(offset));
    }
    assert_eq!(compare_bytes(b"abc", b"abcd").first_difference, Some(3));
    assert_eq!(compare_bytes(b"abcd", b"abc").first_difference, Some(3));
}

#[test]
fn container_success_does_not_claim_runtime_or_semantic_acceptance() {
    let image = MediaImage::Xa(XaImage::from_bytes(stream()).unwrap());
    let report = Report::inspect("stream.str".into(), &image, None);
    assert_eq!(report.rendering, "not_checked");
    assert_eq!(report.translation, "not_checked");
    assert_eq!(report.identity_consistency, "not_checked");
    assert!(report.byte_equality.is_none());
}

#[test]
#[ignore = "requires user-supplied corpus via BOF3_AUDIO_CORPUS"]
fn local_corpus_preserves_every_container_byte() {
    let root = std::path::PathBuf::from(
        std::env::var_os("BOF3_AUDIO_CORPUS").expect("set BOF3_AUDIO_CORPUS to disc root"),
    );
    let mut pending = vec![root];
    let (mut emi_count, mut xa_count) = (0, 0);
    while let Some(directory) = pending.pop() {
        for item in std::fs::read_dir(directory).unwrap() {
            let item = item.unwrap();
            let kind = item.file_type().unwrap();
            if kind.is_dir() {
                pending.push(item.path());
                continue;
            }
            if !kind.is_file() {
                continue;
            }
            let path = item.path();
            let ext = path
                .extension()
                .and_then(|s| s.to_str())
                .unwrap_or("")
                .to_ascii_lowercase();
            if !["emi", "str", "xa"].contains(&ext.as_str()) {
                continue;
            }
            let original = std::fs::read(&path).unwrap();
            let image = MediaImage::read(&path)
                .unwrap_or_else(|error| panic!("{}: {error}", path.display()));
            let rebuilt = match image {
                MediaImage::Emi(archive) => {
                    emi_count += 1;
                    let entries: Vec<_> = (0..archive.entries().len())
                        .map(|index| (index, archive.entry(index).unwrap()))
                        .collect();
                    archive.replace_entries(&entries).unwrap().bytes().to_vec()
                }
                MediaImage::Xa(stream) => {
                    xa_count += 1;
                    (0..stream.sectors().len())
                        .flat_map(|index| stream.sector(index).unwrap().iter().copied())
                        .collect()
                }
            };
            assert_eq!(original, rebuilt, "{}", path.display());
        }
    }
    assert!(
        emi_count > 0 && xa_count > 0,
        "corpus must cover both EMI and XA"
    );
    eprintln!("Preserved {emi_count} EMI archives and {xa_count} XA streams (container API; XML workflow pending)");
}
