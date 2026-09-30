//! Independent extraction-consumer parsing; does not use production manifest/RIFF readers.

use roxmltree::Node;
use std::{
    collections::BTreeMap,
    path::{Component, Path, PathBuf},
};

pub fn attr(node: Node<'_, '_>, name: &str) -> String {
    node.attribute(name)
        .unwrap_or_else(|| panic!("missing {name}"))
        .to_owned()
}
pub fn num(node: Node<'_, '_>, name: &str) -> usize {
    attr(node, name).parse().unwrap()
}
pub fn children<'a, 'i>(node: Node<'a, 'i>, name: &str) -> Vec<Node<'a, 'i>> {
    node.children().filter(|n| n.has_tag_name(name)).collect()
}
pub fn child<'a, 'i>(node: Node<'a, 'i>, name: &str) -> Node<'a, 'i> {
    let found = children(node, name);
    assert_eq!(found.len(), 1, "expected one {name}");
    found[0]
}
pub fn hex(text: &str) -> Vec<u8> {
    let text: String = text.chars().filter(|c| !c.is_whitespace()).collect();
    assert_eq!(text.len() % 2, 0);
    text.as_bytes()
        .as_chunks::<2>()
        .0
        .iter()
        .map(|pair| u8::from_str_radix(std::str::from_utf8(pair).unwrap(), 16).unwrap())
        .collect()
}
pub fn relative(root: &Path, base: &Path, name: &str) -> PathBuf {
    let path = Path::new(name);
    assert!(!path.is_absolute());
    assert!(!path
        .components()
        .any(|part| matches!(part, Component::RootDir | Component::Prefix(_))));
    let result = base.join(path).canonicalize().unwrap();
    assert!(result.starts_with(root.canonicalize().unwrap()));
    result
}
pub fn u16le(bytes: &[u8], at: usize) -> u16 {
    u16::from_le_bytes(bytes[at..at + 2].try_into().unwrap())
}
pub fn u32le(bytes: &[u8], at: usize) -> u32 {
    u32::from_le_bytes(bytes[at..at + 4].try_into().unwrap())
}
pub fn riff(bytes: &[u8]) -> BTreeMap<String, Vec<u8>> {
    assert_eq!(&bytes[..4], b"RIFF");
    assert_eq!(&bytes[8..12], b"WAVE");
    assert_eq!(u32le(bytes, 4) as usize + 8, bytes.len());
    let mut at = 12;
    let mut chunks = BTreeMap::new();
    while at < bytes.len() {
        let key = String::from_utf8(bytes[at..at + 4].to_vec()).unwrap();
        let size = u32le(bytes, at + 4) as usize;
        assert!(chunks
            .insert(key, bytes[at + 8..at + 8 + size].to_vec())
            .is_none());
        at += 8 + size + (size & 1);
    }
    assert_eq!(at, bytes.len());
    assert_eq!(u16le(&chunks["fmt "], 0), 1);
    assert_eq!(u16le(&chunks["fmt "], 14), 16);
    chunks
}
