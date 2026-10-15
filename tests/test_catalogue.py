"""Run the actual interactive C program in isolated temporary working directories.

All accounts and planet records below are fictional test fixtures.
"""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

BINARY = Path(sys.argv.pop(1) if len(sys.argv) > 1 else 'build/planet_catalogue').resolve()


class CatalogueTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.directory = Path(self.temp.name)

    def tearDown(self):
        self.temp.cleanup()

    def run_app(self, text):
        result = subprocess.run([str(BINARY)], input=text, text=True,
                                cwd=self.directory, capture_output=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertNotIn('runtime error:', result.stderr)
        self.assertNotIn('AddressSanitizer', result.stderr)
        return result.stdout

    def seed_accounts(self):
        (self.directory / 'land.txt').write_text('admin_demo demo_only 1\nresearcher_demo demo_only 0\n')
        (self.directory / 'Researchers.txt').write_text('REGISTERED RESEARCHERS: 1\n')

    def login(self, role, action):
        return f'1\n{role}_demo\ndemo_only\n{action}\n'

    def add(self, name='Demo-A', discovered=1, observed=0):
        return self.login('admin', 1) + f'admin_demo\n{name}\n1.50\n2e6\n24\n4\n100\n3\n{discovered}\n{observed}\n'

    def read_catalogue(self):
        return (self.directory / 'planetcat.txt').read_text()

    def seed_planets(self, rows):
        (self.directory / 'planetcat.txt').write_text(
            'TOTAL COUNT: 0\nDiscovered Planets: 0\nObserved Planets: 0\n' + '\n'.join(rows) + '\n')

    def row(self, name, discovered=1, observed=0):
        return f'{name}\t1.50\t2e6\t24.00\t4.00\t100.00\t3.00\t2026-01-01 12:00:00\t{discovered}\t{observed}\tadmin_demo'

    def test_first_run_and_empty_statistics(self):
        self.run_app('4\n')
        self.assertEqual((self.directory / 'land.txt').read_text(), '')
        self.assertIn('TOTAL COUNT: 0', self.read_catalogue())
        self.seed_accounts()
        output = self.run_app(self.login('researcher', 2) + 'n\n')
        self.assertIn('TOTAL NUMBER OF CATALOGUED PLANETS: 0', output)
        self.assertIn('TOTAL NUMBER OF REGISTERED RESEARCHERS: 1', output)

    def test_registration_roles_and_duplicate_after_restart(self):
        output = self.run_app('2\nadmin_demo\ndemo_only\ndemo_only\n1\nn\n')
        self.assertIn('ACCOUNT CREATED SUCCESSFULLY', output)
        output = self.run_app('2\nresearcher_demo\ndemo_only\ndemo_only\n2\nn\n')
        self.assertIn('ACCOUNT CREATED SUCCESSFULLY', output)
        self.assertEqual((self.directory / 'Researchers.txt').read_text(), 'REGISTERED RESEARCHERS: 1')
        output = self.run_app('2\nbad_demo\nfirst_demo\nsecond_demo\nn\n')
        self.assertIn('PASSWORD ARE NOT SAME', output)
        output = self.run_app('2\nresearcher_demo\nn\n')
        self.assertIn('NAME USED', output)
        self.assertEqual(len((self.directory / 'land.txt').read_text().splitlines()), 2)
        output = self.run_app('1\nadmin_demo\nwrong_demo\nn\n')
        self.assertIn('PASSWORD WRONG', output)
        output = self.run_app('1\nmissing_demo\nwrong_demo\nn\n')
        self.assertIn('ACCOUNT DOES NOT EXIST', output)

    def test_add_search_modify_delete_and_fresh_contributions(self):
        self.seed_accounts()
        output = self.run_app(self.add() + 'y\n' + self.add('Demo-B', 0, 1) + 'n\n')
        self.assertEqual(output.count('Data saved successfully!'), 2)
        self.assertTrue(self.read_catalogue().startswith('TOTAL COUNT: 2\nDiscovered Planets: 1\nObserved Planets: 1\n'))
        # Populate the linked-list cache, modify a flag, delete a different row,
        # then query contributions again within the same process.
        text = ('3\nadmin_demo\ny\n' + self.login('admin', 2) + 'Demo-A\n9\n1\ny\n'
                + self.login('admin', 4) + 'Demo-A\n1\ny\n3\nadmin_demo\nn\n')
        output = self.run_app(text)
        self.assertIn('Planet modified successfully!', output)
        self.assertIn('Planet deleted successfully!', output)
        final_query = output.rsplit('Please enter your account number:', 1)[1]
        self.assertIn('Name: Demo-B', final_query)
        self.assertNotIn('Name: Demo-A', final_query)
        self.assertTrue(self.read_catalogue().startswith('TOTAL COUNT: 1\nDiscovered Planets: 0\nObserved Planets: 1\n'))
        self.assertEqual(self.read_catalogue().count('TOTAL COUNT:'), 1)

    def test_all_seven_search_criteria_and_multiword_name(self):
        self.seed_accounts()
        self.run_app(self.add('Demo World') + 'n\n')
        for criterion, value in ((1, 'Demo World'), (2, '1.5'), (3, '2e6'), (4, '24'),
                                 (5, '4'), (6, '100'), (7, '3')):
            with self.subTest(criterion=criterion):
                output = self.run_app(self.login('researcher', 1) + f'{criterion}\n{value}\nn\n')
                self.assertIn('Name: Demo World', output)
        output = self.run_app(self.login('researcher', 1) + '1\nMissing World\nn\n')
        self.assertIn('No planets found matching', output)

    def test_shorter_password_rewrite_does_not_leave_stale_accounts(self):
        self.seed_accounts()
        output = self.run_app(self.login('admin', 5) + 'researcher_demo\nx\nn\n')
        self.assertIn('SUCCESSFULLY UPDATED', output)
        rows = (self.directory / 'land.txt').read_text().splitlines()
        self.assertEqual(len(rows), 2)
        self.assertIn('researcher_demo x 0', rows)
        output = self.run_app('1\nresearcher_demo\nx\n2\nn\n')
        self.assertIn('RESEARCHER LOGIN', output)

    def test_large_catalogue_and_count_digit_boundary(self):
        self.seed_accounts()
        self.seed_planets([self.row(f'Demo-{i}', 1, int(1 <= i <= 9)) for i in range(100)])
        output = self.run_app(self.login('admin', 2) + 'Demo-0\n9\n1\nn\n')
        self.assertIn('Planet modified successfully!', output)
        self.assertTrue(self.read_catalogue().startswith('TOTAL COUNT: 100\nDiscovered Planets: 100\nObserved Planets: 10\n'))
        before = self.read_catalogue()
        output = self.run_app(self.login('admin', 1) + 'n\n')
        self.assertIn('PLANET ARRAY IS FULL', output)
        self.assertEqual(before, self.read_catalogue())
        output = self.run_app(self.login('admin', 4) + 'Demo-0\n1\nn\n')
        self.assertIn('Planet deleted successfully!', output)
        self.assertTrue(self.read_catalogue().startswith('TOTAL COUNT: 99\nDiscovered Planets: 99\nObserved Planets: 9\n'))

    def test_cancelled_and_missing_deletions_preserve_data(self):
        self.seed_accounts()
        self.run_app(self.add() + 'n\n')
        before = self.read_catalogue()
        output = self.run_app(self.login('admin', 4) + 'Demo-A\n0\nn\n')
        self.assertIn('Deletion cancelled', output)
        self.assertEqual(before, self.read_catalogue())
        output = self.run_app(self.login('admin', 2) + 'Demo-A\n99\nn\n')
        self.assertIn('Invalid choice!', output)
        self.assertEqual(before, self.read_catalogue())
        output = self.run_app(self.login('admin', 4) + 'Missing\nn\n')
        self.assertIn('Planet not found', output)
        self.assertEqual(before, self.read_catalogue())

    def test_invalid_numeric_input_and_eof_do_not_hang(self):
        output = self.run_app('not-a-number\n4\n')
        self.assertIn('Invalid input. Try again:', output)
        output = self.run_app('')
        self.assertIn('Input ended.', output)


if __name__ == '__main__':
    unittest.main(verbosity=2)
