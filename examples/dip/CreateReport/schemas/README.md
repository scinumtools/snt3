# Reusable schemas

[Sensor](sensor.dipl) and [analysis](analysis.dipl) declare typed values with
defaults and descriptions. A guided field uses `!setup`; a basic field may
also carry `?visibility "basic"` so it appears in a large-file overview.

[Instrument](instrument.dipl) defines the shape of each record in the
[catalog](../catalog/README.md). Its field declarations are separate from the
two assigned records, so the guide can show both schema provenance and
effective values.
