#!/usr/bin/env python

import asyncio
import json
import random
from websockets.asyncio.server import serve


async def handler(websocket):
    while True:
        message1 = {
    "id": "device_1",
    "x": random.randint(0, 300),
    "y": random.randint(0, 300),
}

        await websocket.send(json.dumps(message1))
        print(message1)

        message2 = {
    "id": "device_2",
    "x": random.randint(0, 300),
    "y": random.randint(0, 300),
}

        await websocket.send(json.dumps(message2))
        print(message2)
        await asyncio.sleep(3)


async def main():
    async with serve(handler, "localhost", 8765) as server:
        await server.serve_forever()


if __name__ == "__main__":
    asyncio.run(main())
