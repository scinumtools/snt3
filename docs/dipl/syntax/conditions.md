# DIPL - Language Specification

« Back to [specification](../specification.md#language-syntax)

## 3.9. Conditions

Conditions in DIPL provide a mechanism for controlling node parsing flow based on the evaluation of logical expressions.
They allow values and parameters to be assigned dynamically depending on runtime state, enabling branching behavior within scripts and configurations.
Each condition is evaluated in sequence, and only the first matching branch is executed.

``` DIPL-Schema
# Schemas of condition directives

# condition with only one case
@if ("<expression>")
  ...
@end                   # end is optional

# condition with multiple cases
@if ("<expression>")
  ...
@elif ("<expression>")
  ...
@else                  # else is optional
  ...
@end                   # end is optional

# consecutive conditions
@if ("<expression>")   # first condition
  ...
@else
  ...
@end                   # end is necessary
@if ("<expression>")   # second condition
  ...
```

DIPL supports the following set of condition clauses that define this branching structure.
These clauses can be combined to express simple decisions as well as more complex, multi-branch logic.
The structure is indentation-sensitive, meaning that all statements belonging to a clause must be nested under it with a higher indentation level.

- ``@if ("<expression>")``
  is a starting clause of each condition and requires as an argument result of a logical expression.
  If the expression is evaluated as true, its child nodes will be taken for further parsing and the condition is terminated.

- ``@elif ("<expression>")``
  is a subsequent condition clause and requires as an argument result of a logical expression.
  If the expression is evaluated as true, its child nodes will be taken for further parsing and the condition is terminated.
  If the expression is false, its child nodes are skipped and subsequent clauses are evaluated.
  For testing purposes, one can also use direct ``true`` or ``false`` keywords instead of expressions.

- ``@else``
  is a special, optional, clause that is at the very end of conditions and does not require any expression.
  Each condition can have only one ``@else`` clause.
  If all previous ``@if``-s and ``@elif``-s are evaluated as false, child nodes of ``@else`` are taken.

- ``@end``
  is a terminating clause whose main purpose is to separate two consecutive conditions.
  If omitted, a single condition terminates after last ``@if``, ``@elif`` or ``@else``.
      
``` DIPL
winner int = 1

@if ("{?winner} == 1")     # first case
  name str = "John"
@elif ("{?winner} == 2")     # second case
  name str = "Jenny"
@else                        # default case
  name str = "Jonas"
@end                         # end of first condition

@if ("{?name} == 'Jenny'")
  toy str = "doll"
@else
  toy str = "robot"
```

Nested conditions are also possible, provided that each child condition has indent higher than the parent condition.

``` DIPL
@if false
  flower str = "rose"
@else
  flower str = "dandelion"
  @if false
    color str = "red"
  @elif false
    color str = "blue"
  @else
    @elif true
      leaves int = 234
    color str = "yellow"
  tree str = "maple"

# The nodes above will be parsed as:
#
# flower = "dandelion"
# leaves = 234
# color  = "yellow"
# tree   = "maple"
```

> [!NOTE]
> Although nodes within a clause are indented more deeply than their surrounding siblings, this indentation does not affect the naming hierarchy.
> Internally, nodes inside a branching clause are assigned a temporary “clause parent” (e.g. ``foo.@if1.bar``).
> Once the condition is resolved and the branch is applied, this intermediate scope is removed, and the node assumes its final name (e.g. ``foo.bar``).
