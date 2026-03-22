# DIPL - Language Specification

« Back to [specification](../specification.md#language-syntax)

## 3.6. Units

Units in DIPL follow a dedicated domain specific language designed for this purpose and defined in a separate [specification](docs/puql/specification.md).
Consequently, every DIPL implementation MUST either provide a fully compliant units parser or utilize the reference units parser available in the [SciNumTools v3](https://github.com/vrtulka23/scinumtools3) repository.

All values MUST be converted to a canonical unit representation before comparison or condition evaluation.

### 3.6.1 Standard Units

Each node has default units assigned at definition, or declaration.
Subsequent modification without given units assume to be in default units.
Values of modifications with different units (but same dimension) are converted to default units.

``` DIPL
# definitions
age int = 30 a
height float = 185 cm
weight float = 80 kg

# modifications
age = 35
height = 190 cm      
weight = 90000 g

# Modified values:
#
# age = 35 a
# height = 190 cm
# weight = 90 kg
```


### 3.6.2 Custom Units

Similarly as in case of references, it is also possible to define new units directly in the DIPL code. This can be achieved by a special node directive ``$unit``.

``` DIPL-Schema
# Schema of a custom unit definition

<indent>$unit <name> = <value> <unit>  # if value is a number
<indent>$unit <name> = <value>         # if value is reference, or expression
```

Names of the custom units are automatically wrapped into square brackets.
If the name of a custom unit is already used, the code will raise an error.

``` DIPL
$unit mass = 30 AU
$unit length = 10 pc
$unit time = 1 Gy

velocity float = 2 [length]/[time]
density float = 34 [mass]/[lenght]3
```

Each DIPL implementation MUST support the definition and integration of custom units directly through the code interface.
