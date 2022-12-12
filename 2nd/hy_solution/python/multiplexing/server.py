"""
https://soooprmx.com/selector를-사용한-소켓-멀티플렉싱/
250개 이상 연결이 안됨...
"""

import selectors
import socket
from typing import Tuple
import logging

logging.basicConfig(level=logging.INFO)

_logger = logging.getLogger(__name__)

_CONNECTED_SOCKET_COUNT = 0


def echo_handler(
    conn: socket.socket, addr: Tuple[str, int], sel: selectors.BaseSelector
):
    global _CONNECTED_SOCKET_COUNT

    try:
        msg = conn.recv(1024)
        if not msg:
            sel.unregister(conn)
            conn.close()
            _CONNECTED_SOCKET_COUNT -= 1
            _logger.info(f"Disconnected from {addr}, {_CONNECTED_SOCKET_COUNT}")
            return

        _logger.info(f"Client from {addr} >> {str(msg)}")

        conn.sendall(msg)
    except Exception as ex:
        _logger.info(f"error {ex}")
        sel.unregister(conn)
        conn.close()
        _CONNECTED_SOCKET_COUNT -= 1
        _logger.info(f"Disconnected from {addr}, {_CONNECTED_SOCKET_COUNT}")


def accept_handler(sock: socket.socket, _, sel: selectors.BaseSelector):
    global _CONNECTED_SOCKET_COUNT

    conn, addr = sock.accept()

    _CONNECTED_SOCKET_COUNT += 1
    _logger.info(f"Connected {addr}, {_CONNECTED_SOCKET_COUNT}")

    sel.register(conn, selectors.EVENT_READ, (echo_handler, addr))


def run_server(host="0.0.0.0", port=9090):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    selector = selectors.DefaultSelector()
    selector.register(s, selectors.EVENT_READ, (accept_handler, None))

    _logger.info("Start Echo Multiplexing Server")

    try:
        s.bind((host, port))
        s.setblocking(False)
        s.listen()

        while True:
            for (key, _) in selector.select():
                key: selectors.SelectorKey
                srv_sock, (callback, addr) = key.fileobj, key.data
                callback(srv_sock, addr, selector)

    except KeyboardInterrupt:
        pass
    finally:
        selector.unregister(s)
        s.close()
        _logger.info("Close Echo Multiplexing Server")


if __name__ == "__main__":
    run_server()
