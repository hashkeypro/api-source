import asyncio
import websockets

async def hello(uri):
    async with websockets.connect(uri) as websocket:
        await websocket.send('''{ "type": "subscribe", "channel": {"ticker":["ETH-BTC", "CYB-BTC"]} }''')
        while (1) :
            response = await websocket.recv()
            print(response)
        
if __name__ == '__main__': 
    asyncio.get_event_loop().run_until_complete(
        hello("wss://pro.hashkey.com/APITradeWS/v1/messages"))
