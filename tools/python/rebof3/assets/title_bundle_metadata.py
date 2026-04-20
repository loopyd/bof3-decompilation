from __future__ import annotations

from dataclasses import asdict, dataclass
from typing import Any


TITLE_VALIDATED_GRAPH_TYPE = 0


@dataclass(frozen=True)
class TitleAssetSpec:
    name: str
    layout_index: int
    source_function: str
    tpage_graph_type_0: int
    tpage_graph_type_1: int
    screen_x: int
    screen_y: int
    confidence: str = "validated"
    semi_trans: bool = False
    animated_alpha: bool = False
    notes: tuple[str, ...] = ()

    def to_metadata(self) -> dict[str, Any]:
        data = asdict(self)
        data["tpage_by_graph_type"] = {
            "0": f"0x{self.tpage_graph_type_0:03x}",
            "1": f"0x{self.tpage_graph_type_1:03x}",
        }
        data["screen_position"] = {"x": self.screen_x, "y": self.screen_y}
        return data


@dataclass(frozen=True)
class TitleCompositePiece:
    name: str
    x: int
    y: int


TITLE_VALIDATED_ASSETS: tuple[TitleAssetSpec, ...] = (
    TitleAssetSpec(
        name="logo_main_a",
        layout_index=4,
        source_function="0x801d150c",
        tpage_graph_type_0=0x0B9,
        tpage_graph_type_1=0x2A9,
        screen_x=-6,
        screen_y=0x1C,
        notes=("left main title slab",),
    ),
    TitleAssetSpec(
        name="logo_main_b",
        layout_index=5,
        source_function="0x801d150c",
        tpage_graph_type_0=0x0B9,
        tpage_graph_type_1=0x2A9,
        screen_x=218,
        screen_y=0x1C,
        notes=("right main title slab",),
    ),
    TitleAssetSpec(
        name="mark_main",
        layout_index=2,
        source_function="0x801d16dc",
        tpage_graph_type_0=0x0BB,
        tpage_graph_type_1=0x2AB,
        screen_x=0x1A,
        screen_y=0x18,
        notes=("main mark panel",),
    ),
    TitleAssetSpec(
        name="mark_tail",
        layout_index=3,
        source_function="0x801d16dc",
        tpage_graph_type_0=0x0BB,
        tpage_graph_type_1=0x2AB,
        screen_x=266,
        screen_y=136,
        notes=("tail panel paired with mark_main",),
    ),
    TitleAssetSpec(
        name="press_start_button",
        layout_index=7,
        source_function="0x801d12cc",
        tpage_graph_type_0=0x0BD,
        tpage_graph_type_1=0x2AD,
        screen_x=0x30,
        screen_y=0xB8,
        animated_alpha=True,
        notes=("blink alpha driven by popup_blink_counter",),
    ),
    TitleAssetSpec(
        name="copyright_line_a",
        layout_index=8,
        source_function="0x801d12cc",
        tpage_graph_type_0=0x0BD,
        tpage_graph_type_1=0x2AD,
        screen_x=0x0C,
        screen_y=0xC8,
    ),
    TitleAssetSpec(
        name="copyright_line_b",
        layout_index=19,
        source_function="0x801d12cc",
        tpage_graph_type_0=0x0BD,
        tpage_graph_type_1=0x2AD,
        screen_x=0x0C,
        screen_y=0xD4,
    ),
    TitleAssetSpec(
        name="copyright_line_c",
        layout_index=9,
        source_function="0x801d12cc",
        tpage_graph_type_0=0x0BD,
        tpage_graph_type_1=0x2AD,
        screen_x=0xAC,
        screen_y=0xD4,
    ),
)

TITLE_CANDIDATE_ASSETS: tuple[TitleAssetSpec, ...] = (
    TitleAssetSpec(
        name="logo_overlay_a",
        layout_index=15,
        source_function="0x801d150c",
        tpage_graph_type_0=0x0D9,
        tpage_graph_type_1=0x329,
        screen_x=-6,
        screen_y=0x1C,
        confidence="code_backed_candidate",
        semi_trans=True,
    ),
    TitleAssetSpec(
        name="logo_overlay_b",
        layout_index=16,
        source_function="0x801d150c",
        tpage_graph_type_0=0x0D9,
        tpage_graph_type_1=0x329,
        screen_x=218,
        screen_y=0x1C,
        confidence="code_backed_candidate",
        semi_trans=True,
    ),
    TitleAssetSpec(
        name="selection_marker",
        layout_index=1,
        source_function="0x801d12cc",
        tpage_graph_type_0=0x02F,
        tpage_graph_type_1=0x08F,
        screen_x=0x106,
        screen_y=0x82,
        confidence="code_backed_candidate",
    ),
    TitleAssetSpec(
        name="popup_panel",
        layout_index=10,
        source_function="0x801d11e4",
        tpage_graph_type_0=0x002,
        tpage_graph_type_1=0x002,
        screen_x=0xC0,
        screen_y=0x04,
        confidence="code_backed_candidate",
    ),
)

TITLE_LOGO_MAIN_COMPOSITE: tuple[TitleCompositePiece, ...] = (
    TitleCompositePiece("logo_main_a", 0, 0),
    TitleCompositePiece("logo_main_b", 224, 0),
)

TITLE_LOGO_OVERLAY_COMPOSITE: tuple[TitleCompositePiece, ...] = (
    TitleCompositePiece("logo_overlay_a", 0, 0),
    TitleCompositePiece("logo_overlay_b", 224, 0),
)

TITLE_MARK_COMPOSITE: tuple[TitleCompositePiece, ...] = (
    TitleCompositePiece("mark_main", 0, 0),
    TitleCompositePiece("mark_tail", 240, 112),
)

TITLE_LOGO_FULL_COMPOSITE: tuple[TitleCompositePiece, ...] = (
    TitleCompositePiece("mark_main", 0x1A, 0x18),
    TitleCompositePiece("mark_tail", 266, 136),
    TitleCompositePiece("logo_main_a", -6, 0x1C),
    TitleCompositePiece("logo_main_b", 218, 0x1C),
)

TITLE_MENU_SCENE_CORE: tuple[TitleCompositePiece, ...] = (
    TitleCompositePiece("press_start_button", 0x30, 0xB8),
    TitleCompositePiece("mark_main", 0x1A, 0x18),
    TitleCompositePiece("mark_tail", 266, 136),
    TitleCompositePiece("logo_main_a", -6, 0x1C),
    TitleCompositePiece("logo_main_b", 218, 0x1C),
)

TITLE_MENU_SCENE_TEXT: tuple[TitleCompositePiece, ...] = (
    TitleCompositePiece("copyright_line_a", 0x0C, 0xC8),
    TitleCompositePiece("copyright_line_b", 0x0C, 0xD4),
    TitleCompositePiece("copyright_line_c", 0xAC, 0xD4),
)

TITLE_MENU_SCENE_OVERLAY: tuple[TitleCompositePiece, ...] = (
    TitleCompositePiece("logo_overlay_a", -6, 0x1C),
    TitleCompositePiece("logo_overlay_b", 218, 0x1C),
)


def title_bundle_metadata() -> dict[str, Any]:
    return {
        "validated_graph_type": TITLE_VALIDATED_GRAPH_TYPE,
        "assets": {
            "validated": [asset.to_metadata() for asset in TITLE_VALIDATED_ASSETS],
            "candidates": [asset.to_metadata() for asset in TITLE_CANDIDATE_ASSETS],
        },
        "composites": {
            "validated": {
                "title_logo_main": {
                    "size": {"width": 320, "height": 128},
                    "pieces": [asdict(piece) for piece in TITLE_LOGO_MAIN_COMPOSITE],
                },
                "title_mark": {
                    "size": {"width": 288, "height": 160},
                    "pieces": [asdict(piece) for piece in TITLE_MARK_COMPOSITE],
                },
                "title_logo_full": {
                    "size": {"width": 320, "height": 184},
                    "pieces": [asdict(piece) for piece in TITLE_LOGO_FULL_COMPOSITE],
                },
                "title_copyright": {
                    "size": {"width": 304, "height": 28},
                    "pieces": [
                        {"name": "copyright_line_a", "x": 0, "y": 0},
                        {"name": "copyright_line_b", "x": 0, "y": 12},
                        {"name": "copyright_line_c", "x": 160, "y": 12},
                    ],
                },
                "title_menu_scene_core": {
                    "size": {"width": 320, "height": 240},
                    "pieces": [asdict(piece) for piece in TITLE_MENU_SCENE_CORE],
                },
                "title_menu_scene_text": {
                    "size": {"width": 320, "height": 240},
                    "pieces": [asdict(piece) for piece in TITLE_MENU_SCENE_TEXT],
                },
            },
            "candidates": {
                "title_logo_overlay": {
                    "size": {"width": 320, "height": 128},
                    "pieces": [asdict(piece) for piece in TITLE_LOGO_OVERLAY_COMPOSITE],
                },
                "title_menu_scene_overlay": {
                    "size": {"width": 320, "height": 240},
                    "pieces": [asdict(piece) for piece in TITLE_MENU_SCENE_OVERLAY],
                },
            },
        },
        "draw_sequences": {
            "window_fx_tick": {
                "output": {
                    "filename": "title_window_fx_sequence.png",
                    "size": {"width": 320, "height": 240},
                },
                "calls": [
                    {
                        "draws": [
                            {"asset": "selection_marker", "x": 0x106, "y": 0x82},
                            {"asset": "copyright_line_a", "x": 0x0C, "y": 0xC8},
                            {"asset": "copyright_line_b", "x": 0x0C, "y": 0xD4},
                            {"asset": "copyright_line_c", "x": 0xAC, "y": 0xD4},
                        ]
                    },
                    {
                        "draws": [
                            {"asset": "mark_main", "x": 0x1A, "y": 0x18},
                            {"asset": "mark_tail", "x": 266, "y": 136},
                        ]
                    },
                    {
                        "draws": [
                            {"asset": "logo_main_a", "x": -6, "y": 0x1C},
                            {"asset": "logo_main_b", "x": 218, "y": 0x1C},
                            {"asset": "logo_overlay_a", "x": -6, "y": 0x1C},
                            {"asset": "logo_overlay_b", "x": 218, "y": 0x1C},
                            {"asset": "press_start_button", "x": 0x30, "y": 0xB8},
                        ]
                    },
                ],
            }
        },
    }
