import asyncio
import websockets
import datetime
import json

# class Event(object):
#     NEW_QUOTE = 0
#
#     def __init__(self, event_type, **kwargs):
#         self.type = event_type
#         for k,v in kwargs.items():
#             setattr(self, k, v)

# async def run(self):
#     while True:
#         completed, pending = asyncio.wait(self.tasks, return_when=concurrent.futures.FIRST_COMPLETED)
#         if len(completed) > 0:
#             for task in completed:
#                 event = await task
#                 if event.type == Event.NEW_QUOTE:
#                     pending.append(asyncio.ensure_future(self.process_new_quote(event.quote)))

async def run():
    def convert_time(t):
        return datetime.datetime.strptime(t[:26], '%Y-%m-%dT%H:%M:%S.%f')

    async with websockets.connect('wss://api.stockfighter.io/ob/api/ws/DES66129119/venues/IRSBEX/tickertape') as websocket:
        lastQuoteTime = datetime.datetime.utcnow()
        lastBid = 0
        lastAsk = 0
        while True:
            quote = json.loads(await websocket.recv())['quote']
            lastTrade = convert_time(quote['lastTrade'])
            quoteTime = convert_time(quote['quoteTime'])
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
