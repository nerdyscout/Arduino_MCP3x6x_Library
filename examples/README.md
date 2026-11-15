# Examples

Build every example for every environment:

```sh
./examples/build.sh
```

Build for selected environments:

```sh
./examples/build.sh ESP32 SAMD21G18A
```

Build a specific example:

```sh
./examples/build.sh MCP3461_Mux_SingleEnded
```

Build and upload a single example:

```sh
./examples/upload.sh MCP3564_DMA ATMEGA328P
# or
pio run -t upload_MCP3564_DMA -e ATMEGA328P
```

See `examples/build.sh` and `examples/upload.sh` for details.
