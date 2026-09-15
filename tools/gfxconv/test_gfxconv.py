import struct, unittest
from gba import bgr555_to_psx, decode_4bpp_tile
from tim import tim4
class Tests(unittest.TestCase):
 def test_colour(self):
  self.assertEqual(bgr555_to_psx(0x1234),0x9234); self.assertEqual(bgr555_to_psx(0,True),0)
 def test_tile_nibbles(self):
  p=decode_4bpp_tile(bytes([0x21])*32); self.assertEqual(p[:4],[1,2,1,2]); self.assertEqual(len(p),64)
 def test_tim_header(self):
  d=tim4(bytes(32),[0]*16); self.assertEqual(struct.unpack_from("<II",d,0),(0x10,0x08))
if __name__=="__main__": unittest.main()
