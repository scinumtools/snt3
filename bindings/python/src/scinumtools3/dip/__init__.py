from .._snt import dip as _dip

Environment = _dip.Environment
Cursor = _dip.Cursor
DIP = _dip.DIP
ValueNode = _dip.ValueNode
ValueNodeData = _dip.ValueNodeData
PathKind = _dip.PathKind
ExportFormat = _dip.ExportFormat
ReportFormat = getattr(_dip, "ReportFormat", None)
Adapter = _dip.Adapter
AdapterContext = _dip.AdapterContext
ExistingOutputPolicy = _dip.ExistingOutputPolicy
run_adapter = _dip.run_adapter
run_adapter_project = _dip.run_adapter_project
run_adapter_snapshot = _dip.run_adapter_snapshot
ComparisonScope = _dip.ComparisonScope
DifferenceKind = _dip.DifferenceKind
ComparisonOptions = _dip.ComparisonOptions
Difference = _dip.Difference
ComparisonResult = _dip.ComparisonResult
compare = _dip.compare
compare_diph5 = _dip.compare_diph5
render_comparison = _dip.render_comparison

PybindException = _dip.PybindException

ArtifactKind = _dip.ArtifactKind
ValueChangeKind = _dip.ValueChangeKind
ValueChange = _dip.ValueChange
ValueInspection = _dip.ValueInspection
InspectionCapabilities = _dip.InspectionCapabilities
CompositionKind = _dip.CompositionKind
OperationType = _dip.OperationType
CompositionNode = _dip.CompositionNode
CompositionGraph = _dip.CompositionGraph
DependencyEventKind = _dip.DependencyEventKind
DependencyEdge = _dip.DependencyEdge
DependencyEvent = _dip.DependencyEvent
DependencyGraph = _dip.DependencyGraph
TableColumnInspection = _dip.TableColumnInspection
TableInspection = _dip.TableInspection
SourceLocation = _dip.SourceLocation
DiagnosticSeverity = _dip.DiagnosticSeverity
Diagnostic = _dip.Diagnostic

detect_artifact = _dip.detect_artifact
open_artifact = _dip.open_artifact
reload_artifact = _dip.reload_artifact
inspect_value = _dip.inspect_value
inspect_values = _dip.inspect_values
inspect_capabilities = _dip.inspect_capabilities
inspect_dependency_graph = _dip.inspect_dependency_graph
inspect_table = _dip.inspect_table
inspect_tables = _dip.inspect_tables
read_value_slice = _dip.read_value_slice
SemanticDescription = _dip.SemanticDescription
OverrideTargetKind = _dip.OverrideTargetKind
OverrideContract = _dip.OverrideContract
SemanticList = _dip.SemanticList
PreviewOverrideKind = _dip.PreviewOverrideKind
PreviewOverride = _dip.PreviewOverride
PreviewResult = _dip.PreviewResult
describe = _dip.describe
override_contract = _dip.override_contract
list_descriptions = _dip.list_descriptions
preview = _dip.preview


def diagnostic_from_exception(error):
    """Return the structured diagnostic attached to a DIP exception, if any."""
    return getattr(error, "diagnostic", None)
