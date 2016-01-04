import yieldfrom.requests
import asyncio
import json
import argparse
import logging

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


class Spread:
    def __init__(self, bid, ask):
        self.bid = bid
        self.ask = ask


def setup_flags():
    parser = argparse.ArgumentParser(description='Stockfighter solver')
    parser.add_argument('--log', dest='loglevel', type=str, help='The log level')
    args = parser.parse_args()
    logging.basicConfig(level=getattr(logging, args.loglevel.upper(), None))
