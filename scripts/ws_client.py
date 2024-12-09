#!/usr/bin/env python

import asyncio
from websockets.asyncio.client import connect


async def receive():
    async with connect("ws://localhost:8765") as websocket:
        await websocket.send()
        message = await websocket.recv()
        print(message)


if __name__ == "__main__":
    asyncio.run(receive())
