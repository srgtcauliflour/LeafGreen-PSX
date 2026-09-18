import unittest
from charmap import decode_text, encode_text, EOS, CONTROL_LINE_FEED, CONTROL_PARAGRAPH


class CharmapTests(unittest.TestCase):
    def test_decodes_plain_letters_and_digits(self):
        # 'A' 'B' 'C' '1' '2' '3' EOS
        data = bytes([0xBB, 0xBC, 0xBD, 0xA2, 0xA3, 0xA4, EOS])
        self.assertEqual(decode_text(data), 'ABC123')

    def test_stops_at_eos_ignoring_trailing_bytes(self):
        data = bytes([0xBB, EOS, 0xBC, 0xBD])
        self.assertEqual(decode_text(data), 'A')

    def test_decodes_without_trailing_eos(self):
        data = bytes([0xBB, 0xBC])
        self.assertEqual(decode_text(data), 'AB')

    def test_control_codes_become_escapes(self):
        data = bytes([0xBB, CONTROL_LINE_FEED, 0xBC, CONTROL_PARAGRAPH, EOS])
        self.assertEqual(decode_text(data), 'A\nB\\p')

    def test_unmapped_byte_placeholder(self):
        data = bytes([0xBB, 0x99, 0xBC, EOS])  # 0x99 has no mapping
        self.assertEqual(decode_text(data), 'A\\x99B')

    def test_unmapped_byte_strict_raises(self):
        data = bytes([0x99])
        with self.assertRaises(ValueError):
            decode_text(data, on_unmapped='strict')

    def test_encode_round_trips_plain_text(self):
        text = 'HELLO WORLD 123!'
        encoded = encode_text(text)
        self.assertEqual(encoded[-1], EOS)
        self.assertEqual(decode_text(encoded), text)

    def test_encode_round_trips_control_codes(self):
        text = 'Line one\nLine two\\p'
        encoded = encode_text(text)
        self.assertEqual(decode_text(encoded), text)

    def test_encode_without_terminator(self):
        encoded = encode_text('AB', terminate=False)
        self.assertEqual(encoded, bytes([0xBB, 0xBC]))

    def test_encode_placeholder_round_trip(self):
        text = 'A\\x99B'
        encoded = encode_text(text, terminate=False)
        self.assertEqual(encoded, bytes([0xBB, 0x99, 0xBC]))
        self.assertEqual(decode_text(encoded), text)

    def test_encode_rejects_unknown_character(self):
        with self.assertRaises(ValueError):
            encode_text('日本語')


if __name__ == '__main__':
    unittest.main()
