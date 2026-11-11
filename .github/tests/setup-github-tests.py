#!/usr/bin/env python3
"""Offline regressions for the GitHub protection request contract."""
import json
from pathlib import Path
import subprocess
import types
import unittest
from unittest.mock import patch


SCRIPT = Path(__file__).resolve().parents[1] / 'scripts/setup-github.sh'
setup = types.ModuleType('setup_github')
source = SCRIPT.read_text().split("<<'PY'\n", 1)[1].rsplit('\nPY', 1)[0]
exec(compile(source, str(SCRIPT), 'exec'), setup.__dict__)


class ProtectionRequests(unittest.TestCase):
    def test_new_protection_uses_only_app_bound_checks(self):
        desired = setup.desired_protection(setup.normalize_protection(None), 15368)
        status = desired['required_status_checks']
        # GitHub rejects a request containing both contexts and checks: its
        # mutually exclusive schema alternatives would both match.
        self.assertNotIn('contexts', status)
        self.assertEqual(status, {'strict': True, 'checks': [
            {'context': 'Portable checks', 'app_id': 15368}]})
        self.assertEqual(desired['required_pull_request_reviews'], {
            'dismiss_stale_reviews': True, 'require_code_owner_reviews': True,
            'require_last_push_approval': False, 'required_approving_review_count': 1})
        self.assertTrue(desired['required_conversation_resolution'])
        self.assertFalse(desired['allow_force_pushes'])
        self.assertFalse(desired['allow_deletions'])
        self.assertFalse(desired['enforce_admins'])

    def test_get_contexts_are_not_echoed_into_put_or_unbind_other_checks(self):
        current = setup.normalize_protection({'required_status_checks': {
            'strict': False, 'contexts': ['Security scan', 'Portable checks'],
            'checks': [{'context': 'Security scan', 'app_id': 42},
                       {'context': 'Portable checks', 'app_id': None}]}})
        desired = setup.desired_protection(current, 15368)
        self.assertEqual(desired['required_status_checks'], {'strict': True, 'checks': [
            {'context': 'Portable checks', 'app_id': 15368},
            {'context': 'Security scan', 'app_id': 42}]})

    def test_validation_error_is_actionable_without_exposing_tokens_or_cli_debug(self):
        token = 'ghp_' + 'x' * 40
        response = subprocess.CompletedProcess([], 1, json.dumps({
            'message': 'Invalid request.\nMore than one subschema in "oneOf" matched. ' + token,
            'status': '422'}), 'Authorization: Bearer private-cli-debug\ngh: HTTP 422')
        with patch.object(setup.subprocess, 'run', return_value=response):
            with self.assertRaises(setup.SetupError) as caught:
                setup.api(setup.PROTECTION, method='PUT', payload={})
        message = str(caught.exception)
        self.assertIn('HTTP 422', message)
        self.assertIn('More than one subschema', message)
        self.assertNotIn(token, message)
        self.assertNotIn('private-cli-debug', message)


if __name__ == '__main__':
    unittest.main()
