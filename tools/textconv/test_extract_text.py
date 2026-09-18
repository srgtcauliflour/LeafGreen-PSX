import tempfile
import unittest
from pathlib import Path
import extract_text


class Tests(unittest.TestCase):
    def test_sha1_stream(self):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / "x"
            path.write_bytes(b"abc")
            self.assertEqual(extract_text.sha1(path), "a9993e364706816aba3e25717850c26c9cd0d89d")

    def test_rejects_rom_with_wrong_hash(self):
        with tempfile.TemporaryDirectory() as d:
            rom = Path(d) / "rom.gba"
            rom.write_bytes(b"not the real rom")
            manifest = Path(d) / "manifest.json"
            manifest.write_text('{"schema":1,"target":"leafgreen-us-rev1","assets":[]}')
            with self.assertRaises(SystemExit):
                import sys
                old_argv = sys.argv
                sys.argv = ["extract_text.py", str(rom), str(manifest)]
                try:
                    extract_text.main()
                finally:
                    sys.argv = old_argv


if __name__ == "__main__":
    unittest.main()
