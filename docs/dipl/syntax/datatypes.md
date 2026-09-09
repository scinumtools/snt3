# Data Types

In this section we describe standard and derived data types that can be used in DIPL code.
Standard types are categorized based on their value types into **booleans**, **integer numbers**, **floating-point numbers** and **strings**.

Numerical types may have derived types that specify additional representation
properties such as **width**, **precision**, or **signedness**.

For integer types, derived types include signed and unsigned variants with
different widths, such as `int8`, `int16`, `int32`, `int64`, `uint8`,
`uint16`, `uint32`, and `uint64`.

For floating-point types, derived types specify the floating-point precision,
such as `float16`, `float32`, `float64`, and `float128`.

## Standard data types

DIPL provides a minimal set of four built-in scalar data types. These types are designed to cover the most common use cases while keeping the language simple and predictable. Each type represents a distinct category of values and enforces basic constraints on how data is stored and interpreted.

Below is an overview of the supported scalar types and their typical usage.

**Boolean** (``bool``)

The Boolean type represents logical truth values. It is primarily used for control flow, conditional evaluation, and state representation.

``` DIPL 
day bool = true
night bool = false
```

**Integer** (`int`, `int32`, or `uint`, `uint32`)

The Integer types represent whole numbers without fractional components. The
signed Integer type (`int`, or equivalently `int32`) has a signed 32-bit semantic
range, allowing both positive and negative values within a fixed range. The
unsigned Integer type (`uint`, or equivalently `uint32`) has an unsigned 32-bit
semantic range and therefore represents only non-negative values.

In the reference implementation, Integer values are internally represented
using signed or unsigned 64-bit integers (`int64_t` or `uint64_t`), respectively.

```DIPL
year int = 2023
count uint = 42
```

**Float** (`float`, or equivalently `float64`, internally represented as a 64-bit floating-point value)

The Float type represents real numbers with fractional components. It has a
64-bit floating-point semantic representation, providing a wide range of
values, including very large and very small numbers. In the reference
implementation, Float values are internally represented using `double`.

``` DIPL
duration float = 10            # integer form
weight float = 23.3            # floating form
distance float = 2.3e20        # scientific form
```

**String** (``str``)

The String type represents sequences of characters.
In DIPL, all strings must be enclosed in double quotes, or in triple double quotes when spanning multiple lines.
It is used for textual data, labels, and any form of human-readable content.

``` DIPL
country str = "United States"  # single line text
address str = """
350 Fifth Avenue
NY 10118
"""                            # text on multiple lines
```

## Derived data types

When DIPL parses parameters for programming languages such as C/C++ and
Fortran, it is sometimes necessary to explicitly specify the precision
and representation of integer and floating-point values. This is
achieved through the use of derived data types.

Derived data types are semantically related to their corresponding
standard types but may use a different internal representation. They
carry additional metadata describing properties such as signedness and
precision. Multiple DIPL types may share the same internal
representation while retaining distinct semantics.

In C/C++, a string is typically represented as a sequence of characters.
In DIPL, ``char`` and ``string`` are distinct types. The ``char`` type
represents a single unsigned 8-bit character value, while ``string``
represents a sequence of characters. This distinction facilitates
interoperability with C/C++ without imposing C/C++'s implementation-
defined ``char`` signedness on DIPL.

The ``byte`` and ``char`` types are optional 8-bit types representing a
single byte. Both are internally represented by ``uint8_t``. The
``byte`` type represents an unsigned 8-bit numeric value intended for
raw binary data, while ``char`` represents an unsigned 8-bit character
value. Neither type specifies an inherent text encoding.

**List of standard and derived data types**
  
| Standard     | Derived                                       | Internal representation |
|--------------|-----------------------------------------------|-------------------------|
| ``bool``     |                                               | ``uint8_t``             |
| ``int``      | ``int8``, ``int16``, ``int32``, ``int64``     | ``int64_t``             |
| ``uint``     | ``uint8``, ``uint16``, ``uint32``, ``uint64`` | ``uint64_t``            |
| ``float``    | ``float16``, ``float32``, ``float64``         | ``double``              |
| ``float128`` |                                               | implementation-defined  |
| ``char``     |                                               | ``uint8_t``             |
| ``byte``     |                                               | ``uint8_t``             |
| ``string``   |                                               | ``string``              |

> [!NOTE]
> The float16 and float128 types are optional. 
> Many platforms and compilers do not provide native support for 16-bit and 128-bit floating-point arithmetic. 
> If supported, their availability and internal representation must be explicitly documented by the respective 
> DIPL implementation.The ``float16`` and ``float128`` types are optional.
>
> The ``byte`` and ``char`` types are optional.
> Both types are unsigned 8-bit types and are internally represented
> by ``uint8_t``. ``byte`` values may be assigned from integer literals,
> while ``char`` values may be assigned from character or string
> literals. Neither type specifies a text encoding.
