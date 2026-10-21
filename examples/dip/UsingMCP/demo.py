"""Prepare a validated channel-flow candidate using only the public MCP interface."""

import argparse
import asyncio
import base64
import hashlib
import json
from pathlib import Path
import shutil

from mcp import Client
from mcp.client.stdio import StdioServerParameters


async def demonstrate(options):
    here = Path(__file__).resolve().parent
    if options.url:
        server = options.url
    else:
        executable = shutil.which(options.snt) or str(Path(options.snt).resolve())
        server = StdioServerParameters(command=executable, args=[
            "mcp", "--project", f"{options.environment}={here / 'DIPfile'}"])
    destination = Path(options.output_dir).resolve()
    destination.mkdir(parents=True, exist_ok=True)
    if any(destination.iterdir()):
        raise RuntimeError(f"Output directory must be empty to avoid overwriting files: {destination}")

    async with Client(server, mode="legacy" if options.legacy else "auto") as client:
        tools = {tool.name for tool in (await client.list_tools()).tools}
        required = {"dipl_snapshot", "dipl_preview", "dipl_compare", "dipl_export", "artifact_read"}
        assert required <= tools, f"Server is missing required tools: {required - tools}"
        print(f"Connected: discovered {len(tools)} tools")
        guide = await client.read_resource("snt://help/dipl")
        assert guide.contents

        async def read_bytes(artifact):
            data = bytearray()
            offset = 0
            while True:
                response = await client.call_tool("artifact_read", {
                    "id": artifact["id"], "offset": offset, "length": 1024})
                assert not response.is_error, response
                chunk = response.structured_content
                assert chunk["offset"] == len(data)
                decoded = base64.b64decode(chunk["data"], validate=True)
                assert len(decoded) == chunk["length"]
                data.extend(decoded)
                if chunk["next_offset"] is None:
                    break
                assert chunk["next_offset"] > offset
                offset = chunk["next_offset"]
            assert len(data) == artifact["size"]
            assert hashlib.sha256(data).hexdigest() == artifact["sha256"]
            return bytes(data)

        async def call(name, **arguments):
            response = await client.call_tool(name, arguments)
            assert not response.is_error, response
            value = response.structured_content
            if "result_ref" in value:
                value = json.loads(await read_bytes(value["result_ref"]))
            return value

        async def save(artifact, filename):
            path = destination / filename
            path.write_bytes(await read_bytes(artifact))
            print(f"  Saved {filename} ({artifact['size']} bytes; SHA-256 verified)")

        environments = await call("dipl_environments")
        source = next(item for item in environments["environments"] if item["name"] == options.environment)
        assert source["capabilities"]["overrides"]
        baseline = await call("dipl_snapshot", environment=options.environment)
        baseline_id = baseline["environment"]
        parameters = []
        page = await call("dipl_list", environment=baseline_id, limit=2)
        while True:
            parameters.extend(item["path"] for item in page["items"])
            if page["next_cursor"] is None:
                break
            page = await call("dipl_list", cursor=page["next_cursor"], limit=2)
        assert len(parameters) == len(set(parameters)) == 5
        print("Parameters (paginated): " + ", ".join(parameters))

        original = await call("dipl_describe", environment=baseline_id, path="flow.reynolds")
        assert float(original["value"]["text"]) == 1000
        assert original["dependencies"]["available"]
        print("Baseline: inlet velocity 0.1 m/s; Reynolds number 1000")
        contract = await call("dipl_override_contract", environment=options.environment, path="flow.velocity")
        assert contract["requires_preview"]

        rejected = await call("dipl_preview", environment=options.environment,
                              overrides=["flow.velocity = 0.5 m/s"])
        assert not rejected["candidate_valid"] and rejected["candidate_diagnostics"]
        print("Rejected 0.5 m/s: the derived Reynolds number would violate the model constraint")
        overrides = ["flow.velocity = 0.2 m/s"]
        preview = await call("dipl_preview", environment=options.environment, overrides=overrides)
        assert preview["candidate_valid"]
        candidate = await call("dipl_snapshot", environment=options.environment, overrides=overrides)
        candidate_id = candidate["environment"]
        description = await call("dipl_describe", environment=candidate_id, path="flow.reynolds")
        assert float(description["value"]["text"]) == 2000
        difference = await call("dipl_compare", before=baseline_id, after=candidate_id)
        assert difference["changed"] == 2
        assert {item["path"] for item in difference["differences"]} == {"flow.velocity", "flow.reynolds"}
        print("Accepted 0.2 m/s: Reynolds number 2000; two evaluated parameters changed")
        exported = await call("dipl_export", environment=candidate_id, format="json")
        await save(exported["artifact"], "candidate.json")
        values = json.loads((destination / "candidate.json").read_bytes())
        assert values["flow"]["velocity"] == .2 and values["flow"]["reynolds"] == 2000
        await save(baseline["artifact"], "baseline.diph5")
        await save(candidate["artifact"], "candidate.diph5")
        if "dipl_report" in tools:
            report = await call("dipl_report", environment=candidate_id, format="md")
            await save(report["artifact"], "candidate.md")
        else:
            print("  Report skipped: this SNT build has report support disabled")
        (destination / "comparison.json").write_text(json.dumps(difference, indent=2) + "\n", encoding="utf-8")
        # Verify that the source environment has not inherited candidate overrides.
        unchanged = await call("dipl_describe", environment=options.environment, path="flow.reynolds")
        assert float(unchanged["value"]["text"]) == 1000
        print(f"Source model unchanged. Outputs: {destination}")
        print("MCP demonstration passed.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    connection = parser.add_mutually_exclusive_group()
    connection.add_argument("--snt", default="snt", help="Native snt executable (default: snt on PATH)")
    connection.add_argument("--url", help="Existing MCP HTTP endpoint exposing this example as a named environment")
    parser.add_argument("--environment", default="flow", help="Named environment (default: flow)")
    parser.add_argument("--output-dir", default=str(Path(__file__).resolve().parent / "output"), help="Empty directory for downloaded outputs")
    parser.add_argument("--legacy", action="store_true", help="Exercise legacy MCP initialization")
    asyncio.run(demonstrate(parser.parse_args()))


if __name__ == "__main__":
    main()
