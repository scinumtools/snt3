# DIPL - Language Specification

« Back to [specification](../specification.md#language-syntax)

## 3.2. Data Types

In this section we describe standard and derived data types that can be used in DIPL code.
Standard types are categorized based on their value types into **booleans**, **integer numbers**, **floating-point numbers** and **strings**.
Numerical types can have additionally derived types that specify their **signedness** and **precision**.

### 3.2.1. Standard data types

DIPL provides a minimal set of four built-in scalar data types. These types are designed to cover the most common use cases while keeping the language simple and predictable. Each type represents a distinct category of values and enforces basic constraints on how data is stored and interpreted.

Below is an overview of the supported scalar types and their typical usage.

**Boolean** (``bool``)

The Boolean type represents logical truth values. It is primarily used for control flow, conditional evaluation, and state representation.

``` DIPL 
day bool = true
night bool = false
```

**Integer** (``int``, signed 32-bit)

The Integer type represents whole numbers without fractional components.
It uses a signed 32-bit representation, allowing both positive and negative values within a fixed range.

``` DIPL
year int = 2023
```

**Float** (``float``, 64-bit)

The Float type represents real numbers with fractional components.
It uses a 64-bit floating-point representation, allowing for a wide range of values, including very large and very small numbers.

``` DIPL
duration float = 10            # integer form
weight float = 23.3            # floating form
distance float = 2.3e20        # scientific form
```

**String** (``str``)

The String type represents sequences of characters.
It is used for textual data, labels, and any form of human-readable content.

``` DIPL
name str = John                # single word
city str = 'New York'          # multiple words
country str = "United States"  # multiple words
address str = """              # multiple lines
350 Fifth Avenue
NY 10118
"""
```

### 3.2.2. Derived data types

When DIPL is parsing parameters for programming languages like C/C++ and Fotran, it is sometimes necessary to closely specify which precision float and integer values have.
This can be done using derived data types.
Such data types are internally identical to their standard counterparts, however, they carry additional information about their **signedness** (``int``) and **precision** (both ``int`` and ``float``).
The table below gives a list of all standard and corresponding derived data types that can be used in DIPL.

**List of standard and derived data types**
   
| Standard      | Derived                                                                                       |
|---------------|-----------------------------------------------------------------------------------------------|								
| ``int``       | ``int8``, ``int16``, ``int32``, ``int64``, ``uint8``, ``uint16``, ``uint32``, ``uint64``	|
| ``float``     | ``float32``, ``float64``, ``float128``                                                        |
