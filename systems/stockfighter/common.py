import yieldfrom.requests
import asyncio
import json
import argparse
import logging
import requests


class Player:
    API_KEY = 'bc9e234c0fc0fc2f45aabf7543bfbee1778921ef'

    INST = None

    def __init__(self, level):
        j = requests.post("https://www.stockfighter.io/gm/levels/{}".format(level),
                          headers={'Cookie':"api_key={}".format(Player.API_KEY)}).json()
        self.account = j['account']
        self.venue = j['venues'][0]
        self.stock = j['tickers'][0]
        self.auth = {'X-Starfighter-Authorization': Player.API_KEY}

    @staticmethod
    def get(level=None):
        if Player.INST:
            return Player.INST
        Player.INST = Player(level)
        return Player.INST


class StockUrl:
    def __init__(self, account=None, stock=None, id=None, is_quote=False, is_ticker_tape=False, is_execution=False):
        self.venue = Player.get().venue
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
            'account': Player.get().account,
            'venue': Player.get().venue,
            'symbol': Player.get().stock,
            'price': price,
            'qty': amount,
            'direction': direction,
            'orderType': order_type,
        }
        try:
            r = yield from yieldfrom.requests.post(StockUrl(stock=Player.get().stock).str(), data=json.dumps(order),
                                                   headers=Player.get().auth)
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
            r = yield from yieldfrom.requests.get(StockUrl(stock=Player.get().stock, id=order['id']).str(),
                                                  headers=Player.get().auth)
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
                r = yield from yieldfrom.requests.delete(StockUrl(stock=Player.get().stock, id=order_id).str(),
                                                         headers=Player.get().auth)
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
                r = yield from yieldfrom.requests.get(
                        StockUrl(account=Player.get().account, stock=Player.get().stock).str(),
                        headers=Player.get().auth)
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
                r = yield from yieldfrom.requests.get(StockUrl(stock=Player.get().stock, is_quote=True).str(),
                                                      headers=Player.get().auth)
                response = yield from r.json()
                if not response['ok']:
                    raise Exception(response['error'])
                return response
            except Exception as e:
                print('Error querying quote: {}'.format(e))
                yield from asyncio.sleep(1)


class Spread:
    def __init__(self, bid, ask):
        self.bid = bid
        self.ask = ask


def setup_flags():
    parser = argparse.ArgumentParser(description='Stockfighter solver')
    parser.add_argument('--log', dest='loglevel', type=str, help='The log level')
    args = parser.parse_args()
    logging.basicConfig(level=getattr(logging, args.loglevel.upper(), None))
