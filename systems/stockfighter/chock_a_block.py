from datetime import timedelta
import asyncio
import datetime
import json
import time
import types
import websockets
import yieldfrom.requests
import argparse
import logging
import concurrent.futures

# -----------------------------------
# Problems so far:
#
# *) Order of canceling and new order seems reversed
# *) Need to think more on how to move the spread: someone crosses me higher, if I also move higher, I stand to lose
# -----------------------------------

# ----------- Back up ---------------
# url = 'https://api.stockfighter.io/ob/api/venues/{venue}/accounts/{account}/stocks/{stock}/orders'.format(venue=venue, account=account, stock=stock)
# pprint.pprint(r.json())
# r = requests.post(url, data=json.dumps(order), headers=auth)
# print(r.status_code)
# print(r.text)
# Tracks all the securities that I have.
# It is a map from security name to its positions.
# class AllPositions:
#    def __init__(self):
#        self.map = dict()
# async with websockets.connect('wss://api.stockfighter.io/ob/api/ws/EXB123456/venues/TESTEX/executions') as websocket:
# ----------------------------------

account = 'STB5052744'
venue = 'KOPEX'
stock = 'EPG'
auth = {'X-Starfighter-Authorization': 'bc9e234c0fc0fc2f45aabf7543bfbee1778921ef'}


class StockUrl:
    def __init__(self, account=None, stock=None, id=None, is_quote=False, is_ticker_tape=False, is_execution=False):
        self.venue = venue
        self.account = account
        self.stock = stock
        self.id = id
        self.isQuote = is_quote
        self.isTickerTape = is_ticker_tape
        self.isExecution = is_execution

    def str(self):
        if self.isTickerTape:
            return 'wss://api.stockfighter.io/ob/api/ws/{}/venues/{}/tickertape/stocks/{}'.format(self.account,
                                                                                                  self.venue,
                                                                                                  self.stock)

        if self.isExecution:
            return 'wss://api.stockfighter.io/ob/api/ws/{}/venues/{}/executions/stocks/{}'.format(self.account,
                                                                                                  self.venue,
                                                                                                  self.stock)

        url = 'https://api.stockfighter.io/ob/api'
        url += '/venues/' + self.venue
        if self.account is not None:
            url += '/accounts/' + self.account
        if self.stock is not None:
            url += '/stocks/' + self.stock
        if self.isQuote:
            url += '/quote'
        else:
            url += '/orders'
            if self.id is not None:
                url += '/{}'.format(self.id)
        return url


# Primitive methods do not throw exceptions
class Primitive:
    @staticmethod
    @asyncio.coroutine
    def place(amount, order_type, price, direction):
        order = {
            'account': account,
            'venue': venue,
            'symbol': stock,
            'price': price,
            'qty': amount,
            'direction': direction,
            'orderType': order_type,
        }
        try:
            r = yield from yieldfrom.requests.post(StockUrl(stock=stock).str(), data=json.dumps(order), headers=auth)
            if r.status_code != 200:
                raise Exception((yield from r.text))
            response = yield from r.json()
            if not response['ok']:
                raise Exception(response['error'])
            print('order {} placed {} at {} {}'.format(response['id'], amount, price, direction))
            return response
        except Exception as e:
            print('Error placing order ({}): {}'.format(order, e))
            return None

    @staticmethod
    @asyncio.coroutine
    def query(order):
        try:
            r = yield from yieldfrom.requests.get(StockUrl(stock=stock, id=order['id']).str(), headers=auth)
            response = yield from r.json()
            if not response['ok']:
                raise Exception(response['error'])
            return response
        except Exception as e:
            print('Error querying {}: {}'.format(order['id'], e))
            # Return the original order
            return order

    @staticmethod
    @asyncio.coroutine
    def cancel(order_id):
        print('canceling {}'.format(order_id))
        while True:
            try:
                r = yield from yieldfrom.requests.delete(StockUrl(stock=stock, id=order_id).str(), headers=auth)
                response = yield from r.json()
                if not response['ok']:
                    raise Exception(response['error'])
                return response
            except Exception as e:
                print('Error canceling {}: {}'.format(order_id, e))
                yield from asyncio.sleep(1)

    @staticmethod
    @asyncio.coroutine
    def all_orders_stock():
        while True:
            try:
                r = yield from yieldfrom.requests.get(StockUrl(account=account, stock=stock).str(), headers=auth)
                response = yield from r.json()
                if not response['ok']:
                    raise Exception(response['error'])
                return response['orders']
            except Exception as e:
                print('Error querying all orders: {}'.format(e))
                yield from asyncio.sleep(1)

    @staticmethod
    @asyncio.coroutine
    def quote():
        while True:
            try:
                r = yield from yieldfrom.requests.get(StockUrl(stock=stock, is_quote=True).str(), headers=auth)
                response = yield from r.json()
                if not response['ok']:
                    raise Exception(response['error'])
                return response
            except Exception as e:
                print('Error querying quote: {}'.format(e))
                yield from asyncio.sleep(1)


class Fill:
    def __init__(self, price, amount, order_id):
        self.price = price
        self.amount = amount
        self.orderId = order_id


class Positions:
    # Prices are in cents
    def __init__(self):
        self.fills = []

    def total(self):
        return sum(fill.amount for fill in self.fills)

    def totalCost(self):
        return sum(fill.price * fill.amount for fill in self.fills)

    def averageCost(self):
        return self.totalCost() / self.total() if self.total() != 0 else 0

    # Remove all existing fills with the same order id, and re-add all fills from the order
    def update(self, order):
        self.fills = [fill for fill in self.fills if fill.orderId != order['id']]
        self.fills[len(self.fills):] = [
            Fill(fill['price'], fill['qty'] * (1 if order['direction'] == 'buy' else -1), order['id']) for fill in
            order['fills']]

    # How much cash do I have can be inferred by my past trade
    def cash(self):
        return -self.totalCost()

    def totalValue(self, marketPrice):
        return self.total() * marketPrice

    def profit(self, marketPrice):
        return self.totalValue(marketPrice) + self.cash()


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
        async with websockets.connect(StockUrl(account=account, stock=stock, is_ticker_tape=True).str()) as websocket:
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
        async with websockets.connect(StockUrl(account=account, stock=stock, is_execution=True).str()) as websocket:
            while True:
                # print(json.loads(await websocket.recv()))
                execution = json.loads(await websocket.recv())
                await self.callback(execution)


class Spread:
    def __init__(self, bid, ask):
        self.bid = bid
        self.ask = ask


class SpreadCalculator:
    @staticmethod
    def get(last):
        margin = 25
        myBid = last - margin
        myAsk = last + margin
        return Spread(bid=myBid, ask=myAsk)


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
                                   'symbol'] == stock and order[
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
        asyncio.get_event_loop().run_until_complete(MarketMaker().run())


class Orders:
    def __init__(self):
        self.orders = dict()
        self.positions = Positions()
        # Keeps track of all orders we have ever processed (open or closed)
        self.all_orders = set()

    @classmethod
    async def create(cls):
        order = cls()
        await order.initialize()
        return order

    async def initialize(self):
        # Initialize my orders
        self.orders = {order['id']: order for order in await Primitive.all_orders_stock()}
        self.all_orders = {order_id for order_id, order in self.orders.items()}
        await self.wait(timedelta(seconds=1))
        print('I have {} {}'.format(self.pos().total(), stock))

    def pos(self):
        return self.positions

    async def place(self, amount, order_type, direction, price=0):
        while True:
            order = await Primitive.place(amount=amount, order_type=order_type, price=price, direction=direction)
            if order is not None:
                self.orders[order['id']] = order
                self.all_orders.add(order['id'])
                return order
            else:
                print(
                        'Error when ordering({}) {} shares of {} with type {}'.format(direction, amount, stock,
                                                                                      order_type))
                time.sleep(5)  # Wait some time for the state to be committed in remote

                # This is a tricky situation - we initiated an `insertion` into the order book but cannot confirm its
                # success or failure. We need to request all orders that the order book and check whether it has
                # anything we don't have.
                new_orders = await self.sync_orders()
                # Quirk: query returns 'orderType' but place returns 'type'
                possible_orders = [order for order in new_orders if
                                   order['originalQty'] == amount and order['symbol'] == stock and order[
                                       'price'] == price and order['direction'] == direction and order[
                                       'orderType'] == order_type]
                if len(possible_orders) > 0:
                    order = possible_orders[0]
                    print('Recovered {} from remote'.format(order))
                    return order
                else:
                    print('Did not recover any orders')

    # Call this whenever we suspect our view of the orders differs from that of the remote.
    async def sync_orders(self):
        server_orders = await Primitive.all_orders_stock()
        not_local = []
        for order in server_orders:
            if not order['id'] in self.all_orders:
                print('Order {} present in server but not in local.'.format(order))
                self.orders[order['id']] = order
                self.all_orders.add(order['id'])
                not_local.append(order)
        return not_local

    # This executes a somewhat complicated behavior:
    # The caller specifies a range of acceptable prices to buy/sell shares,
    # and the function executes the orders with decreasingly favorable prices.
    # In the current iteration, requires synchronous execution.
    async def guessAndPlace(self, amount, direction, price_low, priceHigh, duration):
        left = amount
        ranges = range(price_low, priceHigh + 1, 1) if direction == 'buy' else range(priceHigh, price_low - 1, -1)
        fills = []
        for price in ranges:
            order = await self.place(left, 'limit', direction, price)
            order = await self.wait_one(order, duration)
            if order['open']:
                order = await self.cancel(order)
            fills.extend(order['fills'])
            filled = order['totalFilled']
            left -= filled
            if left <= 0:
                break

        average = sum(fill['price'] * fill['qty'] for fill in fills) / sum(fill['qty'] for fill in fills) if len(
                fills) > 0 else 0

        print('transacted({}) {}/{} between {} and {} with average price {}'.format(direction, amount - left, amount,
                                                                                    price_low, priceHigh, average))
        return amount - left, average

    async def cancel(self, order):
        if order['open']:
            order = await Primitive.cancel(order['id'])
            self.orders[order['id']] = order
            # TODO: revisit how state management is done
            await self.refresh()
            return order

    async def cancel_all(self):
        orders = [order for order in self.orders.values() if order['open']]
        for order in orders:
            await self.cancel(order)

    # Wait until the specific order is closed
    async def wait_one(self, order, duration):
        now = datetime.datetime.now()
        while order['open'] and datetime.datetime.now() - now <= duration:
            order = await Primitive.query(order)

        self.orders[order['id']] = order
        self.positions.update(order)
        return order

    # Refresh the current status of the open orders
    # Ideally we want an event driven model
    async def refresh(self):
        # Create a new dictionary
        new_orders = dict()
        for order_id, order in self.orders.items():
            if order['open']:
                order = await Primitive.query(order)
                new_orders[order_id] = order
            # TODO: we need a systematic position state management model
            self.positions.update(order)

        self.orders = new_orders

    # Wait for all orders to be closed
    async def wait(self, duration=timedelta.max):
        now = datetime.datetime.now()
        # Run the loop at least once
        while len(self.orders) != 0:
            # print('# open orders: ', len(self.orders))
            await self.refresh()
            # Sleep 10 ms to be nice
            time.sleep(0.01)
            if datetime.datetime.now() - now > duration:
                break
        return len(self.orders) == 0


class ChockABlockStrategy:
    def __init__(self):
        pass

    # Winning strategy:
    # Sell 1 market order to get the real target price
    # Set the actual target price to be ~$2 lower the target price
    # Let the strategy sell limit order for you. It sometimes places multiple concurrent orders, but that's no harm.
    async def run(self):
        orders = await Orders.create()
        await orders.place(amount=1, order_type='market', direction='buy')
        await orders.wait(timedelta(seconds=1))
        # Now we should have the back office target
        target_price = await int(input('What is the target price: '))
        while orders.pos().total() <= 100000:
            print('total positions: {} average: {} cash: {}'.format(orders.pos().total(), orders.pos().averageCost(),
                                                                    orders.pos().cash()))
            await orders.place(amount=1000, order_type='limit', price=target_price, direction='buy')
            success = await orders.wait(timedelta(seconds=30))
            if not success:
                print('Warning: order took too long to execute')
                # The market seems to have moved away from us. Try to cause a market crash.
                # print('Try to cause market crash by selling {} shares at {}'
                # .format(orders.pos().total() / 10, targetPrice - 10))
                # orders.sell(orders.pos().total() / 10, 'limit', targetPrice - 10)
            time.sleep(0.034)

    @staticmethod
    def bootstrap():
        asyncio.get_event_loop().run_until_complete(ChockABlockStrategy().run())


# This is a relatively simple strategy.
# I quote the spread by sampling the last N trades.
# Then I buy and sell shares sequentially,
# but I bid a more and more favorable price if I don't get enough fills in time.
class SellSideStrategy:
    def __init__(self):
        pass

    @classmethod
    async def create(cls):
        strategy = cls()
        strategy.orders = await Orders.create()
        return strategy

    @staticmethod
    async def get_median_last():
        n = 10
        last = []
        for i in range(0, n):
            last.append((await Primitive.quote())['last'])
            # Wait 15 s
            time.sleep(0.00085)
        last.sort()
        return last[n // 2]

    async def one_round(self):
        last = await SellSideStrategy.get_median_last()
        spread = SpreadCalculator().get(last)
        print(
                'Current quote:(last:{last}) my quote:(bid:{myBid},ask:{myAsk}) total value:${totalValue} '
                'cash:${cash} profit:${profit}'.format(
                        last=last,
                        myBid=spread.bid,
                        myAsk=spread.ask,
                        totalValue=self.orders.pos().totalValue(last) / 100,
                        cash=self.orders.pos().cash() / 100,
                        profit=self.orders.pos().profit(last) / 100));

        # ------------- Previous strategy -----------------
        # toBuy = 1000 - self.orders.pos().total()
        # if (toBuy > 0):
        #     self.orders.place(amount=toBuy, direction='buy', price=spread.bid, orderType='limit')
        # toSell = 1000 + self.orders.pos().total()
        # if (toSell > 0):
        #     self.orders.place(amount=toSell, direction='sell', price=spread.ask, orderType='limit')
        # self.orders.wait()
        # Cancel my existing orders
        # self.orders.cancel_all()
        # ------------------------------- -----------------

        midpoint = (spread.bid + spread.ask) // 2

        to_buy = 0
        to_sell = 0
        actual_buy = 0
        buy_average = 0

        to_buy = 1000 - self.orders.pos().total()
        if to_buy > 0:
            actual_buy, buyAverage = await self.orders.guessAndPlace(to_buy, 'buy', spread.bid, midpoint, timedelta(
                    milliseconds=35))  # TODO: allow directly enter game time
        to_sell = self.orders.pos().total()
        if to_sell > 0:
            # Instead of setting low to midpoint, allow `bloodletting` to some extent.
            # It's still better than trying to sell in a much lower price in the next round.
            # (The reason we are not going all the way to 0 is that
            # it takes too much time and the market could be so far from us)
            # (and we seem to be causing a market crash by repeatedly placing and canceling a very high sell order)
            actualSell, sellAverage = await self.orders.guessAndPlace(to_sell, 'sell', max(1, spread.bid), spread.ask,
                                                                      timedelta(milliseconds=35))

        if to_buy > 0 and to_sell > 0:
            print(
                    'We bought {} at {}, and sold {} at {}; made ${}'.format(actual_buy, buyAverage, actualSell,
                                                                             sellAverage,
                                                                             (
                                                                                 actualSell * sellAverage - actual_buy * buyAverage) / 100))

        return self.orders.pos().profit(last)

    @staticmethod
    async def run():
        task = await SellSideStrategy.create()
        while True:
            profit = await task.one_round()
            if profit >= 250000 * 100:
                break

    @staticmethod
    def bootstrap():
        asyncio.get_event_loop().run_until_complete(SellSideStrategy.run())


def setup_flags():
    parser = argparse.ArgumentParser(description='Stockfighter solver')
    parser.add_argument('--log', dest='loglevel', type=str, help='The log level')
    args = parser.parse_args()
    logging.basicConfig(level=getattr(logging, args.loglevel.upper(), None))


setup_flags()

# ChockABlockStrategy().bootstrap()
# SellSideStrategy.bootstrap()
SellSideStrategy2.bootstrap()
