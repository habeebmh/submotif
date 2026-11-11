#!/usr/bin/env python3
"""Semantic fixtures for descriptive audit metrics; no live generator required."""
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('audit', Path(__file__).resolve().parents[1] / 'scripts/audit_generator.py')
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


def note(start, pitch=36, duration=.25, velocity=100):
    return dict(start=start, pitch=pitch, duration=duration, velocity=velocity, locked=False)


def row(notes, bars=2):
    pattern = dict(notes=notes, lengthBeats=bars * 4, seed=1)
    return dict(pattern=pattern, variation=pattern)


class AuditMetricsTests(unittest.TestCase):
    def test_long_phrase_density_and_empty_bars(self):
        result = audit.describe(row([note(0), note(4)], 4))
        self.assertEqual(result['notes_per_bar'], .5)
        self.assertEqual(result['empty_bar_pct'], 50)
        # Legacy bar-comparison fields continue to compare bars one and two.
        self.assertEqual(result['exact_bar_pitch_rhythm_pct'], 100)

    def test_articulation_retention_is_conditional_on_shared_onsets(self):
        result = audit.describe(row([note(0), note(1), note(4, 43, .5, 90)]))
        self.assertAlmostEqual(result['repeat_lag1_onset_overlap_pct'], 200 / 3)
        for dimension in ('pitch', 'duration', 'velocity', 'full_note'):
            self.assertEqual(result[f'repeat_lag1_{dimension}_retention_pct'], 0)
        self.assertIsNone(result['repeat_lag2_onset_overlap_pct'])

    def test_two_bar_call_answer_is_visible_at_lag_two(self):
        result = audit.describe(row([note(0), note(5, 43), note(8), note(13, 43)], 4))
        self.assertEqual(result['repeat_lag1_onset_overlap_pct'], 0)
        self.assertIsNone(result['repeat_lag1_pitch_retention_pct'])
        for dimension in ('pitch', 'duration', 'velocity', 'full_note'):
            self.assertEqual(result[f'repeat_lag2_{dimension}_retention_pct'], 100)
        self.assertEqual(result['repeat_lag2_onset_overlap_pct'], 100)

    def test_diversity_ignores_articulation_and_small_timing_jitter(self):
        a = row([note(.01), note(4)])
        b = row([note(.02, duration=.5, velocity=80), note(4)])
        c = row([note(.01, 43), note(4)])
        result = audit.describe_diversity([a, b, c])
        self.assertEqual(result['distinct_first_bar_onset_motifs'], 1)
        self.assertEqual(result['distinct_first_bar_pitch_onset_motifs'], 2)
        self.assertAlmostEqual(result['first_bar_pitch_onset_collision_pct'], 100 / 3)
        self.assertEqual(result['distinct_first_two_bar_pitch_onset_motifs'], 2)
        self.assertIsNone(audit.describe_diversity([a])['first_bar_pitch_onset_collision_pct'])

    def test_second_bar_changes_do_not_inflate_first_bar_diversity(self):
        result = audit.describe_diversity([row([note(0), note(4)]), row([note(0), note(5)])])
        self.assertEqual(result['distinct_first_bar_onset_motifs'], 1)
        self.assertEqual(result['distinct_first_two_bar_onset_motifs'], 2)
        self.assertEqual(result['first_bar_pitch_onset_collision_pct'], 100)
        self.assertEqual(result['first_two_bar_pitch_onset_collision_pct'], 0)


if __name__ == '__main__':
    unittest.main()
