//! Source-preserving song folders with independent MIDI sequences and paired SF2.
use crate::{
    archive::MediaImage, catalog::loader, catalog::model::Catalog, document::manifest,
    document::xml, machine::executable::Executable, music::document::SongReport,
    publication::require_absent, publication::write_new, publication::Publication,
    voice::tuning::Reference, Result,
};
use serde::Serialize;
use std::{
    collections::BTreeMap,
    fmt::Write as _,
    fs,
    path::{Path, PathBuf},
};

pub struct Options {
    pub disc_root: Option<PathBuf>,
    pub archives: Vec<PathBuf>,
    pub executable: PathBuf,
    pub output: PathBuf,
    pub id: Option<String>,
    pub loops: u32,
    pub allow_approximations: bool,
}

#[derive(Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub output: String,
    pub songs: Vec<SongReport>,
    pub limitations: Vec<&'static str>,
}
pub fn songs(options: &Options) -> Result<Report> {
    if !options.allow_approximations {
        return Err("music extract: current SF2 envelope/gain/reverb, controller/bend and fixed-loop approximations require explicit --allow-approximations; complete runtime fidelity is not established".into());
    }
    if options.loops == 0 {
        return Err("music extract: --loops must be positive".into());
    }
    require_absent(&options.output)?;
    let mut catalog = Catalog::read(options.disc_root.as_deref(), &options.archives)?;
    let executable = Executable::from_bytes(fs::read(&options.executable)?)?;
    let mapping = loader::resolve(&mut catalog, &executable)?;
    let selected = if let Some(id) = &options.id {
        vec![catalog.select("music", Some("song"), id)?]
    } else {
        catalog
            .assets
            .iter()
            .filter(|a| a.kind() == "song")
            .collect::<Vec<_>>()
    };
    if selected.is_empty() {
        return Err("music extract: no songs in selected sources".into());
    }
    let mut reference = Reference::from_executable(&executable)?;
    let mut envelopes = BTreeMap::new();
    let publication = Publication::new(&options.output)?;
    let mut root = String::from(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<music schema=\"bof3.music-extraction/v1\">\n",
    );
    let mut report = Report {
        schema: "bof3.music-extract-report/v1",
        output: options.output.display().to_string(),
        songs: Vec::new(),
        limitations: crate::music::document::LIMITATIONS.to_vec(),
    };
    for song in selected {
        let source_path = options
            .disc_root
            .as_ref()
            .map_or_else(|| PathBuf::from(&song.source), |r| r.join(&song.source));
        let MediaImage::Emi(image) = MediaImage::read(&source_path)? else {
            return Err("music extract: song is not in EMI".into());
        };
        let prepared = crate::music::document::prepare(
            &catalog,
            song,
            &image,
            &mapping,
            &mut reference,
            &mut envelopes,
            options.loops,
        )?;
        let relative = Path::new(&prepared.report.path);
        let directory = publication
            .staging
            .join(relative.parent().ok_or("song folder missing")?);
        fs::create_dir_all(&directory)?;
        for (name, bytes) in &prepared.files {
            checked_write(&directory.join(name), bytes)?;
        }
        checked_write(&directory.join("song.xml"), prepared.document.as_bytes())?;
        writeln!(
            root,
            "<song id=\"{}\" bank=\"{}\" path=\"{}\" source_sha256=\"{}\"/>",
            xml::attribute(&prepared.report.id)?,
            xml::attribute(&prepared.report.bank)?,
            prepared.report.path,
            prepared.report.source_sha256
        )?;
        report.songs.push(prepared.report);
    }
    root.push_str("</music>\n");
    manifest::parse(&root, Default::default())?;
    checked_write(&publication.staging.join("music.xml"), root.as_bytes())?;
    checked_write(
        &publication.staging.join("extract.json"),
        &serde_json::to_vec_pretty(&report)?,
    )?;
    publication.publish()?;
    Ok(report)
}

fn checked_write(path: &Path, bytes: &[u8]) -> Result<()> {
    write_new(path, bytes)?;
    if fs::read(path)? != bytes {
        return Err(format!("{}: read-back verification failed", path.display()).into());
    }
    Ok(())
}
