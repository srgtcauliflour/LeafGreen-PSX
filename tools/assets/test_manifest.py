import unittest
from manifest import validate
class Tests(unittest.TestCase):
 def test_valid(self):
  self.assertEqual(validate({"schema":1,"target":"leafgreen-us-rev1","assets":[]}),[])
 def test_duplicate(self):
  a={"id":"x","kind":"palette","source":{},"output":"x"}; e=validate({"schema":1,"target":"leafgreen-us-rev1","assets":[a,a]}); self.assertTrue(any("duplicate" in x for x in e))
if __name__=="__main__": unittest.main()
