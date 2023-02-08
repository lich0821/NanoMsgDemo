# Demo for Nanomsg Next Generation (NNG)
[NNG](https://github.com/nanomsg/nng) is a Modern re-implementation of [Nanomsg](https://nanomsg.org/documentation.html).

This project demo a simple Req/Rep protocol with [nngpp](https://github.com/cwzx/nngpp) and [pynng](https://github.com/codypiersall/pynng).

## Quickstart
### Install packages.
* [nngpp](https://github.com/cwzx/nngpp)
```sh
vcpkg install nngpp:x86-windows-static
```

* [pynng](https://github.com/codypiersall/pynng)
```sh
pip install pynng
```

### Build
* VS2019

Build like other VS projects.

### Run
* Start `Server`
```sh
# Start server
NNGPP.exe server "tcp://0.0.0.0:5555"

```

* Start `Client`
```sh
# NNGPP
NNGPP.exe client "tcp://127.0.0.1:5555" 3
```

```sh
# PYNNG
python main.py
```