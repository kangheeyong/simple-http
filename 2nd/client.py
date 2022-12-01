import socket

lst = []
for i in range(30000):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("localhost", 8090))
    text = f"{i+1}st connect!\n"
    s.send(text.encode())
    print(text)
    lst.append(s)