# C binding example

This standalone C project uses only `snt/c/puq.h`. It evaluates a PUQ quantity,
converts it to centimetres, formats the result, and releases the opaque
handles through the C ABI.

```sh
cmake -S examples/puq/CBinding -B build-c-binding \
  -DCMAKE_PREFIX_PATH=/path/to/snt/install
cmake --build build-c-binding
./build-c-binding/puq-c-example
```
