use bof3_audio::{
    digest::sha256_hex,
    document::manifest::{self, Limits},
};

#[test]
fn xml_parser_preserves_normalized_values_comments_cdata_and_unicode() {
    let root = manifest::parse("<?xml version='1.0' encoding='UTF-8'?>\r\n<!-- kept as annotation -->\r\n<bank source='é &amp; &quot; &#9;&#10;&#13;\t\r\nx'><preservation><![CDATA[ab]]> cd<!-- note --> ef</preservation></bank>", Limits::default()).unwrap();
    assert_eq!(root.attribute("source").unwrap(), "é & \" \t\n\r  x");
    assert_eq!(root.child("preservation").unwrap().text, "ab cd ef");
    assert_eq!(root.children.len(), 1);
}

#[test]
fn malformed_ambiguous_external_and_unsupported_xml_fail() {
    for source in [
        "",
        "<a>",
        "<a/><b/>",
        "<a x='1' x='2'/>",
        "<a><b></a></b>",
        "<a>&unknown;</a>",
        "<a>&#0;</a>",
        "<a>&#xD800;</a>",
        "<a>&#x110000;</a>",
        "<?xml version='1.1'?><a/>",
        "<?xml version='1.0' encoding='ISO-8859-1'?><a/>",
        "<!DOCTYPE a [<!ENTITY file SYSTEM 'file:///etc/passwd'>]><a>&file;</a>",
        "<!DOCTYPE a><a/>",
        "<a xmlns='urn:x'/>",
        "<a xmlns:q='urn:x'/>",
        "<?process instruction?><a/>",
        "<a xml:lang='en'/>",
    ] {
        assert!(
            manifest::parse(source, Limits::default()).is_err(),
            "accepted {source}"
        );
    }
    let literal = manifest::parse(
        "<a><!-- &#xD800; --><![CDATA[&#xD800;]]>&amp;#xD800;</a>",
        Limits::default(),
    )
    .unwrap();
    assert_eq!(literal.text, "&#xD800;&#xD800;");
}

#[test]
fn explicit_document_node_and_depth_limits_are_enforced() {
    assert!(manifest::parse(
        "<a/>",
        Limits {
            bytes: 3,
            ..Limits::default()
        }
    )
    .is_err());
    assert!(manifest::parse(
        "<a><b/><c/></a>",
        Limits {
            nodes: 2,
            ..Limits::default()
        }
    )
    .is_err());
    assert!(manifest::parse(
        "<a><b><c/></b></a>",
        Limits {
            depth: 2,
            ..Limits::default()
        }
    )
    .is_err());
    assert!(manifest::parse(
        "<a><b/></a>",
        Limits {
            depth: 2,
            ..Limits::default()
        }
    )
    .is_ok());
}

#[test]
fn preservation_checks_exact_bytes_digest_scope_and_unknown_fields() {
    let source = format!("<preservation encoding='hex' scope='entire_source_archive' bytes='3' sha256='{}'>00 AB\nff</preservation>", sha256_hex(&[0, 0xab, 255]));
    let parsed = manifest::parse(&source, Limits::default()).unwrap();
    assert_eq!(
        manifest::preservation(&parsed, Some("entire_source_archive")).unwrap(),
        [0, 0xab, 255]
    );
    assert!(manifest::preservation(&parsed, None).is_err());
    for bad in [
        source.replace("bytes='3'", "bytes='4'"),
        source.replace("AB", "AG"),
        source.replace("ff</", "f</"),
        source.replace("00 AB", "01 AB"),
        source.replace("encoding='hex'", "encoding='base64'"),
        source.replace("bytes='3'", "bytes='3' extra='x'"),
    ] {
        assert!(manifest::preservation(
            &manifest::parse(&bad, Limits::default()).unwrap(),
            Some("entire_source_archive")
        )
        .is_err());
    }
}

#[test]
fn metadata_changes_unknown_fields_and_duplicate_required_children_are_visible() {
    let expected = serde_json::json!({"id": 2, "signed": -1, "flag": true, "name": "a"});
    let good = manifest::parse(
        "<tone id='002' signed='-01' flag='true' name='a'/>",
        Limits::default(),
    )
    .unwrap();
    good.scalars(&expected, &[], &[]).unwrap();
    for (field, value) in [
        ("id", "3"),
        ("signed", "1"),
        ("flag", "false"),
        ("new", "0"),
    ] {
        let mut changed = good.clone();
        changed.attributes.insert(field.into(), value.into());
        assert!(changed.scalars(&expected, &[], &[]).is_err());
    }
    let root = manifest::parse("<a><bank/><bank/></a>", Limits::default()).unwrap();
    assert!(root.child("bank").is_err());
    assert!(root.child("missing").is_err());
    assert!(root.shape(&[], &["song"], false).is_err());
    let non_xml_space = manifest::parse("<a>&#160;</a>", Limits::default()).unwrap();
    assert!(non_xml_space.shape(&[], &[], false).is_err());
}

#[test]
fn relative_paths_resolve_within_root_and_reject_escape_or_nonfiles() {
    let root = std::env::temp_dir().join(format!("bof3-manifest-{}", std::process::id()));
    std::fs::create_dir(&root).unwrap();
    let bank = root.join("bank");
    std::fs::create_dir(&bank).unwrap();
    std::fs::write(root.join("asset.wav"), b"test").unwrap();
    assert_eq!(
        manifest::relative_file(&root, &bank, "../asset.wav").unwrap(),
        root.join("asset.wav").canonicalize().unwrap()
    );
    for path in [
        "",
        "/etc/passwd",
        "../../etc/passwd",
        "..",
        "C:\\asset.wav",
        "file:asset.wav",
    ] {
        assert!(manifest::relative_file(&root, &bank, path).is_err());
    }
    #[cfg(unix)]
    {
        std::os::unix::fs::symlink("/etc/passwd", bank.join("outside.wav")).unwrap();
        assert!(manifest::relative_file(&root, &bank, "outside.wav").is_err());
    }
    std::fs::remove_dir_all(root).unwrap();
}
