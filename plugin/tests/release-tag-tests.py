#!/usr/bin/env python3
import importlib.util
from pathlib import Path
import subprocess
import tempfile
import unittest

spec=importlib.util.spec_from_file_location('release_tag',Path(__file__).resolve().parents[1]/'scripts/release-tag.py')
release=importlib.util.module_from_spec(spec);spec.loader.exec_module(release)

class ReleaseTagTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.root=Path(self.temp.name)
        self.git('init','-b','main');self.git('config','user.name','Build test');self.git('config','user.email','build@example.invalid')
        (self.root/'plugin').mkdir();(self.root/'plugin/CMakeLists.txt').write_text('project(Undertow VERSION 1.2.2 LANGUAGES C CXX)\n')
        self.git('add','.');self.git('commit','-m','Fixture')
    def tearDown(self): self.temp.cleanup()
    def git(self,*args): return subprocess.check_output(['git','-C',str(self.root),*args],stderr=subprocess.STDOUT,text=True).strip()
    def test_matching_annotated_tag_accepted(self):
        self.git('tag','-a','v1.2.2','-m','Release')
        release.validate_tag(self.root,'v1.2.2',main_ref='main')
    def test_lightweight_tag_rejected(self):
        self.git('tag','v1.2.2')
        with self.assertRaisesRegex(ValueError,'annotated'): release.validate_tag(self.root,'v1.2.2',main_ref='main')
    def test_version_mismatch_rejected(self):
        self.git('tag','-a','v2.0.0','-m','Wrong version')
        with self.assertRaisesRegex(ValueError,'version'): release.validate_tag(self.root,'v2.0.0',main_ref='main')
    def test_tag_must_point_to_checked_out_commit(self):
        self.git('tag','-a','v1.2.2','-m','Release');(self.root/'new').write_text('change')
        self.git('add','.');self.git('commit','-m','New work')
        with self.assertRaisesRegex(ValueError,'HEAD'): release.validate_tag(self.root,'v1.2.2',main_ref='main')
    def test_unmerged_topic_commit_cannot_be_released(self):
        self.git('switch','-c','test-topic');(self.root/'new').write_text('change')
        self.git('add','.');self.git('commit','-m','Unmerged');self.git('tag','-a','v1.2.2','-m','Release')
        with self.assertRaisesRegex(ValueError,'main'): release.validate_tag(self.root,'v1.2.2',main_ref='main')
    def test_non_version_tag_rejected(self):
        with self.assertRaisesRegex(ValueError,'vMAJOR'): release.validate_tag(self.root,'latest',main_ref='main')

if __name__=='__main__': unittest.main()
