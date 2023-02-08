#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import binascii
import pynng
import time


def hex2int(b):
    return int(binascii.hexlify(b), 16)


def rpc_call(sock, data):
    print(f"Client: SENDING REQUEST")
    sock.send(data)
    rsp = sock.recv_msg()
    msg = hex2int(rsp.bytes)
    if data[0] == 0x03:
        msg = time.asctime(time.localtime(msg))

    print(f"Client: RECEIVED {msg}")


if __name__ == "__main__":
    url = "tcp://127.0.0.1:5555"
    with pynng.Req0() as sock:
        sock.dial(url)
        for i in range(1, 5):
            data = bytes([i])
            rpc_call(sock, data)
