# Compile
gcc main.c

# Run
./a.out

# Test
./nc localhost 8080

# Questions

## Case 1. 왜 바로 끊기지 않을까요?
Terminal 1(server)
./a.out

Terminal 2(client)
nc localhost 8080

이렇게 연결해두고 client 에서 메시지를 입력하면 정상동작합니다.
이 상태에서 server로 가서 Ctrl+C를 누르면 서버는 즉시 종료됩니다.
그런데 client는 왜 바로 끊기지 않을까요?

반대로 client에서 Ctrl+C를 누르면 서버에서는 즉시 끊깁니다.
server에서 Ctrl+C를 눌렀을때 client가 즉시 종료임을 알게 하려면 어떻게 해야하나요?
