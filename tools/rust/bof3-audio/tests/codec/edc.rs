use bof3_audio::codec::edc::{check_form2, checksum, Status};

#[test]
fn cd_edc_check_vector_and_form2_coverage_include_both_subheaders_and_spare_bytes() {
    assert_eq!(checksum(b"123456789"), 0x6ec2_edc4);
    assert_eq!(checksum(&[]), 0);
    let mut sector = vec![0; 2336];
    sector[..8].copy_from_slice(&[1, 2, 0x64, 0, 1, 2, 0x64, 0]);
    for (i, b) in sector[8..2332].iter_mut().enumerate() {
        *b = (i * 37) as u8;
    }
    assert_eq!(check_form2(&sector).unwrap(), Status::Absent);
    let crc = checksum(&sector[..2332]);
    sector[2332..].copy_from_slice(&crc.to_le_bytes());
    assert_eq!(check_form2(&sector).unwrap(), Status::Valid(crc));
    for index in [8, 2311, 2312, 2331, 2335] {
        let mut bad = sector.clone();
        bad[index] ^= 1;
        assert!(check_form2(&bad).is_err(), "undetected change at {index}");
    }
    sector[0] = 9;
    sector[4] = 9;
    assert!(check_form2(&sector).is_err());
}

#[test]
fn wrong_sector_size_form_or_subheader_copy_cannot_be_accepted() {
    for size in [0, 2, 2335, 2337] {
        assert!(check_form2(&vec![0; size]).is_err());
    }
    let mut sector = vec![0; 2336];
    assert!(check_form2(&sector).is_err());
    sector[2] = 0x24;
    assert!(check_form2(&sector).is_err());
}

#[test]
#[ignore = "requires BOF3_AUDIO_TRACK; validate checksum polynomial against original nonzero Form 1 EDC"]
fn checksum_matches_original_disc_data_sectors() {
    use std::{fs::File, io::Read};
    let mut file = File::open(std::env::var_os("BOF3_AUDIO_TRACK").unwrap()).unwrap();
    let mut bytes = vec![0; 2352 * 32];
    file.read_exact(&mut bytes).unwrap();
    let mut checked = 0;
    for sector in bytes.as_chunks::<2352>().0 {
        if sector[15] == 2 && sector[18] & 0x20 == 0 {
            let stored = u32::from_le_bytes(sector[2072..2076].try_into().unwrap());
            assert_ne!(stored, 0);
            assert_eq!(checksum(&sector[16..2072]), stored);
            checked += 1;
        }
    }
    assert!(
        checked >= 16,
        "insufficient independent checksum sectors: {checked}"
    );
    eprintln!("Original-disc EDC polynomial agrees for {checked} nonzero data-sector checksums");
}
