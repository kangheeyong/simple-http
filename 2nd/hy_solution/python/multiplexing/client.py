import socket

import fire

_SOCKETS = []
_CONNECTED_SOCKET_COUNT = 0


def run_broadcasting_client(host: str = "localhost", port: int = 9090, total: int = 10):
    """Broadcasting Client"""

    global _CONNECTED_SOCKET_COUNT

    print("Start client")

    for _ in range(total):
        try:
            _SOCKETS.append(socket.socket(socket.AF_INET, socket.SOCK_STREAM))
        except Exception as ex:
            print(f"error {ex}")
            break

        s = _SOCKETS[-1]

        try:
            s.connect((host, port))
        except Exception as ex:
            print(f"error {ex}")
            break

        _CONNECTED_SOCKET_COUNT += 1
        print(f"Connecting to {host} {port}, {_CONNECTED_SOCKET_COUNT}")

    try:
        while True:

            print("Client >> ", end="")
            send_msg = bytes(input().encode())
            recv_cnt = 0
            for s in _SOCKETS:
                s.send(send_msg)
                recv_msg = s.recv(1024)
                if recv_msg:
                    recv_cnt += 1
            print(f"Server sends {recv_cnt} messages.")
    except KeyboardInterrupt:
        pass
    finally:
        for s in _SOCKETS:
            s.close()
        print("Close client")


if __name__ == "__main__":
    fire.Fire(run_broadcasting_client)
