from __future__ import annotations

import struct
import tempfile
import unittest
from pathlib import Path
from subprocess import CompletedProcess
from unittest.mock import patch

from scripts.rebof3.re.services.asm_normalize import AddressSymbolResolver
from scripts.rebof3.re.services import spimdisasm_backend as MODULE


def _build_fake_emi(payload: bytes, *, load_arg: int) -> bytes:
    header = bytearray(0x800)
    struct.pack_into("<I", header, 0x0, 1)
    header[8:16] = b"MATH_TBL"
    struct.pack_into("<IIIHH", header, 0x10, len(payload), load_arg, 0, 0, 0)
    return bytes(header) + payload


class SpimdisasmBackendTests(unittest.TestCase):
    def test_parse_psx_exe_header_reads_text_bounds(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            path = Path(tmp_dir) / "SLUS_004.22"
            data = bytearray(0x900)
            data[:8] = b"PS-X EXE"
            struct.pack_into("<I", data, 0x18, 0x80100000)
            struct.pack_into("<I", data, 0x1C, 0x80)
            path.write_bytes(data)

            header = MODULE.parse_psx_exe_header(path)

        self.assertEqual(header["text_addr"], 0x80100000)
        self.assertEqual(header["text_size"], 0x80)

    def test_slice_function_binary_from_psx_exe(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            path = Path(tmp_dir) / "SLUS_004.22"
            text = bytes(range(0x40))
            data = bytearray(0x800 + len(text))
            data[:8] = b"PS-X EXE"
            struct.pack_into("<I", data, 0x18, 0x80100000)
            struct.pack_into("<I", data, 0x1C, len(text))
            data[0x800 : 0x800 + len(text)] = text
            path.write_bytes(data)
            slice_path = Path(tmp_dir) / "func.bin"

            slice_info = MODULE.slice_function_binary(
                source_text=str(path),
                function_payload={"body_min": "80100010", "body_max": "8010001f"},
                slice_path=slice_path,
            )

            self.assertEqual(slice_info.source_kind, "psx-exe")
            self.assertEqual(slice_path.read_bytes(), text[0x10:0x20])

    def test_slice_function_binary_from_emi_entry(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            path = Path(tmp_dir) / "GAME.EMI"
            payload = bytes(range(0x40))
            path.write_bytes(_build_fake_emi(payload, load_arg=0x801D0C00))
            slice_path = Path(tmp_dir) / "func.bin"

            slice_info = MODULE.slice_function_binary(
                source_text=f"{path}#0",
                function_payload={"body_min": "801d0c08", "body_max": "801d0c0f"},
                slice_path=slice_path,
            )

            self.assertEqual(slice_info.source_kind, "emi")
            self.assertEqual(slice_path.read_bytes(), payload[0x08:0x10])

    def test_write_spim_symbol_addrs_formats_functions_and_labels(self) -> None:
        resolver = AddressSymbolResolver(
            function_symbols={0x80123456: "FUN_80123456"},
            data_symbols={0x80146494: "DAT_80146494", 0x801621E8: "LAB_801621E8"},
        )
        with tempfile.TemporaryDirectory() as tmp_dir:
            path = Path(tmp_dir) / "symbol_addrs.txt"

            written = MODULE.write_spim_symbol_addrs(path, resolver)
            text = path.read_text(encoding="utf-8")

        self.assertEqual(written, path)
        self.assertIn("func_80123456 = 0x80123456; // type:func", text)
        self.assertIn("DAT_80146494 = 0x80146494;", text)
        self.assertIn("LAB_801621E8 = 0x801621e8; // type:label", text)

    def test_build_spimdisasm_command_uses_psx_profile(self) -> None:
        command = MODULE.build_spimdisasm_command(
            slice_path=Path("/tmp/func.bin"),
            output_path=Path("/tmp/func.spim.s"),
            slice_vram=0x80161FDC,
            slice_size=0x184,
            symbol_addrs_path=Path("/tmp/symbol_addrs.txt"),
        )

        self.assertIn("spimdisasm", command)
        self.assertIn("r3000gte", command)
        self.assertIn("PSYQ", command)
        self.assertIn("little", command)
        self.assertIn("/tmp/symbol_addrs.txt", command)

    def test_run_spimdisasm_function_asm_invokes_tool(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            payload_path = Path(tmp_dir) / "payload.bin"
            payload_path.write_bytes(b"\x00" * 0x40)
            manifest_path = Path(tmp_dir) / "emi.json"
            manifest_path.write_text(
                '{"entries":[{"name":"payload.bin","index":0,"type":0,"ram_ptr":2148819968}]}',
                encoding="utf-8",
            )
            output_path = Path(tmp_dir) / "func.spim.s"
            def _fake_run(command, **_kwargs):
                tool_output_dir = Path(command[5])
                tool_output_dir.mkdir(parents=True, exist_ok=True)
                (tool_output_dir / "func.text.s").write_text(".text\n", encoding="utf-8")
                return CompletedProcess(args=["spimdisasm"], returncode=0, stdout="", stderr="")
            with patch.object(
                MODULE,
                "run_command",
                side_effect=_fake_run,
            ):
                metadata = MODULE.run_spimdisasm_function_asm(
                    source_text=str(payload_path),
                    function_payload={"body_min": "80146400", "body_max": "8014640f"},
                    output_path=output_path,
                    resolver=None,
                )

            self.assertEqual(metadata["status"], "ok")
            self.assertTrue((output_path.with_suffix(".bin")).exists())
            self.assertTrue(output_path.is_file())


if __name__ == "__main__":
    unittest.main()
