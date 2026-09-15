import tempfile,unittest
from pathlib import Path
import extract
class Tests(unittest.TestCase):
 def test_sha1_stream(self):
  with tempfile.TemporaryDirectory() as d:
   p=Path(d)/"x";p.write_bytes(b"abc");self.assertEqual(extract.sha1(p),"a9993e364706816aba3e25717850c26c9cd0d89d")
if __name__=="__main__":unittest.main()
