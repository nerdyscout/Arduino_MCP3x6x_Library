# Examples

Build every example for every environment:

```sh
./examples/build.sh
```

Build only specific environments:

```sh
./examples/build.sh ESP32 SAMD21G18A
```

Build only a specific example (all environments):

```sh
./examples/build.sh MCP3461_Mux_SingleEnded
```

Build and upload a single example to a board:

```sh
./examples/upload.sh MCP3564_DMA ATMEGA328P

# Or via PlatformIO custom target:
pio run -t upload_MCP3564_DMA -e ATMEGA328P
```

See `examples/build.sh` and `examples/upload.sh` for full usage details.
