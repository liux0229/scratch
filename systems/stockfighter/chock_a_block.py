from datetime import timedelta
import asyncio
import collections
import datetime
import json
import pprint
import requests
import time
import types
import websockets
import yieldfrom.requests

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

account = 'IPS15564725'
venue = 'RUYJEX'
stock = 'TPI'
auth = {'X-Starfighter-Authorization': 'bc9e234c0fc0fc2f45aabf7543bfbee1778921ef'}

class StockUrl:
    def __init__(self, account=None, stock=None, id=None, isQuote=False):
        self.venue = venue
        self.account = account
        self.stock = stock
        self.id = id
        self.isQuote = isQuote
    def str(self):
        url = 'https://api.stockfighter.io/ob/api'
        url += '/venues/' + self.venue
        if (self.account is not None):
            url += '/accounts/' + self.account
        if (self.stock is not None):
            url += '/stocks/' + self.stock
        if (self.isQuote):
            url += '/quote'
        else:
            url += '/orders'
            if (self.id is not None):
                url += '/{}'.format(self.id)
        return url

class Primitive:
    @staticmethod
    def place(amount, orderType, price, direction):
        order = {
            'account': account,
            'venue': venue,
            'symbol': stock,
            'price': price,
            'qty': amount,
            'direction': direction,
            'orderType': orderType,
        }
        # print(json.dumps(order))
        r = requests.post(StockUrl(stock=stock).str(), data=json.dumps(order), headers=auth)
        if (r.status_code != 200):
            raise Exception(r.text)
        response = r.json()
        if (not response['ok']):
            raise Exception(response['error'])
        return response

    @staticmethod
    def buy(amount, orderType, price):
        return Primitive.place(amount, orderType, price, 'buy')
    @staticmethod
    def sell(amount, orderType, price):
        return Primitive.place(amount, orderType, price, 'sell')

    @staticmethod
    def query(id):
        r = requests.get(StockUrl(stock=stock, id=id).str(), headers=auth)
        response = r.json()
        if (not response['ok']):
            raise Exception(response['error'])
        return response

    @staticmethod
    def cancel(id):
        r = requests.delete(StockUrl(stock=stock, id=id).str(), headers=auth)
        response = r.json()
        if (not response['ok']):
            raise Exception(response['error'])
        return response

    @staticmethod
    def queryAll():
        r = requests.get(StockUrl(account=account, stock=stock).str(), headers=auth)
        response = r.json()
        if (not response['ok']):
            raise Exception(response['error'])
        return response['orders']

    @staticmethod
    # @types.coroutine
    def quote():
        # r = yield from yieldfrom.requests.get(StockUrl(stock=stock, isQuote=True).str(), headers=auth)
        r = requests.get(StockUrl(stock=stock, isQuote=True).str(), headers=auth)
        # response = yield from r.json()
        response = r.json()
        if (not response['ok']):
            raise Exception(response['error'])
        return response

class Fill:
    def __init__(self, price, amount, orderId):
        self.price = price
        self.amount = amount
        self.orderId = orderId

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

    # Remove all existing fills with the same order id, and readd all fills from the order
    def update(self, order):
        self.fills = [fill for fill in self.fills if fill.orderId != order['id']]
        self.fills[len(self.fills):] = [Fill(fill['price'], fill['qty'] * (1 if order['direction'] == 'buy' else -1), order['id']) for fill in order['fills']]

    # How much cash do I have can be inferred by my past trade
    def cash(self):
        return -self.totalCost()
    def totalValue(self, marketPrice):
        return self.total() * marketPrice
    def profit(self, marketPrice):
        return self.totalValue(marketPrice) + self.cash()

class Orders:
    def __init__(self):
        self.orders = dict()
        self.positions = Positions()
        self.initialize()

    def initialize(self):
        # Initialize my orders
        self.orders = {order['id']: order for order in Primitive.queryAll()}
        self.wait(timedelta(seconds = 1))
        print('I have {} {}'.format(self.pos().total(), stock))

    def pos(self):
        return self.positions

    def place(self, amount, orderType, direction, price=0):
        while True:
            try:
                order = Primitive.place(amount=amount, orderType=orderType, price=price, direction=direction)
                self.orders[order['id']] = order
                return order
            except Exception as e:
                print('Error when buying {} shares of {} with type {}: {}'.format(amount, stock, orderType, e))
                pass

    # This executes a somewhat complicated behavior:
    # The caller specifies a range of acceptable prices to buy/sell shares, and the function executes the orders with decreasingly favorable prices.
    # In the current iteration, requires synchronous execution.
    def guessAndPlace(self, amount, direction, priceLow, priceHigh, duration):
        left = amount
        ranges = range(priceLow, priceHigh + 1, 1) if direction == 'buy' else range(priceHigh, priceLow - 1, -1)
        fills = []
        for price in ranges:
            order = self.place(left, 'limit', direction, price)
            order = self.waitOne(order, duration)
            if (order['open']):
                order = self.cancel(order)
            fills.extend(order['fills'])
            filled = order['totalFilled']
            left -= filled
            if (left <= 0):
                break

        average = sum(fill['price'] * fill['qty'] for fill in fills) / sum(fill['qty'] for fill in fills) if len(fills) > 0 else 0

        print('transacted({}) {}/{} between {} and {} with average price {}'.format(direction, amount - left, amount, priceLow, priceHigh, average))
        return (amount - left, average)

    def cancel(self, order):
        if (order['open']):
            order = Primitive.cancel(order['id'])
            self.orders[order['id']] = order
            # TODO: revisit how state management is done
            self.refresh()
            return order

    def cancelAll(self):
        orders = [order for order in self.orders.values() if order['open']]
        for order in orders:
            self.cancel(order)

    # Wait until the specific order is closed
    def waitOne(self, order, duration):
        now = datetime.datetime.now()
        while (order['open'] and datetime.datetime.now() - now <= duration):
            order = Primitive.query(order['id'])
        self.orders[order['id']] = order
        self.positions.update(order)
        return order

    # Refresh the current status of the open orders
    # Ideally we want an event driven model
    def refresh(self):
        # Create a new dictionary
        newOrders = dict()
        for id, order in self.orders.items():
            if (order['open']):
                order = Primitive.query(id)
                newOrders[id] = order
            # TODO: we need a systematic position state management model
            self.positions.update(order)

        self.orders = newOrders

    # Wait for all orders to be closed
    def wait(self, duration=timedelta.max):
        now = datetime.datetime.now()
        # Run the loop at least once
        while (len(self.orders) != 0):
            # print('# open orders: ', len(self.orders))
            self.refresh()
            # Sleep 10 ms to be nice
            time.sleep(0.01)
            if (datetime.datetime.now() - now > duration):
                break
        return len(self.orders) == 0

class ChockABlockStrategy:
    # Winning strategy:
    # Sell 1 market order to get the real target price
    # Set the actual target price to be ~$2 lower the target price
    # Let the strategy sell limit order for you. It sometimes places multiple concurrent orders, but that's no harm.
    def run(self):
        orders = Orders()
        orders.place(amount=1, orderType='market', direction='buy')
        orders.wait(timedelta(seconds = 1))
        # Now we should have the back office target
        self.targetPrice = int(input('What is the target price: '))
        while (orders.pos().total() <= 100000):
           print('total positions: {} average: {} cash: {}'.format(orders.pos().total(), orders.pos().averageCost(), orders.pos().cash()))
           orders.place(amount=1000, orderType='limit', price=self.targetPrice, direction='buy')
           success = orders.wait(timedelta(seconds = 30))
           if (not success):
                print('Warning: order took too long to execute')
               # The market seems to have moved away from us. Try to cause a market crash.
               # print('Try to cause market crash by selling {} shares at {}'.format(orders.pos().total() / 10, targetPrice - 10))
               # orders.sell(orders.pos().total() / 10, 'limit', targetPrice - 10)
           time.sleep(0.034)

class Spread:
    def __init__(self, bid, ask):
        self.bid = bid
        self.ask = ask

class SpreadCalculator:
    # @staticmethod
    # def get(quote):
    #     # print(quote)
    #     # I can do better than what the current market offers by this much
    #     # If either bid or ask is not available, then I use last
    #     last = quote.get('last', None)
    #     if (last is None):
    #         return None
    # 
    #     margin = 10
    #     myBid = last - margin
    #     myAsk = last + margin
    #     # if ('bid' in quote):
    #     #   myBid = max(myBid, quote['bid'])
    #     # if ('ask' in quote):
    #     #   myAsk = min(myAsk, quote['ask'])
    #     return Spread(bid = myBid, ask = myAsk)

    @staticmethod
    def get(last):
        margin = 25
        myBid = last - margin
        myAsk = last + margin
        return Spread(bid = myBid, ask = myAsk)

# This is a relatively simple strategy.
# I quote the spread by sampling the last N trades.
# Then I buy and sell shares sequentially, but I bid a more and more favorable price if I don't get enough fills in time.
class SellSideStrategy:
    def __init__(self):
        self.orders = Orders()
    def getMedianLast(self):
        n = 10
        last = []
        for i in range(0, n):
            last.append(Primitive.quote()['last'])
            # Wait 15 s
            time.sleep(0.00085)
        last.sort()
        return last[n // 2]
    def oneRound(self):
        last = self.getMedianLast()
        spread = SpreadCalculator().get(last)
        print('Current quote:(last:{last}) my quote:(bid:{myBid},ask:{myAsk}) total value:${totalValue} cash:${cash} profit:${profit}'.format(
            last = last,
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
        # self.orders.cancelAll()
        # ------------------------------- -----------------

        midpoint = (spread.bid + spread.ask) // 2
        toBuy = 1000 - self.orders.pos().total()
        if (toBuy > 0):
            actualBuy, buyAverage = self.orders.guessAndPlace(toBuy, 'buy', spread.bid, midpoint, timedelta(milliseconds = 35)) # TODO: allow directly enter game time
        toSell = self.orders.pos().total()
        if (toSell > 0):
            # Instead of setting low to midpoint, allow `bloodletting` to some extent. It's still better than trying to sell in a much lower price in the next round.
            # (The reason we are not going all the way to 0 is that it takes too much time and the market could be so far from us)
            # (and we seem to be causing a market crash by repeatedly placing and canceling a very high sell order)
            actualSell, sellAverage = self.orders.guessAndPlace(toSell, 'sell', max(1, spread.bid), spread.ask, timedelta(milliseconds = 35))
            # TODO: rewrite this
            if (toBuy > 0 and toSell > 0):
                print('We bought {} at {}, and sold {} at {}; made ${}'.format(actualBuy, buyAverage, actualSell, sellAverage, (actualSell * sellAverage - actualBuy * buyAverage) / 100))

        return self.orders.pos().profit(last)
    def run(self):
        while (True):
            profit = self.oneRound()
            if (profit >= 250000 * 100):
                break

# ChockABlockStrategy().run()
SellSideStrategy().run()

# async def hello():
#    quote = await Primitive.quote()
#    print(quote)

# asyncio.get_event_loop().run_until_complete(hello())
# quote = Primitive.quote()
# print(quote)
