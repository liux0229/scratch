from datetime import timedelta
import asyncio
import datetime
import json
import time
import websockets
import concurrent.futures
from common import StockUrl, Primitive, Spread, setup_flags, Player


class QuoteSource:
    def __init__(self, callback):
        self.callback = callback

    async def run(self):
        while True:
            try:
                await self.query()
            except websockets.exceptions.ConnectionClosed:
                print('Websocket closed. Reconnect.')

    async def query(self):
        async with websockets.connect(StockUrl(account=Player.get().account, stock=Player.get().stock,
                                               is_ticker_tape=True).str()) as websocket:
            while True:
                # print(json.loads(await websocket.recv()))
                quote = json.loads(await websocket.recv())['quote']
                await self.callback(quote)


# Use polling instead of push
class QuoteSource2:
    def __init__(self, callback):
        self.callback = callback

    async def run(self):
        while True:
            quote = await Primitive.quote()
            await self.callback(quote)
            # Just experiment with this
            await asyncio.sleep(1)


class ExecutionSource:
    def __init__(self, callback):
        self.callback = callback

    async def run(self):
        while True:
            try:
                await self.query()
            except websockets.exceptions.ConnectionClosed:
                print('Websocket closed. Reconnect.')

    async def query(self):
        async with websockets.connect(
                StockUrl(account=Player.get().account, stock=Player.get().stock, is_execution=True).str()) as websocket:
            while True:
                # print(json.loads(await websocket.recv()))
                execution = json.loads(await websocket.recv())
                await self.callback(execution)


# This represents the client's view of an order.
# The class is responsible for synchronizing itself with the remote.
class Order:
    def __init__(self, order_set, direction, order_type, amount, price):
        self.orderSet = order_set
        self.direction = direction
        self.orderType = order_type
        self.amount = amount
        self.price = price
        self.remote_order = None
        self.syncTask = asyncio.ensure_future(
                Primitive.place(amount=amount, order_type=order_type, price=price, direction=direction))
        self.syncTask.add_done_callback(self.place_order_finished)
        self.recover_order_task = None
        self.cancelled = False

    @property
    def id(self):
        return self.remote_order['id'] if self.remote_order else None

    @property
    def fills(self):
        return self.remote_order['fills'] if self.remote_order else []

    @property
    def is_open(self):
        if self.remote_order:
            return self.remote_order['open']
        # We don't have remote_order
        # Are we still synchronizing or it has already failed?
        if not self.syncTask.done():
            return True
        if not self.recover_order_task:
            # Our callback has not run yet. Assume still open.
            return True
        return not self.recover_order_task.done()

    @property
    def outstanding(self):
        if not self.is_open:
            return 0
        if not self.remote_order:
            return self.amount
        return self.amount - sum(fill['qty'] for fill in self.remote_order['fills'])

    def place_order_finished(self, task):
        try:
            self.remote_order = task.result()
        except Exception as e:
            print('Placing order returned exception: {}'.format(e))
            # The order might have been created in the remote
            # We need to recover it (so that we can keep track of it and retain the ability to cancel it)
            self.recover_order_task = asyncio.ensure_future(self.recover_order)

    async def recover_order(self):
        started = datetime.datetime.now()
        while True:
            if datetime.datetime.now() - started >= datetime.timedelta(seconds=30):
                print('Did not recover any order')
                break
            await asyncio.sleep(1)

            if self.remote_order:
                # We have already recovered it through other means
                break

            existing_ids = self.order_set.get_all_order_ids()
            server_orders = await Primitive.all_orders_stock()
            matching_orders = [order for order in server_orders if
                               order['id'] not in existing_ids and order['originalQty'] == self.amount and order[
                                   'symbol'] == Player.get().stock and order[
                                   'price'] == self.price and order['direction'] == self.direction and order[
                                   'orderType'] == self.orderType]
            if len(matching_orders) > 0:
                # If there are multiple matching orders then pick any one
                self.remote_order = matching_orders[0]
                print('Recovered order {}'.format(self.remote_order))
                break

    # By making the method async, I am allowing the caller to be able to wait upon the completion of the call
    async def cancel(self):
        if self.cancelled:
            return
        self.cancelled = True

        if not self.syncTask.done():
            # We should wait upon the task so that we can cancel it. Canceling the task does not buy us much.
            # Note that callbacks are called in the order they are registered. This is exactly what we need:
            # We need the handling registered in the constructor to be run before we continue here.
            await asyncio.wait([self.syncTask])
        if self.recover_order_task and not self.recover_order_task.done():
            # Wait until we have an id of the order so we can cancel.
            await asyncio.wait([self.recover_order_task])
        if not self.remote_order:
            # Still does not have the remote order. Nothing to cancel.
            return
        if self.remote_order['open']:
            self.remote_order = await Primitive.cancel(self.remote_order['id'])

    def add_fill(self, fill):
        # Only add the fill if it's not already present
        assert self.remote_order
        if len([fill for fill in self.remote_order['fills'] if fill == fill]) == 0:
            self.remote_order['fills'].append(fill)


class OrderSet:
    def __init__(self):
        self.orders = []

    def new_order(self, **kwargs):
        order = Order(self, **kwargs)
        self.orders.append(order)
        return order

    def get_order(self, order_id):
        matching = [order for order in self.orders if order.id == order_id]
        if len(matching) > 0:
            assert len(matching) == 1
            return matching[0]
        return None

    @staticmethod
    def quantity(order, fill):
        return (1 if order.direction == 'buy' else -1) * fill['qty']

    def total(self):
        return sum(OrderSet.quantity(order, fill) for order in self.orders for fill in order.fills)

    def total_cost(self):
        return sum(OrderSet.quantity(order, fill) * fill['price'] for order in self.orders for fill in
                   order.fills)

    # How much cash do I have can be inferred by my past trade
    def cash(self):
        return -self.total_cost()

    def total_value(self, market_price):
        return self.total() * market_price

    def profit(self, market_price):
        return self.total_value(market_price) + self.cash()


class State:
    MAX_POSITION = 100

    def __init__(self):
        self.orderSet = OrderSet()
        self.spread = None

    async def update_spread(self, spread):
        self.spread = spread
        tasks = [
            asyncio.ensure_future(self.update_price('buy', spread.bid)),
            asyncio.ensure_future(self.update_price('sell', spread.ask)),
        ]
        await asyncio.wait(tasks)

    async def update_price(self, direction, new_price):
        # Cancel all buys with a different bid
        cancels = []
        for order in self.orderSet.orders:
            if order.is_open and order.direction == direction and order.price != new_price:
                cancels.append(asyncio.ensure_future(order.cancel()))
        # If we don't wait for all the cancels to finish first, we risk placing too many orders
        # and hold too long (short) a position
        if len(cancels):
            await asyncio.wait(cancels)
        amount = self.get_amount(direction)
        if amount > 0:
            self.orderSet.new_order(direction=direction, price=new_price, amount=amount, order_type='limit')

    async def add_order_fill(self, order):
        matching = self.orderSet.get_order(order['id'])
        if matching:
            matching.add_fill(order['fills'][0])
        else:
            print('{} not synchronized to local yet.'.format(order['id']))
            candidates = [c for c in self.orderSet.orders if
                          c.direction == order['direction'] and c.amount == order[
                              'originalQty'] and c.price == order['price'] and order[
                              'symbol'] == stock and c.orderType == order['orderType']]
            if len(candidates) > 0:
                print('Recovered {} through execution stream'.format(order))
                candidates[0].remote_order = order
        await self.update_spread(self.spread)

    # How many to order for the given direction, given the positions we have
    def get_amount(self, direction):
        if direction == 'buy':
            buy_outstanding = sum(
                    order.outstanding for order in self.orderSet.orders if order.direction == 'buy' and order.is_open)
            return State.MAX_POSITION - buy_outstanding - self.orderSet.total()
        else:
            sell_outstanding = sum(
                    order.outstanding for order in self.orderSet.orders if order.direction == 'sell' and order.is_open)
            return State.MAX_POSITION - sell_outstanding + self.orderSet.total()


class Planner:
    def __init__(self, state):
        self.state = state

    async def new_quote(self, quote):
        quote_bid = quote.get('bid', 0)
        quote_ask = quote.get('ask', 1000000 * 100)

        print(
                'new quote. bid:{} ask:{} total: {}, cash: {}, profit: {}'.format(quote_bid, quote_ask,
                                                                                  self.state.orderSet.total(),
                                                                                  self.state.orderSet.cash(),
                                                                                  self.state.orderSet.profit(
                                                                                          quote.get('last', 0))))

        # Let's ignore market opportunities if the spread is already very small
        if quote_ask - quote_bid < 3:
            return

        new_spread = Spread(bid=quote_bid, ask=quote_ask)

        quote_bid_size = quote.get('bidSize', 0)
        open_buys = sum(order.amount for order in self.state.orderSet.orders if
                        order.is_open and order.direction == 'buy' and order.price == quote_bid)
        if open_buys < quote_bid_size:
            # There are other participants who bid at the same price.
            # We should bid higher.
            new_spread.bid = quote_bid + 1

        quote_ask_size = quote.get('askSize', 0)
        open_sells = sum(order.amount for order in self.state.orderSet.orders if
                         order.is_open and order.direction == 'sell' and order.price == quote_ask)
        if open_sells < quote_ask_size:
            # There are other participants who ask at the same price.
            # We should sell lower.
            new_spread.ask = quote_ask - 1

        await self.state.update_spread(new_spread)


class MarketMaker:
    def __init__(self):
        self.state = State()
        self.planner = Planner(self.state)
        self.tasks = [
            asyncio.ensure_future(QuoteSource2(self.process_new_quote).run()),
            asyncio.ensure_future(ExecutionSource(self.process_new_execution).run()),
        ]

    async def run(self):
        await asyncio.wait(self.tasks, return_when=concurrent.futures.ALL_COMPLETED)

    async def process_new_quote(self, quote):
        asyncio.ensure_future(self.planner.new_quote(quote))

    async def process_new_execution(self, execution):
        asyncio.ensure_future(self.state.add_order_fill(execution['order']))


class SellSideStrategy2:
    @staticmethod
    def bootstrap():
        Player.get('sell_side')
        asyncio.get_event_loop().run_until_complete(MarketMaker().run())


setup_flags()
SellSideStrategy2.bootstrap()
