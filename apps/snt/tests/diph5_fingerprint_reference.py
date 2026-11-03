"""Small independent DIPH5 v3 reader and canonical configuration encoder.

The reader intentionally uses h5py alone. It exercises the documented archive
layout without calling SNT's native loader.
"""

import hashlib
import struct


def read_archive(path):
    """Read the portable v3 contract and result inventory from *path*.

    Return the profile, state, original source bytes, resolved scalar/array
    values, and result names. Reject a broken source manifest or fingerprint.
    """
    import h5py
    import json

    def text(value):
        return value.decode("utf-8") if isinstance(value, bytes) else str(value)

    with h5py.File(path, "r") as archive:
        assert text(archive.attrs["_DIPH5_Format"]) == "SciNumTools3 DIPH5"
        assert int(archive.attrs["_DIPH5_Schema_Major"]) == 3
        profile = text(archive.attrs["_DIPH5_Profile"])
        state = text(archive.attrs["_DIPH5_State"])
        assert profile in ("environment", "experiment")
        assert ("execution" in archive) == (profile == "experiment")

        sources = []
        manifest = []
        for key, source in sorted(archive["model/sources"].items()):
            assert key == f"{len(sources):06d}"
            content = source["content"][:].tobytes()
            sha256 = hashlib.sha256(content).hexdigest()
            assert sha256 == text(source.attrs["sha256"])
            sources.append(content)
            manifest.append({
                "source_id": text(source.attrs["source_id"]),
                "name": text(source.attrs["name"]),
                "parent_source_id": text(source.attrs["parent_source_id"]),
                "parent_line": int(source.attrs["parent_line"]),
                "role": text(source.attrs["role"]),
                "sha256": sha256,
            })
        encoded = json.dumps(manifest, ensure_ascii=False, sort_keys=True,
                             separators=(",", ":")).encode("utf-8")
        model_hash = hashlib.sha256(encoded).hexdigest()
        assert model_hash == text(archive["model/manifest"].attrs["model_sha256"])
        provenance = archive["configuration/provenance"].attrs
        assert model_hash == text(provenance["model_sha256"])
        assert text(provenance["fingerprint_status"]) == "pass"
        stored_fingerprint = text(provenance["configuration_sha256"])

        values = {}
        def visit(group):
            for child in group.values():
                if isinstance(child, h5py.Group):
                    visit(child)
                elif "_DIPL_Path" in child.attrs:
                    value = child[()]
                    values[text(child.attrs["_DIPL_Path"])] = (
                        value.tolist() if hasattr(value, "tolist") else value
                    )
        visit(archive["configuration/resolved"])

        results = []
        if "results" in archive:
            for category in ("datasets", "artifacts", "diagnostics"):
                if category in archive["results"]:
                    results.extend(text(item.attrs["name"])
                                   for item in archive[f"results/{category}"].values())

    # Reopen through the separate canonical encoder; it does not trust the
    # archive's stored digest or its physical HDF5 layout.
    assert stored_fingerprint == digest(path)
    return {"profile": profile, "state": state, "sources": sources,
            "values": values, "results": sorted(results)}


def digest(path):
    import h5py
    import numpy as np

    hasher = hashlib.sha256()

    def number(value):
        hasher.update(struct.pack("<Q", value))

    def text(value):
        if isinstance(value, bytes):
            value = value.decode("utf-8")
        encoded = value.encode("utf-8")
        number(len(encoded))
        hasher.update(encoded)

    def normalized_presence(values):
        decoded = [v.decode("utf-8") if isinstance(v, bytes) else str(v) for v in values]
        return [value for value in decoded if value not in ("created", "modified")]

    def attributes(obj):
        names = sorted(
            name for name in obj.attrs
            if name not in ("created", "modified")
            and (name != "metadata_present" or normalized_presence(obj.attrs[name]))
        )
        number(len(names))
        for name in names:
            value = obj.attrs[name]
            dtype = obj.attrs.get_id(name).dtype
            rank = np.ndim(value)
            text(name)
            if dtype.kind == "S" and rank == 0:
                hasher.update(b"s")
                text(value)
            elif dtype.kind == "S" and rank == 1:
                hasher.update(b"l")
                if name == "metadata_present":
                    values = normalized_presence(value)
                else:
                    values = [v.decode("utf-8") if isinstance(v, bytes) else str(v) for v in value]
                number(len(values))
                for item in values:
                    text(item)
            elif dtype.kind == "u" and rank == 0:
                hasher.update(b"u")
                number(int(value))
            else:
                raise ValueError(f"Unsupported attribute {name}: {dtype}, rank {rank}")

    def dataset(obj):
        shape = obj.shape
        if shape is None:
            hasher.update(b"n")
            return
        hasher.update(b"v")
        number(len(shape))
        for extent in shape:
            number(extent)
        dtype = obj.attrs["_DIPL_Value_Type"].decode("utf-8")
        if dtype == "str":
            for value in np.array(obj[()]).reshape(-1):
                text(value)
        else:
            types = {
                "bool": "u1", "char": "i1", "int8": "i1", "int16": "<i2", "int32": "<i4",
                "int64": "<i8", "uint8": "u1", "uint16": "<u2", "uint32": "<u4", "uint64": "<u8",
                "float32": "<f4", "float64": "<f8",
            }
            hasher.update(np.asarray(obj[()], dtype=types[dtype]).tobytes(order="C"))

    def group(parent):
        names = sorted(parent.keys())
        number(len(names))
        for name in names:
            child = parent[name]
            is_group = isinstance(child, h5py.Group)
            hasher.update(b"g" if is_group else b"d")
            text(name)
            attributes(child)
            if is_group:
                group(child)
            else:
                dataset(child)

    with h5py.File(path, "r") as archive:
        hasher.update(b"diph5-3.0")
        text(archive["model/manifest"].attrs["model_sha256"])
        overrides = archive["configuration/overrides"]
        number(len(overrides))
        for override in overrides.values():
            text(override.attrs["sha256"])
        group(archive["configuration/resolved"])
    return hasher.hexdigest()
