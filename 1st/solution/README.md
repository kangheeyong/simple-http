# Q1. 왜 바로 안끊길까?
Ctrl+C랑 똑같은 양식을 보이는 것은 다른 터미널에서 
./kill INT PID
명령을 사용하는 것과 같다. 즉 Ctrl+C는 해당 프로세스에 SIGINT signal을 날린다.
SIGINT는 handling가능한 signal인데 sig\_handler를 등록하지 않는다면 기본 action은 프로세스를 그냥 죽이는 것이다.

프로세스가 그냥 죽어버렸으므로 연결된 connection에 아무런 내용을 전송하지 않고 그냥 자원을 정리해버린 것.
=> 은 틀렸음. 헷갈렸;;; 프로세스가 죽을때 os가 모든 fd를 닫게되므로 FIN이 되었던 RST가 되었던 날아가게 되는데 nc가 그 이후에 적절한 메시지를 내보내 주지 않을 것 뿐.
어쨌든 signal.c 참조

# Q2. 커넥션이 살아있는 경우 read는 항상 0보다 큰 값을 리턴하는가?
Nope.
signal.c를 실행시킨 후 nc를 통해 connection을 연결하고 server에서 Ctrl+C로 SIGINT를 줘보자.
원래는 EINTR이 발생하면서 system call이 중단되어 리턴되는데 커널 구현이 바뀐듯하다.
man read 해서 read의 ERROR를 확인해보면
```
EINTR  The call was interrupted by a signal before any data was read; see signal(7).
```
그 외에도 async처리할때는 EWOULDBLOCK이나 EAGAIN같은 에러가 나기도 하는데 이런 것들은 에러라기보다는 제어권 처리방법이라 잘 처리해야한다.


strace ./a.out 으로 실제 일어나는 일을 관찰해보면 다음과 같다.
세상 편리해진듯.
```
{sa_family=AF_INET, sin_port=htons(38190), sin_addr=inet_addr("127.0.0.1")}, [16]) = 4
read(4, "asdf\n", 8192)                 = 5
write(1, "received 5 bytes:\n", 18received 5 bytes:
)     = 18
write(1, "asdf\n", 5asdf
)                   = 5
write(4, "asdf\n", 5)                   = 5
read(4, "asdf\n", 8192)                 = 5
write(1, "received 5 bytes:\n", 18received 5 bytes:
)     = 18
write(1, "asdf\n", 5asdf
)                   = 5
write(4, "asdf\n", 5)                   = 5
read(4, 0x7ffde5065b40, 8192)           = ? ERESTARTSYS (To be restarted if SA_RESTART is set)
--- SIGTERM {si_signo=SIGTERM, si_code=SI_USER, si_pid=39391, si_uid=1000} ---

```

# Q3. REUSEADDR은 뭐고 왜 사용하는거지?
### By Junghoon Lee
소켓통신이 끝나면 소켓이 종료되고 포트와의 연결을 끊습니다. 연결 종료를 먼저 시작한 쪽은 상대방으로부터 FIN 패킷을 받고, FIN\_ACK 패킷을 전송한 후 일정 시간 동안 소켓을 종료하지 않고 커널이 해당 소켓을 점유합니다.
즉, 먼저 종료를 시작하는 시스템은 응용 프로그램이 종료되더라도 소켓은 커널에서 일정 시간 동안 점유 중인 상태가 됩니다. 이런 경우에 응용 프로그램을 재실행하면 bind 함수를 호출할 때 아직 점유 중인 포트를 연결하려는 시도 때문에 오류가 발생합니다. 일정 시간이 지나야 연결이 가능합니다.
REUSEADDR 옵션을 설정하면 커널이 소켓의 포트를 점유 중인 상태에서도 서버 프로그램을 다시 구동할 수 있습니다.
### Comment
사실 이것보다 조금더 복잡한데 그런 케이스들은 진짜 특별한 케이스들이고 이 옵션을 쓰는 주 이유가 저 이유. 저렇게만 알고 있어도 충분.

# Q4. INADDR\_ANY는 뭘까? 이거 말고 다른거 쓰면 어떻게 되는거지?
### By Junghoon Lee
INADDR\_ANY는 0의 값을 가지고 있으며, 자동으로 이 컴퓨터에 존재하는 랜카드 중 사용가능한 랜카드의 IP주소를 사용하라는 뜻입니다.
컴퓨터 내에 두 개 이상의 IP를 할당받아서 사용하는 경우 할당받은 IP 중 어떤 주소를 통해서 데이터가 들어오더라도 PORT번호만 일치하면 수신할 수 있습니다.
INADDR\_ANY 대신 IP를 바로 적으면 해당 IP주소로 들어오는 데이터만 수신합니다. (만약, PC에 IP주소가 여러개 있는 경우 다른 IP주소로 들어오는 데이터는 수신 못합니다.)

# Q5.  htons함수는 뭐하는거고 왜 쓰는거야?
CPU에서 숫자를 표기할 때 Little-endian / Big-endian 두가지 방법이 있다. MSB가 왼쪽인지 오른쪽인지에 대한 차이인데 이 차이로 발생하는 성능상 이점은 잘 모르겠다.
10진수 10000은 16진수로 2710인데 이게 8bit little endian에서는 1027로 표기된다. ( 왜 0172라고 물어볼 사람은 없겠지? 16bit bit little endian에서는 어떻게 표기될까? )
하여튼 network에서는 이 bit-stream이 표준화될 필요가 있고 표준은 big-endian이다. 그래서 htons는
h -> host, to -> to, n -> network, s -> short
short int(16 bit) 정수를 host 타입에서 network 타입으로 바꾸라는 명령. 
당연하게도 ntohs 나 htonl 이런 함수들도 있다.
( 토막 상식. little endian / big endian이란 말은 걸리버 여행기의 소인국 이야기에서 유래했다. 달걀을 깨먹을때 둥근 부분을 깨냐 뾰족한 부분으로 깨냐를 두고 두 왕국이 전쟁까지 벌이던 그 에피소드에서 둥근쪽을 깨자는 사람들을 가르켜서 big endian, 뾰족한 쪽을 깨자는 사람들을 little endian이라고 소설에서 불렀다. )

# Q6. 코딩 숙제: 1주차 샘플은 오직 한번에 하나의 client만 접속 가능하다. 여러 클라이언트가 동시에 접속하게 하려면 어떻게 고쳐야하나?
### From Hyungjoo
accept 다음 라인에서 fork()로 자식 프로세스 생성하고 child 프로세스가 echo 함수롤 실행하도록 하고, parent 프로세스는 accept 후 child 프로세스가 echo 함수를 실행하도록 하는 식으로 가능하지 않을까 싶습니다..!
```
while (1) {
    client_len = sizeof(client_address);
    client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_address, &client_len);

    int pid;
    pid = fork();
    if (pid == 0) {  // child process
        echo(client_sockfd, client_address);
    }
    else if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
}
wait(NULL);
```
### Comment
이전에 이런 방식으로 짜는 경우도 있었다.
몇가지 단점이 있는데
1. child process생성은 thread 생성보다 비싸다.
2. main process가 child process에 간섭할 방법이 제한적이다. 
3. thread로 구현할때는 thread pool을 만들어서 thread를 한번 생성후 순차적으로 처리하도록 제어가능하지만 process로 만들 때는 process pool을 만들 수가 없다. ( 왜? )
그래서 보통 thread pool로 구현하며 이렇게 accept후 worker thread가 해당 connection을 가져가서 처리하는 방식을 Boss-worker방식이라고 한다.
Boss(accept를 하는 thread)가 worker(connection을 받아서 서빙하는 thread)에게 일을 시키는 방식이라서 그렇다.
대표적으로 apache가 그런식으로 처리하며 django쪽도 gunicorn이 그런식으로 처리된다. ( 단 gunicon은 boss도 여러개다. boss도 한개가 아닐 수 있다. 이건 추후 더 이야기하는 걸로 )
