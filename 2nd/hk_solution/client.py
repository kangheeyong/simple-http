import socket

lst = []
for i in range(50000):
    s = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
    s.connect(("localhost", 8091))
    text = f"{i+1}st connect!\n"
    s.send(text.encode())
    print(text)
    lst.append(s)