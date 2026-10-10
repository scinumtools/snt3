import { loadDIPL, loadPUQ } from "../index.mjs";

const dipl = await loadDIPL();
const source = [
  "$schema settings",
  "  distance float = 24 m",
  "  duration float = 8 s",
  "  speed float = ({?simulation.distance} / {?simulation.duration}) m/s",
  "simulation : settings",
].join("\n");

const model = dipl.parse(source);
if (model.describe("simulation.speed").valueText !== "3")
  throw new Error("WASM evaluation differs from the expected C++ result");
if (model.schemas().definitions[0].id !== "settings")
  throw new Error("WASM schema inspection failed");
const changed = model.withOverride("simulation.distance = 32 m");
if (changed.describe("simulation.speed").valueText !== "4")
  throw new Error("WASM override reevaluation failed");
const invalid = dipl.validate(source, "missing = 1");
if (invalid.valid || !invalid.diagnostic?.code)
  throw new Error("WASM diagnostics failed");
const project = {
  units: [{ name: "furlong", text: "201.168*m" }],
  schemas: [{ name: "settings", text: "distance float = 1 furlong\nduration float = 8 s" }],
  code: [
    { name: "model.dip", text: "simulation : settings" },
    { name: "derived.dip", text: "speed float = ({?simulation.distance} / {?simulation.duration}) m/s" },
  ],
};
const projectModel = dipl.parseProject(project);
if (Math.abs(Number(projectModel.describe("speed").valueText) - 25.146) > 1e-10)
  throw new Error("WASM in-memory project evaluation failed");
const projectChanged = projectModel.withOverride("simulation.distance = 2 furlong");
if (Math.abs(Number(projectChanged.describe("speed").valueText) - 50.292) > 1e-10)
  throw new Error("WASM in-memory project override failed");
const projectInvalid = dipl.validateProject(project, "unknown = 1");
if (projectInvalid.valid || !projectInvalid.diagnostic?.code)
  throw new Error("WASM in-memory project validation failed");
projectChanged.dispose();
projectModel.dispose();
changed.dispose();
model.dispose();
console.log("DIPL WebAssembly smoke test passed");

const puq = await loadPUQ();
if (!puq.systems().includes("SI") || puq.convert("35*eV", "J") !== "5.60762e-18*J")
  throw new Error("WASM PUQ conversion failed");
if (puq.convert("12*statA", "A", {
  inputSystem: "ESU", outputSystem: "SI", outputQuantity: "I",
}) !== "4.00277e-9*A")
  throw new Error("WASM contextual PUQ conversion failed");
let incompatibleRejected = false;
try { puq.convert("1*m", "s"); } catch { incompatibleRejected = true; }
if (!incompatibleRejected)
  throw new Error("WASM PUQ accepted incompatible dimensions");
for (const expression of ["35**eV", "1*m/", "*m"]) {
  let syntaxRejected = false;
  try { puq.convert(expression, "J"); } catch (error) {
    syntaxRejected = String(error).includes("Invalid expression syntax");
  }
  if (!syntaxRejected)
    throw new Error(`WASM PUQ accepted malformed expression: ${expression}`);
}
console.log("PUQ WebAssembly smoke test passed");
