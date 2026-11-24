"""Synthetic-only format boundary tests; no original game execution."""
import struct
import unittest
from tools.read_save import read_save


def fixture(order="<", count=1, tool=12):
    return ((b"II" if order == "<" else b"MM")
            + struct.pack(order + "HHh", 0xF0AD, 10, count)
            + (struct.pack(order + "h", tool) + b"Ana\0" + bytes(8)
               + bytes(range(36)) + bytes(range(18)) + bytes(range(9))) * max(count, 0))


class SaveTests(unittest.TestCase):
    def test_both_endian_orders_and_every_field_boundary(self):
        for order in ("<", ">"):
            cards = read_save(fixture(order, 2))
            self.assertEqual(len(cards), 2)
            self.assertEqual(cards[0].tool, 12)
            self.assertEqual(cards[0].name, b"Ana\0" + bytes(8))
            self.assertEqual(cards[0].flags, bytes(range(36)))
            self.assertEqual(cards[0].progress, bytes(range(18)))
            self.assertEqual(cards[0].counters, bytes(range(9)))

    def test_empty(self):
        self.assertEqual(read_save(fixture(count=0)), ())

    def test_every_truncation(self):
        data = fixture()
        for length in range(len(data)):
            with self.assertRaises(ValueError):
                read_save(data[:length])

    def test_invalid_headers_tools_and_policy(self):
        original = fixture()
        for offset, replacement in ((0, b"XX"), (2, b"\0\0"),
                                     (4, b"\x0b\0"), (6, b"\xff\xff")):
            data = original[:offset] + replacement + original[offset + 2:]
            with self.assertRaises(ValueError):
                read_save(data)
        for data in (fixture(tool=-1), fixture(tool=13), fixture(count=51), original + b"x"):
            with self.assertRaises(ValueError):
                read_save(data)


if __name__ == "__main__":
    unittest.main()
