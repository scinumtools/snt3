# Beyond YAML: Why Application Parameters Deserve Their Own Language

The setup of an application generally consists of more than merely a collection of names and values. For instance, a timeout has a unit attached to it, a retry schedule has a length linked to it, and a request budget might also depend on other settings. It is essential that these requirements are defined in advance if the application is to make use of the values.

There are ways provided by JSON, YAML, TOML, INI, and XML for representing structured data, after which applications add the meaning of that data by specifying which types are allowed, how the units are to be interpreted, and which combinations of settings are valid.

Consider a small retry schedule:

```dipl
retry_delays float32[3] = [100, 200, 400] ms
```

Here you get more than just a name and a list; `retry_delays` has a numerical representation (`float32`), a shape (`[3]`), a unit (`ms`), and a value. In an application it might also have validity constraints, descriptive metadata, dependencies on other parameters, or details regarding where its value was defined and then changed.

General-purpose serialization formats can store this information, but applications must supply its interpretation.

The fact that this observation was made led to the creation of **DIPL, the Dimensional Input Parameter Language**, a domain-specific language which was at first developed for use with input parameters in scientific simulations and numerical software. Similarly, the same fundamental problem arises in service configuration, background jobs, and other situations that involve quantities and associated settings.

We look at the language by means of familiar application parameters; the issue is what happens when a parameter, together with its requirements and relationships, becomes the basic object in the configuration language.

## A Parameter Is More Than a Value

Suppose an HTTP client uses three delays before successive retries. A YAML representation might look like this:

```yaml
retry_delays:
  type: float32
  shape: [3]
  value: [100, 200, 400]
  unit: ms
```

YAML represents the information, but `type`, `shape`, and `unit` are ordinary keys. The application must interpret them and detect invalid types, shapes, or units. In the DIPL declaration above, they are part of the language.

The declaration can be read from left to right:

```text
retry_delays   float32   [3]   = [100, 200, 400]   ms
     name       type    shape       value        unit
```

The difference is semantic: these are components of a parameter declaration, rather than conventions attached to a generic mapping. The processor can therefore check them before the application uses the retry schedule.

## Parameters Form a Hierarchy

Application settings naturally contain structure. DIPL uses indentation for this:

```dipl
application
  client
    timeout float = 500 ms
    retry_delay float = 250 ms

  worker
    concurrency uint32 = 4
    poll_interval float = 1 s
```

The hierarchy also defines the parameter namespace. For example, `timeout` can be addressed as:

```text
application.client.timeout
```

The hierarchical structure shown in the text and the parameter path illustrate the same structure, and this is useful in cases where parameters depend on each other.

## References Should Express Parameter Relationships

It is common for parameters to be related; for example, a retry interval is determined by the delay and the time taken by an attempt, and a batch duration is based on the number of items and the time allowed per item. When we only store the final values we have to ensure that they remain consistent if there is a change in the input.

DIPL makes these relationships explicit through references:

```dipl
timeout float = 500 ms
retry_delay float = 250 ms
attempt_interval float = ({?timeout} + {?retry_delay}) ms
```

Here `{?timeout}` and `{?retry_delay}` refer to values elsewhere in the parameter model. If every attempt consumes its timeout, the next attempt starts 750 milliseconds after the previous one started.

References can be absolute or relative to their position in the hierarchy. An absolute reference such as `{?application.client.timeout}` identifies the complete path. Relative references such as `{.timeout}` let a reusable structure refer to its own settings. Additional leading dots allow references to settings in enclosing groups. The special reference `{.}` is used in conditions to check the current parameter’s evaluated value.

The same model extends across document boundaries. For example, a file can name a source containing shared defaults and refer to one of its values:

```dipl
$source defaults = "defaults.dip"
timeout float = {defaults?application.client.timeout} ms
```

The source file provides the value of the referenced parameter and this value has to meet the requirements of the receiving parameter; the references thus create a dependency graph which can extend over several sources since settings are linked by the values they use even if they are defined in different files.

## Physical Units Are Part of the Language

Time units are familiar outside scientific computing. A service may express its timeout in milliseconds while a library expects seconds. The conversion is simple, but the configuration loader must know that it is required.

Consider:

```yaml
timeout:
  value: 500
  unit: ms
retry_delay:
  value: 0.25
  unit: s
```

For YAML, `ms` and `s` are treated as strings; it is up to the application or to the unit library that it uses to interpret them as durations.

DIPL treats units as part of parameter semantics:

```dipl
timeout float64 = 500 ms
retry_delay float64 = 0.25 s
attempt_interval float64 = ({?timeout} + {?retry_delay}) s
```

The resulting interval is 0.75 seconds. The processor checks dimensional compatibility and converts compatible units during evaluation. A value expressed in meters cannot replace a duration, because length and time are different physical dimensions.

The fact that these checks show the units to be compatible does not prove that the timeout is appropriate for a given service; for that an application rule or knowledge of the service's behaviour is needed.

## Shape Belongs Next to Type

An array’s shape can also be part of a parameter’s requirements. A retry schedule may require three delays. A table may contain two schedules, each with three entries.

DIPL places shape directly next to the data type:

```dipl
retry_delays float64[3] = [100, 200, 400] ms
schedules float64[2,3] = [[100, 200, 400], [250, 500, 1000]] ms
```

The first declaration requires three values; the second requires two rows of three values. Ranges such as `[3:]` can require at least three values. DIPL checks shape alongside type and units when it evaluates the parameter. The same mechanism applies to vectors and matrices in numerical software.

## Constraints Should Live With the Parameter

A parameter may have requirements that go beyond its type and unit; for example, a duration can be a valid one and yet still be too long for the application.

DIPL allows declarative properties and conditions to be attached to nodes:

```dipl
timeout float = 500 ms
  !condition ({.} > 0 ms && {.} <= 2 s)
```

Here `{.}` refers to the evaluated value of `timeout`. We require it to be positive and no greater than two seconds.

The condition accompanies the parameter, which means that an application gets a value which has already undergone this check. Moreover, conditions are also able to refer to other parameters, enabling us to compare a calculated duration against a separately defined budget.

String parameters can have format requirements:

```dipl
profile str = "search_client"
  !format "[a-z][a-z0-9_]*"
```

This profile name must start with a lowercase letter and contain only lowercase letters, digits, or underscores. Descriptive metadata can also be attached to a node without treating it as another application setting.

## Schemas Describe Parameter Contracts

Once parameter declarations include these requirements, reusable structures become useful. DIPL provides schemas for this purpose:

```dipl
$schema http_client
  name str
  timeout float s
  attempts uint32
```

Instances can then provide values while retaining the declared contract:

```dipl
clients map : http_client

clients[catalog]
  name = "Catalog service"
  timeout = 500 ms
  attempts = 3
```

The schema states that `timeout` is a floating-point duration given in seconds; the instance provides 500 milliseconds, which is converted to 0.5 seconds, and also gives a string name together with an unsigned integer count.

Schemas can include defaults and conditions, and can be combined and nested to build larger parameter models. A shared client schema therefore provides the same requirements to each client that uses it.

## Configuration and Override Are Different Operations

In a “last value wins” configuration, it can be difficult to distinguish a parameter declaration from an intentional change to a particular run or deployment. DIPL makes that change explicit:

```dipl
client
  timeout float = 500 ms
    !condition ({.} > 0 ms)
  attempts uint32 = 3
    !condition ({.} >= 1)

$override
  client.timeout = 1 s
  client.attempts = 4
```

The effective value is replaced by the override while the target's type, units, schema, and conditions are kept unchanged. Dependent parameters will then have the new value; the one-second timeout is converted into the specified number of milliseconds, but as before, failure to provide a compatible unit or a nonpositive duration still results in invalidation.

An implementation can also retain both the original declaration and the override location. This helps trace which input supplied a value and where it was changed.

## From Parameter File to Evaluated Model

References and expressions help derive inputs, but DIPL’s job is to describe, validate, and resolve parameters. Application behavior remains in the application. In the example below, Python calculates a retry schedule using the evaluated settings. A client written in another language would use the same separation between parameters and behavior.

To assess DIPL it is necessary to do more than just deserialise the text; the processor has to resolve the references, evaluate the derived values, carry out unit conversions, and check the declared types, shapes, conditions, and schemas.

The result is an evaluated parameter model that applications can use without repeating those checks. The model also exposes parameter paths, units, and descriptive metadata, so other software can inspect more than the numerical values alone.

## A Small Runnable Comparison

We compare YAML and DIPL using two HTTP client policies, one for a catalog service and one for a search service. Both allow three attempts, including the initial request, with a fixed 250-millisecond delay between attempts. The default timeout is 500 milliseconds; search overrides it to one second. Each complete sequence must fit within a four-second budget.

In carrying out this calculation we assume that each attempt uses up its entire timeout period and that there is no delay following the last attempt; the maximum duration in this model is:

```text
maximum duration = attempts × timeout + (attempts − 1) × retry delay
```

Three attempts therefore include two delays. The catalog policy takes at most two seconds in this model, and search takes 3.5 seconds. The programs calculate the schedules without sending requests or sleeping. We assume that the timeout covers a complete attempt and ignore scheduling overhead; a real client still needs to enforce its timeouts and deadline at runtime.

The YAML version in `example/yaml/` uses [Pint](https://pint.readthedocs.io/en/stable/getting/tutorial.html) for units. Its input defines both clients and the search timeout override:

```yaml
clients:
  catalog: &retry_policy
    attempts: 3
    timeout:
      value: 500
      unit: ms
      provenance:
        title: Timeout per attempt
        description: Maximum duration allowed for one request attempt
    retry_delay: {value: 250, unit: ms}
    budget: {value: 4, unit: s}
  search:
    <<: *retry_policy

overrides:
  clients.search.timeout: {value: 1, unit: s}
```

The YAML anchor is used to reuse the catalog data when performing a search; it doesn't specify a type or include any validation rule. Pint converts the units and discards those with incompatible dimensions. Python determines how the override is to be applied, checks each policy, and calculates the maximum duration. Part of this process is as follows:

```python
timeout = quantity(policy["timeout"], "s")
delay = quantity(policy["retry_delay"], "s")
budget = quantity(policy["budget"], "s")
duration = attempts * timeout + (attempts - 1) * delay
if duration > budget:
    raise ValueError(f"{name}: maximum retry duration exceeds budget")
```

The `quantity()` helper checks the YAML fields and calls Pint’s `.to()` method for conversion. The runner validates both policies before printing any schedule.

In `example/dipl/`, a schema supplies the same retry policy to both clients:

```dipl
$schema retry_policy
  attempts uint32 = 3
    !condition ({.} >= 1 && {.} <= 10)
  timeout float = 500 ms
    !condition ({.} > 0 ms)
    ?title "Timeout per attempt"
    ?descr "Maximum duration allowed for one request attempt"
  retry_delay float = 250 ms
    !condition ({.} >= 0 ms)
  budget float = 4 s
    !condition ({.} > 0 s)
  timeout_seconds float = {.timeout} s
  delay_seconds float = {.retry_delay} s
  retries uint32 = ({.attempts} - 1)
  max_duration float = ({.attempts} * {.timeout} + {.retries} * {.retry_delay}) s
    !condition ({.} <= {.budget})

clients map : retry_policy
clients[catalog]
clients[search]

$override
  clients[search].timeout = 1 s
```

The relative references resolve within each policy instance. The schema also provides values converted to seconds for the Python calculation. We choose a limit of ten attempts for this example; it is an application constraint expressed through `!condition`.

Both versions use the same Python function to calculate attempt start times. With DIPL, Python reads the evaluated settings and maximum duration through paths such as `clients[search].max_duration`. Unit conversion, override handling, and the budget check are performed when the parameter file is evaluated. This comparison concerns where those rules are defined; it does not measure application performance or development effort.

The script for setting up the companion creates a Python virtual environment, installs the required packages, and then runs both versions. If Python 3.12 or a later version has been installed, run:

```sh
cd example
./setup.sh
```

The script selects an available Python 3.12 or later. Use `./setup.sh --python python3.12` to select the interpreter used to create the environment, or `./setup.sh --help` for usage. An existing compatible environment is reused; an incompatible one is backed up before replacement. Both programs print the same result and parameter metadata:

```text
catalog attempt starts = 0.000, 0.750, 1.500 s
catalog max duration = 2.000 s (budget 4.000 s)
search attempt starts = 0.000, 1.250, 2.500 s
search max duration = 3.500 s (budget 4.000 s)
parameter title = Timeout per attempt
description = Maximum duration allowed for one request attempt
```

Change the search timeout override to `2 s` in DIPL and `{value: 2, unit: s}` in YAML. Both programs reject the resulting 6.5-second duration before printing a schedule. The timeout is valid on its own, but its relationship to the attempt count, delay, and budget makes the complete policy invalid.

An override of `1 m` provides a different test. Both programs reject a length where a duration is required. Pint checks the conversion requested by Python’s `quantity()` helper; DIPL checks the override against the parameter declaration. An override of `1000 ms` produces the same result as `1 s` in both versions.

The timeout also has a title and description. In DIPL, `?title` and `?descr` attach to the declaration, and the evaluated parameter exposes them through its provenance metadata after the override. In YAML, the runner explicitly preserves the nested `provenance` mapping when it replaces the value. These fields provide descriptive context; the schedule calculation uses only the numerical values.

## Why Not Just Extend YAML?

Such capabilities can be based on YAML; the runnable example does precisely that by using Pint and Python, and schema and validation libraries can as well be used to organize those rules.

As an application adds conventions for types, units, references, and overrides, however, it defines how a parameter model is represented on top of YAML’s generic mappings. DIPL gives those rules a syntax designed around parameters. The distinction is where the meaning is specified and which component is responsible for checking it.

## Domain-Specific Syntax Can Be a Feature

YAML, JSON and Pint offer well-established tools, although using a specialized language introduces a dependency and requires additional learning. The decision comes down to how much of the parameter contract an application needs to define on its own, since a number of independent settings might need no more than a simple validation function.

Next to each parameter DIPL includes the contract concerning the type, shape, units, dependencies, and constraints. The application then obtains a model that has been evaluated and validated. In our example Python is still responsible for calculating the retry schedule, but it no longer needs to interpret the unit fields or implement the relationships which determine whether a policy is valid.

That model is available to other software through parameter paths, values, units, and provenance. It can support viewers, reports, generated interfaces, and tools for managing different configurations. An LLM agent could propose an override; the same DIPL checks would validate it against the declared rules before the application uses it. Those checks establish compliance with the model, not whether the proposed settings are appropriate for a particular workload. Similar automation is possible with YAML when the application supplies the corresponding parameter semantics.
