import asyncio
import websockets
import datetime
import json


def convertTime(t):
    return datetime.datetime.strptime(t[:26], '%Y-%m-%dT%H:%M:%S.%f')

async def run():
    async with websockets.connect('wss://api.stockfighter.io/ob/api/ws/DES66129119/venues/IRSBEX/tickertape') as websocket:
        lastQuoteTime = datetime.datetime.utcnow()
        lastBid = 0
        lastAsk = 0
        while True:
            quote = json.loads(await websocket.recv())['quote']
            lastTrade = convertTime(quote['lastTrade'])
            quoteTime = convertTime(quote['quoteTime'])
            bid = quote.get('bid', 0)
            ask = quote.get('ask', 0)
            print('{}last trade before quote: {}; quote interval: {}'.format('[*]' if bid > lastBid or ask < lastAsk else '[ ]', quoteTime - lastTrade, quoteTime - lastQuoteTime))
            lastQuoteTime = quoteTime
            lastBid = bid
            lastAsk = ask

async def hello():
    while True:
        try:
            await run()
        except websockets.exceptions.ConnectionClosed:
            print('Websocket closed. Reconnect.')

asyncio.get_event_loop().run_until_complete(hello())
