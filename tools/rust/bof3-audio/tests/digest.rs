use bof3_audio::digest::sha256_hex;

#[test]
fn published_sha256_vectors() {
    assert_eq!(
        sha256_hex(b"abc"),
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
    );
    assert_eq!(
        sha256_hex(&vec![b'a'; 1_000_000]),
        "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0"
    );
}

#[test]
fn hashlib_reference_vectors_cover_padding_and_block_boundaries() {
    for (size, expected) in [
        (
            0,
            "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
        ),
        (
            1,
            "ca358758f6d27e6cf45272937977a748fd88391db679ceda7dc7bf1f005ee879",
        ),
        (
            55,
            "8aa994584139d128848eeebc4e815639ba5ab6e6e39574195a63ac4f14f7c43b",
        ),
        (
            56,
            "ad574708f75c044c9b85de64cb568ee7711ff4f36448c6242f053ba8f6cc2b63",
        ),
        (
            63,
            "280ed3e8ff1df845b2e7dfe6ac6cee817bef20e783cc65abc41b818b4d2fe076",
        ),
        (
            64,
            "c6ab9724ade5b6a7a1edfffb12f3aa9181351355af8fd08c919952ad211339dd",
        ),
        (
            65,
            "788367c73c7ddf4c53f65e68cc0d943e6227ab55b0e78ba63ace822b1c6301c0",
        ),
        (
            119,
            "3d610547d68216dedf7435a4fb6260353911f6b3fd3f18805ddb8be285d726fe",
        ),
        (
            120,
            "1f80156a804cb7862ad113e8200e9d74499723e7c7854d5f48776d3148e09656",
        ),
        (
            127,
            "192409cd280e14b743642ad1343fbd3e82d9305de72c078117745a679210cc3d",
        ),
        (
            128,
            "cc548ca2dec1f6fe4f58b2e27aa9c7521607df1130d140b55a4dad0665302356",
        ),
        (
            129,
            "81e89a7b2911aaa7795f9e3d4910cb47d6cd2b00d83b8399481527261a1a7519",
        ),
        (
            1000,
            "5097e7d587352f5097062ae679f37bda5802d9f875aba14c8cb4d1a188ada179",
        ),
    ] {
        let bytes: Vec<_> = (0..size).map(|i| ((i * 31 + 7) % 256) as u8).collect();
        assert_eq!(sha256_hex(&bytes), expected, "length {size}");
    }
}
