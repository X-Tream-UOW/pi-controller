# Pi Controller – Acquisition Firmware

This repository contains the C firmware for ADC acquisition on the Raspberry Pi. It includes a standalone executable for testing and a shared library (`.so`) for integration.

---

## Build the CLI executable

```bash
make
```

Produces: `./master`

Run it with:

```bash
./master <duration_ms>
```
This will perform an acquisition of the given time.

---

## Build the shared library (`.so`)

```bash
make lib
```

This builds `libacquisition.so`, excluding the CLI entry point (`main.c`).

This library can be used via dynamic linking (e.g., in the Python API using `ctypes`).

