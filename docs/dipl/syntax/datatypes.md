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
In DIPL, all strings must be enclosed in double quotes, or in triple double quotes when spanning multiple lines.
It is used for textual data, labels, and any form of human-readable content.

``` DIPL
country str = "United States"  # single line text
address str = """              # text on multiple lines
350 Fifth Avenue
NY 10118
"""
```

### 3.2.2. Derived data types

When DIPL parses parameters for programming languages such as C/C++ and Fortran, it is sometimes necessary to explicitly specify the precision and representation of integer and floating-point values. 
This is achieved through the use of derived data types.


Derived data types are internally equivalent to their corresponding standard types but carry additional metadata describing properties such as signedness (for integers) and precision (for both integers and floating-point numbers).

In C/C++, a string is typically defined as a sequence of characters. 
In DIPL, this relationship is conceptually inverted: ``char`` is defined as a derived type of string to better support interoperability with C/C++.

The ``byte`` and ``char`` types are optional 8-bit types representing a single byte. 
The ``byte`` type is a numeric type intended for raw binary data and is equivalent to an implementation-defined 8-bit integer type (``int8`` or ``uint8``), while ``char`` is provided for compatibility with C/C++ character types. 
Neither type has inherent text encoding semantics.

**List of standard and derived data types**
   
| Standard   | Derived                                                                                            |
|------------|----------------------------------------------------------------------------------------------------|
| ``int``    | ``byte``, ``int8``, ``int16``, ``int32``, ``int64``, ``uint8``, ``uint16``, ``uint32``, ``uint64`` |
| ``float``  | ``float32``, ``float64``, ``float128``                                                             |
| ``string`` | ``char``                                                                                           |

> [!NOTE]
> The ``float128`` type is optional. 
> Many platforms and compilers do not provide native support for 128-bit floating-point arithmetic. 
> If supported, its availability must be explicitly documented by the respective DIPL implementation.
>
> The ``byte`` and ``char`` types are optional. 
> Implementations shall specify:
> whether each type is signed or unsigned, and whether values can be assigned from integer literals, string literals, or both.
