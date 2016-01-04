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
