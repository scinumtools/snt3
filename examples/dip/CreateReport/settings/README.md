# Run settings

[Experiment settings](parameters.dip) define the run duration, sampling,
flow values, derived distance, and readings table. The guided fields are a
short starting point for this demonstration; the source page and search index
retain every value and its provenance.

[Laboratory environment](environment.dip) supplies ambient values and then
changes `experiment.sample_count`. The generated value page distinguishes the
original declaration from that later assignment.

The separate `../overrides.dip` is applied by the evaluated run report. The
settings guide omits its contents so it describes defaults that a reader can
adapt for a new run. A path or other environment-dependent string should be
chosen for the reader's own setup rather than copied from this example.
