use std::fs;
use std::path::Path;

use quick_xml::events::{BytesEnd, BytesStart, Event};
use quick_xml::Writer;
use serde::Serialize;

use crate::{Entry, Error};

#[derive(Serialize)]
struct LbaDocument {
    entries: Vec<LbaRow>,
}

// nlohmann::json emits object keys in lexical order. Keep this declaration in
// that same order so serde_json reproduces the v1 byte-level schema.
#[derive(Serialize)]
struct LbaRow {
    archive_name: String,
    archive_type: String,
    bytes: u32,
    family: String,
    lba: u32,
    length: u64,
    #[serde(skip_serializing_if = "Option::is_none")]
    manifest_path: Option<String>,
    name: String,
    row_type: &'static str,
    source_path: String,
    timecode: String,
}

pub fn write_project(path: &Path, entries: &[Entry], image_name: &str) -> Result<(), Error> {
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent)?;
    }
    let mut writer = Writer::new_with_indent(Vec::new(), b' ', 4);
    let mut project = BytesStart::new("iso_project");
    project.push_attribute(("image_name", image_name));
    project.push_attribute(("cue_sheet", "mkpsxiso.cue"));
    write_xml(&mut writer, Event::Start(project))?;
    let mut track = BytesStart::new("track");
    track.push_attribute(("type", "data"));
    track.push_attribute(("xa_edc", "false"));
    track.push_attribute(("new_type", "false"));
    write_xml(&mut writer, Event::Start(track))?;
    let mut license = BytesStart::new("license");
    license.push_attribute(("file", "license_data.dat"));
    write_xml(&mut writer, Event::Empty(license))?;
    write_xml(&mut writer, Event::Start(BytesStart::new("directory_tree")))?;
    for entry in entries.iter().filter(|entry| !entry.is_directory) {
        let source = entry.path.to_string_lossy().replace('\\', "/");
        let mut file = BytesStart::new("file");
        file.push_attribute((
            "name",
            entry
                .path
                .file_name()
                .and_then(|value| value.to_str())
                .unwrap_or(""),
        ));
        file.push_attribute(("source", source.as_str()));
        file.push_attribute(("type", "data"));
        write_xml(&mut writer, Event::Empty(file))?;
    }
    for name in ["directory_tree", "track", "iso_project"] {
        write_xml(&mut writer, Event::End(BytesEnd::new(name)))?;
    }
    let mut xml = writer.into_inner();
    xml.push(b'\n');
    fs::write(path, xml)?;
    Ok(())
}

pub fn write_lba_json(
    path: &Path,
    entries: &[Entry],
    raw_root: &Path,
    extracted_root: &Path,
) -> Result<(), Error> {
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent)?;
    }
    let mut files = entries
        .iter()
        .filter(|entry| !entry.is_directory)
        .collect::<Vec<_>>();
    files.sort_by_key(|entry| entry.lba);

    let rows = files
        .into_iter()
        .map(|entry| {
            let source_path = extracted_root
                .join(&entry.path)
                .to_string_lossy()
                .replace('\\', "/");
            let extension = entry
                .path
                .extension()
                .and_then(|value| value.to_str())
                .unwrap_or("")
                .to_ascii_uppercase();
            let archive_name = entry
                .path
                .file_stem()
                .and_then(|value| value.to_str())
                .unwrap_or("");
            let sector_size = if entry.is_xa { 2_336 } else { 2_048 };
            LbaRow {
                archive_name: archive_name.to_owned(),
                archive_type: extension.clone(),
                bytes: entry.size,
                family: family(&entry.path),
                lba: entry.lba,
                length: u64::from(entry.size).div_ceil(sector_size),
                manifest_path: (extension == "EMI")
                    .then(|| manifest_path(raw_root, extracted_root, &entry.path)),
                name: entry
                    .path
                    .file_name()
                    .and_then(|value| value.to_str())
                    .unwrap_or("")
                    .to_owned(),
                row_type: if entry.is_xa { "XA" } else { "File" },
                source_path,
                timecode: timecode(150 + entry.lba),
            }
        })
        .collect();
    let mut json = serde_json::to_string_pretty(&LbaDocument { entries: rows })
        .map_err(|error| Error::InvalidImageDetail(error.to_string()))?;
    json.push('\n');
    fs::write(path, json)?;
    Ok(())
}

fn manifest_path(raw_root: &Path, extracted_root: &Path, entry_path: &Path) -> String {
    let has_bin_component = entry_path.components().any(|part| {
        matches!(part, std::path::Component::Normal(value) if value.eq_ignore_ascii_case("BIN"))
    });
    let mut manifest = if has_bin_component {
        raw_root.join(entry_path)
    } else {
        extracted_root.join(entry_path)
    };
    manifest.set_extension("");
    manifest
        .join("emi.json")
        .to_string_lossy()
        .replace('\\', "/")
}

fn family(path: &Path) -> String {
    let parts: Vec<_> = path
        .components()
        .filter_map(|part| match part {
            std::path::Component::Normal(value) => value.to_str(),
            _ => None,
        })
        .collect();
    parts
        .windows(2)
        .find(|pair| pair[0].eq_ignore_ascii_case("BIN"))
        .map_or_else(|| "unknown".to_owned(), |pair| pair[1].to_owned())
}

fn timecode(sectors: u32) -> String {
    let minutes = sectors / 4500;
    let remainder = sectors % 4500;
    format!("{minutes:02}:{:02}:{:02}", remainder / 75, remainder % 75)
}

fn write_xml(writer: &mut Writer<Vec<u8>>, event: Event<'_>) -> Result<(), Error> {
    writer
        .write_event(event)
        .map_err(|error| Error::InvalidImageDetail(format!("cannot encode project XML: {error}")))
}
