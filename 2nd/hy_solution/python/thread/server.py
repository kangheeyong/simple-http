"""
https://soooprmx.com/파이썬-소켓-연결-사용법/
250개 이상 연결이 안됨...
"""

import socket
import threading
from typing import Tuple

_CONNECTED_SOCKET_COUNT = 0


def echo_handler(conn: socket.socket, addr: Tuple[str, int]):
    global _CONNECTED_SOCKET_COUNT

    _CONNECTED_SOCKET_COUNT += 1
    print(f"Connected {addr}, {_CONNECTED_SOCKET_COUNT}")

    try:
        while True:
            msg = conn.recv(1024)
            if not msg:
                break
            print(f"Client from {addr} >> {str(msg)}")
            conn.sendall(msg)
    except Exception as ex:
        print(f"error {ex}")
    finally:
        conn.close()
        _CONNECTED_SOCKET_COUNT -= 1
        print(f"Disconnected {addr}, {_CONNECTED_SOCKET_COUNT}")


def run_server(host="localhost", port=9090):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print("Start Echo Thread Server")

    try:
        s.bind((host, port))
        while True:
            s.listen()
            conn, addr = s.accept()

            t = threading.Thread(target=echo_handler, args=(conn, addr))
            t.start()
    except KeyboardInterrupt:
        pass
    except Exception as ex:
        print(f"error {ex}")
    finally:
        s.close()
        print("Close Echo Thread rServer")


if __name__ == "__main__":
    run_server()
