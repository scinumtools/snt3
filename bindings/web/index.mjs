import createDIPLRuntime from "./dist/dipl_web.mjs";

export class DIPLModel {
  constructor(runtime, native) {
    this.runtime = runtime;
    this.native = native;
  }

  paths() { return this.native.paths(); }
  describe(path, maxValueElements = 16) {
    return this.native.describe(path, maxValueElements);
  }
  schemas() { return this.native.schemas(); }
  withOverride(body) {
    return new DIPLModel(this.runtime, this.native.withOverride(body));
  }
  dispose() {
    if (this.native) {
      this.native.delete();
      this.native = null;
    }
  }
}

export async function loadDIPL(options = {}) {
  const runtime = await createDIPLRuntime(options);
  return {
    parse(source) { return new DIPLModel(runtime, new runtime.NativeModel(source, "")); },
    parseProject(project) {
      return new DIPLModel(runtime, runtime.parseProjectDIPL(project, ""));
    },
    validate(source, overrideBody = "") {
      return runtime.validateDIPL(source, overrideBody);
    },
    validateProject(project, overrideBody = "") {
      return runtime.validateProjectDIPL(project, overrideBody);
    },
  };
}
