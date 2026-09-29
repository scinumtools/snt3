from .._snt import dip as _dip

Environment = _dip.Environment
Cursor = _dip.Cursor
DIP = _dip.DIP
ValueNode = _dip.ValueNode
ValueNodeData = _dip.ValueNodeData
PathKind = _dip.PathKind
ExportFormat = _dip.ExportFormat
ReportFormat = _dip.ReportFormat

PybindException = _dip.PybindException

ArtifactKind = _dip.ArtifactKind
ValueChangeKind = _dip.ValueChangeKind
ValueChange = _dip.ValueChange
ValueInspection = _dip.ValueInspection
InspectionCapabilities = _dip.InspectionCapabilities
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
inspect_table = _dip.inspect_table
inspect_tables = _dip.inspect_tables
read_value_slice = _dip.read_value_slice


def diagnostic_from_exception(error):
    """Return the structured diagnostic attached to a DIP exception, if any."""
    return getattr(error, "diagnostic", None)
