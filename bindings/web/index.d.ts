export interface SourceLocation { source: string; line: number }
export interface Diagnostic {
  code: string;
  message: string;
  details: string;
  suggestion: string;
  path: string | null;
  location: SourceLocation | null;
}
export interface Validation { valid: boolean; diagnostic: Diagnostic | null }
export interface Metadata {
  description: string;
  category: string;
  visibility: string;
  rationale: string;
  recommendedRange: string;
  scientificImpact: string;
  performanceImpact: string;
  deprecated: string;
  replacement: string;
  since: string;
  native: string[];
  requires: string[];
  conflicts: string[];
  implies: string[];
  see: string[];
  example: string[];
}
export interface Description {
  path: string;
  kind: string;
  declaredType: string;
  storedType: string;
  shape: number[];
  elements: number;
  valueText: string | null;
  valueUnavailableReason: string;
  units: string | null;
  metadata: Metadata;
  tags: string[];
  overridden: boolean;
  options: string[];
  condition: string | null;
  declaration: SourceLocation | null;
  overrideLocation: SourceLocation | null;
  dependenciesRecorded: boolean;
  dependencies: { target: string; request: string }[];
}
export interface SchemaMember {
  name: string;
  relativePath: string;
  kind: string;
  type: string | null;
  units: string | null;
  schemaRefs: string[];
  options: string[];
  condition: string | null;
  metadata: Metadata;
  origin: SourceLocation | null;
  dimensions: { min: number; max: number | null }[];
  members: SchemaMember[];
}
export interface SchemaHierarchy {
  schema: "snt-schema-hierarchy/1";
  definitionsAvailable: boolean;
  applicationsComplete: boolean;
  definitions: { id: string; name: string; metadata: Metadata;
    origin: SourceLocation | null; members: SchemaMember[] }[];
  applications: { path: string; kind: string; schemaIds: string[];
    inheritedFromCollection: boolean | null; origin: SourceLocation | null }[];
  values: { path: string; appliedSchemaIds: string[];
    contributingSchemaId: string | null }[];
}
/** Text-backed registrations. `name` is the unit/schema name or code source identity. */
export interface NamedText { name: string; text: string }
/** In-memory equivalent of DIPfile registration phases; code and overrides retain order. */
export interface ProjectInput {
  units?: NamedText[];
  schemas?: NamedText[];
  code: NamedText[];
  overrides?: NamedText[];
}
export class DIPLModel {
  paths(): string[];
  describe(path: string, maxValueElements?: number): Description;
  schemas(): SchemaHierarchy;
  /** Reevaluate the original source with this replacement override body. */
  withOverride(body: string): DIPLModel;
  dispose(): void;
}
export function loadDIPL(options?: Record<string, unknown>): Promise<{
  parse(source: string): DIPLModel;
  parseProject(project: ProjectInput): DIPLModel;
  validate(source: string, overrideBody?: string): Validation;
  validateProject(project: ProjectInput, overrideBody?: string): Validation;
}>;
