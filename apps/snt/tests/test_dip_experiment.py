import sys
import re
import hashlib
import json
import os
import struct
import random

import pytest

from conftest import checked
from diph5_fingerprint_reference import digest as independent_configuration_digest, read_archive


def independent_hdf5():
    if os.environ.get("SNT_REQUIRE_H5PY") == "1":
        import h5py
        return h5py
    return pytest.importorskip("h5py")


def test_independent_candidate_configuration_vector(cli, tmp_path):
    independent_hdf5()
    archive = tmp_path / "candidate.diph5"
    checked(cli("dip", "parse", "-i", "string", "answer int = 7\n", "--save", archive))
    assert independent_configuration_digest(archive) == "b75c824d5e69cbbb6abed9f33adfc503cd0c347969087746044eb24dff64dcac"
    with independent_hdf5().File(archive, "r") as saved:
        provenance = saved["configuration/provenance"].attrs
        status = provenance["fingerprint_status"]
        digest = provenance["configuration_sha256"]
        assert (status.decode() if isinstance(status, bytes) else status) == "pass"
        assert (digest.decode() if isinstance(digest, bytes) else digest) == independent_configuration_digest(archive)
    rich = tmp_path / "rich.diph5"
    checked(cli("dip", "parse", "-i", "string",
                'a int8[2] = [-128, 127]\nb float32 = -0.0\nc str = "μ"\n', "--save", rich))
    expected = "a6fbef7eca599dacb5ae4aa9bb59269a2c1ed146fe409da84238573743d7bbff"
    assert independent_configuration_digest(rich) == expected
    tree = tmp_path / "tree.diph5"
    checked(cli("dip", "parse", "-i", "string",
                'physics\n  radius float64 = 4 m\n  limit int16 = 6\n'
                'catalog map\ncatalog[alpha]\n  label str = "x"\n', "--save", tree))
    assert independent_configuration_digest(tree) == "70e39d758705403cace4b6c6c7c4a36b1f46a4d548950b39ba23d526f65a4b5f"
    h5py = independent_hdf5()
    with h5py.File(rich, "r+") as f:
        group = f["configuration/resolved"]
        original = group["a"]
        values = original[:]
        attributes = [(name, original.attrs[name], original.attrs.get_id(name).dtype) for name in original.attrs]
        del group["a"]
        packed = group.create_dataset("a", data=values, chunks=(1,), compression="gzip")
        for name, value, dtype in attributes:
            packed.attrs.create(name, value, dtype=dtype)
    assert independent_configuration_digest(rich) == expected


def test_independent_archive_reader_handles_environment_and_experiment(cli, tmp_path):
    independent_hdf5()
    source = b'answer int = 7\nlabel str = "sample"\n'
    environment = tmp_path / "environment.diph5"
    experiment = tmp_path / "experiment.diph5"
    checked(cli("dip", "parse", "-i", "string", source.decode(), "--save", environment))
    read = read_archive(environment)
    assert read["profile"] == "environment" and read["state"] == "prepared"
    assert read["sources"] == [source]
    assert read["values"]["answer"] == 7
    assert read["values"]["label"] in (b"sample", "sample")
    assert read["results"] == []

    checked(cli("dip", "experiment", "prepare", "--input", environment, "--output", experiment))
    checked(cli("dip", "experiment", "start", "--file", experiment, "--cwd", tmp_path, "--", "sim"))
    checked(cli("dip", "experiment", "add", "--file", experiment, "--name", "energy", "--values", "2.5"))
    checked(cli("dip", "experiment", "finish", "--file", experiment, "--status", "completed"))
    read = read_archive(experiment)
    assert read["profile"] == "experiment" and read["state"] == "completed"
    assert read["sources"] == [source]
    assert read["values"]["answer"] == 7
    assert read["results"] == ["energy"]


def test_interrupted_run_remains_recoverable_and_unfinished(cli, tmp_path):
    h5py = independent_hdf5()
    environment = tmp_path / "environment.diph5"
    experiment = tmp_path / "interrupted.diph5"
    checked(cli("dip", "parse", "-i", "string", "answer int = 7\n", "--save", environment))
    checked(cli("dip", "experiment", "prepare", "--input", environment, "--output", experiment))
    checked(cli("dip", "experiment", "start", "--file", experiment, "--cwd", tmp_path, "--", "sim"))
    checked(cli("dip", "experiment", "add", "--file", experiment, "--name", "partial", "--values", "2.5"))

    # A stopped writer leaves its registered data readable, and never turns a
    # running attempt into a completed one merely because a result exists.
    with h5py.File(experiment, "r") as archive:
        assert archive.attrs["_DIPH5_State"] == b"running"
        assert archive["execution/outcome"].attrs["status"] == b"running"
        assert not bool(archive["results/manifest"].attrs["finalized"])
    assert read_archive(experiment)["results"] == ["partial"]
    assert "Status: running" in checked(cli("dip", "experiment", "show", "--file", experiment))
    checked(cli("dip", "experiment", "finish", "--file", experiment, "--status", "failed", "--exit-code", "1"))
    assert read_archive(experiment)["state"] == "failed"


def test_randomized_malformed_node_attributes_are_rejected(cli, tmp_path):
    h5py = independent_hdf5()
    import shutil

    original = tmp_path / "original.diph5"
    checked(cli("dip", "parse", "-i", "string", "answer int64 = 1000\n", "--save", original))
    mutations = [("_DIPL_Kind", "invalid"), ("_DIPL_Node_Type", "invalid"),
                 ("_DIPL_Value_Type", "int8"), ("_DIPL_Path", "other"),
                 ("_DIPL_Array", 2), ("_DIPL_Constant", 2), ("_DIPL_Override", 2)]
    rng = random.Random(20261103)
    for index in range(21):
        field, value = rng.choice(mutations)
        damaged = tmp_path / f"damaged-{index}.diph5"
        shutil.copyfile(original, damaged)
        with h5py.File(damaged, "r+") as archive:
            archive["configuration/resolved/answer"].attrs.modify(field, value)
        result = cli("dip", "parse", "--load", damaged, "-r", "answer", "--value")
        assert result.returncode != 0, field


def test_typed_numeric_results_are_stored_and_hashed_at_declared_width(cli, tmp_path):
    h5py = independent_hdf5()
    environment, archive = tmp_path / "env.diph5", tmp_path / "run.diph5"
    checked(cli("dip", "parse", "-i", "string", "answer int = 7\n", "--save", environment))
    checked(cli("dip", "experiment", "prepare", "--input", environment, "--output", archive))
    checked(cli("dip", "experiment", "start", "--file", archive, "--cwd", tmp_path, "--", "sim"))
    cases = [("int8", "-128,127", "b", [-128, 127]),
             ("int16", "-32768,32767", "h", [-32768, 32767]),
             ("int32", "-2147483648,2147483647", "i", [-2147483648, 2147483647]),
             ("int64", "-9223372036854775808,9223372036854775807", "q", [-9223372036854775808, 9223372036854775807]),
             ("uint8", "0,255", "B", [0, 255]), ("uint16", "0,65535", "H", [0, 65535]),
             ("uint32", "0,4294967295", "I", [0, 4294967295]),
             ("uint64", "0,18446744073709551615", "Q", [0, 18446744073709551615]),
             ("float32", "1.5,-2.25", "f", [1.5, -2.25]),
             ("float64", "1.5,-2.25", "d", [1.5, -2.25]),
             ("bool", "0,1", "B", [0, 1]), ("char", "-1,42", "b", [-1, 42])]
    golden = {
        "int8": "c4f9f86b002092ad9a988fbadea14cf8295cf434434a19272cebf3c78052ac91",
        "uint64": "dc49b5b6538e8d07a52a74bb8cbe24ec73bc42dddcddf04159bc2845dbb86b34",
        "float32": "c653b9cecb96ab3d43901dcf9cd7e52e281e8b4b0f591ccd7d6d1860d2213652",
        "float64": "33513fb83f8295b28b20fe7d7028cd07b00cc82f88b98cbdd5ce78553b02293d",
        "bool": "95ec3f3b94e79f708efb8499c1debf260fdc009b52432713229d5b8aacdcf624",
    }
    for dtype, values, _, _ in cases:
        checked(cli("dip", "experiment", "add", "--file", archive, "--name", dtype, "--dtype", dtype,
                    "--values", values, "--shape", "2"))
    assert cli("dip", "experiment", "add", "--file", archive, "--name", "bad", "--dtype", "int8",
               "--values", "128").returncode != 0
    checked(cli("dip", "experiment", "finish", "--file", archive, "--status", "completed"))
    with h5py.File(archive, "r") as f:
        def text(value):
            return value.decode("utf-8") if isinstance(value, bytes) else str(value)
        by_name = {text(x.attrs["name"]): x for x in f["results/datasets"].values()}
        for dtype, _, fmt, values in cases:
            item = by_name[dtype]
            data = item["data"]
            assert data.dtype.itemsize == struct.calcsize(fmt)
            assert data[:].tolist() == values
            prefix = b"diph5-result-v1" + struct.pack("<Q", len(dtype)) + dtype.encode()
            prefix += struct.pack("<QQQQ", 1, 2, 0, 0)
            expected = hashlib.sha256(prefix + struct.pack("<" + fmt * 2, *values)).hexdigest()
            if dtype in golden:
                assert expected == golden[dtype]
            assert text(item.attrs["data_sha256"]) == expected


@pytest.mark.parametrize("status", ["completed", "failed"])
def test_independent_result_hash_and_manifest(cli, tmp_path, status):
    h5py = independent_hdf5()
    environment = tmp_path / "env.diph5"
    archive = tmp_path / "run.diph5"
    artifact = tmp_path / "output.nc"
    artifact.write_bytes(b"CDF\x00independent fixture")
    checked(cli("dip", "parse", "-i", "string", "answer int = 7\n", "--save", environment))
    checked(cli("dip", "experiment", "prepare", "--input", environment, "--output", archive))
    checked(cli("dip", "experiment", "start", "--file", archive, "--cwd", tmp_path, "--", "sim"))
    checked(cli("dip", "experiment", "add", "--file", archive, "--name", "spectrum", "--values", "1,2,3,4",
                "--shape", "2,2", "--unit", "m", "--dimensions", "row,column"))
    checked(cli("dip", "experiment", "attach", "--file", archive, "--name", "output", "--artifact", artifact))
    checked(cli("dip", "experiment", "finish", "--file", archive, "--status", status))

    def text(value):
        return value.decode("utf-8") if isinstance(value, bytes) else str(value)

    def u64(value):
        return struct.pack("<Q", value)

    def string(value):
        value = text(value).encode("utf-8")
        return u64(len(value)) + value

    with h5py.File(archive, "r") as f:
        assert text(f.attrs["_DIPH5_State"]) == status
        source_manifest = []
        for source in f["model/sources"].values():
            digest = hashlib.sha256(source["content"][:].tobytes()).hexdigest()
            assert digest == text(source.attrs["sha256"])
            source_manifest.append({
                "source_id": text(source.attrs["source_id"]),
                "name": text(source.attrs["name"]),
                "parent_source_id": text(source.attrs["parent_source_id"]),
                "parent_line": int(source.attrs["parent_line"]),
                "role": text(source.attrs["role"]),
                "sha256": digest,
            })
        encoded_manifest = json.dumps(source_manifest, ensure_ascii=False, sort_keys=True,
                                      separators=(",", ":")).encode("utf-8")
        assert hashlib.sha256(encoded_manifest).hexdigest() == text(f["model/manifest"].attrs["model_sha256"])
        for override in f["configuration/overrides"].values():
            assert hashlib.sha256(override["content"][:].tobytes()).hexdigest() == text(override.attrs["sha256"])
        results = f["results"]
        manifest = results["manifest"]
        assert manifest.attrs["result_count"] == 2
        assert bool(manifest.attrs["finalized"]) == (status == "completed")
        dataset = next(iter(results["datasets"].values()))
        data = dataset["data"]
        dimensions = [text(x) for x in dataset["dimensions"][:]]
        payload = b"diph5-result-v1" + string("float64") + u64(len(data.shape))
        payload += b"".join(u64(x) for x in data.shape) + string(dataset.attrs["units"])
        payload += u64(len(dimensions)) + b"".join(string(x) for x in dimensions)
        payload += b"".join(struct.pack("<d", float(x)) for x in data[:].flat)
        assert hashlib.sha256(payload).hexdigest() == text(dataset.attrs["data_sha256"])
        attached = next(iter(results["artifacts"].values()))
        assert hashlib.sha256(artifact.read_bytes()).hexdigest() == text(attached.attrs["sha256"])
        for category in ("datasets", "artifacts"):
            for identifier, item in results[category].items():
                entry = manifest["entries"][identifier]
                assert text(entry.attrs["name"]) == text(item.attrs["name"])
                assert text(entry.attrs["path"]) == item.name


def test_managed_and_host_reported_experiments(cli, tmp_path):
    environment = tmp_path / "environment.diph5"
    managed = tmp_path / "managed.diph5"
    external = tmp_path / "external.diph5"
    imported = tmp_path / "imported.diph5"
    missing = tmp_path / "missing-command.diph5"
    signalled = tmp_path / "signalled.diph5"

    checked(cli("dip", "parse", "-i", "string", "answer int = 7\n", "--save", environment))
    assert environment.exists()

    checked(cli("dip", "experiment", "run", "--input", environment, "--output", managed,
                "--", sys.executable, "-c", "print('done')", "two words", ""))
    shown = checked(cli("dip", "experiment", "show", "--file", managed))
    assert "Status: completed\n" in shown
    assert "argv[3]: two words\n" in shown
    assert "argv[4]: \n" in shown
    assert "Exit code: 0\n" in shown
    assert re.search(r"Executable SHA-256: [0-9a-f]{64}\n", shown)
    assert checked(cli("dip", "parse", "--load", managed, "-r", "answer", "--value")) == "7\n"

    checked(cli("dip", "experiment", "prepare", "--input", environment, "--output", external))
    checked(cli("dip", "experiment", "start", "--file", external, "--origin", "host_reported",
                "--role", "outer_launcher",
                "--cwd", tmp_path, "--", "scheduler", "--job", "abc"))
    artifact = tmp_path / "output.nc"
    artifact.write_bytes(b"CDF\x00data")
    checked(cli("dip", "experiment", "add", "--file", external, "--name", "energy/J",
                "--values", "42.5", "--unit", "J"))
    checked(cli("dip", "experiment", "add", "--file", external, "--name", "spectrum",
                "--values", "1,2,3,4", "--shape", "2,2", "--dimensions", "row,column"))
    checked(cli("dip", "experiment", "diagnostic", "--file", external, "--name", "NaN count",
                "--values", "0", "--category", "observed"))
    checked(cli("dip", "experiment", "attach", "--file", external, "--name", "output.nc",
                "--artifact", artifact, "--format", "NetCDF"))
    inventory = checked(cli("dip", "experiment", "results", "--file", external))
    assert "energy/J" in inventory and "spectrum" in inventory and "output.nc" in inventory
    checked(cli("dip", "experiment", "finalize", "--file", external))
    assert cli("dip", "experiment", "add", "--file", external, "--name", "late",
               "--values", "1").returncode != 0
    checked(cli("dip", "experiment", "finish", "--file", external,
                "--status", "failed", "--exit-code", "42"))
    verification = checked(cli("dip", "experiment", "verify", "--file", external))
    assert "Integrity: pass" in verification
    assert "Configuration fingerprint: pass" in verification
    artifact.write_bytes(b"changed")
    verify = cli("dip", "experiment", "verify", "--file", external)
    assert verify.returncode != 0 and b"External artifacts: fail" in verify.stdout
    external_show = checked(cli("dip", "experiment", "show", "--file", external))
    assert "Status: failed\n" in external_show
    assert "Command role: outer_launcher\n" in external_show
    assert cli("dip", "experiment", "finish", "--file", external, "--status", "completed").returncode != 0

    checked(cli("dip", "experiment", "import", "--input", environment, "--output", imported,
                "--status", "aborted"))
    shown = checked(cli("dip", "experiment", "show", "--file", imported))
    assert "Status: aborted\n" in shown
    assert "argv[" not in shown

    result = cli("dip", "experiment", "run", "--input", environment, "--output", missing,
                 "--", "snt-command-that-does-not-exist")
    assert result.returncode != 0
    assert "Status: failed\n" in checked(cli("dip", "experiment", "show", "--file", missing))

    checked(cli("dip", "experiment", "prepare", "--input", environment, "--output", signalled))
    checked(cli("dip", "experiment", "start", "--file", signalled,
                "--cwd", tmp_path, "--", "scheduler", "job"))
    checked(cli("dip", "experiment", "finish", "--file", signalled,
                "--status", "failed", "--termination-signal", "9"))
    shown = checked(cli("dip", "experiment", "show", "--file", signalled))
    assert "Termination signal: 9\n" in shown
    assert "Exit code:" not in shown
