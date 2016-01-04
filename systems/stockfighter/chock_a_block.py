from datetime import timedelta
import asyncio
import datetime
import time
from common import stock, Primitive, Spread, setup_flags


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


class SpreadCalculator:
    @staticmethod
    def get(last):
        margin = 25
        myBid = last - margin
        myAsk = last + margin
        return Spread(bid=myBid, ask=myAsk)


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


setup_flags()

# ChockABlockStrategy().bootstrap()
SellSideStrategy.bootstrap()

