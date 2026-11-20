//! Original-US file identity used by coverage and loader-specific payload rules.

use std::sync::OnceLock;

use serde::Deserialize;

use crate::digest::sha256_hex;
use crate::models::{Entry, Error, Result};

pub const DISC: &str = include_str!("../../../../config/original-us-disc.json");
pub const US_EXECUTABLE_SHA256: &str =
    "0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a";

#[derive(Deserialize)]
pub struct Disc {
    pub release: String,
    pub project_sha256: String,
    pub files: Vec<File>,
}

#[derive(Deserialize)]
pub struct File {
    pub path: String,
    pub size: usize,
    pub sha256: String,
    pub emi_entries: Option<usize>,
}

pub fn disc() -> Result<&'static Disc> {
    static ORIGINAL: OnceLock<std::result::Result<Disc, String>> = OnceLock::new();
    ORIGINAL
        .get_or_init(|| serde_json::from_str(DISC).map_err(|error| error.to_string()))
        .as_ref()
        .map_err(|error| Error::Invalid(format!("original US disc qualification: {error}")))
}

/// Whole-archive identity, independent of the input's path or regional label.
pub fn archive(data: &[u8]) -> Result<bool> {
    let original = disc()?;
    let digest = sha256_hex(data);
    Ok(original
        .files
        .iter()
        .any(|file| file.emi_entries.is_some() && file.size == data.len() && file.sha256 == digest))
}

pub struct XaSource {
    pub file: &'static File,
    expected_audio_bodies: usize,
    expected_empty_payloads: usize,
    expected_packet_headers: usize,
}

pub struct NontextRange<'a> {
    pub offset: usize,
    pub length: usize,
    pub evidence: &'a str,
}

const SPRITE_CONSUMER_INSTRUCTION_EVIDENCE: &str = "Original US sprite consumer instructions; complete bodies with conditional original direct-call or finite handler-cell entries, full raw-word and structural control-flow review, and all control-transfer delay words including returns. The five-case switch at 0x8014CE78 uses the separately qualified table at 0x80149998. Exact original whole executable or archive/raw-payload identities and load addresses gate classification. tools/rust/bof3-text/docs/formats.md lists the body boundaries and entry routes. Ordinary callee returns, valid nonaliasing memory, intact frames and stable code/tables remain conditions. Referenced data, external callees and neighbors are excluded. No matching ELF, observed invocation, all-path feasibility, general selector bounds, transitive effects, resource safety, display or safe-edit claim.";
const BATTLE_CALLEE_INSTRUCTION_EVIDENCE: &str = "Original US shared battle instructions; 33 complete bodies with original direct-call entries from already qualified instructions, full raw-word and structural control-flow review, and all 602 control-transfer delay words including returns. The bounded switches at 0x800A805C and 0x800B3AE8 use separately qualified numeric tables. Exact original archive/raw-payload identities and load address gate classification. tools/rust/bof3-text/docs/formats.md records boundaries and entry sites. Call continuations assume ordinary returns, valid nonaliasing memory, intact frames and stable code/tables. Referenced data and external callees retain prior status. No ELF match, observed invocation, all-path feasibility, transitive effects, resource safety, display or safe-edit claim.";
const BATTLE_CALLEE_TABLE_EVIDENCE: &str = "Original US shared battle numeric jump tables at 0x80096974..0x80096994 and 0x80096A5C..0x80096A74. Complete original bodies 0x800A805C and 0x800B3AE8 apply unsigned bounds of eight and six before scaled table loads and register jumps. Every original target lies inside its consumer body. Exact original archive/raw-payload identities and load address gate classification. Neighboring bytes retain prior status. Stable tables and valid nonaliasing memory remain conditions. No observed invocation, general selector bound, resource-safety or safe-edit claim.";

const BATTLE_RESIDENT_TABLE_EVIDENCE: &str = "Original US battle resident animation numeric tables. Dispatcher 0x801E6C84 copies all five address words at 0x801D0F44..0x801D0F58 before its guarded indirect call. Initializer 0x801E6E54 and continuing handler 0x801E6FB0 read request-table indices 0/3/4/6 at 0x801EB494 under the reviewed mask-0x58 selector and initialized state. Exact original archive and raw-payload identities bind the consumer. Stable tables, valid work and nonaliasing memory remain conditions. Other request bytes retain their prior status. No general selector bound, actual invocation, indirect-call safety, display or safe-edit claim.";

const US_DIRECT_CALL_INSTRUCTION_EVIDENCE: &str = "Original US direct-call instruction bodies; original JAL entries from previously qualified instruction records, complete raw-word and structural control-flow review, and every control-transfer delay word including returns. Exact original whole executable or archive/raw-payload identities and load addresses gate classification. GAME 0x801AF494 uses the separately qualified five-case numeric table. SYSCALL continuations at 0x8017EE10/0x8017EE20 assume exception return to the next instruction. The five-word 0x8017F3D0 body ends in an opaque pointer-loaded tail transfer; pointer storage, target and effects are excluded. tools/rust/bof3-text/docs/formats.md records body boundaries and entry sites. Ordinary returns, valid nonaliasing memory, intact frames and stable code/tables remain conditions. Referenced data, external callees and neighbors retain prior status. No ELF match, observed invocation, all-path feasibility, transitive effects, resource safety, display or safe-edit claim.";
const US_DIRECT_CALL_TABLE_EVIDENCE: &str = "Original US direct-call numeric jump table at 0x80195F7C..0x80195F90; five little-endian targets. Complete original GAME body 0x801AF494..0x801AF5B0 masks the selector to eight bits and applies unsigned < 5 before the scaled table load and register jump. Every original target stays inside the consumer; complete guard, table, load/jump and delay words reconcile. Exact original archive/raw-payload identities and load address gate classification. Stable code/table and valid nonaliasing memory remain conditions. Neighboring bytes retain prior status. No actual dispatch, general selector bound, resource safety, display or safe-edit claim.";

const US_FOLLOWUP_INSTRUCTION_EVIDENCE: &str = "Original US further direct-call instructions; complete original GAME and shared battle bodies with original JAL entries in previously qualified instruction records. All raw words, structural control flow and control-transfer delay words reconcile. GAME 0x80196784 uses the separately qualified five-case table at 0x80195804. Exact original archive/raw-payload identities and load addresses gate classification. tools/rust/bof3-text/docs/formats.md records boundaries and entries. Ordinary returns, valid nonaliasing memory, intact frames and stable code/tables remain conditions. Referenced storage, external callees and neighboring bytes retain prior status. No ELF match, observed invocation, all-path feasibility, transitive effects, resource safety, display or safe-edit claim.";
const GAME_SLOT_TABLE_EVIDENCE: &str = "Original US GAME five-case slot jump table at 0x80195804..0x80195818; five original little-endian target words. Complete original body 0x80196784..0x801968BC masks its selector to eight bits and tests unsigned < 5 at 0x801967C8 before scaled load 0x801967EC and JR 0x801967F4. Original guard, table, branch/load/jump delay words and all internal targets reconcile. Exact original archive/raw-payload identities and load address gate classification. Stable code/table and valid nonaliasing memory remain conditions. No actual dispatch, general selector bound, referenced-storage, resource-safety or safe-edit claim.";

/// Reviewed numeric fields and instruction bodies in exact original executables.
pub fn executable_nontext_ranges<'a>(
    data: &[u8],
    textures: &'a [crate::textures::Range],
) -> Vec<NontextRange<'a>> {
    if data.len() == 122_880
        && sha256_hex(data) == "21b3f791e450ff308a874e3691d0c0c88df428464f9ad08ea2c7a680a6c0dd2c"
    {
        return vec![
            NontextRange {
                offset: 3928,
                length: 1828,
                evidence: "original US LOGO 0x801CE758..0x801CEE7C; twelve complete instruction-only function bodies; original bytes match freshly compiled executable ELF function symbols and reviewed source boundaries; exact executable identity; no runtime execution claim",
            },
            NontextRange {
                offset: 9700,
                length: 32,
                evidence: "original US LOGO 0x801CFDE4..0x801CFE04; complete CdFlush wrapper from PsyQ libcd.a s_008.o executable section with CD_flush relocation to 0x801D141C; original caller at 0x801D1E84 and reviewed call/return boundaries; exact executable identity; no runtime execution or callee classification claim",
            },
            NontextRange {
                offset: 9756,
                length: 52,
                evidence: "original US LOGO 0x801CFE1C..0x801CFE50; complete CdComstr instruction body from PsyQ libcd.a s_010.o after all five table/local-text/rdata relocations; reviewed branches join at return and delay slot; SDK text padding excluded; exact executable identity; no runtime execution, table/string classification or complete indirect-reference claim",
            },
            NontextRange {
                offset: 10908,
                length: 32,
                evidence: "original US LOGO 0x801D029C..0x801D02BC; complete PsyQ libcd.a s_020.o executable section with CD_vol relocation to 0x801D1394; original typed callers at 0x801CED74 and 0x801CEDD0; exact executable identity; no runtime execution or complete callee claim",
            },
            NontextRange {
                offset: 15252,
                length: 136,
                evidence: "original US LOGO 0x801D1394..0x801D141C; complete CD_vol instruction body from PsyQ libcd.a bios_1.o; all fourteen HI16/LO16 relocations and external pointer words match original bytes; typed CdlATV argument through original CdMix caller; reviewed straight-line body ends with return and delay slot before CD_flush; exact executable identity; no runtime execution or pointer-storage classification claim",
            },
            NontextRange {
                offset: 20_028,
                length: 2416,
                evidence: "original US LOGO complete instruction body 0x801D263C..0x801D2FAC; original JAL 0x801CECBC in the already qualified caller 0x801CECA4 conditionally enters this body when the word at 0x801EB678 is nonzero; all 604 original words, 43 conditional branches, fifteen internal jumps, ten direct calls, two indirect calls, sole JR RA and delay slots structurally covered; no disconnected data or external noncall edges; exact whole executable identity; ordinary call-return/frame/nonalias/stable-code assumptions; indirect targets and referenced fields remain unqualified, including the opaque CAPCOM header word at H+12; no matching callee ELF, observed execution, guard availability, argument/resource safety, rendering or editing claim",
            },
            NontextRange {
                offset: 22_444,
                length: 52,
                evidence: "original US LOGO complete 52-byte instruction body 0x801D2FAC..0x801D2FE0; original JAL 0x801D2878 in the qualified caller 0x801D263C conditionally enters with count eight when the word at 0x801ECEA4 is nonzero; all thirteen original words, two conditional branches, sole JR RA and three delay words structurally covered; no calls or disconnected data; exact whole executable identity; ordinary argument/memory/return/nonalias/stable-code assumptions; copied bytes and pointed storage remain unqualified, including CAPCOM H+12; no matching callee ELF, observed execution, guard availability, argument/resource safety, rendering or editing claim",
            },
            NontextRange {
                offset: 50_568,
                length: 160,
                evidence: "original US LOGO 0x801D9D88..0x801D9E28; 32 command and eight interrupt little-endian pointer words target supported literal allocations, including twelve aliases of the shared question mark; original table-loaded diagnostic printf arguments and exact executable identity; stored representation only, no arbitrary-index bound or runtime execution claim",
            },
        ];
    }
    if data.len() != 1_445_888 || sha256_hex(data) != US_EXECUTABLE_SHA256 {
        return Vec::new();
    }
    let mut ranges = vec![
        NontextRange {
            offset: 0xb74e0,
            length: 108,
            evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 0xb8cac,
            length: 64,
            evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 0xba098,
            length: 332,
            evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 0xcb454,
            length: 948,
            evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 0xd128c,
            length: 184,
            evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 0xdc12c,
            length: 208,
            evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 0xe45e4,
            length: 60,
            evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 0xe4620,
            length: 208,
            evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 0xe48c8,
            length: 36,
            evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 0xe4904,
            length: 40,
            evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 0xe4a6c,
            length: 20,
            evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 0xe62d8,
            length: 88,
            evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 0xe8e0c,
            length: 16,
            evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 0xe8e1c,
            length: 16,
            evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 0xe93d0,
            length: 20,
            evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 735_640,
            length: 20,
            evidence: "Original US sprite palette jump table; five little-endian numeric targets at 0x80149998..0x801499AC. Complete original SLUS body 0x8014CE78..0x8014CF9C bounds the selector by unsigned < 5 before scaled load 0x8014CEB8 and JR 0x8014CEC0. All five original targets remain inside that body; raw words, guard and load/jump delays reconcile under exact whole executable identity. Numeric storage only; stable code/table and valid nonaliasing memory remain conditions. No actual dispatch, general selector bound, resource safety, display or editing claim.",
        },
        NontextRange {
            offset: 738_132,
            length: 20,
            evidence: "original US SLUS; five little-endian u32 control-flow targets at 0x8014A354..0x8014A368; complete original body 0x80175C60..0x801761E8 unsigned selector-minus-one guard accepts only indices 0..4, scaled load at 0x80175EBC and JR at 0x80175EC4; all five exact original targets remain inside that body; complete original guard, load delays, table and captured resident bytes reconcile; exact whole executable identity; numeric data separate from instructions; no observed execution, table immutability, argument-validity, callee-semantics or resource-safety claim",
        },
        NontextRange {
            offset: 739_844,
            length: 8,
            evidence: "original US SLUS; complete instruction body 0x8014AA04..0x8014AA0C including return delay slot; complete original word and control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 739_852,
            length: 36,
            evidence: "original US SLUS startup entry 0x8014AA0C..0x8014AA30; original PS-X header PC and nine original instruction words establish the complete finite initialization loop, one backward branch and its NOP delay; constructs destination 0x8018B2B8 and exclusive end 0x80190C60, stores zero then advances four bytes and tests the unsigned end; exactly 5738 stores under normal loader entry, mapped RAM, stable code and noninterference assumptions; exact whole executable identity; entry-loop interval only, no complete startup-function or observed execution claim",
        },
        NontextRange {
            offset: 740_040,
            length: 472,
            evidence: "original US SLUS; complete instruction body 0x8014AAC8..0x8014ACA0 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 66 text relocations reconcile with original and captured resident bytes; compiler epilogue 0x8014AC90..0x8014ACA0 is unreachable from the original entry and receives no executable-path claim; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 740_512,
            length: 136,
            evidence: "original US SLUS; complete instruction body 0x8014ACA0..0x8014AD28 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 740_648,
            length: 224,
            evidence: "original US SLUS; complete instruction body 0x8014AD28..0x8014AE08 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 24 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 740_872,
            length: 148,
            evidence: "original US SLUS; complete instruction body 0x8014AE08..0x8014AE9C including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 741_020,
            length: 68,
            evidence: "original US SLUS; complete instruction body 0x8014AE9C..0x8014AEE0 including return delay slot; complete original word and control-flow review, sized executable ELF function section and its single text relocation reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 741_088,
            length: 224,
            evidence: "original US SLUS; complete instruction body 0x8014AEE0..0x8014AFC0 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 23 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 741_312,
            length: 96,
            evidence: "original US SLUS; complete original instruction body 0x8014AFC0..0x8014B020 including return delay slots; original JAL at 0x8014AB60 in the qualified executable caller at 0x8014AAC8 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 741_408,
            length: 208,
            evidence: "original US SLUS; complete original instruction body 0x8014B020..0x8014B0F0 including return delay slot; original JALs at 0x8014ABB0 and 0x8014AD0C in qualified executable callers conditionally enter this body; complete original word and control-flow review and captured resident bytes reconcile; the compiled callee remains partial and is not the admission basis; exact whole executable identity; neighboring functions and data excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 741_616,
            length: 140,
            evidence: "original US SLUS; complete instruction body 0x8014B0F0..0x8014B17C including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 741_756,
            length: 40,
            evidence: "original US SLUS; complete instruction body 0x8014B17C..0x8014B1A4 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 741_796,
            length: 408,
            evidence: "original US SLUS; complete instruction body 0x8014B1A4..0x8014B33C including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 48 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 742_204,
            length: 136,
            evidence: "original US SLUS; complete instruction body 0x8014B33C..0x8014B3C4 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 743_092,
            length: 136,
            evidence: "original US SLUS; complete original instruction body 0x8014B6B4..0x8014B73C including return delay slot; original JAL at 0x8014ACD0 in the qualified executable caller at 0x8014ACA0 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 743_228,
            length: 280,
            evidence: "original US SLUS; complete instruction body 0x8014B73C..0x8014B854 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 25 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 743_508,
            length: 40,
            evidence: "original US SLUS; complete instruction body 0x8014B854..0x8014B87C including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 743_548,
            length: 52,
            evidence: "original US SLUS; complete instruction body 0x8014B87C..0x8014B8B0 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 743_600,
            length: 80,
            evidence: "original US SLUS; complete instruction body 0x8014B8B0..0x8014B900 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 743_680,
            length: 84,
            evidence: "original US SLUS; complete instruction body 0x8014B900..0x8014B954 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 744_020,
            length: 100,
            evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 744_224,
            length: 336,
            evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 744_560,
            length: 2_060,
            evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 746_708,
            length: 2_420,
            evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 749_176,
            length: 292,
            evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 749_468,
            length: 708,
            evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 750_224,
            length: 592,
            evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 751_288,
            length: 36,
            evidence: "original US SLUS 0x8014D6B8..0x8014D6DC; complete nine-word instruction-only wrapper including direct-call and return delay slots; original whole executable/raw payload and retained resident RAM bytes agree; conditionally entered by original START#8 JAL at 0x801E3FB8 under the reviewed initializer route; all words and structural call/return flow reviewed; ordinary-return/frame/nonalias assumptions; exact executable identity; no callee semantics, actual START invocation, argument/resource safety, transfer, display or whole-layout claim",
        },
        NontextRange {
            offset: 751_324,
            length: 504,
            evidence: "original US SLUS 0x8014D6DC..0x8014D8D4; complete 126-word instruction-only body including all 11 control delay slots; original whole executable/raw and retained resident RAM bytes agree; conditional original entry at reviewed wrapper JAL 0x8014D6C4; all words and four conditional/two internal direct/four external direct/sole return transfers reviewed, internal CFG closed; ordinary external-return/frame/nonalias assumptions; exact executable identity; no callee semantics, data/argument/resource safety, actual START invocation, transfer, display or whole-layout claim",
        },
        NontextRange {
            offset: 751_828,
            length: 60,
            evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 751_888,
            length: 104,
            evidence: "Exact original complete 26-word instruction body 0x8014D910..0x8014D978, conditionally entered by original JAL 0x8014D8B0 with its 0x8014D8B4 delay supplying A0=S2&0xFFFF; the reviewed wrapper path saves A1=0 into S2. All original words and both control-transfer delay slots match retained shared code RAM. The sole external JAL at 0x8014D960 calls 0x8014DB98 with the work+0x54 store in its delay; the sole JR RA at 0x8014D970 has a NOP delay. The 0x18-byte frame saves/restores RA and restores SP. Stored instruction role only, with ordinary external call-return, frame and nonalias assumptions; no matching ELF, native execution, resource semantics or rendering claim.",
        },
        NontextRange {
            offset: 751_992,
            length: 172,
            evidence: "Exact original complete 43-word instruction body 0x8014D978..0x8014DA24, conditionally entered by original GAME JAL 0x801B0354 with NOP delay after the reviewed B0320 selected substate handler returns. All words and all 6 control-transfer delays reconcile with original raw and retained shared code RAM. Complete 7-block CFG has 3 conditional branches, one internal J, one direct JAL to 0x8014DB98 and sole JR RA; the 0x18-byte frame restores S0/RA/SP. Stored instruction role only, with original B0320 entry, selector0/1/2, ordinary handler/callee return, frame and nonalias assumptions; no matching ELF, native retirement, resource semantics or rendering claim.",
        },
        NontextRange {
            offset: 752_536,
            length: 112,
            evidence: "original US SLUS 0x8014DB98..0x8014DC08; complete 28-word call-free body conditionally entered by original JAL 0x8014D960 after its 0x8014D964 delay; all words and the sole JR RA delay match original raw bytes and retained shared code RAM; exact whole executable identity; stored instruction role under ordinary caller return/frame/nonalias and stable incoming work assumptions; no actual execution, matching ELF, resource/field semantics or rendering claim",
        },
        NontextRange {
            offset: 752_648,
            length: 308,
            evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
        },
        NontextRange {
            offset: 752_956,
            length: 316,
            evidence: "original US SLUS; complete instruction body 0x8014DD3C..0x8014DE78 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 25 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 753_832,
            length: 84,
            evidence: "original US SLUS; complete original instruction body 0x8014E0A8..0x8014E0FC including return delay slots; original JAL at 0x8014E154 in the qualified executable caller at 0x8014E0FC conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 753_916,
            length: 304,
            evidence: "original US SLUS; complete instruction body 0x8014E0FC..0x8014E22C including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 16 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 754_220,
            length: 88,
            evidence: "original US SLUS; complete original instruction body 0x8014E22C..0x8014E284 including return delay slots; original JAL at 0x8014AB40 in the qualified executable caller at 0x8014AAC8 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 754_676,
            length: 144,
            evidence: "original US SLUS; complete 36-word leaf instruction body 0x8014E3F4..0x8014E484; original GAME JAL 0x8019FB00 conditionally enters after clearAllRecords returns, through the entry-reachable reload call in the complete sized applySelectionContext ELF root; all original words, four control-transfer delays, closed seven-block flow and fixed slot stores match retained RAM; exact whole executable identity; ordinary incoming return control, valid nonaliasing stores and stable code remain conditions; referenced slot data, neighboring code and auxiliary area storage excluded; no matching callee ELF, actual invocation, auxiliary initialization or ownership, resource safety, rendering or editing claim",
        },
        NontextRange {
            offset: 755_044,
            length: 60,
            evidence: "original US SLUS; complete original instruction body 0x8014E564..0x8014E5A0 including return delay slots; original JAL at 0x8014ACF4 in the qualified executable caller at 0x8014ACA0 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 755_104,
            length: 160,
            evidence: "original US SLUS; complete instruction body 0x8014E5A0..0x8014E640 including all three control-transfer delay slots; complete original ASM word and structural control-flow review, sized executable ELF function section and all eleven object text relocations reconcile with original and captured resident bytes; conditional original entries at emi/battle/battle/15 JAL sites 0x800AEA1C, 0x800AEBB8 and 0x800AED3C; call continuation assumes ordinary return, preserved saved values, intact caller frame and nonaliasing dynamic stores; exact whole executable identity; referenced globals, tables, buffers, callee and neighboring bodies excluded; no observed-execution, feasible-path, argument-validity, selector-bound, callee-semantics, resource-safety, safe-editing, unique-supplier or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 755_264,
            length: 144,
            evidence: "original US SLUS; complete 36-word leaf instruction body 0x8014E640..0x8014E6D0, conditionally entered by original SHOP JAL 0x801D137C with argument mask in its delay slot; all original words, five control-transfer delays, four load delays and closed six-block flow match retained RAM; exact whole executable identity; stable code, valid nonaliasing memory and incoming return control remain conditions; referenced input fields, repeat state and neighboring code excluded; no matching ELF, actual invocation, physical-button mapping, VSync timing, resource safety, rendering or editing claim",
        },
        NontextRange {
            offset: 755_408,
            length: 296,
            evidence: "original US SLUS; complete original instruction body 0x8014E6D0..0x8014E7F8 including return delay slots; original JAL at 0x8014AB48 in the qualified executable caller at 0x8014AAC8 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 755_704,
            length: 260,
            evidence: "original US SLUS; complete original instruction body 0x8014E7F8..0x8014E8FC including return delay slots; original JAL at 0x8014E774 in the qualified executable caller at 0x8014E6D0 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 755_964,
            length: 388,
            evidence: "original US SLUS; complete original instruction body 0x8014E8FC..0x8014EA80 including return delay slots; original JAL at 0x8014E79C in the qualified executable caller at 0x8014E6D0 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 756_352,
            length: 484,
            evidence: "original US SLUS; complete 121-word instruction body 0x8014EA80..0x8014EC64 including return delay slot; qualified original boot loop installs this callback pointer at 0x8014AB08; complete raw original words, decoder, structural control flow, load delays, 24-byte frame and saved RA reconcile with captured resident bytes; native loader entry RA 0x8014EABC binds the interior JAL at 0x8014EAB4 only; call continuations assume ordinary returns, preserved caller frame and saved return storage, nonaliasing stores and stable code; exact whole executable identity; referenced globals, tables, callees and neighboring bodies excluded; no matching ELF, observed callback entry or whole-body execution, argument-validity, callee-semantics, resource-safety, rendering or safe-editing claim",
        },
        NontextRange {
            offset: 756_836,
            length: 72,
            evidence: "original US SLUS; complete 18-word instruction body 0x8014EC64..0x8014ECAC including return delay slot; qualified original initializer supplies this callback pointer to installer call 0x8014EC44; complete raw original words, decoder, structural control flow, load delay, 24-byte frame and saved RA reconcile with captured resident bytes; call continuations assume ordinary returns, preserved caller frame and saved return storage, nonaliasing stores and stable code; exact whole executable identity; referenced callees and neighboring bodies excluded; no matching ELF, observed callback entry or execution, argument-validity, callee-semantics, resource-safety, rendering or safe-editing claim",
        },
        NontextRange {
            offset: 762_576,
            length: 52,
            evidence: "original US SLUS; complete 13-word leaf instruction body 0x801502D0..0x80150304 including JR RA delay slot; complete original word, load-delay and structural-flow review reconciles with retained native entry and return; observed ID 0x40D0 selects bank 1, zero-based row 208 and cell 0x8001B834, returning 0x8001C9F7; exact whole executable identity; stable code and descriptor/table bytes, valid readable memory and incoming return control remain conditions; referenced bank data and neighboring code excluded; no matching ELF, arbitrary-ID bounds, general pointer safety, edited rendering or unique-supplier claim",
        },
        NontextRange {
            offset: 762_848,
            length: 144,
            evidence: "Original US SLUS, complete 36-word instruction body 0x801503E0..0x80150470. Original state-2 JAL 0x80159568 and its NOP delay conditionally enter this wrapper. Every original word, local branch, three calls, return and five delay slots agrees with retained RAM. The 24-byte frame and saved RA are restored on return. The exact whole executable identity gates classification. Ordinary callee returns and ABI preservation, valid memory and return control, an intact disjoint frame and stable code/data remain conditions. Referenced fields, callees and neighbors are excluded. No invocation, action-60 script supplier, termination, completion, latch release, resource safety, rendering, editing or ELF-match claim.",
        },
        NontextRange {
            offset: 763_836,
            length: 128,
            evidence: "Original US SLUS, complete 32-word instruction body 0x801507BC..0x8015083C. Original wrapper JAL 0x80150420 and its NOP delay conditionally enter this dispatcher. Every original word, copied pointer, indirect-call and return delay agrees with retained RAM; the 56-byte frame and saved RA are restored on return. Finite selector alternatives 0 and 2 bind original table targets 0x8015083C and 0x80150E48, without a general selector bound. The exact whole executable identity gates classification. Valid memory and return control, stable code/table bytes, nonaliasing, intact frame and ordinary selected-callee return remain conditions. Table bytes, selected handlers, fields and neighbors are excluded. No invocation, general target safety, callee effects, action-60 script supplier, termination, completion, latch release, rendering, editing or ELF-match claim.",
        },
        NontextRange {
            offset: 763_964,
            length: 1_496,
            evidence: "Original US SLUS, complete 374-word instruction body 0x8015083C..0x80150E14. Original dispatcher table selector 0 conditionally enters this parser. Every original word, branch, internal jump, sole direct call, return and 60 delay slots agrees with retained RAM; the 48-byte frame and S0..S5/RA are restored on return. Five stored instructions at 0x801509D0..0x801509E4 have an infeasible unsigned-byte-equals-0x100 incoming condition; their classification does not assert execution. The exact whole executable identity gates classification. Valid memory/cursor and return control, ordinary callee return and ABI preservation, intact disjoint frame and stable code/data remain conditions. Script bytes, cursors, tables, external callee and neighbors are excluded. No invocation, cursor bounds, actual action-60 commands, finite termination, completion, latch release, resource safety, rendering, editing or ELF-match claim.",
        },
        NontextRange {
            offset: 765_512,
            length: 80,
            evidence: "Original US SLUS, complete 20-word instruction body 0x80150E48..0x80150E98. Original outer-dispatcher table selector 2 conditionally enters this body. Every original word, selected indirect call, return and both delay slots agrees with retained RAM; the 32-byte frame and saved RA are restored on return. Finite subselector alternatives 0 and 1 bind the two in-body constant targets 0x80150E98 and 0x80150F08, without a general selector bound. The exact whole executable identity gates classification. Valid memory and return control, intact disjoint frame, nonaliasing, stable selector/code and ordinary selected-handler return remain conditions. Fields, handlers and neighbors are excluded. No invocation, general target safety, actual action-60 commands, completion, latch release, resource safety, rendering, editing or ELF-match claim.",
        },
        NontextRange {
            offset: 765_592,
            length: 112,
            evidence: "Original US SLUS, complete 28-word leaf instruction body 0x80150E98..0x80150F08. Original dispatcher 0x80150E48 conditionally calls this constant target for subselector 0. Every original word, branch, return and three delay slots agrees with retained RAM. This leaf preserves SP, RA and saved registers. The exact whole executable identity gates classification. Valid memory and return control, nonaliasing and stable code/data remain conditions. Fixed task-slot and interpreter fields and neighbors are excluded. No invocation, selected-task identity, actual action-60 commands, completion, latch release, resource safety, rendering, editing or ELF-match claim.",
        },
        NontextRange {
            offset: 765_704,
            length: 52,
            evidence: "Original US SLUS, complete 13-word leaf instruction body 0x80150F08..0x80150F3C. Original dispatcher 0x80150E48 conditionally calls this constant target for subselector 1. Every original word, branch, return and two delay slots agrees with retained RAM. This leaf preserves SP, RA and saved registers. The exact whole executable identity gates classification. Valid memory and return control, nonaliasing and stable code/data remain conditions. Fixed task-slot and interpreter fields and neighbors are excluded. No invocation, selected-task identity, actual action-60 commands, consistent boolean result, completion, latch release, resource safety, rendering, editing or ELF-match claim.",
        },
        NontextRange {
            offset: 780_988,
            length: 1_044,
            evidence: "original US SLUS complete 1044-byte instruction body 0x80154ABC..0x80154ED0; original direct JAL 0x80154FA4 from the wrapper bound to the retained GAME 0x801BB774 return witness; all 261 aligned words, ten conditional branches, seven internal jumps, sole JR RA and every delay slot form a closed whole-entry structural CFG; call-free 24-byte frame restored before JR RA 0x80154EC8 and NOP; original and retained origin/endpoint RAM agree; ordinary return, writable disjoint frame, nonalias and stable-code assumptions; no matching ELF, all-path native execution, argument/resource safety, full coverage, rendering or editing claim",
        },
        NontextRange {
            offset: 782_032,
            length: 88,
            evidence: "original US SLUS complete 88-byte instruction body 0x80154ED0..0x80154F28; original direct JAL 0x80154F94 from the wrapper bound to the retained GAME 0x801BB774 return witness; all 22 aligned words, two conditional branches, one internal jump, sole JR RA and every delay slot form a closed whole-entry structural CFG; call-free and frame-free with unchanged RA, JR RA 0x80154F20 and NOP; original and retained origin/endpoint RAM agree; ordinary return, nonalias and stable-code assumptions; no matching ELF, all-path native execution, pointer/resource safety, full coverage, rendering or editing claim",
        },
        NontextRange {
            offset: 782_120,
            length: 80,
            evidence: "original US SLUS complete 80-byte instruction body 0x80154F28..0x80154F78; original GAME 0x801C26A0 JAL in the qualified native-observed caller 0x801C2538 conditionally enters this wrapper after ordinary return from 0x80154F78 and a nonzero scratch-byte test; all 20 original words, two direct calls, sole JR RA and delay slots structurally covered; 32-byte frame and S0/S1/RA restored; original and retained before/after RAM agree; exact whole executable identity; ordinary call-return/frame/nonalias/stable-code assumptions; no matching ELF, observed wrapper execution, state-0 invocation, argument/resource safety, sampler semantics, rendering or editing claim",
        },
        NontextRange {
            offset: 782_200,
            length: 96,
            evidence: "original US SLUS complete 96-byte instruction body 0x80154F78..0x80154FD8; actual original GAME 0x801BB774 JAL and NOP bound by retained preinstruction return 0x801BB77C and downstream predicate observations; all 24 aligned words, two direct calls at 0x80154F94 to 0x80154ED0 and 0x80154FA4 to 0x80154ABC, sole JR RA and every delay slot structurally accounted; 32-byte frame and saved S0/S1/S2/RA restored before JR RA 0x80154FD0 and NOP; original and retained origin/endpoint RAM agree; ordinary call-return/frame/nonalias/stable-code assumptions; no matching ELF, whole-body native execution, argument/resource safety, full coverage, rendering or editing claim",
        },
        NontextRange {
            offset: 797_824,
            length: 312,
            evidence: "Original US SLUS, complete 78-word instruction body 0x80158C80..0x80158DB8. Original GAME call 0x801993D0 conditionally enters it. All original words, local control flow, delay slots and restored frame agree with retained RAM. An active slot with byte 1 equal to 7 selects 0x801E27BC through the original nine-pointer table copied from 0x80149B0C. The exact whole executable identity gates classification. Stable code, tables and selectors, valid callback targets and nonaliasing memory, intact frames and ordinary prior/callee returns remain conditions. Supporting table cells and referenced data are excluded. Selector producers, general bounds, indirect-call effects, historical execution, resource safety, rendering and editing remain unqualified.",
        },
        NontextRange {
            offset: 814_780,
            length: 392,
            evidence: "original US SLUS; complete original instruction body 0x8015CEBC..0x8015D044 including return delay slots; original JAL at 0x8014AC00 in the qualified executable caller at 0x8014AAC8 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 815_172,
            length: 116,
            evidence: "original US SLUS; complete original instruction body 0x8015D044..0x8015D0B8 including return delay slots; original JAL at 0x8014AB68 in the qualified executable caller at 0x8014AAC8 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 818_968,
            length: 2_684,
            evidence: "original US SLUS; complete instruction body 0x8015DF18..0x8015E994 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 408 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 835_416,
            length: 132,
            evidence: "original US SLUS; complete instruction body 0x80161F58..0x80161FDC including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 12 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 835_548,
            length: 388,
            evidence: "original US SLUS; complete instruction body 0x80161FDC..0x80162160 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 56 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 835_936,
            length: 24,
            evidence: "original US SLUS; complete instruction body 0x80162160..0x80162178 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 835_960,
            length: 112,
            evidence: "original US SLUS; complete instruction body 0x80162178..0x801621E8 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 836_072,
            length: 72,
            evidence: "original US SLUS; complete instruction body 0x801621E8..0x80162230 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 836_144,
            length: 720,
            evidence: "original US SLUS; complete instruction body 0x80162230..0x80162500 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 70 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 836_864,
            length: 228,
            evidence: "original US SLUS; complete instruction body 0x80162500..0x801625E4 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 17 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 837_092,
            length: 52,
            evidence: "original US SLUS; complete instruction body 0x801625E4..0x80162618 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 837_144,
            length: 128,
            evidence: "original US SLUS; complete instruction body 0x80162618..0x80162698 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 837_272,
            length: 248,
            evidence: "original US SLUS; complete instruction body 0x80162698..0x80162790 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 32 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 837_784,
            length: 344,
            evidence: "original US SLUS; complete instruction body 0x80162898..0x801629F0 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 42 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 838_128,
            length: 124,
            evidence: "original US SLUS; complete instruction body 0x801629F0..0x80162A6C including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 838_252,
            length: 156,
            evidence: "original US SLUS; complete instruction body 0x80162A6C..0x80162B08 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 13 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 838_408,
            length: 268,
            evidence: "original US SLUS; complete instruction body 0x80162B08..0x80162C14 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 33 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 838_676,
            length: 196,
            evidence: "original US SLUS; complete instruction body 0x80162C14..0x80162CD8 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 20 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 838_872,
            length: 40,
            evidence: "original US SLUS; complete instruction body 0x80162CD8..0x80162D00 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 838_912,
            length: 24,
            evidence: "original US SLUS; complete instruction body 0x80162D00..0x80162D18 including return delay slot; complete original word and control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 838_936,
            length: 132,
            evidence: "original US SLUS; complete instruction body 0x80162D18..0x80162D9C including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 839_696,
            length: 76,
            evidence: "original US SLUS; complete original instruction body 0x80163010..0x8016305C including return delay slots; original JAL at 0x8014AC48 in the qualified executable caller at 0x8014AAC8 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 839_772,
            length: 468,
            evidence: "original US SLUS; complete original instruction body 0x8016305C..0x80163230 including return delay slots; original JAL at 0x80163028 in the qualified executable caller at 0x80163010 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 841_904,
            length: 264,
            evidence: "original US SLUS; complete original instruction body 0x801638B0..0x801639B8 including return delay slots; original JAL at 0x80163044 in the qualified executable caller at 0x80163010 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 843_400,
            length: 24,
            evidence: "original US SLUS; complete original instruction body 0x80163E88..0x80163EA0 including terminal delay slots; original JAL at 0x80163974 in the qualified executable caller at 0x801638B0 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 849_140,
            length: 124,
            evidence: "original US SLUS; complete instruction body 0x801654F4..0x80165570 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 849_396,
            length: 160,
            evidence: "original US SLUS; complete instruction body 0x801655F4..0x80165694 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 851_272,
            length: 188,
            evidence: "original US SLUS; complete instruction body 0x80165D48..0x80165E04 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 16 text relocations reconcile with original and captured resident bytes; five bounded internal switch targets, with the separate address table excluded; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 855_216,
            length: 60,
            evidence: "original US SLUS; complete instruction body 0x80166CB0..0x80166CEC including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 860_896,
            length: 148,
            evidence: "original US SLUS; complete original instruction body 0x801682E0..0x80168374 including return delay slots; original JAL at 0x8015D070 in the qualified executable caller at 0x8015D044 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 862_560,
            length: 656,
            evidence: "original US SLUS; complete original instruction body 0x80168960..0x80168BF0 including terminal delay slots; original JAL at 0x8016F47C in the qualified executable caller at 0x8016F378 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 863_456,
            length: 72,
            evidence: "original US SLUS; complete original instruction body 0x80168CE0..0x80168D28 including terminal delay slots; original JAL at 0x8016ED38 in the qualified executable caller at 0x8016E8A4 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 863_848,
            length: 44,
            evidence: "original US SLUS; complete original instruction body 0x80168E68..0x80168E94 including terminal delay slots; original JAL at 0x80168B48 in the qualified executable caller at 0x80168960 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 863_892,
            length: 44,
            evidence: "original US SLUS; complete original instruction body 0x80168E94..0x80168EC0 including terminal delay slots; original JAL at 0x80168B38 in the qualified executable caller at 0x80168960 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 864_404,
            length: 36,
            evidence: "original US SLUS; complete original instruction body 0x80169094..0x801690B8 including terminal delay slots; original JAL at 0x8016BA60 in the qualified executable caller at 0x8016BA24 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 864_440,
            length: 52,
            evidence: "original US SLUS; complete original instruction body 0x801690B8..0x801690EC including return delay slots; original JAL at 0x801628BC in the qualified executable caller at 0x80162898 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 871_308,
            length: 380,
            evidence: "original US SLUS; complete original instruction body 0x8016AB8C..0x8016AD08 including terminal delay slots; original JAL at 0x8016AD38 in the qualified executable caller at 0x8016AD2C conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 871_724,
            length: 36,
            evidence: "original US SLUS; complete original instruction body 0x8016AD2C..0x8016AD50 including return delay slots; original JAL at 0x8015CF54 in the qualified executable caller at 0x8015CEBC conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 872_704,
            length: 196,
            evidence: "original US SLUS; complete original instruction body 0x8016B100..0x8016B1C4 including return delay slots; original JAL at 0x8015D020 in the qualified executable caller at 0x8015CEBC conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 875_012,
            length: 32,
            evidence: "original US SLUS; complete original instruction body 0x8016BA04..0x8016BA24 including return delay slots; original JAL at 0x8015D028 in the qualified executable caller at 0x8015CEBC conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 875_044,
            length: 124,
            evidence: "original US SLUS; complete original instruction body 0x8016BA24..0x8016BAA0 including terminal delay slots; original JAL at 0x8016BA0C in the qualified executable caller at 0x8016BA04 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 884_412,
            length: 312,
            evidence: "original US SLUS; complete original instruction body 0x8016DEBC..0x8016DFF4 including return delay slots; original JAL at 0x8015CEC8 in the qualified executable caller at 0x8015CEBC conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 886_796,
            length: 152,
            evidence: "original US SLUS; complete original instruction body 0x8016E80C..0x8016E8A4 including return delay slots; original JAL at 0x8015D018 in the qualified executable caller at 0x8015CEBC conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 886_948,
            length: 1_272,
            evidence: "original US SLUS; complete original instruction body 0x8016E8A4..0x8016ED9C including terminal delay slots; original JAL at 0x8016E88C in the qualified executable caller at 0x8016E80C conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 888_348,
            length: 140,
            evidence: "original US SLUS; complete original instruction body 0x8016EE1C..0x8016EEA8 including terminal delay slots; original JAL at 0x8016E944 in the qualified executable caller at 0x8016E8A4 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 888_488,
            length: 1_232,
            evidence: "original US SLUS; complete original instruction body 0x8016EEA8..0x8016F378 including terminal delay slots; original JAL at 0x8016ED00 in the qualified executable caller at 0x8016E8A4 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 889_720,
            length: 412,
            evidence: "original US SLUS; complete original instruction body 0x8016F378..0x8016F514 including terminal delay slots; original JAL at 0x8016ED20 in the qualified executable caller at 0x8016E8A4 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 890_132,
            length: 164,
            evidence: "original US SLUS; complete original instruction body 0x8016F514..0x8016F5B8 including return delay slots; original JAL at 0x8015D00C in the qualified executable caller at 0x8015CEBC conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 890_312,
            length: 208,
            evidence: "original US SLUS; complete original instruction body 0x8016F5C8..0x8016F698 including terminal delay slots; original JAL at 0x8016F580 in the qualified executable caller at 0x8016F514 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 890_520,
            length: 32,
            evidence: "original US SLUS; complete original instruction body 0x8016F698..0x8016F6B8 including return delay slots; original JAL at 0x8015D004 in the qualified executable caller at 0x8015CEBC conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 891_128,
            length: 108,
            evidence: "original US SLUS; complete original instruction body 0x8016F8F8..0x8016F964 including return delay slots; original JAL at 0x8015E22C in the qualified executable caller at 0x8015DF18 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 897_000,
            length: 44,
            evidence: "original US SLUS; complete original instruction body 0x80170FE8..0x80171014 including terminal delay slots; original JAL at 0x80173CFC in the qualified executable caller at 0x80173CB0 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 897_044,
            length: 24,
            evidence: "original US SLUS; complete original instruction body 0x80171014..0x8017102C including terminal delay slots; original JAL at 0x80173CE0 in the qualified executable caller at 0x80173CB0 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 903_632,
            length: 1_344,
            evidence: "original US SLUS; complete original instruction body 0x801729D0..0x80172F10 including terminal delay slots; original JAL at 0x8016ABAC in the qualified executable caller at 0x8016AB8C conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 905_244,
            length: 156,
            evidence: "original US SLUS; complete instruction body 0x8017301C..0x801730B8 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 907_288,
            length: 132,
            evidence: "original US SLUS; complete original instruction body 0x80173818..0x8017389C including return delay slots; original JAL at 0x801628E8 in the qualified executable caller at 0x80162898 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 907_420,
            length: 124,
            evidence: "original US SLUS; complete original instruction body 0x8017389C..0x80173918 including return delay slots; original JAL at 0x8017385C in the qualified executable caller at 0x80173818 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 907_544,
            length: 776,
            evidence: "original US SLUS; complete original instruction body 0x80173918..0x80173C20 including terminal delay slots; original JAL at 0x80173900 in the qualified executable caller at 0x8017389C conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 908_368,
            length: 48,
            evidence: "original US SLUS; complete original instruction body 0x80173C50..0x80173C80 including return delay slots; original JAL at 0x8016292C in the qualified executable caller at 0x80162898 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 908_464,
            length: 988,
            evidence: "original US SLUS; complete original instruction body 0x80173CB0..0x8017408C including return delay slots; original JAL at 0x80173C64 in the qualified executable caller at 0x80173C50 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 909_452,
            length: 712,
            evidence: "original US SLUS; complete original instruction body 0x8017408C..0x80174354 including terminal delay slots; original JAL at 0x80173F38 in the qualified executable caller at 0x80173CB0 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 910_952,
            length: 80,
            evidence: "original US SLUS; complete original instruction body 0x80174668..0x801746B8 including return delay slots; original JAL at 0x8014ADE0 in the qualified executable caller at 0x8014AD28 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 911_032,
            length: 40,
            evidence: "original US SLUS; complete original instruction body 0x801746B8..0x801746E0 including return delay slots; original JAL at 0x8014AFC8 in the qualified executable caller at 0x8014AFC0 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 911_072,
            length: 32,
            evidence: "original US SLUS; complete original instruction body 0x801746E0..0x80174700 including return delay slot; original JAL at 0x8014AF1C in the qualified executable caller at 0x8014AEE0 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 911_104,
            length: 328,
            evidence: "original US SLUS; complete original instruction body 0x80174700..0x80174848 including return delay slot; original JAL at 0x8014AB10 in the qualified executable caller at 0x8014AAC8 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 911_432,
            length: 156,
            evidence: "original US SLUS; complete original instruction body 0x80174848..0x801748E4 including return delay slots; original JAL at 0x80174790 in the qualified executable caller at 0x80174700 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 911_588,
            length: 48,
            evidence: "original US SLUS; complete original instruction body 0x801748E4..0x80174914 including return delay slots; original JAL at 0x8014AD30 in the qualified executable caller at 0x8014AD28 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 911_636,
            length: 48,
            evidence: "original US SLUS; complete original instruction body 0x80174914..0x80174944 including terminal delay slots; original JAL at 0x8016B180 in the qualified executable caller at 0x8016B100 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 911_684,
            length: 48,
            evidence: "original US SLUS; complete original instruction body 0x80174944..0x80174974 including terminal delay slots; original JAL at 0x801690A0 in the qualified executable caller at 0x80169094 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 911_732,
            length: 52,
            evidence: "original US SLUS; complete original instruction body 0x80174974..0x801749A8 including terminal delay slots; original JAL at 0x8016B14C in the qualified executable caller at 0x8016B100 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 911_832,
            length: 48,
            evidence: "original US SLUS; complete original instruction body 0x801749D8..0x80174A08 including return delay slot; original JAL at 0x8014AF14 in the qualified executable caller at 0x8014AEE0 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; indirect call targets and selectors remain unqualified; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 911_928,
            length: 16,
            evidence: "original US SLUS; complete original instruction body 0x80174A38..0x80174A48 including terminal delay slots; original JAL at 0x8017631C in the qualified executable caller at 0x801761E8 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 911_968,
            length: 28,
            evidence: "original US SLUS; complete original instruction body 0x80174A60..0x80174A7C including terminal delay slots; original JAL at 0x8017DA64 in the qualified executable caller at 0x8017DA54 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 914_372,
            length: 24,
            evidence: "original US SLUS; complete original instruction body 0x801753C4..0x801753DC including return delay slots; original JAL at 0x8014AD64 in the qualified executable caller at 0x8014AD28 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 914_396,
            length: 16,
            evidence: "original US SLUS; complete original instruction body 0x801753DC..0x801753EC including return delay slots; original JAL at 0x8017A540 in the qualified executable caller at 0x8017A514 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 914_412,
            length: 144,
            evidence: "original US SLUS; complete original instruction body 0x801753EC..0x8017547C including return delay slot; original JAL at 0x80161F60 in the qualified executable caller at 0x80161F58 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 914_676,
            length: 16,
            evidence: "original US SLUS; complete original instruction body 0x801754F4..0x80175504 including terminal delay slots; original JAL at 0x801781E0 in the qualified executable caller at 0x80178138 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 914_692,
            length: 16,
            evidence: "original US SLUS; complete original instruction body 0x80175504..0x80175514 including terminal delay slots; original JAL at 0x8017801C in the qualified executable caller at 0x80177F1C conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 914_724,
            length: 16,
            evidence: "original US SLUS; complete original instruction body 0x80175524..0x80175534 including terminal delay slots; original JAL at 0x80177FD0 in the qualified executable caller at 0x80177F1C conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 914_740,
            length: 108,
            evidence: "original US SLUS; complete original instruction body 0x80175534..0x801755A0 including return delay slots; original JAL at 0x801753FC in the qualified executable caller at 0x801753EC conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 914_848,
            length: 32,
            evidence: "original US SLUS; complete original instruction body 0x801755A0..0x801755C0 including terminal delay slots; original JAL at 0x80178000 in the qualified executable caller at 0x80177F1C conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 915_008,
            length: 32,
            evidence: "original US SLUS; complete original instruction body 0x80175640..0x80175660 including return delay slots; original JAL at 0x8016206C in the qualified executable caller at 0x80161FDC conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 915_040,
            length: 32,
            evidence: "original US SLUS; complete original instruction body 0x80175660..0x80175680 including return delay slots; original JAL at 0x80162258 in the qualified executable caller at 0x80162230 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 915_072,
            length: 24,
            evidence: "original US SLUS; complete original instruction body 0x80175680..0x80175698 including return delay slots; original JAL at 0x8016213C in the qualified executable caller at 0x80161FDC conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 915_096,
            length: 24,
            evidence: "original US SLUS; complete original instruction body 0x80175698..0x801756B0 including return delay slots; original JAL at 0x8016212C in the qualified executable caller at 0x80161FDC conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 915_120,
            length: 312,
            evidence: "original US SLUS; complete original instruction body 0x801756B0..0x801757E8 including return delay slots; original JAL at 0x8014E140 in the qualified executable caller at 0x8014E0FC conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 915_432,
            length: 300,
            evidence: "original US SLUS; complete original instruction body 0x801757E8..0x80175914 including terminal delay slots; original JAL at 0x80177F7C in the qualified executable caller at 0x80177F1C conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 915_732,
            length: 324,
            evidence: "original US SLUS; complete original instruction body 0x80175914..0x80175A58 including return delay slots; original JAL at 0x8014E1D0 in the qualified executable caller at 0x8014E0FC conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 916_088,
            length: 32,
            evidence: "original US SLUS; complete original instruction body 0x80175A78..0x80175A98 including return delay slots; original JAL at 0x80162274 in the qualified executable caller at 0x80162230 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 916_188,
            length: 260,
            evidence: "original US SLUS; complete original instruction body 0x80175ADC..0x80175BE0 including return delay slot; original JAL at 0x8014E1C0 in the qualified executable caller at 0x8014E0FC conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 916_448,
            length: 128,
            evidence: "original US SLUS; complete original instruction body 0x80175BE0..0x80175C60 including return delay slots; original JAL at 0x8016227C in the qualified executable caller at 0x80162230 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 916_576,
            length: 1_416,
            evidence: "original US SLUS; complete original instruction body 0x80175C60..0x801761E8 including return delay slot; original JAL at 0x80176344 in qualified executable caller 0x801761E8 conditionally enters this body; complete original word, decoder, control-flow and 48-byte save/restore frame review; unsigned selector-minus-one guard closes the five original u32 targets for JR at 0x80175EC4; all branch targets and delays and captured resident bytes reconcile; structural call continuations assume ordinary returns with caller stack and saved return storage preserved and dynamic stores nonaliasing that storage; exact whole executable identity; separate jump table, neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 917_992,
            length: 640,
            evidence: "original US SLUS; complete original instruction body 0x801761E8..0x80176468 including return delay slots; original JAL at 0x80175648 in the qualified executable caller at 0x80175640 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 918_632,
            length: 716,
            evidence: "original US SLUS; complete original instruction body 0x80176468..0x80176734 including return delay slots; original JAL at 0x80175668 in the qualified executable caller at 0x80175660 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 919_348,
            length: 1_052,
            evidence: "original US SLUS; complete original instruction body 0x80176734..0x80176B50 including return delay slots; original JAL at 0x8017573C in the qualified executable caller at 0x801756B0 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 920_536,
            length: 224,
            evidence: "original US SLUS; complete original instruction body 0x80176BD8..0x80176CB8 including terminal delay slots; original JAL at 0x80176300 in the qualified executable caller at 0x801761E8 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 920_760,
            length: 244,
            evidence: "original US SLUS; complete original instruction body 0x80176CB8..0x80176DAC including return delay slots; original JAL at 0x80175578 in the qualified executable caller at 0x80175534 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 921_004,
            length: 80,
            evidence: "original US SLUS; complete original instruction body 0x80176DAC..0x80176DFC including return delay slots; original JAL at 0x8017554C in the qualified executable caller at 0x80175534 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 921_084,
            length: 492,
            evidence: "original US SLUS; complete original instruction body 0x80176DFC..0x80176FE8 including return delay slots; original JAL at 0x8017555C in the qualified executable caller at 0x80175534 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 921_940,
            length: 256,
            evidence: "original US SLUS; complete original instruction body 0x80177154..0x80177254 including return delay slots; original JAL at 0x80175A80 in the qualified executable caller at 0x80175A78 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 922_440,
            length: 716,
            evidence: "original US SLUS; complete original instruction body 0x80177348..0x80177614 including return delay slots; original JAL at 0x8014E12C in the qualified executable caller at 0x8014E0FC conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 923_156,
            length: 32,
            evidence: "original US SLUS; complete original instruction body 0x80177614..0x80177634 including return delay slots; original JAL at 0x80177540 in the qualified executable caller at 0x80177348 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 923_188,
            length: 736,
            evidence: "original US SLUS; complete original instruction body 0x80177634..0x80177914 including return delay slots; original JAL at 0x80177380 in the qualified executable caller at 0x80177348 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 923_924,
            length: 168,
            evidence: "original US SLUS; complete original instruction body 0x80177914..0x801779BC including return delay slots; original JAL at 0x80177418 in the qualified executable caller at 0x80177348 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 924_092,
            length: 684,
            evidence: "original US SLUS; complete original instruction body 0x801779BC..0x80177C68 including return delay slots; original JAL at 0x801774AC in the qualified executable caller at 0x80177348 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 924_776,
            length: 108,
            evidence: "original US SLUS; complete original instruction body 0x80177C68..0x80177CD4 including terminal delay slots; original JAL at 0x80177664 in the qualified executable caller at 0x80177634 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 924_884,
            length: 52,
            evidence: "original US SLUS; complete original instruction body 0x80177CD4..0x80177D08 including terminal delay slots; original JAL at 0x801777F8 in the qualified executable caller at 0x80177634 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 924_936,
            length: 532,
            evidence: "Original US SDK read callback instructions at 0x80177D08..0x80177F1C; exact whole executable identity. Qualified installer 0x80177F1C loads this address at 0x80178068/0x8017806C and calls setter 0x80175698 at 0x80178078. The setter stores it at 0x80185784; qualified dispatcher 0x80176468 loads that slot at 0x801765EC and conditionally calls it at 0x80176608. Complete original registration, setter, dispatch, 133 body words and all control-transfer delay words reconcile. Pointer continuity, ordinary helper returns, valid nonaliasing memory, intact frames and stable code remain conditions. Completion callback loaded from 0x80185A94, referenced data and neighbors retain prior status. No ELF match, observed invocation, callback lifetime, all-path feasibility, transfer safety, XA payload role, display or safe-edit claim.",
        },
        NontextRange {
            offset: 925_468,
            length: 460,
            evidence: "original US SLUS; complete original instruction body 0x80177F1C..0x801780E8 including terminal delay slots; original JAL at 0x80178200 in the qualified executable caller at 0x80178138 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 926_008,
            length: 224,
            evidence: "original US SLUS; complete original instruction body 0x80178138..0x80178218 including return delay slots; original JAL at 0x8014E0C0 in the qualified executable caller at 0x8014E0A8 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 926_232,
            length: 204,
            evidence: "original US SLUS; complete original instruction body 0x80178218..0x801782E4 including return delay slots; original JAL at 0x8014E0CC in the qualified executable caller at 0x8014E0A8 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 926_436,
            length: 24,
            evidence: "original US SLUS; complete original instruction body 0x801782E4..0x801782FC including return delay slots; original JAL at 0x80175438 in the qualified executable caller at 0x801753EC conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 935_188,
            length: 208,
            evidence: "original US SLUS; complete original instruction body 0x8017A514..0x8017A5E4 including return delay slot; original JAL at 0x8014AE34 in the qualified executable caller at 0x8014AE08 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 936_076,
            length: 60,
            evidence: "original US SLUS; complete original instruction body 0x8017A88C..0x8017A8C8 including return delay slot; original JAL at 0x8014B144 in the qualified executable caller at 0x8014B0F0 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 937_740,
            length: 388,
            evidence: "original US SLUS; complete original instruction body 0x8017AF0C..0x8017B090 including return delay slots; original JAL at 0x8014AD5C in the qualified executable caller at 0x8014AD28 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 938_676,
            length: 16,
            evidence: "original US SLUS; complete original instruction body 0x8017B2B4..0x8017B2C4 including return delay slots; original JAL at 0x8017A594 in the qualified executable caller at 0x8017A514 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 938_708,
            length: 92,
            evidence: "original US SLUS; complete original instruction body 0x8017B2D4..0x8017B330 including return delay slots; original JAL at 0x8014AAF8 in the qualified executable caller at 0x8014AAC8 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 938_800,
            length: 156,
            evidence: "original US SLUS; complete original instruction body 0x8017B330..0x8017B3CC including return delay slot; original JAL at 0x8014AFA4 in the qualified executable caller at 0x8014AEE0 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; indirect call targets and selectors remain unqualified; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 938_956,
            length: 108,
            evidence: "original US SLUS; complete original instruction body 0x8017B3CC..0x8017B438 including return delay slot; original JAL at 0x8014AC60 in the qualified executable caller at 0x8014AAC8 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; indirect call targets and selectors remain unqualified; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 939_064,
            length: 296,
            evidence: "original US SLUS; complete original instruction body 0x8017B438..0x8017B560 including terminal delay slots; original JAL at 0x8017B590 in the qualified executable caller at 0x8017B560 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 939_360,
            length: 148,
            evidence: "original US SLUS; complete original instruction body 0x8017B560..0x8017B5F4 including return delay slots; original JAL at 0x8014E588 in the qualified executable caller at 0x8014E564 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 939_664,
            length: 100,
            evidence: "original US SLUS; complete original instruction body 0x8017B690..0x8017B6F4 including return delay slots; original JAL at 0x8014E264 in the qualified executable caller at 0x8014E22C conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 940_060,
            length: 184,
            evidence: "original US SLUS; complete original instruction body 0x8017B81C..0x8017B8D4 including return delay slot; original JAL at 0x8014AF60 in the qualified executable caller at 0x8014AEE0 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; indirect call targets and selectors remain unqualified; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 940_244,
            length: 152,
            evidence: "original US SLUS; complete original instruction body 0x8017B8D4..0x8017B96C including return delay slot; original JAL at 0x8014ABA8 in the qualified executable caller at 0x8014AAC8 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; indirect call targets and selectors remain unqualified; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 940_492,
            length: 116,
            evidence: "original US SLUS; complete original instruction body 0x8017B9CC..0x8017BA40 including return delay slot; original JAL at 0x8014AB58 in the qualified executable caller at 0x8014AAC8 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; indirect call targets and selectors remain unqualified; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 940_608,
            length: 260,
            evidence: "original US SLUS; complete original instruction body 0x8017BA40..0x8017BB44 including return delay slots; original JAL at 0x8014AB38 in the qualified executable caller at 0x8014AAC8 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 941_208,
            length: 1_192,
            evidence: "original US SLUS; complete original instruction body 0x8017BC98..0x8017C140 including return delay slots; original JAL at 0x8014AB28 in the qualified executable caller at 0x8014AAC8 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 943_432,
            length: 656,
            evidence: "original US SLUS; complete original instruction body 0x8017C548..0x8017C7D8 including return delay slots; original JAL at 0x8017BA94 in the qualified executable caller at 0x8017BA40 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 944_088,
            length: 88,
            evidence: "original US SLUS; complete original instruction body 0x8017C7D8..0x8017C830 including terminal delay slots; original JAL at 0x8017C5CC in the qualified executable caller at 0x8017C548 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 944_176,
            length: 204,
            evidence: "original US SLUS; complete original instruction body 0x8017C830..0x8017C8FC including terminal delay slots; original JAL at 0x8017C568 in the qualified executable caller at 0x8017C548 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 944_380,
            length: 204,
            evidence: "original US SLUS; complete original instruction body 0x8017C8FC..0x8017C9C8 including terminal delay slots; original JAL at 0x8017C5A0 in the qualified executable caller at 0x8017C548 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 944_584,
            length: 68,
            evidence: "original US SLUS; complete original instruction body 0x8017C9C8..0x8017CA0C including terminal delay slots; original JAL at 0x8017C5B4 in the qualified executable caller at 0x8017C548 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 944_652,
            length: 132,
            evidence: "original US SLUS; complete original instruction body 0x8017CA0C..0x8017CA90 including terminal delay slots; original JAL at 0x8017C5D8 in the qualified executable caller at 0x8017C548 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 944_784,
            length: 176,
            evidence: "original US SLUS; complete original instruction body 0x8017CA90..0x8017CB40 including return delay slots; original JAL at 0x8017BD00 in the qualified executable caller at 0x8017BC98 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 948_820,
            length: 348,
            evidence: "original US SLUS; complete original instruction body 0x8017DA54..0x8017DBB0 including terminal delay slots; original JAL at 0x8017AFB4 in the qualified executable caller at 0x8017AF0C conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 949_912,
            length: 220,
            evidence: "original US SLUS; complete original instruction body 0x8017DE98..0x8017DF74 including terminal delay slots; original JAL at 0x8017DB94 in the qualified executable caller at 0x8017DA54 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 950_132,
            length: 44,
            evidence: "original US SLUS; complete original instruction body 0x8017DF74..0x8017DFA0 including return delay slots; original JAL at 0x8017B384 in the qualified executable caller at 0x8017B330 conditionally enters this body; complete raw original word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume returns with the caller stack preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 950_176,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017DFA0..0x8017DFAC including terminal delay slots; original JAL at 0x8014B31C in the qualified executable caller at 0x8014B1A4 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 950_312,
            length: 84,
            evidence: "original US SLUS; complete original instruction body 0x8017E028..0x8017E07C including terminal delay slots; original JAL at 0x8014B30C in the qualified executable caller at 0x8014B1A4 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 950_396,
            length: 56,
            evidence: "original US SLUS; complete original instruction body 0x8017E07C..0x8017E0B4 including terminal delay slots; original JAL at 0x8014B314 in the qualified executable caller at 0x8014B1A4 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 950_452,
            length: 48,
            evidence: "original US SLUS; complete original instruction body 0x8017E0B4..0x8017E0E4 including return delay slot; original JAL at 0x8014AF24 in the qualified executable caller at 0x8014AEE0 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 950_500,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017E0E4..0x8017E0F0 including terminal delay slots; original JAL at 0x8017E048 in the qualified executable caller at 0x8017E028 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 950_516,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017E0F4..0x8017E100 including terminal delay slots; original JAL at 0x8017E08C in the qualified executable caller at 0x8017E07C conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 950_532,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017E104..0x8017E110 including terminal delay slot; original JAL at 0x8017E0BC in the qualified executable caller at 0x8017E0B4 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; original in-body constants bind the external terminal jump; BIOS behavior and returns remain unqualified; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 950_636,
            length: 136,
            evidence: "original US SLUS; complete original instruction body 0x8017E16C..0x8017E1F4 including terminal delay slots; original JAL at 0x8017E050 in the qualified executable caller at 0x8017E028 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns; original incoming return address is saved and restored through absolute word 0x8018B2CC; conditional return identity assumes this cell is preserved by calls and dynamic stores, no aliasing write and no reentrancy; original SP is unchanged; the cell is not classified; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 950_820,
            length: 108,
            evidence: "original US SLUS; complete original instruction body 0x8017E224..0x8017E290 including return delay slot; original JAL at 0x8017E0C4 in the qualified executable caller at 0x8017E0B4 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; indirect call targets and selectors remain unqualified; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 950_964,
            length: 104,
            evidence: "original US SLUS; complete original instruction body 0x8017E2B4..0x8017E31C including return delay slot; original JAL at 0x8017E0CC in the qualified executable caller at 0x8017E0B4 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; indirect call targets and selectors remain unqualified; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 951_100,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017E33C..0x8017E348 including terminal delay slots; original JAL at 0x80174898 in the qualified executable caller at 0x80174848 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 951_156,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017E374..0x8017E380 including terminal delay slots; original JAL at 0x80177968 in the qualified executable caller at 0x80177914 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 951_172,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017E384..0x8017E390 including terminal delay slots; original JAL at 0x8017761C in the qualified executable caller at 0x80177614 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 951_236,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017E3C4..0x8017E3D0 including terminal delay slots; original JAL at 0x8017C114 in the qualified executable caller at 0x8017BC98 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 951_252,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017E3D4..0x8017E3E0 including terminal delay slots; original JAL at 0x8014AB18 in the qualified executable caller at 0x8014AAC8 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 951_268,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017E3E4..0x8017E3F0 including terminal delay slots; original JAL at 0x8017545C in the qualified executable caller at 0x801753EC conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 953_564,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017ECDC..0x8017ECE8 including terminal delay slot; original JAL at 0x8014AF3C in the qualified executable caller at 0x8014AEE0 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; original in-body constants bind the external terminal jump; BIOS behavior and returns remain unqualified; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 953_580,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017ECEC..0x8017ECF8 including terminal delay slot; original JAL at 0x8017E274 in the qualified executable caller at 0x8017E224 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; original in-body constants bind the external terminal jump; BIOS behavior and returns remain unqualified; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 953_596,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017ECFC..0x8017ED08 including terminal delay slots; original JAL at 0x8017AFAC in the qualified executable caller at 0x8017AF0C conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 953_660,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017ED3C..0x8017ED48 including terminal delay slots; original JAL at 0x8014AD94 in the qualified executable caller at 0x8014AD28 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 953_676,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017ED4C..0x8017ED58 including terminal delay slots; original JAL at 0x8016BA70 in the qualified executable caller at 0x8016BA24 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 953_692,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017ED5C..0x8017ED68 including terminal delay slots; original JAL at 0x8016F4AC in the qualified executable caller at 0x8016F378 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 953_724,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017ED7C..0x8017ED88 including terminal delay slots; original JAL at 0x8014AD9C in the qualified executable caller at 0x8014AD28 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 953_740,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017ED8C..0x8017ED98 including terminal delay slots; original JAL at 0x8016BA80 in the qualified executable caller at 0x8016BA24 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 953_756,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017ED9C..0x8017EDA8 including terminal delay slots; original JAL at 0x8014B7C4 in the qualified executable caller at 0x8014B73C conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 953_772,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017EDAC..0x8017EDB8 including terminal delay slots; original JAL at 0x8014B8D8 in the qualified executable caller at 0x8014B8B0 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 953_788,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017EDBC..0x8017EDC8 including terminal delay slots; original JAL at 0x8014B810 in the qualified executable caller at 0x8014B73C conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 953_804,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017EDCC..0x8017EDD8 including terminal delay slots; original JAL at 0x801746C0 in the qualified executable caller at 0x801746B8 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 954_044,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017EEBC..0x8017EEC8 including terminal delay slots; original JAL at 0x8014ADF0 in the qualified executable caller at 0x8014AD28 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 954_076,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017EEDC..0x8017EEE8 including terminal delay slots; original JAL at 0x801748AC in the qualified executable caller at 0x80174848 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 954_500,
            length: 140,
            evidence: "original US SLUS; complete original instruction body 0x8017F084..0x8017F110 including terminal delay slots; original JAL at 0x80174698 in the qualified executable caller at 0x80174668 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 954_828,
            length: 48,
            evidence: "original US SLUS; complete original instruction body 0x8017F1CC..0x8017F1FC including return delay slot; original JAL at 0x801746E8 in the qualified executable caller at 0x801746E0 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 954_876,
            length: 128,
            evidence: "original US SLUS; complete original instruction body 0x8017F1FC..0x8017F27C including terminal delay slots; original JAL at 0x8017F0C8 in the qualified executable caller at 0x8017F084 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 955_004,
            length: 56,
            evidence: "original US SLUS; complete instruction body 0x8017F27C..0x8017F2B4 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 955_260,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017F37C..0x8017F388 including terminal delay slot; original JAL at 0x8017F1DC in the qualified executable caller at 0x8017F1CC conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; original in-body constants bind the external terminal jump; BIOS behavior and returns remain unqualified; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 955_276,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017F38C..0x8017F398 including terminal delay slots; original JAL at 0x8017F0DC in the qualified executable caller at 0x8017F084 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 955_292,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017F39C..0x8017F3A8 including terminal delay slots; original JAL at 0x8017F254 in the qualified executable caller at 0x8017F1FC conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns with the caller stack and saved return storage preserved; original in-body constants bind external terminal jumps; BIOS behavior and returns remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 955_308,
            length: 12,
            evidence: "original US SLUS; complete original instruction body 0x8017F3AC..0x8017F3B8 including terminal delay slot; original JAL at 0x8017F294 in the qualified executable caller at 0x8017F27C conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; original in-body constants bind the external terminal jump; BIOS behavior and returns remain unqualified; no matching callee ELF is claimed; exact whole executable identity; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 955_364,
            length: 104,
            evidence: "original US SLUS; complete original instruction body 0x8017F3E4..0x8017F44C including terminal delay slots; original JAL at 0x8017F0B0 in the qualified executable caller at 0x8017F084 conditionally enters this body; acyclic original caller dependencies, complete raw word, decoder, control-flow and return review and captured resident bytes reconcile; structural call continuations assume ordinary returns; original incoming return address is saved and restored through absolute word 0x8018B2EC; conditional return identity assumes this cell is preserved by calls and dynamic stores, no aliasing write and no reentrancy; original SP is unchanged; the cell is not classified; indirect call targets and behavior remain unqualified; exact whole executable identity; neighboring functions, data and referenced callees excluded; no matching callee ELF, observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 1_004_216,
            length: 22_952,
            evidence: "original US SLUS startup-zero-initialization storage 0x8018B2B8..0x80190C60; original PS-X entry loop 0x8014AA0C..0x8014AA30 overwrites exactly these 5738 aligned words with zero before any call under normal loader entry, mapped RAM, stable code and noninterference assumptions; original stored bytes are all zero but the original loop establishes the extent and role; exact whole executable identity; header b_addr and b_size are zero, no header-BSS, heap, neighboring-storage, later-content or observed execution claim",
        },
    ];
    ranges.extend(textures.iter().map(|range| NontextRange {
        offset: range.offset,
        length: range.length,
        evidence: &range.evidence,
    }));
    ranges
}

// Original LOGO fresh-input frames 1..=221: return frontiers minus eight bytes.
// The excluded lookahead and remaining tails stay unresolved.
const CAPCOM_CONSUMED_PREFIX_BYTES: [usize; 221] = [
    7640, 7690, 7752, 7776, 7810, 7826, 7766, 7668, 7610, 7526, 7484, 7426, 7404, 7440, 7490, 7536,
    7564, 7534, 7622, 7688, 7700, 6704, 7122, 6880, 7080, 6718, 6758, 7266, 6816, 7500, 7442, 7328,
    7322, 7810, 8032, 7804, 7678, 7630, 7874, 6578, 6476, 6468, 6558, 6720, 6744, 6732, 6648, 6814,
    7118, 7036, 7278, 6894, 7190, 7194, 7276, 7704, 7406, 7292, 7786, 7226, 7132, 7182, 7030, 6928,
    6954, 6810, 7098, 6550, 6846, 7328, 7260, 7406, 6818, 7000, 7164, 7076, 7110, 6844, 6596, 6850,
    6642, 7602, 7980, 7348, 7264, 7178, 6746, 6796, 6710, 7072, 6526, 6952, 7090, 6660, 7948, 7054,
    7038, 7076, 6832, 7284, 6984, 7442, 7354, 7068, 7158, 7268, 7634, 7734, 7754, 7754, 7754, 7754,
    7754, 7754, 7754, 7754, 7754, 7754, 7754, 7754, 7754, 7754, 7754, 7744, 7730, 7720, 7706, 7684,
    7660, 7628, 7608, 7590, 7602, 7642, 7692, 7744, 7814, 7862, 7928, 7964, 8014, 6842, 6884, 6916,
    6950, 6986, 7034, 7070, 7100, 7144, 7170, 7202, 7232, 7260, 7284, 7302, 7326, 7348, 7364, 7378,
    7388, 7400, 7404, 7410, 7410, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040,
    8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 7998,
    7948, 7888, 7816, 7720, 7586, 7448, 7292, 7130, 6944, 6766, 6574, 6808, 6572, 7706, 7336, 6954,
    6584, 7240, 6720, 6190, 7372, 6546, 5688, 4922, 3942, 3096, 2704, 2704, 2704,
];

// Frames 222..=231 contain packet-bound copies of frame 221's qualified prefix.
// This identifies stored format only; later decoder invocations remain unverified.
const CAPCOM_COPIED_PREFIX_BYTES: usize = 2704;

fn capcom_coded_prefix(sector: &[u8], start: usize) -> Result<Option<NontextRange<'static>>> {
    let index = usize::from(u16::from_le_bytes([sector[12], sector[13]]));
    let count = usize::from(u16::from_le_bytes([sector[14], sector[15]]));
    let frame = u32::from_le_bytes([sector[16], sector[17], sector[18], sector[19]]) as usize;
    if !(4..=5).contains(&count) || index >= count || !(1..=231).contains(&frame) {
        return Err(Error::Invalid(
            "original US CAPCOM packet disagrees with reviewed frame structure".into(),
        ));
    }
    let (prefix, evidence) = if frame <= CAPCOM_CONSUMED_PREFIX_BYTES.len() {
        (
            CAPCOM_CONSUMED_PREFIX_BYTES[frame - 1],
            "original US CAPCOM coded prefix, frames 1..221; LOGO 0x801D7578 fresh-input return-zero frontier minus eight bytes for lookahead; exact source identity",
        )
    } else {
        (
            CAPCOM_COPIED_PREFIX_BYTES,
            "original US CAPCOM stored coded prefix, frames 222..231; exact packet-bound copies of qualified frame 221 prefix, excluding eight lookahead bytes; no later decoder invocation claim; exact source identity",
        )
    };
    if prefix > count * 2016 {
        return Err(Error::Invalid(
            "original US CAPCOM coded prefix exceeds its source packets".into(),
        ));
    }
    let length = prefix.saturating_sub(index * 2016).min(2016);
    Ok((length != 0).then_some(NontextRange {
        offset: start + 40,
        length,
        evidence,
    }))
}

impl XaSource {
    /// Identify reviewed metadata and payloads; original use remains separate.
    pub fn nontext_ranges(&self, data: &[u8]) -> Result<Vec<NontextRange<'static>>> {
        if data.len() != self.file.size || sha256_hex(data) != self.file.sha256 {
            return Err(Error::Invalid(
                "XA classification requires the exact original source".into(),
            ));
        }
        if data.len() % 2336 != 0 {
            return Err(Error::Invalid(
                "original US XA sector extent disagrees".into(),
            ));
        }
        let mut ranges = Vec::new();
        let mut audio_count = 0;
        let mut empty_count = 0;
        let mut packet_count = 0;
        let mut coded_prefix_count = 0;
        let mut coded_prefix_bytes = 0;
        for (index, sector) in data.chunks_exact(2336).enumerate() {
            let start = index * 2336;
            if sector[..4] != sector[4..8] {
                return Err(Error::Invalid(format!(
                    "original US XA sector {index} has inconsistent subheaders"
                )));
            }
            ranges.push(NontextRange {
                offset: start,
                length: 8,
                evidence: "original US Mode 2 duplicated sector subheader; exact source identity",
            });
            if sector[2] & 0x2e != 0x24 {
                let video =
                    self.expected_packet_headers != 0 && sector[2] & 0x2e == 2 && sector[3] == 0x80;
                if !video && (sector[2] & 0x2e != 0 || sector[3] != 0) {
                    return Err(Error::Invalid(format!(
                        "original US XA sector {index} disagrees with reviewed Form 1 profile"
                    )));
                }
                if video {
                    if sector[8..12] != [0x60, 0x01, 0x01, 0x80] {
                        return Err(Error::Invalid(format!(
                            "original US packet sector {index} disagrees with reviewed marker/type"
                        )));
                    }
                    ranges.push(NontextRange {
                        offset: start + 8,
                        length: 4,
                        evidence: "original US CAPCOM packet marker/type; LOGO reader at 0x801D29B4/0x801D29C8; exact source identity; no video-body or arrival claim",
                    });
                    packet_count += 1;
                    ranges.push(NontextRange {
                        offset: start + 12,
                        length: 8,
                        evidence: "original US CAPCOM packet index/count u16 words and frame-ID u32; original LOGO 0x801D2AF8/0x801D2E04/0x801D2E08 read index/count and 0x801D2E10 tests the final packet; paired-header reads 0x801CEA58/0x801CEAF4 consume the complete frame ID, with exact copies in the other packets of each frame; exact source identity and existing packet gates; numeric stored representation only, no remaining header, coded-length, arrival or playback claim",
                    });
                    if let Some(prefix) = capcom_coded_prefix(sector, start)? {
                        coded_prefix_count += 1;
                        coded_prefix_bytes += prefix.length;
                        ranges.push(prefix);
                    }
                } else if sector[8..2056].iter().all(|byte| *byte == 0) {
                    ranges.push(NontextRange {
                        offset: start + 8,
                        length: 2048,
                        evidence: "original US empty Form 1 user payload; all 2048 bytes zero; original purpose and consumption unverified",
                    });
                    empty_count += 1;
                }
                ranges.push(NontextRange {
                    offset: start + 2056,
                    length: 280,
                    evidence: "original US Form 1 EDC and ECC P/Q fields; reviewed against disk sector owner; exact source identity",
                });
                continue;
            }
            let valid_groups = sector[8..2312].chunks_exact(128).all(|group| {
                group[..4] == group[4..8]
                    && group[8..12] == group[12..16]
                    && group[4..12]
                        .iter()
                        .all(|&parameter| parameter & 15 <= 12 && parameter >> 4 < 4)
            });
            if sector[3] & 0xfa != 0 || !valid_groups {
                return Err(Error::Invalid(format!(
                    "original US XA sector {index} disagrees with reviewed four-bit format"
                )));
            }
            ranges.push(NontextRange {
                offset: start + 8,
                length: 2304,
                evidence: "original US XA encoded body; supported Form 2 four-bit header, verified parameter copies and active parameters; no claim of game selection, arrival or playback",
            });
            if sector[2312..2332].iter().any(|byte| *byte != 0) {
                return Err(Error::Invalid(format!(
                    "original US XA sector {index} disagrees with reviewed zero-byte profile"
                )));
            }
            ranges.push(NontextRange {
                offset: start + 2312,
                length: 20,
                evidence: "original US zero-filled Form 2 bytes between encoded sound groups and EDC; all 20 bytes zero; exact source identity; original purpose and consumption unverified",
            });
            if sector[2332..].iter().any(|byte| *byte != 0) {
                return Err(Error::Invalid(format!(
                    "original US XA sector {index} disagrees with reviewed absent EDC"
                )));
            }
            ranges.push(NontextRange {
                offset: start + 2332,
                length: 4,
                evidence: "original US Form 2 absent EDC field; stored zero, not a validated checksum; exact source identity",
            });
            audio_count += 1;
        }
        if audio_count != self.expected_audio_bodies
            || empty_count != self.expected_empty_payloads
            || packet_count != self.expected_packet_headers
        {
            return Err(Error::Invalid(
                "original US XA body count disagrees with reviewed profile".into(),
            ));
        }
        let expected_prefixes = if self.expected_packet_headers == 0 {
            (0, 0)
        } else {
            (
                892,
                CAPCOM_CONSUMED_PREFIX_BYTES.iter().sum::<usize>()
                    + 10 * CAPCOM_COPIED_PREFIX_BYTES,
            )
        };
        if (coded_prefix_count, coded_prefix_bytes) != expected_prefixes {
            return Err(Error::Invalid(
                "original US CAPCOM coded prefixes disagree with reviewed profile".into(),
            ));
        }
        Ok(ranges)
    }
}

// Exact resident descriptors and payloads from the original AREA cohort.
pub fn payload_nontext_ranges<'a>(
    entry: Entry,
    data: &[u8],
    original_us: bool,
    textures: &'a [crate::textures::Range],
) -> Option<Vec<NontextRange<'a>>> {
    if !original_us {
        return None;
    }
    let mut ranges: Vec<NontextRange<'a>> =
        reviewed_payload_nontext_ranges(entry, data).unwrap_or_default();
    ranges.extend(textures.iter().map(|range| NontextRange {
        offset: range.offset,
        length: range.length,
        evidence: &range.evidence,
    }));
    if let Some(instructions) = matched_payload_instruction_ranges(entry, data, &ranges) {
        ranges.extend(instructions);
    }
    (!ranges.is_empty()).then_some(ranges)
}

fn uncovered_nontext_ranges<'a>(
    size: usize,
    ranges: Vec<NontextRange<'a>>,
    previous: &[NontextRange<'_>],
) -> Option<Vec<NontextRange<'a>>> {
    let mut covered = vec![false; size];
    for range in previous {
        covered
            .get_mut(range.offset..range.offset.checked_add(range.length)?)?
            .fill(true);
    }
    let mut additions = Vec::new();
    for range in ranges {
        let end = range.offset.checked_add(range.length)?;
        let known = covered.get(range.offset..end)?;
        let mut at = 0;
        while at < known.len() {
            if known[at] {
                at += 1;
                continue;
            }
            let start = at;
            while at < known.len() && !known[at] {
                at += 1;
            }
            additions.push(NontextRange {
                offset: range.offset + start,
                length: at - start,
                evidence: range.evidence,
            });
        }
    }
    Some(additions)
}

fn matched_payload_instruction_ranges(
    entry: Entry,
    data: &[u8],
    previous: &[NontextRange<'_>],
) -> Option<Vec<NontextRange<'static>>> {
    if entry.transfer_mode != 0 || usize::try_from(entry.size).ok() != Some(data.len()) {
        return None;
    }
    let instructions: &[(usize, usize)] = match (
        entry.index,
        entry.offset,
        entry.ram_ptr,
        entry.check_word,
        data.len(),
        sha256_hex(data).as_str(),
    ) {
        (
            0,
            2_048,
            0x8019_5800,
            30,
            229_720,
            "1663ff2e78134fe15f3cae6fb4074acf14b68b79cfa02be0c7651eaf6a1728ff",
        ) => &[
            (2_076, 320),
            (4_284, 736),
            (5_360, 148),
            (6_008, 1_704),
            (8_660, 704),
            (9_812, 244),
            (10_608, 256),
            (11_436, 60),
            (12_100, 60),
            (12_548, 60),
            (12_996, 60),
            (13_252, 116),
            (13_484, 1_292),
            (15_032, 80),
            (15_208, 216),
            (15_704, 548),
            (16_364, 128),
            (16_696, 192),
            (18_660, 792),
            (19_772, 68),
            (20_436, 304),
            (21_060, 364),
            (21_556, 80),
            (21_872, 120),
            (22_764, 100),
            (23_396, 68),
            (26_580, 84),
            (26_916, 288),
            (28_192, 68),
            (28_944, 68),
            (29_588, 68),
            (34_252, 136),
            (34_700, 40),
            (35_428, 68),
            (35_876, 132),
            (36_736, 68),
            (37_144, 68),
            (37_588, 100),
            (38_416, 212),
            (39_628, 184),
            (40_984, 40),
            (41_356, 276),
            (42_888, 192),
            (43_752, 152),
            (44_176, 396),
            (49_700, 192),
            (73_476, 372),
            (73_976, 28),
            (74_456, 536),
            (76_700, 64),
            (77_524, 84),
            (79_028, 132),
            (91_872, 80),
            (94_448, 464),
            (95_172, 276),
            (95_760, 284),
            (97_116, 88),
            (97_356, 144),
            (97_948, 152),
            (98_756, 44),
            (99_480, 92),
            (105_072, 288),
            (105_904, 592),
            (112_124, 72),
            (120_064, 220),
            (121_440, 180),
            (121_736, 256),
            (122_176, 172),
            (122_480, 32),
            (122_652, 76),
            (123_900, 208),
            (126_100, 72),
            (130_496, 196),
            (131_064, 140),
            (131_508, 304),
            (132_060, 120),
            (132_796, 172),
            (133_188, 152),
            (134_168, 88),
            (135_616, 68),
            (137_180, 248),
            (137_836, 216),
            (138_160, 156),
            (139_444, 212),
            (140_304, 144),
            (140_544, 372),
            (141_804, 160),
            (142_712, 72),
            (143_212, 76),
            (145_480, 88),
            (145_868, 68),
            (146_840, 72),
            (150_136, 1_024),
            (155_376, 380),
            (155_880, 76),
            (164_732, 380),
            (165_208, 432),
            (167_696, 40),
            (168_224, 656),
            (176_396, 424),
            (177_460, 32),
            (177_736, 1_036),
            (180_288, 60),
            (181_744, 552),
            (183_352, 256),
            (184_080, 208),
            (184_468, 536),
            (186_708, 328),
            (187_800, 180),
            (188_304, 152),
            (188_660, 292),
            (190_016, 64),
            (192_300, 64),
            (193_820, 60),
            (195_928, 200),
            (196_504, 28),
            (196_596, 128),
            (197_756, 56),
            (202_708, 144),
            (203_144, 60),
        ],
        _ => return None,
    };
    let ranges = instructions
        .iter()
        .map(|&(offset, length)| NontextRange {
            offset,
            length,
            evidence: "Original US matched overlay instruction bodies. Fresh target-owned native comparisons reconcile complete function sections, original placements and every linked byte with the original raw entry. The complete selected entry and original archive reconstruct byte-identically. Exact original archive, raw mode, entry geometry, RAM base, size, checkword and full payload identity bind each profile. Existing physical ranges retain their evidence. Initialized data, retained source regions, external callees and referenced storage receive no new role. Complete original instruction role only; no source-only reconstruction, semantic approval, observed invocation, full compiler-input closure, rendering or safe-edit claim.",
        })
        .collect();
    uncovered_nontext_ranges(data.len(), ranges, previous)
}

fn reviewed_payload_nontext_ranges(
    entry: Entry,
    data: &[u8],
) -> Option<Vec<NontextRange<'static>>> {
    if entry.transfer_mode == 0
        && usize::try_from(entry.size).ok() == Some(data.len())
        && matches!(
            entry.ram_ptr,
            0x801f_6c00 | 0x801d_0c00 | 0x801c_e400 | 0x801f_2c00
        )
    {
        let instructions: &[(usize, usize)] = match (
            entry.index,
            entry.offset,
            entry.ram_ptr,
            entry.check_word,
            data.len(),
            sha256_hex(data).as_str(),
        ) {
            (
                0,
                2_048,
                0x801f_6c00,
                0x105,
                28_528,
                "c5bda6ff8260b59a361a49f80720bb6942d9eed8d6497d3cc9d1d2b483c3089e",
            ) => &[(448, 19_300), (21_128, 1_304), (23_504, 432)],
            (
                7,
                143_360,
                0x801d_0c00,
                0x19,
                16_948,
                "61e0ed2d90247be2bad9904015ac042ac3a83e638721497a90e298815354e165",
            ) => &[(104, 8_496), (10_644, 5_668)],
            (
                0,
                2_048,
                0x801f_6c00,
                0x112,
                21_488,
                "31859c1ab5aa4a2c8250f0011569ac3b66a442f023717fb677883417ddeab469",
            ) => &[
                (676, 340),
                (1_244, 452),
                (2_116, 64),
                (2_408, 212),
                (3_416, 1_124),
                (6_260, 6_820),
                (17_592, 940),
                (20_020, 1_212),
            ],
            (
                0,
                2_048,
                0x801d_0c00,
                0x1f,
                13_963,
                "42accff83b45c5ef99947f70a6c968fdf2a2a4afd0abdc3fca8874db20ecc8af",
            ) => &[(24, 2_092), (2_940, 1_752), (6_048, 5_764), (12_912, 232)],
            (
                0,
                2_048,
                0x801f_6c00,
                0x109,
                14_176,
                "cb0ff0c46ea33d5c9eef07493b7b2c2673278dbb97373c3eb8f77b8adfdbcb9f",
            ) => &[
                (256, 420),
                (812, 1_680),
                (2_984, 1_352),
                (4_800, 372),
                (5_412, 1_372),
                (9_112, 3_676),
                (13_264, 432),
            ],
            (
                0,
                2_048,
                0x801c_e400,
                0x27,
                9_428,
                "98b83aa1d6951af3844ea34058e34fbcc18acb5d5e8f69eb4aa289107564bfa8",
            ) => &[(4, 9_300)],
            (
                0,
                2_048,
                0x801c_e400,
                0x31,
                9_224,
                "3eee6be151e6a3ffff13ca98efad8521719a29bcd7ffd465f37f2ec5188389e3",
            ) => &[(4, 9_096)],
            (
                0,
                2_048,
                0x801f_6c00,
                0x108,
                25_672,
                "3570700503e37f2a43a3ad9877521773532fd8b2278ceb70ca1f7cb4e37ef7e8",
            ) => &[
                (624, 68),
                (1_632, 748),
                (3_604, 3_816),
                (8_196, 508),
                (9_972, 68),
                (11_256, 136),
                (13_476, 680),
                (22_452, 1_272),
                (23_876, 1_236),
                (25_308, 72),
            ],
            (
                14,
                815_104,
                0x801f_2c00,
                0xad,
                15_547,
                "42da9ce8b9e2225fc93ed1bcb0296fd7a42a341f5a81187fcb0025ac5365a275",
            ) => &[
                (60, 244),
                (448, 92),
                (1_144, 140),
                (1_596, 124),
                (2_072, 1_792),
                (5_340, 244),
                (5_900, 560),
                (6_624, 68),
                (6_996, 40),
                (7_936, 76),
                (9_080, 940),
                (10_112, 1_048),
                (11_544, 1_460),
                (13_304, 1_248),
            ],
            (
                14,
                819_200,
                0x801f_2c00,
                0x81,
                16_248,
                "0987764aeff328845033df60674008b12b1af555fe888c68445459964c292e9e",
            ) => &[
                (4, 68),
                (2_936, 288),
                (4_212, 176),
                (4_612, 68),
                (4_872, 684),
                (5_664, 424),
                (6_480, 1_304),
                (8_412, 68),
                (8_900, 1_928),
                (10_948, 2_956),
            ],
            (
                0,
                2_048,
                0x801d_0c00,
                0x11b,
                13_496,
                "0af89b01778db26d220b3fe5975f766edf0b798c96dc6df1c82031fb9cbea361",
            ) => &[
                (4, 68),
                (192, 3_664),
                (4_308, 1_740),
                (6_844, 632),
                (8_004, 1_172),
                (10_908, 636),
            ],
            _ => &[],
        };
        if !instructions.is_empty() {
            return Some(
                instructions
                    .iter()
                    .map(|&(offset, length)| NontextRange {
                        offset,
                        length,
                        evidence: concat!(
                            "Original US native overlay instruction bodies. ",
                            "Complete freshly compiled and relocated ELF function symbols match ",
                            "the original raw payload; compiler assembly contains no embedded data. ",
                            "Exact original archive and descriptor/payload identities gate classification. ",
                            "Initialized data, retained functions and gaps outside complete symbols ",
                            "are excluded. Stored instruction representation only; no observed execution, ",
                            "callee semantics, resource safety, rendering or safe-edit claim."
                        ),
                    })
                    .collect(),
            );
        }
    }
    if entry.index == 0
        && entry.offset == 2_048
        && entry.transfer_mode == 0
        && entry.ram_ptr == 0x801f_6c00
        && usize::try_from(entry.size).ok() == Some(data.len())
    {
        let instructions: &[(usize, usize)] =
            match (entry.check_word, data.len(), sha256_hex(data).as_str()) {
                (
                    0x107,
                    30_588,
                    "e58e751dc92ba3cc9bbc788bb7d93ae1f4b036968c4bc95e4bf29945dd97a8aa",
                ) => &[
                    (1_436, 96),
                    (3_680, 60),
                    (4_144, 8),
                    (5_224, 652),
                    (6_456, 520),
                    (8_312, 4_652),
                    (16_468, 1_816),
                    (19_140, 420),
                    (20_588, 1_864),
                    (22_808, 368),
                    (23_348, 1_192),
                    (24_852, 1_704),
                    (26_636, 884),
                    (30_072, 244),
                ],
                (
                    0x10b,
                    30_964,
                    "a3f118a0a9a17aefb50b4cd588c9a152a943591462fc9424ddfe9f5c9e9e705b",
                ) => &[
                    (1_688, 6_328),
                    (8_984, 3_804),
                    (14_288, 6_328),
                    (20_872, 4_016),
                    (25_040, 1_516),
                    (27_464, 1_588),
                    (29_228, 1_424),
                ],
                (
                    0x10e,
                    31_064,
                    "3292290e08fdfeee3c46383fc4bfa1ae0666dc5d5724335c1395fb1ab3a5614f",
                ) => &[(1_884, 28_712)],
                (
                    0x110,
                    17_304,
                    "c4463f21512ac74adf7002b7a5654450c2049513bef7c817473a9d5108532239",
                ) => &[(1_204, 332), (1_672, 15_416)],
                _ => &[],
            };
        if !instructions.is_empty() {
            let jump_tables: &[(usize, usize)] = match entry.check_word {
                0x10b => &[(4, 28), (36, 20)],
                0x110 => &[(52, 44)],
                _ => &[],
            };
            return Some(
                instructions
                    .iter()
                    .map(|&(offset, length)| NontextRange {
                        offset,
                        length,
                        evidence: concat!(
                            "Original US SCENA02/SCENA06/SCENA09/SCENA11 entry-0 instruction bodies. ",
                            "Complete freshly compiled and relocated ELF function symbols match ",
                            "the original raw payload; compiler assembly contains no embedded data. ",
                            "Exact original archive and descriptor/payload identities gate classification. ",
                            "Initialized data, gaps and retained functions are excluded. Stored ",
                            "instruction representation only; no observed execution, callee semantics, ",
                            "resource safety, rendering or safe-edit claim."
                        ),
                    })
                    .chain(jump_tables.iter().map(|&(offset, length)| NontextRange {
                        offset,
                        length,
                        evidence: concat!(
                            "Original US SCENA06/SCENA11 entry-0 guarded absolute jump-table words. ",
                            "Reviewed readonly sections match complete native linked bytes. Original ",
                            "dispatches have leading index-load-delay NOPs, unsigned bounds, settled ",
                            "table loads and targets within their complete owning functions. Exact ",
                            "original archive and descriptor/payload identities gate classification. ",
                            "Stored numeric representation only; other tables and gaps, general caller ",
                            "bounds, observed execution, semantic acceptance and safe editing are excluded."
                        ),
                    }))
                    .collect(),
            );
        }
    }
    if entry.index == 4
        && entry.offset == 180_224
        && entry.transfer_mode == 0
        && entry.ram_ptr == 0x801d_0c00
        && entry.check_word == 0x16
        && entry.size == 75_285
        && data.len() == 75_285
        && sha256_hex(data) == "d869b1540648c3fb98fe64cb9ec62a95b4f8082a111cfd15331a102b0d7a7dd2"
    {
        return Some(
            [(152, 68), (1_472, 68_372)]
                .into_iter()
                .map(|(offset, length)| NontextRange {
                    offset,
                    length,
                    evidence: concat!(
                        "Original US AREA030/AREA089/AREA129 entry-4 instruction bodies. ",
                        "Complete freshly compiled and relocated ELF function symbols match ",
                        "the original raw payload; compiler assembly contains no embedded data. ",
                        "Exact original archive and descriptor/payload identities gate classification. ",
                        "Initialized sections, padding outside symbols, tables and referenced storage ",
                        "are excluded. Stored instruction representation only; no observed execution, ",
                        "callee semantics, resource safety, rendering or safe-edit claim."
                    ),
                })
                .collect(),
        );
    }
    if entry.transfer_mode == 0
        && entry.ram_ptr == 0x801e_ec00
        && entry.check_word == 0x141
        && entry.size == 5_268
        && matches!((entry.index, entry.offset), (8, 231_424) | (9, 479_232))
        && data.len() == 5_268
        && sha256_hex(data) == "6e7b93d4fd75586413b5ab3a6785d7b2492d8d1b566a3a6d79941bb656131a4d"
    {
        let ranges = vec![NontextRange {
            offset: 0x67c,
            length: 0x6f0,
            evidence: "original US SHOP entry 8 or START entry 9 complete save-header producer instruction body 0x801EF27C..0x801EF96C including return delay slot; all 444 original words and 47 control transfers reviewed, conditional original incoming SHOP call at 0x801D6A08; sole external call 0x8017E394 requires an ordinary return and bounded valid title; exact original archive, entry geometry, raw load, checkword, size and payload identities; stable source/code, intact frame and nonaliased work are conditions; neighboring functions and data excluded; no observed execution, source-loading, active-supplier, BIOS animation, rendering or safe-edit claim",
        }];
        return Some(ranges);
    }
    match (entry.index, entry.offset, entry.transfer_mode, entry.ram_ptr, data.len()) {
        (0, 2_048, 0, 0x801d_0c00, 87_660)
            if entry.check_word == 0x13
                && sha256_hex(data)
                    == "b9b26c0d387ebbbaa211ed949806f76f5a3cc6ae891df39793ef12335c158219" =>
        {
            Some(vec![
                NontextRange {
                    offset: 672,
                    length: 60,
                    evidence: "original US SHOP entry 0; complete 15-word dispatcher 0x801D0EA0..0x801D0EDC including indirect-call and return delays; original ASM, raw words and retained RAM agree; captured mode 2 selects the original pointer cell at 0x801E514C; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; ordinary selected-callee return, intact frame, valid nonaliasing memory and stable code/table remain conditions; other table cells, referenced data and callees excluded; no general selector bounds, observed entry, all-path execution, resource safety, rendering or editing claim",
                },
                NontextRange {
                    offset: 1_252,
                    length: 60,
                    evidence: "original US SHOP entry 0; complete 15-word dispatcher 0x801D10E4..0x801D1120 selected by original cell 0x801E514C; all original words, load/indirect-call/return delays and closed local flow match retained RAM; captured phase 1 selects cell 0x801E5174; exact archive and payload identities; ordinary selected-callee return, intact frame, valid nonaliasing memory and stable code/table remain conditions; other cells, referenced data and callees excluded; no general selector bounds, observed invocation, resource safety, rendering or editing claim",
                },
                NontextRange {
                    offset: 1_728,
                    length: 60,
                    evidence: "original US SHOP entry 0; complete 15-word dispatcher 0x801D12C0..0x801D12FC selected by original cell 0x801E5174; all original words, load/indirect-call/return delays and closed local flow match retained RAM; captured substep 0 selects cell 0x801E518C; exact archive and payload identities; ordinary selected-callee return, intact frame, valid nonaliasing memory and stable code/table remain conditions; other cells, referenced data and callees excluded; no general selector bounds, observed invocation, resource safety, rendering or editing claim",
                },
                NontextRange {
                    offset: 1_788,
                    length: 436,
                    evidence: "original US SHOP entry 0; complete 109-word instruction body 0x801D12FC..0x801D14B0 selected by original cell 0x801E518C; all original words, twelve control-transfer delays, fifteen loads, closed fifteen-block flow and shared return reconcile with retained RAM; exact archive and payload identities; ordinary returns and preservation from five direct calls, intact frame, valid nonaliasing memory and stable code remain conditions; input/state fields, callees and neighboring bodies excluded; no actual handler entry, physical-input contract, general selector bounds, purchase/sale, resource safety, rendering or editing claim",
                },
                NontextRange {
                    offset: 7_148,
                    length: 60,
                    evidence: "original US SHOP entry 0; complete 15-word dispatcher 0x801D27EC..0x801D2828 selected by original phase-3 cell 0x801E517C; all original words, load/indirect-call/return delays and closed local flow match retained RAM; captured substep 1 selects cell 0x801E51B4; exact archive and payload identities; ordinary selected-callee return, intact frame, valid nonaliasing memory and stable code/table remain conditions; other cells, referenced data and callees excluded; no general selector bounds, actual invocation, resource safety, rendering or editing claim",
                },
                NontextRange {
                    offset: 7_376,
                    length: 1_172,
                    evidence: "original US SHOP entry 0; complete 293-word instruction body 0x801D28D0..0x801D2D64 selected by original cell 0x801E51B4; all original words, 44 control-transfer delays, 49 loads, closed 51-block flow and shared return reconcile with retained RAM; exact archive and payload identities; ordinary returns and preservation from eleven direct calls, intact frame, valid nonaliasing memory and stable code remain conditions; category/input/work fields, callees and neighboring bodies excluded; no whole-handler execution, physical-input producer, general selector or allocation bounds, purchase/sale, resource safety, rendering or editing claim",
                },
                NontextRange {
                    offset: 8_548,
                    length: 688,
                    evidence: "original US SHOP entry 0; complete 172-word instruction body 0x801D2D64..0x801D3014 selected by original cell 0x801E51B8; all original words, 20 control-transfer delays, 28 loads, closed 29-block flow and shared return reconcile with retained RAM; exact archive and payload identities; ordinary returns and preservation from five direct calls, intact frame, valid nonaliasing memory and stable code remain conditions; input/quantity/work fields, callees and neighboring bodies excluded; no actual instruction retirement, general physical-input producer, selector bounds, purchase/sale, resource safety, rendering or editing claim",
                },
                NontextRange {
                    offset: 47_868,
                    length: 1_888,
                    evidence: "Original US SHOP entry 0, complete 472-word instruction body 0x801DC6FC..0x801DCE5C. Original wrapper JAL 0x801E375C with NOP delay conditionally enters it after reloading 0x80148648. All original ASM words, local control flow, delay slots and restored frame agree with original archive and retained RAM. Exact archive and payload identities gate classification. Stable code, tables and selectors, valid applicable memory, nonaliasing, intact frames and ordinary prior/callee returns remain conditions. Referenced records, tables, fields, callees and neighbors are excluded. Selector producers, general bounds, whole-body execution, resource safety, rendering and editing remain unqualified.",
                },
                NontextRange {
                    offset: 72_636,
                    length: 68,
                    evidence: "Original US SHOP entry 0, complete 17-word dispatcher 0x801E27BC..0x801E2800. Original SLUS 0x80158C80 conditionally selects it through source cell 0x80149B28. Panel byte 2 equal to 9 selects wrapper 0x801E3720 through cell 0x801E5D8C. All original ASM words, local control flow, delay slots and restored frame agree with original archive and retained RAM. Exact archive and payload identities gate classification. Stable selectors, code and tables, valid callback targets and nonaliasing memory, intact frames and ordinary prior/callee returns remain conditions. Supporting cells and referenced data are excluded. Selector producers, general bounds, indirect-call effects, historical execution, resource safety, rendering and editing remain unqualified.",
                },
                NontextRange {
                    offset: 76_576,
                    length: 84,
                    evidence: "Original US SHOP entry 0, complete 21-word wrapper 0x801E3720..0x801E3774. Dispatcher 0x801E27BC conditionally selects it through original cell 0x801E5D8C. After the byte-3-selected indirect call, it reloads 0x80148648 and calls 0x801DC6FC at 0x801E375C with NOP delay. All original ASM words, local control flow, delay slots and restored frame agree with original archive and retained RAM. Exact archive and payload identities gate classification. Stable selectors, code and tables, valid callback targets and nonaliasing memory, intact frames and ordinary prior/callee returns remain conditions. Referenced cells, fields and callees are excluded. Selector producers, general bounds, indirect-call effects, historical execution, resource safety, rendering and editing remain unqualified.",
                },
                NontextRange {
                    offset: 83_276,
                    length: 4,
                    evidence: "original US SHOP entry 0; little-endian function-pointer cell 0x801E514C contains 0x801D10E4 and is selected by captured mode 2 in the complete original dispatcher at 0x801D0EA0; original raw and retained RAM agree under exact archive and payload identities; numeric pointer storage only; neighboring cells, selector bounds and actual dispatch excluded",
                },
                NontextRange {
                    offset: 83_316,
                    length: 4,
                    evidence: "original US SHOP entry 0; little-endian function-pointer cell 0x801E5174 contains 0x801D12C0 and is selected by captured phase 1 in the complete original dispatcher at 0x801D10E4; original raw and retained RAM agree under exact archive and payload identities; numeric pointer storage only; neighboring cells, selector bounds and actual dispatch excluded",
                },
                NontextRange {
                    offset: 83_324,
                    length: 4,
                    evidence: "original US SHOP entry 0; little-endian function-pointer cell 0x801E517C contains 0x801D27EC and is selected by captured phase 3 in the complete original dispatcher at 0x801D10E4; original raw and retained RAM agree under exact archive and payload identities; numeric pointer storage only; neighboring cells, selector bounds and actual dispatch excluded",
                },
                NontextRange {
                    offset: 83_340,
                    length: 4,
                    evidence: "original US SHOP entry 0; little-endian function-pointer cell 0x801E518C contains 0x801D12FC and is selected by captured substep 0 in the complete original dispatcher at 0x801D12C0; original raw and retained RAM agree under exact archive and payload identities; numeric pointer storage only; neighboring cells, selector bounds and actual dispatch excluded",
                },
                NontextRange {
                    offset: 83_380,
                    length: 4,
                    evidence: "original US SHOP entry 0; little-endian function-pointer cell 0x801E51B4 contains 0x801D28D0 and is selected by captured substep 1 in the complete original dispatcher at 0x801D27EC; original raw and retained RAM agree under exact archive and payload identities; numeric pointer storage only; neighboring cells, selector bounds and actual dispatch excluded",
                },
                NontextRange {
                    offset: 83_384,
                    length: 4,
                    evidence: "original US SHOP entry 0; little-endian function-pointer cell 0x801E51B8 contains 0x801D2D64 and is selected by captured substep 2 in the complete original dispatcher at 0x801D27EC; original raw and retained RAM agree under exact archive and payload identities; numeric pointer storage only; neighboring cells, selector bounds and actual dispatch excluded",
                },
            ])
        }
        (8, 360_448, 0, 0x801d_0c00, 117_986) | (0, 2_048, 0, 0x801d_0c00, 117_986)
            if entry.check_word == 18
                && sha256_hex(data)
                    == "a47d32865283a01ce87b2f2f8cb19d3305525659b95a8feb4f307eddf63a8cb1" =>
        {
            let mut ranges = Vec::new();
            if entry.index == 8 {
                ranges.push(NontextRange {
                    offset: 2_496,
                    length: 384,
                    evidence: "original US START entry 8; complete instruction body 0x801D15C0..0x801D1740 including all branch and return delay slots; original GAME01 callback registration and typed OpenTh entry-function consumer conditionally bind the alternate-loop and finite START selector-2/name-entry selector-0 route to JAL at 0x801E4678; all 96 words and complete structural flow match original and retained RAM; ordinary helper returns, valid thread arguments, successful unskipped START loading, stable code/selectors and noninterfering callback slots remain conditions; two compiled BREAK division guards are exceptional terminal edges without delay slots, and exception-handler behavior remains unqualified; exact original archive, entry, mode, load, offset, checkword, size and payload identities; neighboring code and character-record data excluded; no matching ELF, observed execution, historical initialization, full selector bounds, argument-validity, resource-safety or unique-supplier claim",
                });
            }
            ranges.extend([
                NontextRange {
                    offset: 404,
                    length: 24,
                    evidence: "original US START entry 8 or STATUS entry 0; six little-endian numeric jump targets at 0x801D0D94..0x801D0DAC; complete original Equipment body 0x801E0A70..0x801E11A8 bounds the indexed LW at 0x801E0AB4 by unsigned selector < 6 at 0x801E0AA0, with invalid selectors bypassing the load; all targets stay inside the qualified body; original words, load/branch delays and retained RAM agree; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; stable code/table, valid memory/control, intact frame and nonaliasing remain conditions; numeric storage only, excluding neighbors, actual dispatch, all-path feasibility, general bounds, transitive effects and safe editing",
                },
                NontextRange {
                    offset: 456,
                    length: 24,
                    evidence: "original US START entry 8 or STATUS entry 0; six little-endian numeric jump targets at 0x801D0DC8..0x801D0DE0; complete original Config phase-2 body 0x801E1C20..0x801E206C bounds the indexed LW at 0x801E1CD0 by unsigned focus-minus-two < 6 at 0x801E1CBC; all targets stay inside the qualified body; original words, load/branch delays and retained RAM agree; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; stable code/table, valid memory/control, intact frame and nonaliasing remain conditions; numeric storage only, excluding neighbors, actual dispatch, all-path feasibility, general bounds, transitive effects and safe editing",
                },
                NontextRange {
                    offset: 47_948,
                    length: 928,
                    evidence: "original US START entry 8 or STATUS entry 0; complete 232-word instruction body 0x801DC74C..0x801DCAEC including all control-transfer delay words; original wrapper JAL 0x801DEEC8 conditionally enters this four-label reader; every original word, complete structural flow, load scheduling and saved frame reconcile with retained RAM; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; ordinary external-call returns and ABI preservation, valid memory and incoming return control, intact saved frame, nonaliasing and stable code remain conditions; referenced data, external callees and neighbors excluded; the wrapper's incoming record/flag producer remains unbound; no observed invocation, ordinary menu route, all-path feasibility, argument bounds, callee effects, rendering, C/ELF match, unique supplier or safe editing",
                },
                NontextRange {
                    offset: 57_904,
                    length: 184,
                    evidence: "original US START entry 8 or STATUS entry 0; complete 46-word instruction wrapper 0x801DEE30..0x801DEEE8 including all control-transfer delay words; original JAL 0x801DEEC8 and its delay conditionally bind the reader at 0x801DC74C; every original word, complete structural flow, load scheduling and saved frame reconcile with retained RAM; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; ordinary external-call returns and ABI preservation, valid memory and incoming return control, intact saved frame, nonaliasing and stable code remain conditions; referenced data, external callees and neighboring three-label entry excluded; incoming record/flag producer remains unbound; no observed invocation, ordinary menu route, all-path feasibility, argument bounds, callee effects, rendering, C/ELF match, unique supplier or safe editing",
                },
                NontextRange {
                    offset: 61_428,
                    length: 1_888,
                    evidence: "original US START entry 8 or STATUS entry 0; complete instruction body 0x801DFBF4..0x801E0354; all 472 original words, 53 control-transfer delay words and complete structural flow reconcile with retained ordinary origin and native call captures; observed internal PC 0x801DFF24 binds executable words without proving first-instruction or all-path execution; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; ordinary call-return, valid-memory, intact-frame, nonaliasing and stable-code assumptions; strings, tables, referenced data, callees and neighbors excluded; no ELF match, general selector bounds, resource safety, safe editing or unique-supplier claim",
                },
                NontextRange {
                    offset: 63_316,
                    length: 1_268,
                    evidence: "original US START entry 8 or STATUS entry 0; complete instruction body 0x801E0354..0x801E0848; all 317 original words, 37 control-transfer delay words and complete structural flow reconcile with retained ordinary origin and native call captures; observed internal PC 0x801E05A0 binds executable words without proving first-instruction or all-path execution; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; ordinary call-return, valid-memory, intact-frame, nonaliasing and stable-code assumptions; strings, tables, referenced data, callees and neighbors excluded; no ELF match, general selector bounds, resource safety, safe editing or unique-supplier claim",
                },
                NontextRange {
                    offset: 65_136,
                    length: 1_848,
                    evidence: "original US START entry 8 or STATUS entry 0; complete instruction body 0x801E0A70..0x801E11A8; all 462 original words, 48 control-transfer delay words and complete structural flow reconcile with retained ordinary origin and native call captures; unsigned selector guard and six exact original targets at 0x801D0D94 close the computed JR at 0x801E0ABC; observed internal PC 0x801E0E2C binds executable words without proving first-instruction or all-path execution; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; ordinary call-return, valid-memory, intact-frame, nonaliasing and stable-code/table assumptions; table bytes, strings, referenced data, callees and neighbors excluded; no ELF match, general selector bounds, resource safety, safe editing or unique-supplier claim",
                },
                NontextRange {
                    offset: 68_224,
                    length: 60,
                    evidence: "original US START entry 8 or STATUS entry 0; complete 15-word instruction body 0x801E1680..0x801E16BC including both indirect-call and return delay words; all original words and complete structural flow reconcile with retained RAM; observed phase-1 and phase-2 entries have return address 0x801E16AC, matching original JALR 0x801E16A4; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; ordinary return from the opaque selected callee, valid memory and incoming return control, intact frame, nonaliasing writes and stable code/table remain conditions; table cells, other handlers and neighbors excluded; no general phase bounds, first-instruction dispatcher execution, all-path feasibility, transitive effects, resource safety, rendering, editing or unique-supplier claim",
                },
                NontextRange {
                    offset: 68_816,
                    length: 848,
                    evidence: "original US START entry 8 or STATUS entry 0; complete 212-word instruction body 0x801E18D0..0x801E1C20 including all 36 control-transfer delay words; complete original structural flow and observed phase-1 entry with return address 0x801E16AC reconcile with retained RAM; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; ordinary returning callees, valid aligned memory and incoming return control, intact saved frame, nonaliasing writes and stable code remain conditions; input/work fields, strings, callees and neighbors excluded; no all-path feasibility, general argument bounds, transitive effects, resource safety, rendering, editing or unique-supplier claim",
                },
                NontextRange {
                    offset: 69_664,
                    length: 1_100,
                    evidence: "original US START entry 8 or STATUS entry 0; complete 275-word instruction body 0x801E1C20..0x801E206C including all 55 control-transfer delay words; complete original structural flow and observed phase-2 entry with return address 0x801E16AC reconcile with retained RAM; unsigned focus-minus-two guard and six exact original targets at 0x801D0DC8 close computed JR 0x801E1CD8 within the body; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; ordinary returning callees, valid aligned memory and incoming return control, intact saved frame, nonaliasing writes and stable code/table remain conditions; table bytes, settings/work fields, strings, callees and neighbors excluded; no all-path feasibility, general selector bounds beyond the local guard, transitive effects, resource safety, rendering, editing or unique-supplier claim",
                },
                NontextRange {
                    offset: 72_296,
                    length: 64,
                    evidence: "original US START entry 8 or STATUS entry 0; complete 16-word instruction body 0x801E2668..0x801E26A8 including both call and return delay words; qualified original phase-body JALs 0x801E1960, 0x801E1AD0 and 0x801E1CA8 conditionally enter this body; every original word, complete structural flow, load scheduling and saved frame reconcile with retained RAM; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; ordinary external-call return and ABI preservation, valid memory and incoming return control, intact saved frame, nonaliasing and stable code remain conditions; referenced data, opaque callee and neighbors excluded; no observed entry, all-path feasibility, argument/string-index bounds, callee effects, rendering, C/ELF match, unique supplier or safe editing",
                },
                NontextRange {
                    offset: 72_360,
                    length: 496,
                    evidence: "original US START entry 8 or STATUS entry 0; complete 124-word instruction body 0x801E26A8..0x801E2898 including all eight control-transfer delay words; all original words, three LWL/LWR merge pairs and complete structural flow reconcile with retained RAM; original JALs 0x801E1950 and 0x801E1C98 in qualified phase bodies conditionally enter this caller, with native interior PC 0x801E2854 and linked reader entry/return observations; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; ordinary returning callees, valid aligned memory and incoming return control, intact saved frame, nonaliasing writes and stable code/table remain conditions; selected records, referenced data, callees and neighbors excluded; no first-instruction caller execution, all-path feasibility, general selector bounds, transitive effects, resource safety, rendering, editing or unique-supplier claim",
                },
                NontextRange {
                    offset: 72_856,
                    length: 872,
                    evidence: "original US START entry 8 or STATUS entry 0; complete 218-word instruction body 0x801E2898..0x801E2C00 including all twelve control-transfer delay words; qualified original caller JAL 0x801E280C and its A0=S0 delay conditionally enter this body; every original word, complete structural flow, eleven LWL/LWR merge-forwarding pairs, load scheduling and saved frame reconcile with retained RAM; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; ordinary external-call returns and ABI preservation, valid memory and incoming return control, intact saved frame, nonaliasing and stable code remain conditions; referenced data, external callees and neighbors excluded; no observed entry, all-path feasibility, argument/string-index bounds, callee effects, rendering, C/ELF match, unique supplier or safe editing",
                },
                NontextRange {
                    offset: 73_728,
                    length: 388,
                    evidence: "original US START entry 8 or STATUS entry 0; complete 97-word instruction body 0x801E2C00..0x801E2D84 including all eight control-transfer delay words; complete original structural flow reconciles with native entry at 0x801E2C00, return address 0x801E285C and linked renderer calls/returns; original JAL 0x801E2854 and its A1=T0 delay bind the qualified caller; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; ordinary returning renderers, valid aligned memory and incoming return control, intact saved frame, nonaliasing writes and stable code/table remain conditions; count/pointer maps, settings records, renderers and neighbors excluded; no all-path feasibility, general selector/count bounds, transitive effects, resource safety, additional rendering, safe editing or unique-supplier claim",
                },
                NontextRange {
                    offset: 102_380,
                    length: 300,
                    evidence: "original US START entry 8 or STATUS entry 0; complete 75-word instruction body 0x801E9BEC..0x801E9D18 including all ten control-transfer delay words; complete original structural flow and retained ordinary Heal entry, lookup and renderer captures reconcile; observed ID 0x40D0 returns pointer 0x8001C9F7, stored at 0x801E9C84 before renderer JAL 0x801E9C9C; native renderer return 0x801E9CA4 restores the caller stack; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; ordinary returns from five direct calls and one opaque linking call, valid memory, intact frame, nonaliasing and stable-code/table assumptions; table, strings, referenced data, callees and neighbors excluded; no matching ELF, all-path execution, general selector bounds, resource safety, safe editing or unique-supplier claim",
                },
                NontextRange {
                    offset: 114_920,
                    length: 8,
                    evidence: "original US START entry 8 or STATUS entry 0; two little-endian numeric handler pointers at 0x801ECCE8..0x801ECCF0 contain 0x801E18D0 and 0x801E1C20; complete original dispatcher 0x801E1680..0x801E16BC loads table 0x801ECCE4 indexed by unsigned phase; retained phase-1/phase-2 entries have return address 0x801E16AC matching its JALR and NOP delay; original words and retained RAM agree under exact archive and payload identities; stable code/table, valid memory/control, intact frame, nonaliasing and ordinary return remain conditions; numeric storage only, excluding other phase cells, general phase bounds, actual dispatcher instruction retirement, transitive effects, rendering and safe editing",
                },
                NontextRange {
                    offset: 115_116,
                    length: 6,
                    evidence: "original US START entry 8 or STATUS entry 0; six u8 record-start indices at 0x801ECDAC..0x801ECDB2 contain 0,3,7,11,13,15; complete original settings reader 0x801E2C00..0x801E2D84 loads the selected byte at 0x801E2C4C, scales by eight and adds record base 0x801ECD24; retained original reader groups 0..5, source bytes, code and RAM agree under exact archive and payload identities; stable code/maps/records, valid memory, intact frame, nonaliasing and ordinary returns remain conditions; numeric storage only, excluding unconsumed neighboring byte pairs, record text/tails, Manual/Auto consumption, general selector bounds, retired loads, new rendering and safe editing",
                },
                NontextRange {
                    offset: 115_124,
                    length: 6,
                    evidence: "original US START entry 8 or STATUS entry 0; six u8 record counts at 0x801ECDB4..0x801ECDBA contain 3,4,4,2,2,0; complete original settings reader 0x801E2C00..0x801E2D84 loads these at 0x801E2C64 and 0x801E2D3C to bound its stride-eight record loop; retained original reader groups 0..5, source bytes, code and RAM agree, with group 5 exiting before record loads, under exact archive and payload identities; stable code/maps/records, valid memory, intact frame, nonaliasing and ordinary returns remain conditions; numeric storage only, excluding unconsumed neighboring byte pairs, record text/tails, Manual/Auto consumption, general selector bounds, retired loads, new rendering and safe editing",
                },
            ]);
            for index in 0..15 {
                ranges.push(NontextRange {
                    offset: 114_980 + 8 * index,
                    length: 2,
                    evidence: "original US START entry 8 or STATUS entry 0; first fifteen eight-byte settings records at 0x801ECD24; each first byte is a u8 glyph limit and each second byte is a signed horizontal offset, loaded and sign-extended by complete original reader 0x801E2C00..0x801E2D84 before both draw branches; six original start/count pairs select exactly these fifteen records; retained original draw arguments reproduce all fifteen counts and signed-offset centering calculations, and source bytes, code and RAM agree under exact archive and payload identities; stable code/maps/records, valid memory, intact frame, nonaliasing and ordinary returns remain conditions; only these two numeric bytes per record qualify; text, tails, Manual/Auto headers, retired loads, glyph reads, additional rendering and safe editing excluded",
                });
            }
            Some(ranges)
        }
        (0, 2_048, 0, 0x801e_ec00, 15_696)
            if entry.check_word == 0x18
                && sha256_hex(data)
                    == "27e06df900c8809cc941f2086f0126cff1c951039ad6e63d6e3c21c6fd708f7a" =>
        {
            Some(vec![NontextRange {
                offset: 10_744,
                length: 64,
                evidence: "original US COMMU00 entry 0; complete 16-word instruction body 0x801F15F8..0x801F1638 including call and return delay words; every original word, complete structural flow, both load delays, restored 24-byte frame, existing sized 64-byte executable ELF function sections and all seven reconstructed object text relocations agree; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; ordinary external-call return and ABI preservation, valid incoming task memory and return control, intact frame, nonaliasing and stable code remain conditions; referenced data, external helper and neighbors excluded; no fresh source build, resident-byte match, observed entry, actual directive, ordinary menu route, callback lifetime, helper effects, argument/resource safety, rendering, safe editing or unique-supplier claim",
            }])
        }
        (0, 2_048, 0, 0x8019_5800, 229_720)
            if entry.check_word == 0x1e
                && sha256_hex(data)
                    == "1663ff2e78134fe15f3cae6fb4074acf14b68b79cfa02be0c7651eaf6a1728ff" =>
        {
            Some(vec![
                NontextRange {
                    offset: 0xa5c,
                    length: 704,
                    evidence: US_FOLLOWUP_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0xf84,
                    length: 312,
                    evidence: US_FOLLOWUP_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0xc0c8,
                    length: 348,
                    evidence: US_FOLLOWUP_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x2cc38,
                    length: 152,
                    evidence: US_FOLLOWUP_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x2d094,
                    length: 268,
                    evidence: US_FOLLOWUP_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x2d1a0,
                    length: 268,
                    evidence: US_FOLLOWUP_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 4,
                    length: 20,
                    evidence: GAME_SLOT_TABLE_EVIDENCE,
                },
                NontextRange {
                    offset: 0xd1c,
                    length: 340,
                    evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0xe70,
                    length: 168,
                    evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x3ab8,
                    length: 80,
                    evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0xad14,
                    length: 264,
                    evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0xaed8,
                    length: 1036,
                    evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x19b90,
                    length: 260,
                    evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x19c94,
                    length: 284,
                    evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x19db0,
                    length: 592,
                    evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x1a000,
                    length: 304,
                    evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x1a130,
                    length: 236,
                    evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x1a21c,
                    length: 756,
                    evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x1a510,
                    length: 264,
                    evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x2965c,
                    length: 704,
                    evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x2991c,
                    length: 812,
                    evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x2da04,
                    length: 152,
                    evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x77c,
                    length: 20,
                    evidence: US_DIRECT_CALL_TABLE_EVIDENCE,
                },
                NontextRange {
                    offset: 3_864,
                    length: 108,
                    evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 5_020,
                    length: 240,
                    evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 8_868,
                    length: 376,
                    evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 9_244,
                    length: 120,
                    evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 9_364,
                    length: 448,
                    evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 15_256,
                    length: 88,
                    evidence: "Original US GAME entry 0, complete 22-word instruction body 0x80199398..0x801993F0. All original ASM words, local control flow, delay slots and restored frame agree with original archive and retained RAM. The complete 88-byte AX text and sized function in the retained relocatable object reproduce all original words after its eight R_MIPS_26 relocations. JAL 0x801993D0 with NOP delay conditionally calls SLUS 0x80158C80. Exact archive and payload identities gate classification. Stable code, valid nonaliasing memory, intact frames and ordinary callee returns remain conditions. Callees and referenced data are excluded. No fresh build, linked-executable match, historical execution, resource-safety, rendering or editing claim follows.",
                },
                NontextRange {
                    offset: 15_424,
                    length: 280,
                    evidence: "Original US GAME entry 0, complete 70-word instruction body 0x80199440..0x80199558. Every original ASM word, local branch, four calls, return and all ten delay slots agree with the original archive and retained RAM. Original AREA030 entry-4 JAL 0x801E0BD8 and its NOP delay conditionally enter this body. The 24-byte frame and RA are restored on return. Exact original archive and entry identities gate classification. Ordinary callee returns and preservation, valid memory and incoming return control, intact nonaliasing frames and stable code remain conditions. Referenced actors, state, tables, callees and neighbors are excluded. No observed entry, selector production, callee effects, resource safety, rendering, editing or ELF match is established.",
                },
                NontextRange {
                    offset: 18_660,
                    length: 176,
                    evidence: "Original US GAME entry 0, complete 44-word instruction body 0x8019A0E4..0x8019A194. Original AREA030 entry-4 JAL 0x801E0C00 and its NOP delay conditionally enter this updater. Every original ASM word, both local branches, indirect call, return and all four delay slots agree with the original archive and retained RAM. The 32-byte frame and S0/S1/S2/RA are restored on return. Exact original archive and entry identities gate classification. Ordinary callee returns and preservation, valid selected records, table and return control, intact disjoint frame and stable code remain conditions. Record fields, table cells, selected handlers and neighbors are excluded. No observed entry, selector bound, callee effects, resource safety, rendering, editing or ELF match is established.",
                },
                NontextRange {
                    offset: 49_056,
                    length: 296,
                    evidence: "Original US GAME entry 0, complete 74-word instruction body 0x801A17A0..0x801A18C8. Every original ASM word, local control path, delay slot and restored 32-byte frame agrees with original archive and retained RAM bytes. Original JAL 0x801A0790 passes actor+0x74 in its delay, so this reader's input+8 aliases actor+0x7C. Ordinary callee returns and ABI preservation, valid memory and return control, intact frame, nonaliasing and stable code/data remain conditions. The exact original archive and entry identities gate classification. Caller prefix, event records, tables, scripts, callees and neighbors are excluded. No invocation, event-trigger, lifetime, resource-safety, rendering, editing or ELF-match claim.",
                },
                NontextRange {
                    offset: 61_716,
                    length: 124,
                    evidence: "Original US GAME entry 0, complete 31-word instruction body 0x801A4914..0x801A4990. Original constructor JALs 0x801A5090 and 0x801A53D0 conditionally enter it. All ASM words, branches, calls, delay slots and the restored 24-byte frame agree with original archive and retained RAM bytes. Ordinary callee returns and ABI preservation, valid scratch-selected work, memory and return control, intact frame, nonaliasing and stable code/data remain conditions. The exact original archive and entry identities gate classification. Work fields, external helpers and neighbors are excluded. No invocation, transitive effects, lifetime, resource-safety, rendering, editing or ELF-match claim.",
                },
                NontextRange {
                    offset: 61_840,
                    length: 280,
                    evidence: "Original US GAME entry 0, complete 70-word instruction body 0x801A4990..0x801A4AA8. Original constructor JALs 0x801A5018 and 0x801A5358 pass record+0x0D in their delay slots. All ASM words, branches, internal jump, division traps, return and delay slots agree with original archive and retained RAM bytes. This leaf preserves SP and RA. Valid record, work and table memory, return control, nonaliasing and stable code/data remain conditions. The exact original archive and entry identities gate classification. Referenced fields, table and neighbors are excluded. No invocation, divisor validity, lifetime, resource-safety, rendering, editing or ELF-match claim.",
                },
                NontextRange {
                    offset: 62_120,
                    length: 216,
                    evidence: "Original US GAME entry 0, complete 54-word instruction body 0x801A4AA8..0x801A4B80. Original constructor JALs 0x801A4DC8 and 0x801A5134 conditionally enter it with NOP delays. All ASM words and the return delay agree with original archive and retained RAM bytes. This straight-line leaf preserves SP and RA. Valid scratch-selected work and return control, nonaliasing and stable code/data remain conditions. The exact original archive and entry identities gate classification. Work fields and neighbors are excluded. No invocation, complete field initialization, lifetime, resource-safety, rendering, editing or ELF-match claim.",
                },
                NontextRange {
                    offset: 62_804,
                    length: 876,
                    evidence: "Original US GAME entry 0, complete 219-word instruction body 0x801A4D54..0x801A50C0. The original ordinary-record dispatcher's type-0 arm calls it at 0x801A4D18 with A0=S0 in the delay. All ASM words, local branches, five calls, return, delay slots and restored 32-byte frame agree with original archive and retained RAM bytes. Ordinary callee returns and ABI preservation, a valid nonnegative slot below 30, readable record, writable work and scratch storage, intact frame, nonaliasing and stable code/data remain conditions. The exact original archive and entry identities gate classification. Records, work fields, tables, callees and neighbors are excluded. No invocation, complete field initialization, lifetime, resource-safety, rendering, editing or ELF-match claim.",
                },
                NontextRange {
                    offset: 63_680,
                    length: 832,
                    evidence: "Original US GAME entry 0, complete 208-word instruction body 0x801A50C0..0x801A5400. The original ordinary-record dispatcher's type-1 arm calls it at 0x801A4D08 with A0=S0 in the delay. All ASM words, local branches, five calls, return, delay slots and restored 32-byte frame agree with original archive and retained RAM bytes. Ordinary callee returns and ABI preservation, a valid nonnegative slot below 30, readable record, writable work and scratch storage, intact frame, nonaliasing and stable code/data remain conditions. The exact original archive and entry identities gate classification. Records, work fields, tables, callees and neighbors are excluded. No invocation, complete field initialization, lifetime, resource-safety, rendering, editing or ELF-match claim.",
                },
                NontextRange {
                    offset: 69_024,
                    length: 800,
                    evidence: "Original US GAME entry 0, complete 200-word instruction body 0x801A65A0..0x801A68C0. The original ordinary-record dispatcher's type-6 arm calls it at 0x801A4C98 with A0=S0 in the delay. Every original ASM word, local branch, five calls, return and nine delay slots agrees with original archive and retained RAM bytes. The 32-byte frame and S0/S1/RA are restored on return. The exact original archive and entry identities gate classification. A valid nonnegative slot below 30, valid record/work/scratch memory, ordinary callee returns and ABI preservation, intact disjoint frame, nonaliasing and stable code/data remain conditions. Records, work fields, tables, callees and neighbors are excluded. No invocation, complete field initialization, callback producer, lifetime, resource safety, rendering, editing or ELF-match claim.",
                },
                NontextRange {
                    offset: 71_512,
                    length: 856,
                    evidence: "Original US GAME entry 0, complete 214-word instruction body 0x801A6F58..0x801A72B0. The original ordinary-record dispatcher's type-5 arm calls it at 0x801A4C68 with A0=S0 in the delay. Every original ASM word, local branch, five calls, return and nine delay slots agrees with original archive and retained RAM bytes. The 32-byte frame and S0/S1/RA are restored on return. The exact original archive and entry identities gate classification. A valid nonnegative slot below 30, valid record/work/scratch memory, ordinary callee returns and ABI preservation, intact disjoint frame, nonaliasing and stable code/data remain conditions. Records, work fields, tables, callees and neighbors are excluded. No invocation, complete field initialization, callback producer, lifetime, resource safety, rendering, editing or ELF-match claim.",
                },
                NontextRange {
                    offset: 72_424,
                    length: 72,
                    evidence: "original US GAME entry 0; complete 18-word instruction body 0x801A72E8..0x801A7330 including all three control-transfer delays; complete original ASM, structural flow and load scheduling reconcile with retained RAM; qualified original ordinary-record call at 0x801A3F80 enters dispatcher 0x801A4BF8, whose high-nibble-D arm calls this setter at 0x801A4C48 with A0=S0 in its delay; the call-free body preserves SP, RA and saved registers; exact original archive, entry, offset, raw mode, load, checkword, size and payload identities; valid incoming record and current-work memory, return control, nonaliasing and stable code/data remain conditions; input/work fields, tables, caller bodies and neighbors excluded; no actual D/nonzero/0C directive, eligible task, callback lifetime, observed entry, ordinary route, matching ELF, argument/resource safety, rendering, safe editing or unique-supplier claim",
                },
                NontextRange {
                    offset: 72_496,
                    length: 980,
                    evidence: "Original US GAME entry 0, complete 245-word instruction body 0x801A7330..0x801A7704. The original ordinary-record dispatcher's type-9 arm calls it at 0x801A4CB8 with A0=S0 in the delay. Every original ASM word, local branch, two internal jumps, six calls, return and seventeen delay slots agrees with original archive and retained RAM bytes. The 32-byte frame and S0/S1/S2/RA are restored on return. The exact original archive and entry identities gate classification. A valid nonnegative slot below 30, valid record/work/scratch memory, ordinary callee returns and ABI preservation, intact disjoint frame, nonaliasing and stable code/data remain conditions. The exact named record can take a gate-dependent inactive return without incrementing the slot counter. Records, work fields, tables, callees and neighbors are excluded. No invocation, complete field initialization, callback producer, lifetime, resource safety, rendering, editing or ELF-match claim.",
                },
                NontextRange {
                    offset: 105_072,
                    length: 48,
                    evidence: "original US GAME entry 0 complete leaf instruction body 0x801AF270..0x801AF2A0 including all three control-transfer delay slots; all twelve original ASM words, retained resident bytes, sized existing executable ELF function section and five text relocations agree; one qualified original JAL at 0x801AF2CC in body 0x801AF2A0..0x801AF390 conditionally enters this leaf; four structural blocks and both return paths preserve SP, RA and saved registers without memory loads or stores; exact original archive, raw mode, load, entry, size, checkword and payload identities; linked table symbols qualify relocation arithmetic only, with tables, returned-pointer validity and consumer safety excluded; stable executable bytes and valid incoming return control remain conditions; no fresh source build, observed invocation, runtime feasibility, resource safety, safe editing or source-semantic acceptance",
                },
                NontextRange {
                    offset: 105_120,
                    length: 240,
                    evidence: "original US GAME entry 0 complete instruction body 0x801AF2A0..0x801AF390 including all eight control-transfer delay slots; complete original ASM word and structural control-flow review, sized existing executable ELF function section and all six text relocations reconcile with original and captured resident bytes; nine qualified original JAL sites in shared battle body 0x800B1A74..0x800B1DAC conditionally enter this body; ordinary callee returns and preservation, valid memory, intact frames and nonaliasing stores remain conditions; exact original archive, raw mode, load, entry, size, checkword and payload identities; referenced tables, pointers, data, buffers and callees excluded; no fresh source build, observed invocation, feasible execution, argument-validity, callee-semantics, resource-safety, safe-editing or unique-supplier claim",
                },
                NontextRange {
                    offset: 108_792,
                    length: 116,
                    evidence: "Original US GAME entry 0, complete 29-word instruction body 0x801B00F8..0x801B016C. Original JAL 0x8019948C and its NOP delay conditionally enter this reader. Every original ASM word, local branch, two direct calls, opaque indirect call, return and all five delay slots agree with the original archive and retained RAM. The 24-byte frame and RA are restored on return. Exact original archive and entry identities gate classification. Ordinary callee returns and preservation, valid selected work and memory, intact nonaliasing frames and stable code and pointers remain conditions. Table cells, selected handlers, work fields and neighbors are excluded. No observed entry, selector bound, callee effects, resource safety, rendering, editing or ELF match is established.",
                },
                NontextRange {
                    offset: 109_344,
                    length: 76,
                    evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 154_296,
                    length: 1_080,
                    evidence: "original US GAME entry 0 complete 1080-byte instruction body 0x801BB2B8..0x801BB6F0; retained native PC 0x801BB52C and original caller/return words bind execution inside this body; all 270 original words, 50 conditional branches, 19 internal jumps, eight direct calls, sole JR RA and delay slots structurally covered; original and retained origin/endpoint RAM agree; exact original archive, raw mode, load, entry, size, checkword and payload identity; ordinary call-return/frame/nonalias/stable-code assumptions; neighboring functions and referenced data excluded; no matching ELF, native first-instruction or all-path execution, incoming-register validity, sampler semantics, argument/resource safety, rendering or editing claim",
                },
                NontextRange {
                    offset: 155_376,
                    length: 112,
                    evidence: "original US GAME entry 0 complete 112-byte instruction body 0x801BB6F0..0x801BB760; original JAL 0x801BB508 in the qualified body at 0x801BB2B8 conditionally enters this body; all 28 original words, five conditional branches, one internal jump, sole JR RA and seven delay slots structurally covered; original and retained RAM agree; exact original archive, raw mode, load, entry, size, checkword and payload identity; ordinary return and stable-code assumptions; neighboring functions and referenced data excluded; no matching ELF, observed helper entry or execution, argument or work validity, domain semantics, resource safety, rendering or editing claim",
                },
                NontextRange {
                    offset: 155_488,
                    length: 80,
                    evidence: "original US GAME entry 0 complete 80-byte instruction body 0x801BB760..0x801BB7B0; original JAL 0x801BB520 and retained paired native PCs 0x801BB77C/0x801BB52C with return addresses 0x801BB77C/0x801BB528 and restored 24-byte frame bind execution inside this body; all 20 original words, one direct call, one conditional branch, sole JR RA and three delay slots structurally covered; original and retained origin/endpoint RAM agree; exact original archive, raw mode, load, entry, size, checkword and payload identity; ordinary call-return/frame/nonalias/stable-code assumptions; neighboring functions and referenced data excluded; no matching ELF, native first-instruction or all-path execution, sampler semantics, argument/resource safety, rendering or editing claim",
                },
                NontextRange {
                    offset: 164_536,
                    length: 196,
                    evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 164_804,
                    length: 308,
                    evidence: "Original US GAME entry 0, complete 77-word instruction body 0x801BDBC4..0x801BDCF8. Original JAL 0x801B0124 and its NOP delay conditionally enter this call-free helper. Every original ASM word, local branch and jump, return and all nine delay slots agree with the original archive and retained RAM. The eight-byte frame is restored and RA and saved registers are unchanged. Exact original archive and entry identities gate classification. Valid readable actors and state, writable output, incoming return control, nonaliasing memory and stable code remain conditions. Actor fields, output storage and neighbors are excluded. No observed entry, argument validity, resource safety, rendering, editing or ELF match is established.",
                },
                NontextRange {
                    offset: 171_080,
                    length: 352,
                    evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 176_744,
                    length: 76,
                    evidence: "Original US GAME entry 0, complete 19-word dispatcher 0x801C0A68..0x801C0AB4. Original JAL 0x80199448 and its NOP delay conditionally enter this body. Every original ASM word, local branch, opaque indirect call, return and all three delay slots agree with the original archive and retained RAM. The 24-byte frame and RA are restored on return. Exact original archive and entry identities gate classification. Ordinary selected-callee return and preservation, valid memory and return control, intact nonaliasing frame and stable code and pointers remain conditions. Callback table, selector fields, selected callee and neighbors are excluded. No observed entry, selector bound, callback effects, resource safety, rendering, editing or ELF match is established.",
                },
                NontextRange {
                    offset: 178_772,
                    length: 428,
                    evidence: "Original US GAME entry 0, complete 107-word instruction body 0x801C1254..0x801C1400. Original type-6 constructor JAL 0x801A660C and its NOP delay conditionally enter this helper. Every original ASM word, local branch, two calls, return and five delay slots agrees with original archive and retained RAM bytes. The 32-byte frame and S0/S1/S2/RA are restored on return. The exact original archive and entry identities gate classification. Valid record/table/scratch-selected work and resource memory, ordinary callee returns and ABI preservation, intact disjoint frame, nonaliasing and stable code/data remain conditions. Referenced fields, tables, external helpers and neighbors are excluded. No invocation, lookup success, resource-pointer safety, transitive effects, callback producer, lifetime, rendering, editing or ELF-match claim.",
                },
                NontextRange {
                    offset: 179_200,
                    length: 560,
                    evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 183_608,
                    length: 472,
                    evidence: "original US GAME entry 0 complete 472-byte instruction body 0x801C2538..0x801C2710; original JAL 0x801C24D8 and retained native PCs 0x801C2628/0x801C2668 with RA 0x801C24E0 bind the executable caller; all 118 original words, eight conditional branches, two direct calls, three terminal BREAK guards, sole JR RA and delay slots structurally covered; original and retained before/after RAM agree; exact original archive, raw mode, load, entry, size, checkword and payload identity; ordinary call-return/frame/nonalias/stable-code assumptions; neighboring caller and referenced data excluded; no matching ELF, native first-instruction or all-path execution, argument/resource safety, state-0 invocation, rendering or editing claim",
                },
                NontextRange {
                    offset: 186_708,
                    length: 176,
                    evidence: "Original US GAME entry 0, complete 44-word instruction body 0x801C3154..0x801C3204. Original JAL 0x801B0100 and its NOP delay conditionally enter this call-free helper. Every original ASM word, both local branches, return and all three delay slots agree with the original archive and retained RAM. SP, RA and saved registers are unchanged. Exact original archive and entry identities gate classification. Valid selected work, readable source and writable palette and state memory, incoming return control, nonaliasing and stable code remain conditions. Palette storage, actor fields and neighbors are excluded. No observed entry, argument validity, resource safety, rendering, editing or ELF match is established.",
                },
            ])
        }
        (_, _, 0, 0x801d_0c00, 118_224)
            if sha256_hex(data)
                == "f25ef137618bc024a6aec98c2c4d9a571c6f23884762fb4567406e6552b383c1" =>
        {
            Some(vec![
                NontextRange {
                    offset: 0xbed8,
                    length: 472,
                    evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x12230,
                    length: 648,
                    evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x344,
                    length: 20,
                    evidence: BATTLE_RESIDENT_TABLE_EVIDENCE,
                },
                NontextRange {
                    offset: 0x1a894,
                    length: 1,
                    evidence: BATTLE_RESIDENT_TABLE_EVIDENCE,
                },
                NontextRange {
                    offset: 0x1a897,
                    length: 2,
                    evidence: BATTLE_RESIDENT_TABLE_EVIDENCE,
                },
                NontextRange {
                    offset: 0x1a89a,
                    length: 1,
                    evidence: BATTLE_RESIDENT_TABLE_EVIDENCE,
                },
                NontextRange {
                    offset: 0x320,
                    length: 12,
                    evidence: "original US battle payload; 0x801D0F20..0x801D0F2C contains 3 numeric function-address words; qualified original dispatcher copies every word to the stack, indexes it by unsigned work byte +1 and calls the selected address at 0x801E6408; finite selector alternatives 0..2 bind the original target words; complete original words, consumer instructions and captured resident bytes reconcile; overall selector bounds, current-work validity and actual invocation remain unqualified; exact archive and payload identities; surrounding header and referenced data excluded; stored numeric representation only",
                },
                NontextRange {
                    offset: 0x3d0,
                    length: 16,
                    evidence: "original US battle payload; 0x801D0FD0..0x801D0FE0 contains 4 numeric function-address words; qualified original dispatcher copies every word to the stack, indexes it by unsigned work byte +3 and calls the selected address at 0x801E9660; finite selector alternatives 0..3 bind the original target words; complete original words, consumer instructions and captured resident bytes reconcile; overall selector bounds, current-work validity and actual invocation remain unqualified; exact archive and payload identities; surrounding header and referenced data excluded; stored numeric representation only",
                },
                NontextRange {
                    offset: 0x2c44,
                    length: 376,
                    evidence: "original US battle payload; complete instruction body 0x801D3844..0x801D39BC including return delay slot; complete original control-flow review, sized executable ELF function section and all 26 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x3c50,
                    length: 624,
                    evidence: "original US battle payload; complete instruction body 0x801D4850..0x801D4AC0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 54 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, battler-index-validity, record-validity, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x4144,
                    length: 708,
                    evidence: "original US battle payload; complete instruction body 0x801D4D44..0x801D5008 including return delay slot; complete original control-flow review, sized executable ELF function section and all 34 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x48f8,
                    length: 352,
                    evidence: "original US battle payload; complete instruction body 0x801D54F8..0x801D5658 including return delay slot; complete original control-flow review, sized executable ELF function section and all 24 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x4a58,
                    length: 340,
                    evidence: "original US battle payload; complete instruction body 0x801D5658..0x801D57AC including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x4bac,
                    length: 352,
                    evidence: "original US battle payload; complete instruction body 0x801D57AC..0x801D590C including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x4d0c,
                    length: 340,
                    evidence: "original US battle payload; complete instruction body 0x801D590C..0x801D5A60 including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x4e60,
                    length: 352,
                    evidence: "original US battle payload; complete instruction body 0x801D5A60..0x801D5BC0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x4fc0,
                    length: 524,
                    evidence: "original US battle payload; complete instruction body 0x801D5BC0..0x801D5DCC including return delay slot; complete original control-flow review, sized executable ELF function section and all 41 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x58c4,
                    length: 168,
                    evidence: "original US battle payload; complete instruction body 0x801D64C4..0x801D656C including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x5b74,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801D6774..0x801D67B0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x5bb0,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801D67B0..0x801D67EC including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x5bec,
                    length: 44,
                    evidence: "original US battle payload; complete instruction body 0x801D67EC..0x801D6818 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x5e14,
                    length: 624,
                    evidence: "original US battle payload; complete instruction body 0x801D6A14..0x801D6C84 including return delay slot; complete original control-flow review, sized executable ELF function section and all 62 text relocations reconcile with original and captured resident bytes; indirect callback target and selector remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x6084,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801D6C84..0x801D6CC0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x60c0,
                    length: 208,
                    evidence: "original US battle payload; complete instruction body 0x801D6CC0..0x801D6D90 including return delay slot; complete original control-flow review, sized executable ELF function section and all 22 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x6190,
                    length: 84,
                    evidence: "original US battle payload; complete instruction body 0x801D6D90..0x801D6DE4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x61e4,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801D6DE4..0x801D6E20 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x6220,
                    length: 140,
                    evidence: "original US battle payload; complete instruction body 0x801D6E20..0x801D6EAC including return delay slot; complete original control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x62ac,
                    length: 64,
                    evidence: "original US battle payload; complete instruction body 0x801D6EAC..0x801D6EEC including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x62ec,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801D6EEC..0x801D6F28 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x6508,
                    length: 100,
                    evidence: "original US battle payload; complete instruction body 0x801D7108..0x801D716C including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x656c,
                    length: 56,
                    evidence: "original US battle payload; complete instruction body 0x801D716C..0x801D71A4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x65a4,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801D71A4..0x801D71E0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x65e0,
                    length: 84,
                    evidence: "original US battle payload; complete instruction body 0x801D71E0..0x801D7234 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x68d4,
                    length: 56,
                    evidence: "original US battle payload; complete instruction body 0x801D74D4..0x801D750C including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x690c,
                    length: 1_332,
                    evidence: "original US battle payload; complete original instruction body 0x801D750C..0x801D7A40 including return delay slot; original JAL at 0x801E91E4 in the already qualified executable caller at 0x801E91CC conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x6e40,
                    length: 720,
                    evidence: "original US battle payload; complete original instruction body 0x801D7A40..0x801D7D10 including return delay slot; original JAL at 0x801E9424 in the already qualified executable caller at 0x801E9390 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x7110,
                    length: 416,
                    evidence: "original US battle payload; complete instruction body 0x801D7D10..0x801D7EB0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 12 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x72b0,
                    length: 480,
                    evidence: "original US battle payload; complete instruction body 0x801D7EB0..0x801D8090 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x7490,
                    length: 480,
                    evidence: "original US battle payload; complete instruction body 0x801D8090..0x801D8270 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x7670,
                    length: 480,
                    evidence: "original US battle payload; complete instruction body 0x801D8270..0x801D8450 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x7850,
                    length: 576,
                    evidence: "original US battle payload; complete instruction body 0x801D8450..0x801D8690 including return delay slot; complete original control-flow review, sized executable ELF function section and all 21 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x7a90,
                    length: 1_108,
                    evidence: "original US battle payload; complete original instruction body 0x801D8690..0x801D8AE4 including return delay slot; original JAL at 0x801E9A98 in the already qualified executable caller at 0x801E99AC conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x7ee4,
                    length: 788,
                    evidence: "original US battle payload; complete instruction body 0x801D8AE4..0x801D8DF8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 26 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x81f8,
                    length: 1_292,
                    evidence: "original US battle payload; complete original instruction body 0x801D8DF8..0x801D9304 including return delay slot; original JAL at 0x801EA15C in the already qualified executable caller at 0x801E9FDC conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8704,
                    length: 132,
                    evidence: "original US battle payload; complete instruction body 0x801D9304..0x801D9388 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8788,
                    length: 92,
                    evidence: "original US battle payload; complete instruction body 0x801D9388..0x801D93E4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x87e4,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801D93E4..0x801D9428 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8828,
                    length: 92,
                    evidence: "original US battle payload; complete instruction body 0x801D9428..0x801D9484 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8884,
                    length: 80,
                    evidence: "original US battle payload; complete instruction body 0x801D9484..0x801D94D4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x88d4,
                    length: 432,
                    evidence: "original US battle payload; complete instruction body 0x801D94D4..0x801D9684 including return delay slot; complete original control-flow review, sized executable ELF function section and all 29 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8a84,
                    length: 384,
                    evidence: "original US battle payload; complete instruction body 0x801D9684..0x801D9804 including return delay slot; complete original control-flow review, sized executable ELF function section and all 23 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8c04,
                    length: 252,
                    evidence: "original US battle payload; complete instruction body 0x801D9804..0x801D9900 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8d00,
                    length: 172,
                    evidence: "original US battle payload; complete instruction body 0x801D9900..0x801D99AC including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8dac,
                    length: 264,
                    evidence: "original US battle payload; complete instruction body 0x801D99AC..0x801D9AB4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8eb4,
                    length: 460,
                    evidence: "original US battle payload; complete instruction body 0x801D9AB4..0x801D9C80 including return delay slot; complete original control-flow review, sized executable ELF function section and all 23 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x9080,
                    length: 316,
                    evidence: "original US battle payload; complete original instruction body 0x801D9C80..0x801D9DBC including return delay slot; original JAL at 0x801D75F4 in the already qualified executable caller at 0x801D750C conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x91bc,
                    length: 224,
                    evidence: "original US battle payload; complete original instruction body 0x801D9DBC..0x801D9E9C including return delay slot; original JAL at 0x801D768C in the already qualified executable caller at 0x801D750C conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x929c,
                    length: 476,
                    evidence: "original US battle payload; complete original instruction body 0x801D9E9C..0x801DA078 including return delay slot; original JAL at 0x801D8C70 in the already qualified executable caller at 0x801D8AE4 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x9478,
                    length: 212,
                    evidence: "original US battle payload; complete instruction body 0x801DA078..0x801DA14C including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x954c,
                    length: 476,
                    evidence: "original US battle payload; complete instruction body 0x801DA14C..0x801DA328 including return delay slot; complete original control-flow review, sized executable ELF function section and all 33 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x9728,
                    length: 224,
                    evidence: "original US battle payload; complete instruction body 0x801DA328..0x801DA408 including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x9808,
                    length: 172,
                    evidence: "original US battle payload; complete instruction body 0x801DA408..0x801DA4B4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x98b4,
                    length: 244,
                    evidence: "original US battle payload; complete instruction body 0x801DA4B4..0x801DA5A8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x99a8,
                    length: 244,
                    evidence: "original US battle payload; complete instruction body 0x801DA5A8..0x801DA69C including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x9a9c,
                    length: 312,
                    evidence: "original US battle payload; complete instruction body 0x801DA69C..0x801DA7D4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xa458,
                    length: 672,
                    evidence: "original US battle payload; complete instruction body 0x801DB058..0x801DB2F8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 18 text relocations reconcile with original and captured resident bytes; four BREAK exception paths retained; exact archive and payload identities; neighboring functions and data excluded; no observed execution, arithmetic-validity, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xa6f8,
                    length: 168,
                    evidence: "original US battle payload; complete instruction body 0x801DB2F8..0x801DB3A0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xa7a0,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801DB3A0..0x801DB3E4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xa7e4,
                    length: 80,
                    evidence: "original US battle payload; complete instruction body 0x801DB3E4..0x801DB434 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xa834,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801DB434..0x801DB494 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xa894,
                    length: 144,
                    evidence: "original US battle payload; complete instruction body 0x801DB494..0x801DB524 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xa924,
                    length: 168,
                    evidence: "original US battle payload; complete instruction body 0x801DB524..0x801DB5CC including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xa9cc,
                    length: 296,
                    evidence: "original US battle payload; complete instruction body 0x801DB5CC..0x801DB6F4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xaaf4,
                    length: 336,
                    evidence: "original US battle payload; complete instruction body 0x801DB6F4..0x801DB844 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xac44,
                    length: 416,
                    evidence: "original US battle payload; complete instruction body 0x801DB844..0x801DB9E4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 25 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xade4,
                    length: 316,
                    evidence: "original US battle payload; complete instruction body 0x801DB9E4..0x801DBB20 including return delay slot; complete original control-flow review, sized executable ELF function section and all 21 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xaf20,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801DBB20..0x801DBB78 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xaf78,
                    length: 1228,
                    evidence: "original US battle payload; complete instruction body 0x801DBB78..0x801DC044 including return delay slot; complete original control-flow review, sized executable ELF function section and all 91 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xb444,
                    length: 1_784,
                    evidence: "original US battle payload; complete original instruction body 0x801DC044..0x801DC73C including return delay slot; original JAL at 0x801DBBFC in the already qualified complete executable caller conditionally enters this body with a2=0xFFFF; complete original control-flow review and captured resident bytes reconcile; the compiled callee remains a partial mismatch and is not the admission basis; exact archive and payload identities; neighboring functions and data excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xbb3c,
                    length: 344,
                    evidence: "original US battle payload; complete original instruction body 0x801DC73C..0x801DC894 including return delay slot; original JAL at 0x801DC52C in the already qualified executable caller at 0x801DC044 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xbc94,
                    length: 580,
                    evidence: "original US battle payload; complete instruction body 0x801DC894..0x801DCAD8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 28 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc0b0,
                    length: 160,
                    evidence: "original US battle payload; complete instruction body 0x801DCCB0..0x801DCD50 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; two BREAK exception paths retained; exact archive and payload identities; neighboring functions and data excluded; no observed execution, arithmetic-validity, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc150,
                    length: 424,
                    evidence: "original US battle payload; complete instruction body 0x801DCD50..0x801DCEF8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 13 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc2f8,
                    length: 216,
                    evidence: "original US battle payload; complete instruction body 0x801DCEF8..0x801DCFD0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 13 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc3d0,
                    length: 188,
                    evidence: "original US battle payload; complete instruction body 0x801DCFD0..0x801DD08C including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc48c,
                    length: 192,
                    evidence: "original US battle payload; complete instruction body 0x801DD08C..0x801DD14C including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc54c,
                    length: 288,
                    evidence: "original US battle payload; complete instruction body 0x801DD14C..0x801DD26C including return delay slot; complete original control-flow review, sized executable ELF function section and all 23 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc66c,
                    length: 48,
                    evidence: "original US battle payload; complete instruction body 0x801DD26C..0x801DD29C including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc69c,
                    length: 180,
                    evidence: "original US battle payload; complete instruction body 0x801DD29C..0x801DD350 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc750,
                    length: 124,
                    evidence: "original US battle payload; complete instruction body 0x801DD350..0x801DD3CC including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc7cc,
                    length: 124,
                    evidence: "original US battle payload; complete instruction body 0x801DD3CC..0x801DD448 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc848,
                    length: 340,
                    evidence: "original US battle payload; complete instruction body 0x801DD448..0x801DD59C including return delay slot; complete original control-flow review, sized executable ELF function section and all 12 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc99c,
                    length: 360,
                    evidence: "original US battle payload; complete instruction body 0x801DD59C..0x801DD704 including return delay slot; complete original control-flow review, sized executable ELF function section and all 23 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xcb04,
                    length: 92,
                    evidence: "original US battle payload; complete instruction body 0x801DD704..0x801DD760 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xcb60,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801DD760..0x801DD7AC including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xcbac,
                    length: 44,
                    evidence: "original US battle payload; complete instruction body 0x801DD7AC..0x801DD7D8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xcbd8,
                    length: 40,
                    evidence: "original US battle payload; complete instruction body 0x801DD7D8..0x801DD800 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xcc58,
                    length: 84,
                    evidence: "original US battle payload; complete instruction body 0x801DD858..0x801DD8AC including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xccac,
                    length: 204,
                    evidence: "original US battle payload; complete instruction body 0x801DD8AC..0x801DD978 including return delay slot; complete original control-flow review, sized executable ELF function section and all 22 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xcd78,
                    length: 316,
                    evidence: "original US battle payload; complete instruction body 0x801DD978..0x801DDAB4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xceb4,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801DDAB4..0x801DDAF0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xcef0,
                    length: 140,
                    evidence: "original US battle payload; complete instruction body 0x801DDAF0..0x801DDB7C including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xcf7c,
                    length: 312,
                    evidence: "original US battle payload; complete instruction body 0x801DDB7C..0x801DDCB4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd0b4,
                    length: 456,
                    evidence: "original US battle payload; complete original instruction body 0x801DDCB4..0x801DDE7C including return delay slot; original JAL at 0x801D56BC in the already qualified executable caller at 0x801D5658 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd27c,
                    length: 92,
                    evidence: "original US battle payload; complete instruction body 0x801DDE7C..0x801DDED8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd2d8,
                    length: 40,
                    evidence: "original US battle payload; complete instruction body 0x801DDED8..0x801DDF00 including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd300,
                    length: 40,
                    evidence: "original US battle payload; complete instruction body 0x801DDF00..0x801DDF28 including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd328,
                    length: 40,
                    evidence: "original US battle payload; complete instruction body 0x801DDF28..0x801DDF50 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd350,
                    length: 156,
                    evidence: "original US battle payload; complete instruction body 0x801DDF50..0x801DDFEC including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd3ec,
                    length: 192,
                    evidence: "original US battle payload; complete instruction body 0x801DDFEC..0x801DE0AC including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd4ac,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801DE0AC..0x801DE0F8 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd4f8,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801DE0F8..0x801DE144 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd544,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801DE144..0x801DE190 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd590,
                    length: 32,
                    evidence: "original US battle payload; complete instruction body 0x801DE190..0x801DE1B0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd5b0,
                    length: 36,
                    evidence: "original US battle payload; complete instruction body 0x801DE1B0..0x801DE1D4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd83c,
                    length: 292,
                    evidence: "original US battle payload; complete instruction body 0x801DE43C..0x801DE560 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; indirect-call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd960,
                    length: 172,
                    evidence: "original US battle payload; complete instruction body 0x801DE560..0x801DE60C including return delay slot; complete original control-flow review, sized executable ELF function section and all 17 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xda0c,
                    length: 132,
                    evidence: "original US battle payload; complete instruction body 0x801DE60C..0x801DE690 including return delay slot; complete original control-flow review, sized executable ELF function section and all 16 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xda90,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801DE690..0x801DE698 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xda98,
                    length: 188,
                    evidence: "original US battle payload; complete instruction body 0x801DE698..0x801DE754 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdb54,
                    length: 168,
                    evidence: "original US battle payload; complete instruction body 0x801DE754..0x801DE7FC including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdbfc,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801DE7FC..0x801DE804 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdc04,
                    length: 84,
                    evidence: "original US battle payload; complete instruction body 0x801DE804..0x801DE858 including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdc58,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801DE858..0x801DE8C0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdcc0,
                    length: 108,
                    evidence: "original US battle payload; complete instruction body 0x801DE8C0..0x801DE92C including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdd2c,
                    length: 32,
                    evidence: "original US battle payload; complete instruction body 0x801DE92C..0x801DE94C including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdd4c,
                    length: 92,
                    evidence: "original US battle payload; complete instruction body 0x801DE94C..0x801DE9A8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdda8,
                    length: 112,
                    evidence: "original US battle payload; complete instruction body 0x801DE9A8..0x801DEA18 including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xde18,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801DEA18..0x801DEA64 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xde64,
                    length: 124,
                    evidence: "original US battle payload; complete instruction body 0x801DEA64..0x801DEAE0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdee0,
                    length: 228,
                    evidence: "original US battle payload; complete original instruction body 0x801DEAE0..0x801DEBC4 including return delay slot; original JAL at 0x801D6D60 in the already qualified executable caller at 0x801D6CC0 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdfc4,
                    length: 100,
                    evidence: "original US battle payload; complete instruction body 0x801DEBC4..0x801DEC28 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe0e0,
                    length: 116,
                    evidence: "original US battle payload; complete instruction body 0x801DECE0..0x801DED54 including return delay slot; complete original control-flow review, sized executable ELF function section and all 17 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe154,
                    length: 144,
                    evidence: "original US battle payload; complete instruction body 0x801DED54..0x801DEDE4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe1e4,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801DEDE4..0x801DEE4C including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe24c,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801DEE4C..0x801DEEB4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe2b4,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801DEEB4..0x801DEF0C including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe3dc,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801DEFDC..0x801DEFE4 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe3e4,
                    length: 872,
                    evidence: "original US battle payload; complete instruction body 0x801DEFE4..0x801DF34C including return delay slot; complete original control-flow review, sized executable ELF function section and all 70 text relocations reconcile with original and captured resident bytes; six internal switch targets reconciled, separate address table excluded; exact archive and payload identities; neighboring functions and data excluded; no observed execution, selector-validity, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe74c,
                    length: 108,
                    evidence: "original US battle payload; complete instruction body 0x801DF34C..0x801DF3B8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe7b8,
                    length: 64,
                    evidence: "original US battle payload; complete instruction body 0x801DF3B8..0x801DF3F8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe7f8,
                    length: 880,
                    evidence: "original US battle payload; complete instruction body 0x801DF3F8..0x801DF768 including return delay slot; complete original control-flow review, sized executable ELF function section and all 64 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; indirect-call table and selector remain unqualified; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xeb68,
                    length: 132,
                    evidence: "original US battle payload; complete instruction body 0x801DF768..0x801DF7EC including return delay slot; complete original control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xebec,
                    length: 192,
                    evidence: "original US battle payload; complete instruction body 0x801DF7EC..0x801DF8AC including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xecac,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801DF8AC..0x801DF914 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xed14,
                    length: 264,
                    evidence: "original US battle payload; complete instruction body 0x801DF914..0x801DFA1C including return delay slot; complete original control-flow review, sized executable ELF function section and all 29 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xee60,
                    length: 260,
                    evidence: "original US battle payload; complete instruction body 0x801DFA60..0x801DFB64 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xef64,
                    length: 120,
                    evidence: "original US battle payload; complete instruction body 0x801DFB64..0x801DFBDC including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 61_404,
                    length: 68,
                    evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0xf020,
                    length: 80,
                    evidence: "original US battle payload; complete instruction body 0x801DFC20..0x801DFC70 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xf070,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801DFC70..0x801DFCB4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xf59c,
                    length: 112,
                    evidence: "original US battle payload; complete instruction body 0x801E019C..0x801E020C including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xf60c,
                    length: 608,
                    evidence: "original US battle payload; complete instruction body 0x801E020C..0x801E046C including return delay slot; complete original control-flow review, sized executable ELF function section and all 49 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xf86c,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E046C..0x801E04B0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xf8b0,
                    length: 164,
                    evidence: "original US battle payload; complete instruction body 0x801E04B0..0x801E0554 including return delay slot; complete original control-flow review, sized executable ELF function section and all 16 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xf954,
                    length: 152,
                    evidence: "original US battle payload; complete instruction body 0x801E0554..0x801E05EC including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xf9ec,
                    length: 256,
                    evidence: "original US battle payload; complete instruction body 0x801E05EC..0x801E06EC including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xfaec,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801E06EC..0x801E0744 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xfb44,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E0744..0x801E0788 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xfb88,
                    length: 988,
                    evidence: "original US battle payload; complete instruction body 0x801E0788..0x801E0B64 including return delay slot; complete original control-flow review, sized executable ELF function section and all 88 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xff64,
                    length: 680,
                    evidence: "original US battle payload; complete instruction body 0x801E0B64..0x801E0E0C including return delay slot; complete original control-flow review, sized executable ELF function section and all 22 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1020c,
                    length: 256,
                    evidence: "original US battle payload; complete instruction body 0x801E0E0C..0x801E0F0C including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1030c,
                    length: 324,
                    evidence: "original US battle payload; complete instruction body 0x801E0F0C..0x801E1050 including return delay slot; complete original control-flow review, sized executable ELF function section and all 29 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10450,
                    length: 120,
                    evidence: "original US battle payload; complete instruction body 0x801E1050..0x801E10C8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x104c8,
                    length: 464,
                    evidence: "original US battle payload; complete instruction body 0x801E10C8..0x801E1298 including return delay slot; complete original control-flow review, sized executable ELF function section and all 40 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10698,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E1298..0x801E12DC including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x106dc,
                    length: 220,
                    evidence: "original US battle payload; complete instruction body 0x801E12DC..0x801E13B8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 12 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x107b8,
                    length: 152,
                    evidence: "original US battle payload; complete instruction body 0x801E13B8..0x801E1450 including return delay slot; complete original control-flow review, sized executable ELF function section and all 13 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10850,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E1450..0x801E1494 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10894,
                    length: 312,
                    evidence: "original US battle payload; complete instruction body 0x801E1494..0x801E15CC including return delay slot; complete original control-flow review, sized executable ELF function section and all 22 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x109cc,
                    length: 164,
                    evidence: "original US battle payload; complete instruction body 0x801E15CC..0x801E1670 including return delay slot; complete original control-flow review, sized executable ELF function section and all 17 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10ab4,
                    length: 468,
                    evidence: "original US battle payload; complete instruction body 0x801E16B4..0x801E1888 including return delay slot; complete original control-flow review, sized executable ELF function section and all 42 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10c88,
                    length: 176,
                    evidence: "original US battle payload; complete instruction body 0x801E1888..0x801E1938 including return delay slot; complete original control-flow review, sized executable ELF function section and all 12 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10d38,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801E1938..0x801E1990 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10ddc,
                    length: 56,
                    evidence: "original US battle payload; complete instruction body 0x801E19DC..0x801E1A14 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10e14,
                    length: 200,
                    evidence: "original US battle payload; complete instruction body 0x801E1A14..0x801E1ADC including return delay slot; complete original control-flow review, sized executable ELF function section and all 18 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10edc,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801E1ADC..0x801E1AE4 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10ee4,
                    length: 32,
                    evidence: "original US battle payload; complete instruction body 0x801E1AE4..0x801E1B04 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10f04,
                    length: 40,
                    evidence: "original US battle payload; complete instruction body 0x801E1B04..0x801E1B2C including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10f2c,
                    length: 48,
                    evidence: "original US battle payload; complete instruction body 0x801E1B2C..0x801E1B5C including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10f5c,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801E1B5C..0x801E1B64 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10fa8,
                    length: 176,
                    evidence: "original US battle payload; complete instruction body 0x801E1BA8..0x801E1C58 including return delay slot; complete original control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11058,
                    length: 32,
                    evidence: "original US battle payload; complete instruction body 0x801E1C58..0x801E1C78 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11078,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E1C78..0x801E1CD8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1111c,
                    length: 108,
                    evidence: "original US battle payload; complete instruction body 0x801E1D1C..0x801E1D88 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11188,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801E1D88..0x801E1DD4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x111d4,
                    length: 168,
                    evidence: "original US battle payload; complete instruction body 0x801E1DD4..0x801E1E7C including return delay slot; complete original control-flow review, sized executable ELF function section and all 13 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x112c0,
                    length: 356,
                    evidence: "original US battle payload; complete instruction body 0x801E1EC0..0x801E2024 including return delay slot; complete original control-flow review, sized executable ELF function section and all 31 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11424,
                    length: 252,
                    evidence: "original US battle payload; complete instruction body 0x801E2024..0x801E2120 including return delay slot; complete original control-flow review, sized executable ELF function section and all 13 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11520,
                    length: 80,
                    evidence: "original US battle payload; complete instruction body 0x801E2120..0x801E2170 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11570,
                    length: 276,
                    evidence: "original US battle payload; complete instruction body 0x801E2170..0x801E2284 including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; indirect-call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11684,
                    length: 144,
                    evidence: "original US battle payload; complete instruction body 0x801E2284..0x801E2314 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11714,
                    length: 484,
                    evidence: "original US battle payload; complete instruction body 0x801E2314..0x801E24F8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 32 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x118f8,
                    length: 132,
                    evidence: "original US battle payload; complete instruction body 0x801E24F8..0x801E257C including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1197c,
                    length: 100,
                    evidence: "original US battle payload; complete instruction body 0x801E257C..0x801E25E0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x119e0,
                    length: 872,
                    evidence: "original US battle payload; complete instruction body 0x801E25E0..0x801E2948 including return delay slot; complete original control-flow review, sized executable ELF function section and all 63 text relocations reconcile with original and captured resident bytes; indirect callback target and selector remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11d48,
                    length: 108,
                    evidence: "original US battle payload; complete instruction body 0x801E2948..0x801E29B4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11db4,
                    length: 212,
                    evidence: "original US battle payload; complete instruction body 0x801E29B4..0x801E2A88 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; two BREAK exception paths retained; exact archive and payload identities; neighboring functions and data excluded; no observed execution, arithmetic-validity, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11e88,
                    length: 540,
                    evidence: "original US battle payload; complete original instruction body 0x801E2A88..0x801E2CA4 including return delay slot; original JAL at 0x801E2860 in the already qualified executable caller at 0x801E25E0 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x120a4,
                    length: 168,
                    evidence: "original US battle payload; complete instruction body 0x801E2CA4..0x801E2D4C including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1214c,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E2D4C..0x801E2D90 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12190,
                    length: 160,
                    evidence: "original US battle payload; complete instruction body 0x801E2D90..0x801E2E30 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x124b8,
                    length: 64,
                    evidence: "original US battle payload; complete instruction body 0x801E30B8..0x801E30F8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x124f8,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801E30F8..0x801E3160 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12560,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801E3160..0x801E31C8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1260c,
                    length: 160,
                    evidence: "original US battle payload; complete instruction body 0x801E320C..0x801E32AC including return delay slot; complete original control-flow review, sized executable ELF function section and all 20 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x126f0,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E32F0..0x801E3334 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12734,
                    length: 100,
                    evidence: "original US battle payload; complete instruction body 0x801E3334..0x801E3398 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12798,
                    length: 160,
                    evidence: "original US battle payload; complete instruction body 0x801E3398..0x801E3438 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12838,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E3438..0x801E347C including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1287c,
                    length: 176,
                    evidence: "original US battle payload; complete instruction body 0x801E347C..0x801E352C including return delay slot; complete original control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1292c,
                    length: 172,
                    evidence: "original US battle payload; complete instruction body 0x801E352C..0x801E35D8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x129d8,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E35D8..0x801E3638 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12a38,
                    length: 516,
                    evidence: "original US battle payload; complete instruction body 0x801E3638..0x801E383C including return delay slot; complete original control-flow review, sized executable ELF function section and all 35 text relocations reconcile with original and captured resident bytes; indirect callback target and selector remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12c3c,
                    length: 132,
                    evidence: "original US battle payload; complete instruction body 0x801E383C..0x801E38C0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12cc0,
                    length: 192,
                    evidence: "original US battle payload; complete instruction body 0x801E38C0..0x801E3980 including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12d80,
                    length: 128,
                    evidence: "original US battle payload; complete instruction body 0x801E3980..0x801E3A00 including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12e44,
                    length: 192,
                    evidence: "original US battle payload; complete instruction body 0x801E3A44..0x801E3B04 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12f04,
                    length: 100,
                    evidence: "original US battle payload; complete instruction body 0x801E3B04..0x801E3B68 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12fac,
                    length: 36,
                    evidence: "original US battle payload; complete instruction body 0x801E3BAC..0x801E3BD0 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12fd0,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E3BD0..0x801E3C14 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13504,
                    length: 72,
                    evidence: "original US battle payload; complete instruction body 0x801E4104..0x801E414C including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1354c,
                    length: 540,
                    evidence: "original US battle payload; complete instruction body 0x801E414C..0x801E4368 including return delay slot; complete original control-flow review, sized executable ELF function section and all 47 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13768,
                    length: 296,
                    evidence: "original US battle payload; complete instruction body 0x801E4368..0x801E4490 including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13890,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E4490..0x801E44D4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x138d4,
                    length: 112,
                    evidence: "original US battle payload; complete instruction body 0x801E44D4..0x801E4544 including return delay slot; complete original control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13944,
                    length: 152,
                    evidence: "original US battle payload; complete instruction body 0x801E4544..0x801E45DC including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x139dc,
                    length: 272,
                    evidence: "original US battle payload; complete instruction body 0x801E45DC..0x801E46EC including return delay slot; complete original control-flow review, sized executable ELF function section and all 22 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13aec,
                    length: 116,
                    evidence: "original US battle payload; complete instruction body 0x801E46EC..0x801E4760 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13b60,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E4760..0x801E47A4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13ba4,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801E47A4..0x801E47E0 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13be0,
                    length: 92,
                    evidence: "original US battle payload; complete instruction body 0x801E47E0..0x801E483C including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13c3c,
                    length: 152,
                    evidence: "original US battle payload; complete instruction body 0x801E483C..0x801E48D4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13cd4,
                    length: 84,
                    evidence: "original US battle payload; complete instruction body 0x801E48D4..0x801E4928 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13d28,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E4928..0x801E496C including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13d6c,
                    length: 228,
                    evidence: "original US battle payload; complete instruction body 0x801E496C..0x801E4A50 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13e50,
                    length: 152,
                    evidence: "original US battle payload; complete instruction body 0x801E4A50..0x801E4AE8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 16 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13f2c,
                    length: 336,
                    evidence: "original US battle payload; complete instruction body 0x801E4B2C..0x801E4C7C including return delay slot; complete original control-flow review, sized executable ELF function section and all 28 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1407c,
                    length: 184,
                    evidence: "original US battle payload; complete instruction body 0x801E4C7C..0x801E4D34 including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14134,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801E4D34..0x801E4D8C including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x141d8,
                    length: 56,
                    evidence: "original US battle payload; complete instruction body 0x801E4DD8..0x801E4E10 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14210,
                    length: 188,
                    evidence: "original US battle payload; complete instruction body 0x801E4E10..0x801E4ECC including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x142cc,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801E4ECC..0x801E4F34 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14334,
                    length: 48,
                    evidence: "original US battle payload; complete instruction body 0x801E4F34..0x801E4F64 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x143a8,
                    length: 280,
                    evidence: "original US battle payload; complete instruction body 0x801E4FA8..0x801E50C0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 27 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x144c0,
                    length: 284,
                    evidence: "original US battle payload; complete instruction body 0x801E50C0..0x801E51DC including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x145dc,
                    length: 172,
                    evidence: "original US battle payload; complete instruction body 0x801E51DC..0x801E5288 including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14688,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E5288..0x801E52E8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x146e8,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801E52E8..0x801E52F0 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x146f0,
                    length: 44,
                    evidence: "original US battle payload; complete instruction body 0x801E52F0..0x801E531C including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1471c,
                    length: 464,
                    evidence: "original US battle payload; complete instruction body 0x801E531C..0x801E54EC including return delay slot; complete original control-flow review, sized executable ELF function section and all 24 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x148ec,
                    length: 400,
                    evidence: "original US battle payload; complete original instruction body 0x801E54EC..0x801E567C including return delay slot; original JAL at 0x801E4910 in the already qualified executable caller at 0x801E48D4 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14a7c,
                    length: 136,
                    evidence: "original US battle payload; complete instruction body 0x801E567C..0x801E5704 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14b04,
                    length: 100,
                    evidence: "original US battle payload; complete instruction body 0x801E5704..0x801E5768 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14b68,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E5768..0x801E57C8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14bc8,
                    length: 92,
                    evidence: "original US battle payload; complete instruction body 0x801E57C8..0x801E5824 including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14c24,
                    length: 232,
                    evidence: "original US battle payload; complete instruction body 0x801E5824..0x801E590C including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; indirect-call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14d0c,
                    length: 124,
                    evidence: "original US battle payload; complete instruction body 0x801E590C..0x801E5988 including return delay slot; complete original control-flow review, sized executable ELF function section and all nine text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, allocator-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14d88,
                    length: 176,
                    evidence: "original US battle payload; complete instruction body 0x801E5988..0x801E5A38 including the store in its return delay slot; complete original control-flow review, sized executable ELF function section and all 22 text relocations reconcile with original and captured resident bytes; conditional entry through qualified caller 0x800AE09C; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, work-record-validity, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14e38,
                    length: 180,
                    evidence: "original US battle payload; complete instruction body 0x801E5A38..0x801E5AEC including return delay slot; complete original control-flow review, sized executable ELF function section and all 22 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14eec,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801E5AEC..0x801E5AF4 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14ef4,
                    length: 152,
                    evidence: "original US battle payload; complete instruction body 0x801E5AF4..0x801E5B8C including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14f8c,
                    length: 144,
                    evidence: "original US battle payload; complete instruction body 0x801E5B8C..0x801E5C1C including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1501c,
                    length: 136,
                    evidence: "original US battle payload; complete instruction body 0x801E5C1C..0x801E5CA4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x150a4,
                    length: 392,
                    evidence: "original US battle payload; complete instruction body 0x801E5CA4..0x801E5E2C including return delay slot; complete original control-flow review, sized executable ELF function section and all 25 text relocations reconcile with original and captured resident bytes; five internal switch targets reconciled, separate address table excluded; indirect callback target and selector remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15488,
                    length: 84,
                    evidence: "original US battle payload; complete instruction body 0x801E6088..0x801E60DC including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x154dc,
                    length: 120,
                    evidence: "original US battle payload; complete instruction body 0x801E60DC..0x801E6154 including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15554,
                    length: 156,
                    evidence: "original US battle payload; complete instruction body 0x801E6154..0x801E61F0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x155f0,
                    length: 204,
                    evidence: "original US battle payload; complete instruction body 0x801E61F0..0x801E62BC including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x156bc,
                    length: 260,
                    evidence: "original US battle payload; complete instruction body 0x801E62BC..0x801E63C0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x157c0,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E63C0..0x801E6420 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15820,
                    length: 148,
                    evidence: "original US battle payload; complete instruction body 0x801E6420..0x801E64B4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x158b4,
                    length: 324,
                    evidence: "original US battle payload; complete original instruction body 0x801E64B4..0x801E65F8 including return delay slot; selector 1 through original numeric function-address table 0x801D0F20 and qualified original JALR at 0x801E6408 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; no matching callee ELF is claimed; overall selector bounds and current-work validity remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x159f8,
                    length: 300,
                    evidence: "original US battle payload; complete original instruction body 0x801E65F8..0x801E6724 including return delay slot; selector 2 through original numeric function-address table 0x801D0F20 and qualified original JALR at 0x801E6408 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; no matching callee ELF is claimed; overall selector bounds and current-work validity remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15b24,
                    length: 120,
                    evidence: "original US battle payload; complete instruction body 0x801E6724..0x801E679C including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15b9c,
                    length: 336,
                    evidence: "original US battle payload; complete instruction body 0x801E679C..0x801E68EC including return delay slot; complete original control-flow review, sized executable ELF function section and all 28 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15cec,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E68EC..0x801E6930 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15d30,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E6930..0x801E6990 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15d90,
                    length: 48,
                    evidence: "original US battle payload; complete instruction body 0x801E6990..0x801E69C0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15dc0,
                    length: 148,
                    evidence: "original US battle payload; complete instruction body 0x801E69C0..0x801E6A54 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15e54,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801E6A54..0x801E6AA0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15ea0,
                    length: 136,
                    evidence: "original US battle payload; complete instruction body 0x801E6AA0..0x801E6B28 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15f28,
                    length: 160,
                    evidence: "original US battle payload; complete instruction body 0x801E6B28..0x801E6BC8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15fc8,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801E6BC8..0x801E6C20 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16020,
                    length: 100,
                    evidence: "original US battle payload; complete instruction body 0x801E6C20..0x801E6C84 including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16084,
                    length: 164,
                    evidence: "original US battle payload; complete instruction body 0x801E6C84..0x801E6D28 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16128,
                    length: 300,
                    evidence: "original US battle payload; complete instruction body 0x801E6D28..0x801E6E54 including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16254,
                    length: 332,
                    evidence: "original US battle payload; complete instruction body 0x801E6E54..0x801E6FA0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 31 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x163a0,
                    length: 16,
                    evidence: "original US battle payload; complete instruction body 0x801E6FA0..0x801E6FB0 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x163b0,
                    length: 572,
                    evidence: "original US battle payload; complete instruction body 0x801E6FB0..0x801E71EC including return delay slot; complete original control-flow review, sized executable ELF function section and all 49 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x165ec,
                    length: 188,
                    evidence: "original US battle payload; complete instruction body 0x801E71EC..0x801E72A8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x166a8,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801E72A8..0x801E72F4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x166f4,
                    length: 452,
                    evidence: "original US battle payload; complete instruction body 0x801E72F4..0x801E74B8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 43 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x168b8,
                    length: 112,
                    evidence: "original US battle payload; complete instruction body 0x801E74B8..0x801E7528 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16928,
                    length: 48,
                    evidence: "original US battle payload; complete instruction body 0x801E7528..0x801E7558 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16958,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801E7558..0x801E75B0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x169b0,
                    length: 132,
                    evidence: "original US battle payload; complete instruction body 0x801E75B0..0x801E7634 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16a34,
                    length: 324,
                    evidence: "original US battle payload; complete instruction body 0x801E7634..0x801E7778 including return delay slot; complete original control-flow review, sized executable ELF function section and all 29 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16b78,
                    length: 160,
                    evidence: "original US battle payload; complete instruction body 0x801E7778..0x801E7818 including return delay slot; complete original control-flow review, sized executable ELF function section and all 12 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16c18,
                    length: 112,
                    evidence: "original US battle payload; complete instruction body 0x801E7818..0x801E7888 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16c88,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801E7888..0x801E78F0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16cf0,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801E78F0..0x801E7948 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16d48,
                    length: 492,
                    evidence: "original US battle payload; complete instruction body 0x801E7948..0x801E7B34 including return delay slot; complete original control-flow review, sized executable ELF function section and all 43 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16f34,
                    length: 108,
                    evidence: "original US battle payload; complete instruction body 0x801E7B34..0x801E7BA0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 94_112,
                    length: 2_488,
                    evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
                },
                NontextRange {
                    offset: 0x17958,
                    length: 212,
                    evidence: "original US battle payload; complete instruction body 0x801E8558..0x801E862C including return delay slot; complete original control-flow review, sized executable ELF function section and all 16 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x17a2c,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801E862C..0x801E8684 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18104,
                    length: 212,
                    evidence: "original US battle payload; complete instruction body 0x801E8D04..0x801E8DD8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 16 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x181d8,
                    length: 464,
                    evidence: "original US battle payload; complete original instruction body 0x801E8DD8..0x801E8FA8 including return delay slot; original JAL at 0x801E6F54 in the already qualified executable caller at 0x801E6E54 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x183a8,
                    length: 204,
                    evidence: "original US battle payload; complete instruction body 0x801E8FA8..0x801E9074 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18474,
                    length: 136,
                    evidence: "original US battle payload; complete instruction body 0x801E9074..0x801E90FC including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x184fc,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E90FC..0x801E915C including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1855c,
                    length: 112,
                    evidence: "original US battle payload; complete instruction body 0x801E915C..0x801E91CC including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x185cc,
                    length: 48,
                    evidence: "original US battle payload; complete instruction body 0x801E91CC..0x801E91FC including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x185fc,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E91FC..0x801E925C including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1865c,
                    length: 32,
                    evidence: "original US battle payload; complete instruction body 0x801E925C..0x801E927C including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18790,
                    length: 172,
                    evidence: "original US battle payload; complete instruction body 0x801E9390..0x801E943C including return delay slot; complete original control-flow review, sized executable ELF function section and all 12 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1883c,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E943C..0x801E949C including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1889c,
                    length: 52,
                    evidence: "original US battle payload; complete instruction body 0x801E949C..0x801E94D0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x188d0,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E94D0..0x801E9530 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18930,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801E9530..0x801E9538 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18938,
                    length: 212,
                    evidence: "original US battle payload; complete instruction body 0x801E9538..0x801E960C including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18a0c,
                    length: 412,
                    evidence: "original US battle payload; complete instruction body 0x801E960C..0x801E97A8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 27 text relocations reconcile with original and captured resident bytes; indirect callback target and selector remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18ba8,
                    length: 360,
                    evidence: "original US battle payload; complete original instruction body 0x801E97A8..0x801E9910 including return delay slot; selector 0 through original numeric function-address table 0x801D0FD0 and qualified original JALR at 0x801E9660 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; structural call continuations assume returns; two BREAK division-trap paths are terminal and divisor validity remains unqualified; no matching callee ELF is claimed; overall selector bounds and current-work validity remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18d10,
                    length: 156,
                    evidence: "original US battle payload; complete instruction body 0x801E9910..0x801E99AC including return delay slot; complete original control-flow review, sized executable ELF function section and all 13 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18dac,
                    length: 364,
                    evidence: "original US battle payload; complete instruction body 0x801E99AC..0x801E9B18 including return delay slot; complete original control-flow review, sized executable ELF function section and all 28 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18f18,
                    length: 400,
                    evidence: "original US battle payload; complete instruction body 0x801E9B18..0x801E9CA8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 25 text relocations reconcile with original and captured resident bytes; indirect callback target and selector remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19318,
                    length: 196,
                    evidence: "original US battle payload; complete instruction body 0x801E9F18..0x801E9FDC including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x193dc,
                    length: 408,
                    evidence: "original US battle payload; complete instruction body 0x801E9FDC..0x801EA174 including return delay slot; complete original control-flow review, sized executable ELF function section and all 32 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19574,
                    length: 48,
                    evidence: "original US battle payload; complete instruction body 0x801EA174..0x801EA1A4 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x195a4,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801EA1A4..0x801EA1E0 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x195e0,
                    length: 116,
                    evidence: "original US battle payload; complete instruction body 0x801EA1E0..0x801EA254 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19930,
                    length: 108,
                    evidence: "original US battle payload; complete instruction body 0x801EA530..0x801EA59C including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19a50,
                    length: 216,
                    evidence: "original US battle payload; complete instruction body 0x801EA650..0x801EA728 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; indirect-call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19b28,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801EA728..0x801EA76C including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19b6c,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801EA76C..0x801EA774 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19b74,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801EA774..0x801EA7DC including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19bdc,
                    length: 108,
                    evidence: "original US battle payload; complete instruction body 0x801EA7DC..0x801EA848 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19c48,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801EA848..0x801EA894 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19cf4,
                    length: 264,
                    evidence: "original US battle payload; complete instruction body 0x801EA8F4..0x801EA9FC including return delay slot; complete original control-flow review, sized executable ELF function section and all 21 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19dfc,
                    length: 188,
                    evidence: "original US battle payload; complete instruction body 0x801EA9FC..0x801EAAB8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19eb8,
                    length: 128,
                    evidence: "original US battle payload; complete instruction body 0x801EAAB8..0x801EAB38 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19f38,
                    length: 44,
                    evidence: "original US battle payload; complete instruction body 0x801EAB38..0x801EAB64 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19f64,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801EAB64..0x801EAB6C including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19f6c,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801EAB6C..0x801EABB8 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
            ])
        }
        (_, _, 0, 0x8009_6800, 133_316)
            if sha256_hex(data)
                == "77d963c56e3ba6b1619323e3007c25d70300e855ac9f4b088ddce2604f92e140" =>
        {
            Some([
                (0x15f54, 124),
                (0x15fd0, 116),
                (0x16044, 44),
                (0x16070, 196),
                (0x16134, 48),
            ]
            .into_iter()
            .map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; five complete instruction bodies spanning 0x800AC754..0x800AC964 including return delay slots; original bytes match sized executable ELF function sections and captured resident bytes; exact archive and payload identities; neighboring functions and tables excluded; no argument-validity, resource-safety, unique-supplier or duplicate-source ownership claim",
            })
            .chain([
                (0x2e8, 60),
                (0x324, 88),
                (0x614, 124),
                (0x690, 112),
                (0x700, 60),
                (0x73c, 60),
                (0x778, 68),
                (0x7bc, 144),
                (0xdd4, 72),
                (0x14bc, 96),
                (0x1558, 104),
                (0x18e4, 96),
                (0x1a54, 144),
                (0x1ae4, 104),
                (0x1bc4, 68),
                (0x1c08, 72),
                (0x21b4, 72),
                (0x2914, 100),
                (0x4a0c, 104),
                (0x4f00, 148),
                (0x4f94, 148),
                (0x527c, 80),
                (0x5f94, 212),
            ].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; 23 complete instruction bodies within 0x80096800..0x8009CFEC including all actual control-transfer delay slots; complete original structural control-flow reviews and sized executable ELF function sections match original and captured resident bytes, with all 222 text relocations independently reconciled from existing objects; direct and indirect-call continuations assume ordinary returns and intact caller frames; three JALR v0 targets, selectors and table bounds remain unqualified; exact archive and payload identities; intervening gaps, refused bodies, neighboring functions and data excluded; no observed-execution, full-function-execution, feasible-path, valid-work-record, selector-bound, indirect-call-safety, argument-validity, callee-semantics, resource-safety, unique-supplier or duplicate-source ownership claim",
            }))
            .chain([
                NontextRange {
                    offset: 0x67ec,
                    length: 428,
                    evidence: "original US shared battle payload; complete instruction body 0x8009CFEC..0x8009D198 including return delay slot; complete original control-flow review and sized executable ELF function section match original and captured resident bytes, with all 21 text relocations reconciled; exact archive and payload identities; neighboring functions and data excluded; no observed-execution, full-function-execution, valid-work-record, selector-bound, argument-validity, callee-semantics, resource-safety, unique-supplier or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x7294,
                    length: 32,
                    evidence: "original US shared battle payload; complete instruction body 0x8009DA94..0x8009DAB4 including return delay slot; complete original control-flow review and sized executable ELF function section match original and captured resident bytes, with all 1 text relocations reconciled; call continuations assume ordinary returns; exact archive and payload identities; neighboring functions and data excluded; no observed-execution, full-function-execution, valid-work-record, selector-bound, argument-validity, callee-semantics, resource-safety, unique-supplier or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x746c,
                    length: 92,
                    evidence: "original US shared battle payload; complete instruction body 0x8009DC6C..0x8009DCC8 including return delay slot; complete original control-flow review and sized executable ELF function section match original and captured resident bytes, with all 8 text relocations reconciled; call continuations assume ordinary returns; exact archive and payload identities; neighboring functions and data excluded; no observed-execution, full-function-execution, valid-work-record, selector-bound, argument-validity, callee-semantics, resource-safety, unique-supplier or duplicate-source ownership claim",
                },
            ])
            .chain([NontextRange {
                offset: 0x6998,
                length: 2300,
                evidence: "original US shared battle payload; complete instruction body 0x8009D198..0x8009DA94 including return delay slot; complete original control-flow review and sized executable ELF function section match original and captured resident bytes, with all 190 text relocations reconciled; indirect-call continuation assumes ordinary return; exact archive and payload identities; neighboring functions, handler tables and data excluded; no observed-execution, valid-work-record, selector-bound, indirect-call-safety, argument-validity, callee-semantics, resource-safety, unique-supplier or duplicate-source ownership claim",
            }])
            .chain([
                (0x7520, 48),
                (0x7550, 200),
                (0x7618, 56),
                (0x7650, 20),
                (0x7664, 20),
                (0x7678, 20),
                (0x768c, 228),
                (0x7770, 76),
                (0x77bc, 48),
                (0x77ec, 48),
                (0x781c, 48),
                (0x784c, 48),
                (0x787c, 264),
                (0x79e0, 32),
                (0x7a00, 36),
                (0x7a24, 36),
                (0x7fc4, 32),
                (0x7fe4, 32),
                (0x8004, 32),
                (0x8024, 32),
                (0x818c, 164),
                (0x828c, 64),
                (0x82cc, 92),
                (0x86cc, 60),
                (0x8708, 32),
                (0x87b8, 88),
                (0x8a58, 32),
                (0x8a78, 56),
                (0x8ab0, 56),
                (0x8ae8, 56),
                (0x8b20, 188),
                (0x8bdc, 56),
            ].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; 32 complete instruction bodies within 0x8009DD20..0x8009F414 including return delay slots; complete original control-flow review and sized executable ELF function sections match original and captured resident bytes, with all 197 text relocations independently reconciled from existing objects; direct-call continuations assume ordinary returns and intact caller frames; exact archive and payload identities; intervening gaps, neighboring functions and data excluded; no observed-execution, full-function-execution, valid-work-record, selector-bound, argument-validity, callee-semantics, resource-safety, unique-supplier or duplicate-source ownership claim",
            }))
            .chain([
                (0x8c14, 56),
                (0x8c4c, 56),
                (0x8c84, 56),
                (0x8cbc, 56),
                (0x8cf4, 56),
                (0x8d2c, 56),
                (0x8d64, 56),
                (0x8d9c, 56),
                (0x8dd4, 56),
                (0x8e0c, 56),
                (0x8e44, 56),
                (0x8e7c, 56),
                (0x8eb4, 56),
                (0x8eec, 108),
                (0x8fec, 40),
                (0x9014, 40),
                (0x903c, 40),
                (0x9064, 32),
                (0x9178, 32),
                (0x9198, 32),
                (0x91b8, 40),
                (0x91e0, 64),
                (0x92e4, 84),
                (0x9610, 96),
                (0x9670, 20),
                (0x9684, 20),
                (0x9698, 20),
                (0x96ac, 36),
                (0x96d0, 36),
                (0x96f4, 136),
                (0x977c, 56),
                (0x97b4, 48),
            ].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; 32 complete instruction bodies within 0x8009F414..0x8009FFE4 including return delay slots; complete original control-flow review and sized executable ELF function sections match original and captured resident bytes, with all 156 text relocations independently reconciled from existing objects; direct-call continuations assume ordinary returns and intact caller frames; exact archive and payload identities; intervening gaps, neighboring functions and data excluded; no observed-execution, full-function-execution, valid-work-record, selector-bound, argument-validity, callee-semantics, resource-safety, unique-supplier or duplicate-source ownership claim",
            }))
            .chain([
                (0x9800, 392),
                (0xa240, 236),
                (0xacb4, 296),
                (0xaddc, 296),
                (0xafa4, 468),
                (0xb9d0, 252),
                (0xc080, 608),
                (0xc2e0, 656),
                (0xc570, 384),
                (0xc6f0, 368),
                (0xcef0, 708),
                (0xd210, 348),
                (0xe7d8, 224),
                (0xe964, 224),
                (0xf500, 224),
                (0xfc88, 308),
                (0x100a4, 904),
                (0x10e94, 548),
                (0x131ac, 216),
                (0x14274, 532),
                (0x14f60, 212),
                (0x1581c, 508),
            ].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; 22 complete instruction bodies within 0x8009FFE4..0x800AC754 including all actual control-transfer delay slots; complete original structural control-flow reviews and sized executable ELF function sections match original and captured resident bytes, with all 628 text relocations independently reconciled from existing objects; direct-call continuations assume ordinary returns and intact caller frames; the body at 0x800AB760..0x800AB834 includes two original compiled BREAK instructions as exceptional terminal edges without delay slots, with exception-handler behavior and continuation unqualified; exact archive and payload identities; intervening gaps, refused bodies, neighboring functions and data excluded; no observed-execution, full-function-execution, feasible-path, valid-work-record, selector-bound, argument-validity, callee-semantics, resource-safety, unique-supplier or duplicate-source ownership claim",
            }))
            .chain([
                (0x9a44, 164),
                (0x9ae8, 144),
                (0x9b78, 180),
                (0x9fd0, 200),
                (0xa32c, 200),
                (0xa668, 92),
                (0xac14, 160),
                (0xaf04, 160),
                (0xb230, 160),
                (0xb338, 172),
                (0xb3e4, 208),
                (0xb4b4, 208),
                (0xb920, 176),
                (0xbc3c, 208),
                (0xc860, 136),
                (0xd1b4, 92),
                (0xd960, 120),
                (0xd9d8, 96),
                (0xe8b8, 104),
                (0xf5e0, 116),
                (0xf82c, 124),
                (0x1042c, 204),
                (0x104f8, 140),
                (0x10584, 144),
                (0x11af8, 104),
                (0x11c50, 172),
                (0x15430, 176),
                (0x1575c, 192),
            ].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; 28 complete instruction bodies within 0x800A0244..0x800AC01C including all actual control-transfer delay slots; complete original structural control-flow reviews and sized executable ELF function sections match original and captured resident bytes, with all 329 text relocations independently reconciled from existing objects; direct-call continuations assume ordinary returns, preserved saved values, intact caller frames and nonaliasing dynamic stores; exact archive and payload identities; intervening gaps, refused bodies, neighboring functions and referenced data excluded; no observed-execution, full-function-execution, feasible-path, valid-work-record, selector-bound, argument-validity, callee-semantics, resource-safety, safe-editing, unique-supplier or duplicate-source ownership claim",
            }))
            .chain([
                (0xf3c, 60),
                (0x151c, 60),
                (0x18a8, 60),
                (0x1b4c, 60),
                (0x1b88, 60),
                (0x2308, 60),
                (0x2978, 60),
                (0x2b28, 60),
                (0x3790, 60),
                (0x3d90, 60),
                (0x4a74, 60),
                (0x4c4c, 60),
                (0x5164, 60),
                (0xa204, 60),
                (0xa3f4, 80),
                (0xbd0c, 84),
                (0xce38, 80),
                (0xda38, 92),
                (0xdf00, 68),
                (0xe3b4, 84),
                (0xe920, 68),
                (0xf01c, 88),
                (0xf654, 68),
                (0x10e48, 76),
                (0x11aa0, 88),
                (0x11bf8, 88),
                (0x11cfc, 68),
                (0x1253c, 68),
                (0x140a4, 88),
                (0x14c70, 76),
                (0x153e0, 72),
                (0x15f04, 80),
            ].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; 32 complete instruction bodies within 0x8009773C..0x800AC754 including all actual control-transfer delay slots; complete original structural control-flow reviews and sized executable ELF function sections match original and captured resident bytes, with all 156 text relocations independently reconciled from existing objects; direct and indirect-call continuations assume ordinary returns, preserved saved values, intact caller frames and nonaliasing dynamic stores; targets, selectors, table contents and bounds for the fifteen indirect-call sites remain unqualified; exact archive and payload identities; intervening gaps, prior refused bodies, neighboring functions and referenced data excluded; no observed-execution, full-function-execution, feasible-path, valid-work-record, selector-bound, indirect-call-safety, argument-validity, callee-semantics, resource-safety, safe-editing, unique-supplier or duplicate-source ownership claim",
            }))
            .chain([
                (0x2b0, 56),
                (0x16b8, 8),
                (0x477c, 8),
                (0x6068, 20),
                (0x607c, 48),
                (0x97e4, 20),
                (0x97f8, 8),
                (0xa444, 36),
                (0xbbcc, 56),
                (0xbc04, 56),
                (0xbd60, 56),
                (0xce88, 52),
                (0xcebc, 52),
                (0xde88, 60),
                (0xdec4, 60),
                (0xe100, 60),
                (0xe2d0, 56),
                (0xe308, 36),
                (0xe408, 60),
                (0xe444, 44),
                (0xeac4, 60),
                (0xeb00, 48),
                (0xf1ac, 60),
                (0xf1e8, 48),
                (0xf7f0, 60),
                (0xf8a8, 52),
                (0x10070, 52),
                (0x14074, 48),
                (0x14cbc, 60),
                (0x153a4, 60),
                (0x15428, 8),
            ].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; 31 complete instruction bodies within 0x80096AB0..0x800ABC30 including all actual control-transfer delay slots; complete original structural control-flow reviews and sized executable ELF function sections match original and captured resident bytes, with all 110 text relocations independently reconciled from existing objects; direct and indirect-call continuations assume ordinary returns, preserved saved values, intact caller frames and nonaliasing dynamic stores; targets, selectors, table contents and bounds for the seven indirect-call sites remain unqualified; exact archive and payload identities; intervening gaps, prior refused bodies, neighboring functions and referenced data excluded; no observed-execution, full-function-execution, feasible-path, valid-work-record, selector-bound, indirect-call-safety, argument-validity, callee-semantics, resource-safety, safe-editing, unique-supplier or duplicate-source ownership claim",
            }))
            .chain([
                (0x17cd0, 164),
                (0x17ec8, 96),
                (0x17f88, 164),
                (0x18e6c, 76),
                (0x19798, 192),
                (0x19858, 192),
                (0x19918, 104),
                (0x19980, 112),
                (0x199f0, 96),
                (0x19b98, 120),
                (0x1a1cc, 96),
                (0x1a254, 92),
                (0x1a2b0, 92),
                (0x1a4e4, 112),
                (0x1a6c4, 108),
                (0x1a730, 108),
                (0x1ab8c, 96),
                (0x1ad5c, 96),
                (0x1ae64, 108),
                (0x1b904, 84),
                (0x1ba58, 84),
                (0x1bb2c, 140),
                (0x1bc78, 84),
                (0x1ca6c, 184),
            ].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; 24 complete instruction bodies within 0x800AC964..0x800B70C4 including all actual control-transfer delay slots; complete original structural control-flow reviews and sized executable ELF function sections match original and captured resident bytes, with all 130 text relocations independently reconciled from existing objects; direct and indirect-call continuations assume ordinary returns, intact caller frames and nonaliasing dynamic stores; targets, selectors and table bounds for the twelve indirect-call sites remain unqualified; exact archive and payload identities; intervening gaps, refused bodies, neighboring functions and data excluded; no observed-execution, full-function-execution, feasible-path, valid-work-record, selector-bound, indirect-call-safety, argument-validity, callee-semantics, resource-safety, unique-supplier or duplicate-source ownership claim",
            }))
            .chain([
                (0x17b4c, 68),
                (0x17b90, 48),
                (0x17c38, 60),
                (0x18f20, 68),
                (0x19454, 68),
                (0x1a22c, 40),
                (0x1a624, 72),
                (0x1acbc, 72),
                (0x1b888, 68),
                (0x1b8cc, 8),
                (0x1b958, 64),
                (0x1b998, 64),
                (0x1b9d8, 64),
                (0x1baec, 64),
                (0x1bc38, 64),
                (0x1bccc, 64),
                (0x1bd8c, 52),
            ].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; 17 complete instruction bodies within 0x800AE34C..0x800B25C0 including all actual control-transfer delay slots; complete original structural control-flow reviews and sized executable ELF function sections match original and captured resident bytes, with all 17 text relocations independently reconciled from existing objects; direct and indirect-call continuations assume ordinary returns, preserved saved values, intact caller frames and nonaliasing dynamic stores; targets, selectors, table contents and bounds for the two indirect-call sites remain unqualified; exact archive and payload identities; intervening gaps, prior refused bodies, missing artifacts, neighboring functions and referenced data excluded; no observed-execution, full-function-execution, feasible-path, valid-work-record, selector-bound, indirect-call-safety, argument-validity, callee-semantics, resource-safety, safe-editing, unique-supplier or duplicate-source ownership claim",
            }))
            .chain([NontextRange {
                offset: 0x16164,
                length: 1564,
                evidence: "original US shared battle payload; complete instruction body 0x800AC964..0x800ACF80 including return delay slot; native entry PC 0x800AC964 and RA 0x801D1A80 bind the complete original control-flow review; original bytes match canonical payload and captured resident bytes; exact archive and payload identities; no ELF-match, full-function execution, argument-validity, resource-safety, callee-semantics or duplicate-source ownership claim",
            }])
            .chain([(0x16780, 116), (0x167f4, 80)].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; two complete instruction bodies spanning 0x800ACF80..0x800AD044 including return delay slots; original bytes match complete sized executable ELF function sections and captured resident bytes, with text relocations reconciled; exact archive and payload identities; neighboring functions and tables excluded; no argument-validity, resource-safety, unique-supplier or duplicate-source ownership claim",
            }))
            .chain([(0x16844, 48), (0x16874, 52), (0x168a8, 452), (0x16a6c, 128)].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; four complete instruction bodies spanning 0x800AD044..0x800AD2EC including return delay slots; original bytes match complete sized executable ELF function sections and captured resident bytes, with text relocations reconciled; exact archive and payload identities; neighboring functions and tables excluded; no argument-validity, selector-bound, indirect-call-safety, resource-safety, unique-supplier or duplicate-source ownership claim",
            }))
            .chain([(0x16aec, 328), (0x16c34, 160)].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; two complete instruction bodies spanning 0x800AD2EC..0x800AD4D4 including return delay slots; complete original control-flow review and conditional entries through original table 0x80096994 consumed by qualified dispatcher 0x800AD26C; original bytes match captured resident bytes; call continuations assume returns; exact archive and payload identities; no ELF-match, observed-execution, selector-bound, indirect-call-safety, argument-validity, resource-safety or duplicate-source ownership claim",
            }))
            .chain([NontextRange {
                offset: 0x16e7c,
                length: 32,
                evidence: "original US shared battle payload; complete instruction body 0x800AD67C..0x800AD69C including return delay slot; original bytes match a complete sized executable ELF function section and captured resident bytes, with its call relocation reconciled; conditional entry through original table 0x80096994 consumed by qualified dispatcher 0x800AD26C; exact archive and payload identities; intervening and neighboring bodies excluded; no observed-execution, selector-bound, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
            }, NontextRange {
                offset: 0x194,
                length: 12,
                evidence: "original US shared battle payload; three numeric function-address words at 0x80096994..0x800969A0; original dispatcher 0x800AD26C loads all three words, copies them to its stack and conditionally calls the indexed address; original words match captured resident bytes; exact archive and payload identities; surrounding header bytes excluded; no selector-bound, current-work-validity, pointer-lifetime or observed-invocation claim",
            }])
            .chain([(0x16cd4, 244), (0x16dc8, 180)].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; two complete instruction bodies spanning 0x800AD4D4..0x800AD67C including return delay slots; original bytes match complete sized executable ELF function sections and captured resident bytes, with all text relocations reconciled; qualified dispatcher 0x800AD434 constructs their addresses for conditional work-byte-2 selection; exact archive and payload identities; no observed-execution, valid-work-record, selector-bound, list-integrity, resource-safety or duplicate-source ownership claim",
            }))
            .chain([(0x16e9c, 128), (0x17130, 124), (0x171ac, 32), (0x171cc, 128), (0x174a4, 32), (0x174c4, 128), (0x1779c, 32), (0x177bc, 88), (0x17814, 88), (0x1786c, 48)].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; ten complete instruction bodies within 0x800AD69C..0x800AE09C including return delay slots; complete original control-flow reviews and sized executable ELF function sections match original and captured resident bytes, with all text relocations reconciled; exact archive and payload identities; no observed-execution, valid-work-record, selector-bound, indirect-call-safety, argument-validity, resource-safety or duplicate-source ownership claim",
            }))
            .chain([(0x16f1c, 236), (0x17008, 208), (0x170d8, 88), (0x1724c, 296), (0x17374, 304), (0x17544, 296), (0x1766c, 304)].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; seven complete instruction bodies within 0x800AD71C..0x800ADF9C including return delay slots; complete original control-flow reviews and conditional entries through reviewed callers that load handler addresses from tables or construct them on the stack; original bytes match captured resident bytes; call continuations assume returns; exact archive and payload identities; no ELF-match, observed-execution, valid-work-record, selector-bound, indirect-call-safety, argument-validity, resource-safety or duplicate-source ownership claim",
            }))
            .chain([0x1a0, 0x1ac, 0x1b8].into_iter().map(|offset| NontextRange {
                offset,
                length: 12,
                evidence: "original US shared battle payload; three numeric function-address words in each original table at 0x800969A0, 0x800969AC and 0x800969B8; complete original dispatchers 0x800AD69C, 0x800AD9CC and 0x800ADCC4 read all three words, copy them to their stacks and conditionally call the indexed address; all table words and complete target bodies match original and captured resident bytes; exact archive and payload identities; surrounding header bytes excluded; no selector-bound, current-work-validity, pointer-lifetime, observed-invocation, resource-safety or duplicate-source ownership claim",
            }))
            .chain([NontextRange {
                offset: 0x1789c,
                length: 108,
                evidence: "original US shared battle payload; complete instruction body 0x800AE09C..0x800AE108 including return delay slot; complete original control-flow review and conditional selector-1 entry through qualified dispatcher 0x800AE014; original bytes match captured resident bytes; call continuations assume returns; exact archive and payload identities; no ELF-match, observed-execution, valid-work-record, selector-bound, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
            }])
            .chain([NontextRange {
                offset: 0x17908,
                length: 240,
                evidence: "original US shared battle payload; complete instruction body 0x800AE108..0x800AE1F8 including return delay slots; complete original control-flow review and captured native entry PC 0x800AE108 with RA 0x801D39F0 from original caller 0x801D39E8; original bytes match captured resident bytes; call continuations assume returns; exact archive and payload identities; no ELF-match, full-function execution, valid-work-record, selector-bound, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
            }])
            .chain([NontextRange {
                offset: 0x179f8,
                length: 104,
                evidence: "original US shared battle payload; complete instruction body 0x800AE1F8..0x800AE260 including return delay slot; complete original control-flow review and captured native entry PC 0x800AE1F8 with RA 0x801D381C from original caller 0x801D3814; original bytes match captured resident bytes; call continuation assumes return; exact archive and payload identities; no ELF-match, full-function execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
            }])
            .chain([NontextRange {
                offset: 0x181c4,
                length: 920,
                evidence: "original US shared battle payload; complete instruction body 0x800AE9C4..0x800AED5C including all 15 control-transfer delay slots; complete original ASM word and structural control-flow review and captured resident bytes reconcile; conditional entries through qualified original JAL sites 0x800AE55C and 0x800AE814; call continuations assume ordinary returns, preserved saved values, intact caller frames and nonaliasing dynamic stores; exact archive and payload identities; referenced tables, data and neighboring bodies excluded; no ELF-match, observed-execution, feasible-path, argument-validity, selector-bound, callee-semantics, resource-safety, safe-editing, unique-supplier or duplicate-source ownership claim",
            }])
            .chain([NontextRange {
                offset: 0x1a79c,
                length: 1008,
                evidence: "original US shared battle payload; complete instruction body 0x800B0F9C..0x800B138C including all 20 control-transfer delay slots; complete original ASM word and structural control-flow review and captured resident bytes reconcile; conditional entries through qualified original JAL sites 0x800B0ECC and 0x800B0F38; call continuations assume ordinary returns, preserved saved values, intact caller frames and nonaliasing dynamic stores; exact archive and payload identities; referenced formats, buffers, tables, data and neighboring bodies excluded; no ELF-match, observed-execution, feasible-path, argument-validity, selector-bound, callee-semantics, resource-safety, safe-editing, unique-supplier or duplicate-source ownership claim",
            }])
            .chain([
                NontextRange {
                    offset: 0x1b274,
                    length: 824,
                    evidence: "original US shared battle payload; complete instruction body 0x800B1A74..0x800B1DAC including all 21 control-transfer delay slots; complete original ASM word and structural control-flow review and captured resident bytes reconcile; conditional entry through qualified original JAL site 0x800B0FDC; call continuations assume ordinary returns, preserved saved values, intact frames and nonaliasing dynamic stores; exact archive and payload identities; referenced tables, data, pointed storage and callees excluded; no ELF-match, observed-execution, feasible-path, argument-validity, table-bound, callee-semantics, resource-safety, safe-editing, unique-supplier or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1b5ac,
                    length: 344,
                    evidence: "original US shared battle payload; complete instruction body 0x800B1DAC..0x800B1F04 including all seven control-transfer delay slots; complete original ASM word and structural control-flow review and captured resident bytes reconcile; conditional entry through qualified original JAL site 0x800B11DC; call continuations assume ordinary returns, preserved saved values, intact frames and nonaliasing dynamic stores; exact archive and payload identities; referenced tables, data, pointed storage and callees excluded; no ELF-match, observed-execution, feasible-path, argument-validity, table-bound, callee-semantics, resource-safety, safe-editing, unique-supplier or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1b704,
                    length: 388,
                    evidence: "original US shared battle payload; complete instruction body 0x800B1F04..0x800B2088 including all eleven control-transfer delay slots; complete original ASM word and structural control-flow review and captured resident bytes reconcile; conditional entry through qualified original JAL site 0x800B131C; call continuations assume ordinary returns, preserved saved values, intact frames and live local bytes, and nonaliasing dynamic stores; exact archive and payload identities; referenced flags, data, pointed storage and callees excluded; no ELF-match, observed-execution, feasible-path, argument-validity, callee-semantics, resource-safety, safe-editing, unique-supplier or duplicate-source ownership claim",
                },
            ])
            .chain([NontextRange {
                offset: 70_676,
                length: 1_096,
                evidence: SPRITE_CONSUMER_INSTRUCTION_EVIDENCE,
            }])
            .chain([
                (0x16c0, 296),
                (0x4784, 184),
                (0x74c8, 88),
                (0x7d00, 708),
                (0xc8e8, 248),
                (0xc9e0, 1112),
                (0xd36c, 304),
                (0xd49c, 348),
                (0xd5f8, 304),
                (0xd728, 276),
                (0xd83c, 148),
                (0xda94, 452),
                (0xdc58, 560),
                (0x10614, 2100),
                (0x1185c, 580),
                (0x12758, 940),
                (0x12b04, 420),
                (0x12ca8, 180),
                (0x12d5c, 180),
                (0x13020, 396),
                (0x133d8, 1004),
                (0x14488, 564),
                (0x146bc, 508),
                (0x154e0, 636),
                (0x15a18, 1260),
                (0x19c98, 604),
                (0x1a30c, 472),
                (0x1af28, 844),
                (0x1bdc0, 1744),
                (0x1c490, 1500),
                (0x1cb24, 1988),
                (0x1d2e8, 1848),
                (0x1da20, 408),
            ].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: BATTLE_CALLEE_INSTRUCTION_EVIDENCE,
            }))
            .chain([(0x174, 32), (0x25c, 24)].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: BATTLE_CALLEE_TABLE_EVIDENCE,
            }))
            .chain([(0x110b8, 648), (0x137c4, 932), (0x19ef4, 728)].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: US_DIRECT_CALL_INSTRUCTION_EVIDENCE,
            }))
            .chain([
                NontextRange {
                    offset: 0x11340,
                    length: 212,
                    evidence: US_FOLLOWUP_INSTRUCTION_EVIDENCE,
                },
            ])
            .collect())
        }
        (13, 733_184, 0, 0x801f_2c00, 2_548)
            if entry.check_word == 0x3b
                && sha256_hex(data)
                    == "b8369f325ddcc7250258e7709dc319ec7d9b7c4582ab6d3f23fb88a1954be356" =>
        {
            let mut ranges = Vec::new();
            for offset in [
                1_285, 1_301, 1_317, 1_332, 1_350, 1_368, 1_471, 1_487, 1_503, 1_518, 1_536,
                1_554,
            ] {
                ranges.push(NontextRange {
                    offset,
                    length: 2,
                    evidence: "Original US AREA007 entry 13, twelve two-byte numeric event selectors in root 0x801F30F4..0x801F3227. Original GAME type-1 constructor 0x801A50C0 reads record +0x0E/+0x0F and type-5 constructor 0x801A6F58 reads +0x0D/+0x0E, high byte first. Stores 0x801A52FC and 0x801A71B4 publish actor halfword +0x7C. The exact fields contain two copies of selectors 0x8000 through 0x8005. Exact original archive and payload identities; valid nonnegative slot below 30, ordinary returns, nonaliasing and stable resources are conditions. Other root bytes, actual construction, actor eligibility and lifetime, rendering and editing remain unqualified.",
                });
            }
            for offset in [2_036, 2_060, 2_084, 2_108, 2_132, 2_156] {
                ranges.push(NontextRange {
                    offset,
                    length: 23,
                    evidence: "Original US AREA007 entry 13, six complete 23-byte event scripts selected by finite source-bound cases 0x8000 through 0x8005. Original GAME readers qualify two three-byte F0 headers, three four-byte D records, FD/FE conditional delimiters and the explicit FF end in each script. All operands are numeric control fields. Every D record has second byte zero and uses setter 0x801A72E8's halfword arm, not its callback-byte arm. Exact original archive and payload identities; construction, actor eligibility, lifetime, ordinary returns, nonaliasing and stable resources are conditions. No directory count, predicate meaning, actual invocation, callback 0x0C selection, rendering or editing claim.",
                });
            }
            ranges.push(NontextRange {
                offset: 2_192,
                length: 24,
                evidence: "Original US AREA007 entry 13, six little-endian event pointer cells at 0x801F3490..0x801F34A8. Source-bound selectors 0x8000 through 0x8005 select these exact cells through original GAME reader 0x801A17A0 and the world-7 descriptor's +4 directory pointer. The pool caller at 0x801A0790 passes actor +0x74, so reader input +8 aliases the constructors' actor +0x7C selector. Exact original archive and payload identities; eligible actor, stable resources, nonaliasing and ordinary returns are conditions. These finite cases do not establish directory count, neighboring cells, actual selection, rendering or editing.",
            });
            Some(ranges)
        }
        _ => None,
    }
}

/// Canonical sources with reviewed sector metadata and bounded payload fields.
pub fn xa_sources(executable: &[u8]) -> Result<Vec<XaSource>> {
    if sha256_hex(executable) != US_EXECUTABLE_SHA256 {
        return Ok(Vec::new());
    }
    let original = disc()?;
    let mut streams = Vec::new();
    for (path, expected_audio_bodies, expected_empty_payloads, expected_packet_headers) in [
        ("BIN/SCE_XA/S_XA00.STR", 11_302, 0, 0),
        ("BIN/BMAG_XA/MAGIC00.STR", 14_856, 0, 0),
        ("BIN/SCE_XA/VOICE.STR", 297, 3239, 0),
        ("LOGO/CAPCOM30.STR", 144, 0, 1011),
    ] {
        let file = original
            .files
            .iter()
            .find(|file| file.path == path)
            .ok_or_else(|| {
                Error::Invalid(format!(
                    "original US XA source missing from manifest: {path}"
                ))
            })?;
        streams.push(XaSource {
            file,
            expected_audio_bodies,
            expected_empty_payloads,
            expected_packet_headers,
        });
    }
    Ok(streams)
}
