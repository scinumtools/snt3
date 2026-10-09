"""Offline command-boundary tests for the compiled snt hub module."""

from __future__ import annotations

import json
import hashlib
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


SNT = str(Path(sys.argv.pop(1)).resolve()) if len(sys.argv) > 1 else "snt"


def git(*args: str) -> str:
    return subprocess.run(["git", *args], check=True, text=True, capture_output=True).stdout.strip()


def commit(repo: Path, message: str) -> str:
    git("-C", str(repo), "-c", "user.name=SNT Test", "-c", "user.email=snt@example.invalid",
        "commit", "-m", message)
    return git("-C", str(repo), "rev-parse", "HEAD")


@unittest.skipIf(os.name == "nt", "The fixture adapter is a POSIX script")
class HubCLITest(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix="snt-hub-cli-")
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)

    def call(self, *args: str, env: dict | None = None, cwd: Path | None = None):
        return subprocess.run([SNT, "hub", *args], text=True, capture_output=True, env=env, cwd=cwd)

    def workspace_fixture(self, capabilities=None):
        source = self.root / "original"
        source.mkdir()
        git("init", "-q", str(source))
        (source / "original.txt").write_text("pinned source\n")
        git("-C", str(source), "add", ".")
        source_sha = commit(source, "source")
        hub = self.root / "published-hub"
        project = hub / "projects/project"
        bundle = project / "dipl"
        bundle.mkdir(parents=True)
        (hub / "hub").mkdir()
        (hub / "hub/pyproject.toml").write_text("[build-system]\nrequires = []\n")
        (bundle / "pyproject.toml").write_text("[build-system]\nrequires = []\n")
        (bundle / "model.dip").write_text("value int = 1\n")
        setups = {"schema_version": 1, "setups": {
            "complete": {"capability": "complete"},
            "inputs": {"capability": "native-inputs-only"},
        }}
        (project / "setups.json").write_text(json.dumps(setups))
        record = {
            "id": "project", "name": "Project", "source_url": source.as_uri(),
            "source_revision": source_sha, "source_path": "projects/project/source",
            "hub": {"adapter_protocol": 1, "runtime_package": "hub",
                    "adapter_package": "projects/project/dipl", "setup_manifest": "projects/project/setups.json",
                    "adapter_executable": "project-adapter"},
        }
        record["hub"].update(capabilities or {})
        (project / "project.json").write_text(json.dumps(record))
        (hub / ".gitmodules").write_text(
            '[submodule "projects/project/source"]\n'
            '\tpath = projects/project/source\n'
            f'\turl = {source.as_uri()}\n'
        )
        git("init", "-q", str(hub))
        git("-C", str(hub), "add", ".")
        git("-C", str(hub), "update-index", "--add", "--cacheinfo",
            f"160000,{source_sha},projects/project/source")
        hub_sha = commit(hub, "published hub")
        catalog = self.root / "catalogue.json"
        catalog.write_text(json.dumps({
            "schema_version": 1, "hub_repository": hub.as_uri(),
            "hub_revision": hub_sha, "projects": [{**record, "setups": setups["setups"]}],
        }))
        workspace = self.root / "study"
        workspace.mkdir()
        env = dict(os.environ, SNT_HUB_LOCAL_FIXTURE="1", SNT_HUB_CATALOG_URL=catalog.as_uri(),
                   GIT_ALLOW_PROTOCOL="file:https")
        return workspace, env, source_sha, hub_sha

    def test_help(self):
        help_text = self.call("--help").stdout
        self.assertIn("local SNT Hub workspace", help_text)
        self.assertNotIn("install PROJECT", help_text)
        self.assertIn("fetch PROJECT", self.call("fetch", "--help").stdout)
        self.assertIn("--override-file", self.call("setup", "--help").stdout)
        self.assertIn("--profile NAME", self.call("build", "--help").stdout)
        self.assertIn("--executable PATH", self.call("run", "--help").stdout)

    def test_removed_install_syntax_is_rejected(self):
        self.assertIn("Unknown hub command", self.call("install", "project").stderr)
        self.assertIn("Unknown hub option", self.call("list", "--prefix", str(self.root)).stderr)
        self.assertIn("Too many hub command arguments",
                      self.call("setup", "project", "demo").stderr)

    def test_fetch_workspace_and_discovery(self):
        workspace, env, source_sha, hub_sha = self.workspace_fixture()
        fetched = self.call("fetch", "project", cwd=workspace, env=env)
        self.assertEqual(fetched.returncode, 0, fetched.stderr)
        lock = json.loads((workspace / ".snthub/lock.json").read_text())
        self.assertEqual(lock["source_revision"], source_sha)
        self.assertEqual(lock["hub_revision"], hub_sha)
        self.assertEqual(git("-C", str(workspace / "source"), "rev-parse", "HEAD"), source_sha)
        self.assertTrue((workspace / "dipl/model.dip").is_file())
        self.assertFalse((workspace / ".snthub/hub/projects/project/source/original.txt").exists())
        self.assertFalse((workspace / ".snthub/runtime").exists())
        nested = workspace / "runs/complete"
        nested.mkdir(parents=True)
        listed = self.call("examples", cwd=nested, env=env)
        self.assertEqual(listed.returncode, 0, listed.stderr)
        self.assertIn("complete\tcomplete", listed.stdout)
        info = self.call("info", cwd=nested, env=env)
        self.assertEqual(info.returncode, 0, info.stderr)
        self.assertEqual(json.loads(info.stdout)["project"], "project")
        self.assertIn("Already fetched", self.call("fetch", "project", cwd=workspace, env=env).stdout)
        (workspace / "dipl/model.dip").write_text("value int = 2\n")
        self.assertIn("Already fetched", self.call("fetch", "project", cwd=workspace, env=env).stdout)
        self.assertEqual((workspace / "dipl/model.dip").read_text(), "value int = 2\n")

    def test_fetch_rejects_existing_paths(self):
        workspace, env, _, _ = self.workspace_fixture()
        (workspace / "source").mkdir()
        result = self.call("fetch", "project", cwd=workspace, env=env)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("already exists", result.stderr)
        self.assertFalse((workspace / ".snthub").exists())

    def test_fetch_rejects_gitlink_and_capability_mismatch(self):
        workspace, env, _, _ = self.workspace_fixture()
        hub = self.root / "published-hub"
        git("-C", str(hub), "update-index", "--add", "--cacheinfo",
            f"160000,{'a' * 40},projects/project/source")
        replacement = commit(hub, "wrong gitlink")
        catalog = self.root / "catalogue.json"
        data = json.loads(catalog.read_text())
        data["hub_revision"] = replacement
        catalog.write_text(json.dumps(data))
        result = self.call("fetch", "project", cwd=workspace, env=env)
        self.assertIn("gitlink disagrees", result.stderr)
        self.assertFalse((workspace / ".snthub").exists())
        git("-C", str(hub), "update-index", "--add", "--cacheinfo",
            f"160000,{data['projects'][0]['source_revision']},projects/project/source")
        data["hub_revision"] = commit(hub, "restore gitlink")
        data["projects"][0]["hub"]["run"] = {"protocol": 1, "modes": ["local"]}
        catalog.write_text(json.dumps(data))
        mismatch = self.call("fetch", "project", cwd=workspace, env=env)
        self.assertIn("disagree on hub.run", mismatch.stderr)
        self.assertFalse((workspace / ".snthub").exists())

    def test_build_and_run_require_reviewed_capabilities(self):
        workspace, env, _, _ = self.workspace_fixture()
        self.assertEqual(self.call("fetch", "project", cwd=workspace, env=env).returncode, 0)
        build = self.call("build", cwd=workspace, env=env)
        self.assertIn("no reviewed build recipe", build.stderr)
        execution = self.call("run", cwd=workspace, env=env)
        self.assertIn("no reviewed run recipe", execution.stderr)
        self.assertFalse((workspace / ".snthub/runtime").exists())

    def test_optional_build_and_run_dispatch(self):
        capabilities = {
            "build": {"protocol": 1, "profiles": ["local"],
                      "default_profile": "local", "requires_setup": True},
            "run": {"protocol": 1, "modes": ["local"]},
        }
        workspace, env, source_sha, hub_sha = self.workspace_fixture(capabilities)
        self.assertEqual(self.call("fetch", "project", cwd=workspace, env=env).returncode, 0)
        prepared = workspace / "runs/complete"
        prepared.mkdir(parents=True)
        (prepared / "setup-lock.json").write_text(json.dumps({
            "schema_version": 1, "project": "project", "hub_revision": hub_sha,
            "source_revision": source_sha, "capability": "complete",
        }))
        runtime = workspace / ".snthub/runtime/bin"
        runtime.mkdir(parents=True)
        packages = [{"name": "project", "version": "1"}]
        python = runtime / "python"
        python.write_text('#!/bin/sh\necho \'[{"name":"project","version":"1"}]\'\n')
        python.chmod(0o755)
        adapter = runtime / "project-adapter"
        adapter.write_text(
            "#!/usr/bin/env python3\n"
            "import json, os, sys\n"
            "from pathlib import Path\n"
            "a = sys.argv[1:]\n"
            "def value(key): return Path(a[a.index(key) + 1])\n"
            "if a[0] == 'build':\n"
            "    out = value('--output')\n"
            "    print('building in', out)\n"
            "    if os.environ.get('SNT_HUB_TEST_BUILD_EXIT'):\n"
            "        out.mkdir(parents=True)\n"
            "        (out / 'build.log').write_text('compiler error\\n')\n"
            "        print('build adapter failed', file=sys.stderr)\n"
            "        sys.exit(7)\n"
            "    out.mkdir(parents=True)\n"
            "    (out / 'solver').write_text('#!/bin/sh\\nexit 0\\n' + '#' * 70055)\n"
            "    (out / 'solver').chmod(0o755)\n"
            "    (out / 'build.log').write_text('built\\n')\n"
            "    (out / 'build-lock.json').write_text(json.dumps({'schema_version': 1, "
            "'executable': os.environ.get('SNT_HUB_TEST_BAD_EXECUTABLE', str(out / 'solver')), "
            "'build_log': str(out / 'build.log'), "
            "'command': ['make']}))\n"
            "elif a[0] == 'run':\n"
            "    if not os.environ.get('SNT_HUB_TEST_SKIP_LOCK'):\n"
            "        (value('--setup-dir') / 'run-lock.json').write_text(json.dumps("
            "{'command': [str(value('--executable'))], 'launcher': 'direct', 'outputs': []}))\n"
            "    sys.exit(int(os.environ.get('SNT_HUB_TEST_EXIT', '0')))\n"
        )
        adapter.chmod(0o755)
        (runtime.parent / "runtime-lock.json").write_text(json.dumps({
            "schema_version": 1, "project": "project", "hub_revision": hub_sha,
            "package_versions": packages,
        }))
        missing = self.call("build", cwd=workspace, env=env)
        self.assertIn("No prepared setup", missing.stderr)
        self.assertIn("Undeclared build profile",
                      self.call("build", "--profile", "other", cwd=workspace, env=env).stderr)
        failed_build = self.call("build", "--setup", "runs/complete", cwd=workspace,
                                 env=dict(env, SNT_HUB_TEST_BUILD_EXIT="1"))
        self.assertEqual(failed_build.returncode, 7)
        self.assertIn("build adapter failed", failed_build.stderr)
        self.assertIn("compiler error", failed_build.stderr)
        self.assertFalse((workspace / "build/local").exists())
        bad_path = self.call("build", "--setup", "runs/complete", cwd=workspace,
                             env=dict(env, SNT_HUB_TEST_BAD_EXECUTABLE=str(self.root / "outside")))
        self.assertIn("escapes build output", bad_path.stderr)
        self.assertFalse((workspace / "build/local").exists())
        built = self.call("build", "--setup", "runs/complete", cwd=workspace, env=env)
        self.assertEqual(built.returncode, 0, built.stderr)
        self.assertIn("Built local\n  Output: " + str((workspace / "build/local").resolve()), built.stdout)
        self.assertNotIn("building in", built.stdout)
        build_lock = json.loads((workspace / "build/local/build-lock.json").read_text())
        self.assertEqual(build_lock["source_revision"], source_sha)
        self.assertEqual(build_lock["executable"], "solver")
        self.assertEqual(build_lock["build_log"], "build.log")
        self.assertEqual(build_lock["executable_sha256"],
                         hashlib.sha256((workspace / "build/local/solver").read_bytes()).hexdigest())
        self.assertEqual(self.call("build", "--setup", "runs/complete", cwd=workspace,
                                   env=env).returncode, 2)
        completed = self.call("run", cwd=prepared, env=env)
        self.assertEqual(completed.returncode, 0, completed.stderr)
        run_lock = json.loads((prepared / "run-lock.json").read_text())
        self.assertEqual(run_lock["exit_status"], 0)
        self.assertEqual(run_lock["executable_source"], "workspace-build")
        self.assertEqual(self.call("run", cwd=prepared, env=env).returncode, 2)
        second = workspace / "runs/second"
        second.mkdir()
        (second / "setup-lock.json").write_bytes((prepared / "setup-lock.json").read_bytes())
        failed_run = self.call("run", "--setup", "runs/second", "--executable",
                               str(workspace / "build/local/solver"), cwd=workspace,
                               env=dict(env, SNT_HUB_TEST_EXIT="7"))
        self.assertEqual(failed_run.returncode, 7, failed_run.stderr)
        failed_lock = json.loads((second / "run-lock.json").read_text())
        self.assertEqual(failed_lock["exit_status"], 7)
        self.assertEqual(failed_lock["executable_source"], "user-supplied")
        third = workspace / "runs/third"
        third.mkdir()
        (third / "setup-lock.json").write_bytes((prepared / "setup-lock.json").read_bytes())
        no_adapter_lock = self.call("run", "--setup", "runs/third", "--executable",
                                    str(workspace / "build/local/solver"), cwd=workspace,
                                    env=dict(env, SNT_HUB_TEST_EXIT="7", SNT_HUB_TEST_SKIP_LOCK="1"))
        self.assertEqual(no_adapter_lock.returncode, 7, no_adapter_lock.stderr)
        self.assertEqual(json.loads((third / "run-lock.json").read_text())["exit_status"], 7)
        inputs_only = workspace / "runs/inputs"
        inputs_only.mkdir()
        (inputs_only / "setup-lock.json").write_text(json.dumps({
            "schema_version": 1, "project": "project", "hub_revision": hub_sha,
            "source_revision": source_sha, "capability": "native-inputs-only",
        }))
        rejected = self.call("run", "--setup", "runs/inputs", "--executable",
                             str(workspace / "build/local/solver"), cwd=workspace, env=env)
        self.assertIn("only complete setups can run", rejected.stderr)

    def test_workspace_setup_is_local_and_records_edits(self):
        workspace, env, _, hub_sha = self.workspace_fixture()
        self.assertEqual(self.call("fetch", "project", cwd=workspace, env=env).returncode, 0)
        runtime = workspace / ".snthub/runtime/bin"
        runtime.mkdir(parents=True)
        packages = [{"name": "project", "version": "1"}]
        python = runtime / "python"
        python.write_text('#!/bin/sh\necho \'[{"name":"project","version":"1"}]\'\n')
        python.chmod(0o755)
        adapter = runtime / "project-adapter"
        adapter.write_text(
            "#!/usr/bin/env python3\n"
            "import json, os, sys\n"
            "from pathlib import Path\n"
            "args = sys.argv[1:]\n"
            "output = Path(args[args.index('--output') + 1])\n"
            "print('creating in', output)\n"
            "if os.environ.get('SNT_HUB_TEST_EXIT'):\n"
            "    print('adapter failed', file=sys.stderr)\n"
            "    sys.exit(7)\n"
            "Path(os.environ['SNT_HUB_TEST_ARGS']).write_text(json.dumps(args))\n"
            "output.mkdir()\n"
            "(output / 'input.txt').write_text('prepared\\n')\n"
            "(output / 'setup-lock.json').write_text(json.dumps({'schema_version': 1, 'capability': "
            "'native-inputs-only' if '--inputs-only' in args else 'complete'}))\n"
        )
        adapter.chmod(0o755)
        (runtime.parent / "runtime-lock.json").write_text(json.dumps({
            "schema_version": 1, "project": "project", "hub_revision": hub_sha,
            "package_versions": packages,
        }))
        override = self.root / "input overrides.dip"
        override.write_text("value = 2\n")
        args_file = self.root / "arguments.json"
        env.update(SNT_HUB_TEST_ARGS=str(args_file))
        (workspace / "dipl/model.dip").write_text("value int = 2\n")
        setup = self.call("setup", "complete", "--override-file", str(override), cwd=workspace, env=env)
        self.assertEqual(setup.returncode, 0, setup.stderr)
        result = workspace / "runs/complete"
        self.assertIn("Prepared complete\n  Output: " + str(result.resolve()), setup.stdout)
        self.assertIn("Setup lock: setup-lock.json", setup.stdout)
        self.assertNotIn("creating in", setup.stdout)
        self.assertNotIn(".complete-setup-", setup.stdout)
        self.assertTrue((result / "input.txt").is_file())
        lock = json.loads((result / "setup-lock.json").read_text())
        self.assertEqual(lock["hub_revision"], hub_sha)
        self.assertTrue(lock["dipl_modified"])
        arguments = json.loads(args_file.read_text())
        self.assertEqual(arguments[arguments.index("--override-file") + 1], str(override.resolve()))
        self.assertEqual(Path(arguments[arguments.index("--bundle") + 1]).resolve(),
                         (workspace / "dipl").resolve())
        self.assertNotEqual(self.call("setup", "complete", cwd=workspace, env=env).returncode, 0)
        self.assertNotEqual(self.call("setup", "inputs", cwd=workspace, env=env).returncode, 0)
        failed = self.call("setup", "inputs", "--inputs-only", cwd=workspace,
                           env=dict(env, SNT_HUB_TEST_EXIT="1"))
        self.assertNotEqual(failed.returncode, 0)
        self.assertIn("adapter failed", failed.stderr)
        self.assertFalse((workspace / "runs/inputs").exists())
        inputs = self.call("setup", "inputs", "--inputs-only", cwd=workspace, env=env)
        self.assertEqual(inputs.returncode, 0, inputs.stderr)
        self.assertEqual(json.loads((workspace / "runs/inputs/setup-lock.json").read_text())
                         ["capability"], "native-inputs-only")

    def test_catalogue_rejects_unpublished_revision_and_unsafe_paths(self):
        project = {
            "id": "project", "name": "Project", "source_url": "https://example.invalid/source.git",
            "source_revision": "a" * 40, "setups": {},
            "hub": {"runtime_package": "hub", "adapter_package": "../escape",
                    "setup_manifest": "projects/project/setups.json",
                    "adapter_executable": "project-adapter", "adapter_protocol": 1},
        }
        catalogue = self.root / "catalogue.json"
        data = {"schema_version": 1, "hub_repository": "https://example.invalid/hub.git",
                "hub_revision": "b" * 40, "projects": [project]}
        catalogue.write_text(json.dumps(data))
        env = dict(os.environ, SNT_HUB_LOCAL_FIXTURE="1", SNT_HUB_CATALOG_URL=catalogue.as_uri())
        result = self.call("list", env=env)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("inside the Hub checkout", result.stderr)
        project["hub"]["adapter_package"] = "projects/project/dipl"
        data["hub_revision"] = "unreleased"
        catalogue.write_text(json.dumps(data))
        result = self.call("list", env=env)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("40-character", result.stderr)
        data["hub_revision"] = "b" * 40
        catalogue.write_text(json.dumps(data))
        result = self.call("list", env=env)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("project\tProject\t" + "a" * 40, result.stdout)



if __name__ == "__main__":
    unittest.main()
