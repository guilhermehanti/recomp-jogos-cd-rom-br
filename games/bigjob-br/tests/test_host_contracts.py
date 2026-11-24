"""Regression checks for ABI mistakes that would corrupt a native bridge."""
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class HostContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.boundaries = json.loads((ROOT / 'analysis/api-boundaries.json').read_text(encoding='utf8'))
        cls.imports = {r['target']: r for r in cls.boundaries['imports']}

    def test_printer_dc_is_four_far_arguments(self):
        row = self.imports['GDI!53']
        self.assertEqual(row['name'], 'CreateDC')
        self.assertEqual(row['argument_widths'], [4, 4, 4, 4])
        self.assertEqual(row['purge_bytes'], 16)

    def test_dib_operation_is_stretch_with_rop(self):
        row = self.imports['GDI!439']
        self.assertEqual(row['name'], 'StretchDIBits')
        self.assertEqual(row['argument_types'][-1], 'long')
        self.assertEqual(row['purge_bytes'], 32)

    def test_selector_shift_is_not_callable(self):
        row = self.imports['KERNEL!113']
        self.assertEqual(row['kind'], 'constant')
        self.assertEqual(row['name'], '__AHSHIFT')
        self.assertEqual(row['constant_value'], 3)
        self.assertIsNone(row['purge_bytes'])
        self.assertEqual(len(row['sites']), 128)

    def test_dynamic_task_enumeration(self):
        evidence = self.boundaries['dynamic_ordinal_evidence']
        self.assertIn('TaskFirst', evidence['TOOLHELP!63']['definition'])
        self.assertIn('TaskNext', evidence['TOOLHELP!64']['definition'])
        self.assertEqual(self.boundaries['dgroup_dynamic_strings']['15CB'], 'QTNOTIFY')

    def test_os_sites_keep_original_instructions(self):
        sites = self.boundaries['os_fixup_sites']
        self.assertEqual(len(sites), 98)
        counts = {opcode: sum(r['original_bytes'].startswith(opcode) for r in sites)
                  for opcode in ('9b', '90')}
        self.assertEqual(counts, {'9b': 87, '90': 11})


if __name__ == '__main__':
    unittest.main()
