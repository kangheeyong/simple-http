import socket

import fire


def run_client(host="localhost", port=9090):
    """
    test
    """
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print("Start client")
    try:
        s.connect((host, port))
        print(f"Connecting to {host} {port}")
        while True:
            print("Client >> ", end="")
            send_msg = bytes(input().encode())
            s.send(send_msg)
            recv_msg = s.recv(1024)
            print(f"Server >> {str(recv_msg)}")
    except KeyboardInterrupt:
        pass
    finally:
        s.close()
        print("Close client")


if __name__ == "__main__":
    fire.Fire(run_client)
