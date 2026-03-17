# DIPL - Language Specification

« Back to [specification](../specification.md#language-syntax)

## 3.2. Data Types

In this section we describe standard and derived data types that can be used in DIPL code.
Standard types are categorized based on their value types into **booleans**, **integer numbers**, **floating-point numbers** and **strings**.
Numerical types can have additionally derived types that specify their **signedness** and **precision**.

### 3.2.1. Standard data types

DIPL uses defaultly a set of 4 basic data types.
Below is an overview of possible scalar values for each of them:

**Boolean** (``bool``)

``` DIPL 
day bool = true
night bool = false
```

**Integer** (``int``, signed 32-bit)

``` DIPL
year int = 2023
```

**Float** (``float``, 64-bit)

``` DIPL
duration float = 10            # integer form
weight float = 23.3            # floating form
distance float = 2.3e20        # scientific form
```

**String** (``str``)

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
