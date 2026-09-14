# DIP C binding example

This standalone C project uses `snt/c.h` to add and parse DIPL text, then
retrieve a scalar value by its node path.

```sh
cmake -S examples/dip/CBinding -B build-dip-c-binding \
  -DCMAKE_PREFIX_PATH=/path/to/snt/install
cmake --build build-dip-c-binding
./build-dip-c-binding/dip-c-example
```
