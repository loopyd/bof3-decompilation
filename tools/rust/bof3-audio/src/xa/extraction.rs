//! XA stream/cue WAV exports with source-wide XML preservation and sector maps.

use crate::{
    archive::MediaImage, archive::XaImage, catalog::loader, catalog::model::Asset,
    catalog::model::AssetData, catalog::model::Catalog, catalog::model::XaCue, digest::sha256_hex,
    document::xml, interchange::wave::Wave, machine::executable::Executable,
    publication::require_absent, publication::write_new, publication::Publication, xa::Arithmetic,
    xa::Decoder, xa::Histories, xa::Stream, Result,
};
use serde::Serialize;
use std::{collections::BTreeMap, fmt::Write, fs, path::PathBuf};

#[derive(Clone, Copy, Debug)]
pub enum Kind {
    Stream,
    Cue,
}
impl Kind {
    pub fn name(self) -> &'static str {
        match self {
            Self::Stream => "xa_stream",
            Self::Cue => "xa_cue",
        }
    }
}

pub struct Options {
    pub disc_root: Option<PathBuf>,
    pub archives: Vec<PathBuf>,
    pub executable: Option<PathBuf>,
    pub output: PathBuf,
    pub id: Option<String>,
    pub kind: Kind,
    pub arithmetic: Arithmetic,
}

#[derive(Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub output: String,
    pub sources: usize,
    pub assets: usize,
    pub sectors: usize,
    pub pcm_frames: usize,
    pub arithmetic: Arithmetic,
    pub initial_history: &'static str,
    pub unverified_sources: Vec<String>,
    pub unselected_sources: Vec<String>,
    pub assumptions: Vec<&'static str>,
}

pub fn extract(options: &Options) -> Result<Report> {
    require_absent(&options.output)?;
    if matches!(options.kind, Kind::Cue) && options.executable.is_none() {
        return Err("XA cue extraction requires --executable for verified cue tables".into());
    }
    let mut catalog = Catalog::read(options.disc_root.as_deref(), &options.archives)?;
    let runtime = options
        .executable
        .as_ref()
        .map(|path| -> Result<_> {
            let executable = Executable::from_bytes(fs::read(path)?)?;
            loader::resolve(&mut catalog, &executable)
        })
        .transpose()?;
    let selected = if let Some(id) = &options.id {
        vec![catalog.select("audio", Some(options.kind.name()), id)?]
    } else {
        catalog
            .assets
            .iter()
            .filter(|a| a.kind() == options.kind.name())
            .collect()
    };
    if selected.is_empty() {
        return Err(format!("no {} assets resolved; cue extraction needs recognized original media and complete cue sectors", options.kind.name()).into());
    }
    let mut grouped = BTreeMap::<&str, Vec<&Asset>>::new();
    for asset in selected {
        grouped.entry(&asset.source).or_default().push(asset);
    }
    let publication = Publication::new(&options.output)?;
    fs::create_dir(publication.staging.join("sources"))?;
    let mut root = format!("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<audio schema=\"bof3.audio-extraction/v1\" mode=\"audio\" kind=\"{}\">\n", options.kind.name());
    let mut report = Report {
        schema: "bof3.xa-extract-report/v1", output: options.output.display().to_string(), sources: 0,
        assets: 0, sectors: 0, pcm_frames: 0, arithmetic: options.arithmetic, initial_history: "zero_per_exported_asset",
        unverified_sources: Vec::new(),
        unselected_sources: catalog.sources.iter().filter(|s| s.container == "xa" && !grouped.contains_key(s.source.as_str())).map(|s| s.source.clone()).collect(),
        assumptions: vec![
            "PCM concatenates selected audio sectors at their encoded rate; XML retains physical sector placement",
            "zero initial decoder history per exported asset is a convention, not verified game seek/reset behavior",
            "decoder arithmetic is explicit; CD resampling, drive timing, de-emphasis and audible cue endpoints are not established",
            "each source STR is preserved once in XML, including unrelated sectors, tails and EDC bytes",
            "SFX/vocal classification and hardware playback remain unverified; packing supports fixed-capacity edits with explicit history boundary checks",
        ],
    };
    for (source_id, assets) in grouped {
        let source = catalog
            .sources
            .iter()
            .find(|s| s.source == source_id)
            .ok_or("XA source missing")?;
        let path = options
            .disc_root
            .as_ref()
            .map_or_else(|| PathBuf::from(source_id), |root| root.join(source_id));
        let MediaImage::Xa(image) = MediaImage::read(&path)? else {
            return Err("selected XA asset is not in an XA source".into());
        };
        let hash = sha256_hex(image.bytes());
        if source.sha256.as_deref() != Some(&hash) {
            return Err(format!("{source_id}: source changed after inventory").into());
        }
        let reference = crate::xa::reference::identify(&hash);
        if reference.as_ref().is_some_and(|r| r.is_truncated()) {
            return Err(format!(
                "{source_id}: known truncated disc extent; supply the complete original STR"
            )
            .into());
        }
        let completeness = if reference.is_some() {
            "known_complete_disc_extent"
        } else {
            report.unverified_sources.push(source_id.to_owned());
            "unrecognized_source_extent_not_verified"
        };
        let snapshot_path = format!("sources/{}.xml", sha256_hex(source_id.as_bytes()));
        let snapshot = format!("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xa_source schema=\"bof3.xa-source/v1\" source=\"{}\" sector_bytes=\"2336\" sectors=\"{}\" completeness=\"{completeness}\"><preservation encoding=\"hex\" bytes=\"{}\" sha256=\"{hash}\">{}</preservation></xa_source>\n", xml::attribute(source_id)?, image.sectors().len(), image.bytes().len(), xml::hex(image.bytes()));
        write_new(
            &publication.staging.join(&snapshot_path),
            snapshot.as_bytes(),
        )?;
        if fs::read(publication.staging.join(&snapshot_path))? != snapshot.as_bytes() {
            return Err("XA snapshot XML read-back failed".into());
        }
        writeln!(
            root,
            "<source id=\"{}\" path=\"{snapshot_path}\" sha256=\"{hash}\"/>",
            xml::attribute(source_id)?
        )?;
        report.sources += 1;
        for asset in assets {
            let (stream, indices) = selection(asset, &image)?;
            let mut decoder = Decoder::new(stream, options.arithmetic, Histories::default())?;
            let folder = format!("xa/{}", sha256_hex(asset.id.as_bytes()));
            let directory = publication.staging.join(&folder);
            fs::create_dir_all(&directory)?;
            let mut pcm = Vec::new();
            let mut sectors = String::new();
            let mut encoded = Vec::new();
            let frames_per_sector = decoder.format().frames_per_sector();
            for (position, &index) in indices.iter().enumerate() {
                let sector = image.sector(index)?;
                pcm.extend(
                    decoder
                        .decode_sector(sector)
                        .map_err(|e| format!("{} sector {index}: {e}", asset.id))?,
                );
                encoded.extend_from_slice(sector);
                writeln!(sectors, "<sector index=\"{index}\" frame_start=\"{}\" frames=\"{frames_per_sector}\" submode=\"{}\"/>", position * frames_per_sector, sector[2])?;
            }
            let wave = Wave::new(
                decoder.format().channels(),
                decoder.format().sample_rate(),
                pcm,
            )?;
            let wav_bytes = wave.to_bytes()?;
            write_new(&directory.join("audio.wav"), &wav_bytes)?;
            let written = fs::read(directory.join("audio.wav"))?;
            if written != wav_bytes || Wave::from_bytes(&written)?.to_bytes()? != wav_bytes {
                return Err("XA WAV read-back failed".into());
            }
            let mut manifest = format!("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xa_asset schema=\"bof3.xa-asset/v1\" id=\"{}\" source=\"{}\" kind=\"{}\" content_type=\"unresolved\">\n<source_ref path=\"../../{snapshot_path}\" sha256=\"{hash}\"/>\n<stream{}{} />\n", xml::attribute(&asset.id)?, xml::attribute(source_id)?, asset.kind(), xml::attributes(&stream, &[])?, xml::attributes(&decoder.format(), &[])?) ;
            let arithmetic = serde_json::to_value(options.arithmetic)?;
            writeln!(manifest, "<decoder arithmetic=\"{}\" initial_history=\"zero_per_exported_asset\" resampling=\"none\" audible_endpoint=\"unverified\"/>", arithmetic.as_str().ok_or("arithmetic must be string")?)?;
            writeln!(
                manifest,
                "<final_history><left{}/><right{}/></final_history>",
                xml::attributes(&decoder.histories().left, &[])?,
                xml::attributes(&decoder.histories().right, &[])?
            )?;
            writeln!(manifest, "<wave path=\"audio.wav\" frames=\"{}\" sha256=\"{}\" selected_sectors_sha256=\"{}\"/>\n<sectors>{sectors}</sectors>", wave.frames(), sha256_hex(&wav_bytes), sha256_hex(&encoded))?;
            if let Some(runtime) = &runtime {
                writeln!(
                    manifest,
                    "<runtime profile=\"{}\" executable_sha256=\"{}\" selector=\"{}\"/>",
                    runtime.profile.id, runtime.profile.exe_sha256, runtime.xa.selector
                )?;
                for mapped in &runtime.xa.cues {
                    if mapped.cue.filter_file != stream.file
                        || mapped.cue.channel != stream.channel
                        || !mapped
                            .bindings
                            .iter()
                            .any(|b| b.source == source_id && b.asset.is_some())
                    {
                        continue;
                    }
                    if let (Ok(first), Ok(last)) = (
                        indices.binary_search(&mapped.cue.sector_start),
                        indices.binary_search(&mapped.cue.last_sector),
                    ) {
                        write_cue(
                            &mut manifest,
                            &mapped.cue,
                            first * frames_per_sector,
                            (last + 1) * frames_per_sector,
                        )?;
                    }
                }
            } else {
                manifest.push_str(
                    "<runtime resolution=\"not_supplied\" cue_placement=\"unresolved\"/>\n",
                );
            }
            manifest.push_str("</xa_asset>\n");
            write_new(&directory.join("xa.xml"), manifest.as_bytes())?;
            if fs::read(directory.join("xa.xml"))? != manifest.as_bytes() {
                return Err("XA asset XML read-back failed".into());
            }
            writeln!(
                root,
                "<asset id=\"{}\" path=\"{folder}/xa.xml\"/>",
                xml::attribute(&asset.id)?
            )?;
            report.assets += 1;
            report.sectors += indices.len();
            report.pcm_frames += wave.frames();
        }
    }
    root.push_str("</audio>\n");
    write_new(&publication.staging.join("audio.xml"), root.as_bytes())?;
    if fs::read(publication.staging.join("audio.xml"))? != root.as_bytes() {
        return Err("XA root XML read-back failed".into());
    }
    publication.publish()?;
    Ok(report)
}

pub(crate) fn selection(asset: &Asset, image: &XaImage) -> Result<(Stream, Vec<usize>)> {
    let (stream, indices) = match &asset.data {
        AssetData::XaStream(stream) => {
            let key = Stream {
                file: stream.file,
                channel: stream.channel,
                coding: stream.coding,
            };
            let indices = image
                .sectors()
                .iter()
                .filter(|s| {
                    s.is_audio()
                        && (s.file, s.channel, s.coding) == (key.file, key.channel, key.coding)
                })
                .map(|s| s.index)
                .collect();
            (key, indices)
        }
        AssetData::XaCue(cue) => {
            let first = image.sector(cue.sector_start)?;
            let key = Stream {
                file: cue.filter_file,
                channel: cue.channel,
                coding: first[3],
            };
            let indices: Vec<_> = (0..cue.sector_count)
                .map(|n| {
                    n.checked_mul(cue.sector_stride)
                        .and_then(|offset| cue.sector_start.checked_add(offset))
                        .ok_or("XA cue sector arithmetic overflow")
                })
                .collect::<std::result::Result<_, _>>()?;
            if indices.last() != Some(&cue.last_sector) {
                return Err("XA cue extent is inconsistent".into());
            }
            (key, indices)
        }
        _ => return Err("unsupported XA extraction asset kind".into()),
    };
    if indices.is_empty() {
        return Err("XA selection has no audio sectors".into());
    }
    Ok((stream, indices))
}

fn write_cue(xml: &mut String, cue: &XaCue, first: usize, end: usize) -> Result<()> {
    writeln!(
        xml,
        "<cue{} frame_start=\"{first}\" frame_end_exclusive=\"{end}\">",
        crate::document::xml::attributes(
            cue,
            &["runtime_start_bcd", "runtime_stop_threshold_bcd"]
        )?
    )?;
    for (tag, bcd) in [
        ("runtime_start_bcd", cue.runtime_start_bcd),
        ("runtime_stop_threshold_bcd", cue.runtime_stop_threshold_bcd),
    ] {
        writeln!(
            xml,
            "<{tag} minute=\"{}\" second=\"{}\" frame=\"{}\"/>",
            bcd[0], bcd[1], bcd[2]
        )?;
    }
    xml.push_str("</cue>\n");
    Ok(())
}
