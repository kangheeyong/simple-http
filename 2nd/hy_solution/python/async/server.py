"""
https://soooprmx.com/파이썬-소켓-연결-사용법/
250개 이상 연결이 안됨...
client에서 disconnect 후 server에서 socket 커널에 바로 반환이 안되는것 같다.
"""

import asyncio

_CONNECTED_SOCKET_COUNT = 0


async def echo_handler(reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
    global _CONNECTED_SOCKET_COUNT

    _CONNECTED_SOCKET_COUNT += 1
    print(f"Connected, {_CONNECTED_SOCKET_COUNT}")

    try:
        while True:
            msg = await reader.read(1024)
            if not msg:
                break
            print(f"Client >> {str(msg)}")

            writer.write(msg)
            await writer.drain()

    except Exception as ex:
        print(f"error {ex}")
    finally:
        _CONNECTED_SOCKET_COUNT -= 1
        print(f"Disconnected, {_CONNECTED_SOCKET_COUNT}")


async def run_server(host="localhost", port=9090):
    print("Start Echo Async Server")
    try:
        server = await asyncio.start_server(echo_handler, host=host, port=port)
        async with server:
            await server.serve_forever()
    except KeyboardInterrupt:
        pass
    except Exception as ex:
        print(f"error {ex}")
    finally:
        print("Close Echo Async rServer")


if __name__ == "__main__":
    asyncio.run(run_server())
