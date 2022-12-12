import socket

import fire


def run_server(host: str = "localhost", port: int = 9090):
    """Base Server"""

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print("Start Echo Server")

    try:
        s.bind((host, port))
        s.listen(1)
        while True:
            conn, addr = s.accept()
            print(f"Connected {addr}")
            try:
                while True:
                    msg = conn.recv(1024)
                    if not msg:
                        break
                    print(f"Client >> {str(msg)}")
                    conn.sendall(msg)
            except Exception as ex:
                print(f"error {ex}")
            finally:
                conn.close()
                print(f"Disconnected {addr}")
    except KeyboardInterrupt:
        pass
    finally:
        s.close()
        print("Close Echo Server")


if __name__ == "__main__":
    fire.Fire(run_server)
