import websocket
import json
import pickle
from market_snapshot import *

try:
    import thread
except ImportError:
    import _thread as thread


shot_map = {}
f = open('crypto.log', 'a')
f2 = open('crypto.pickle', 'ab+')
def on_message(ws,message):
    global ticker, bids, asks, bid_sizes, ask_sizes, last_trade, last_trade_size, volume, turnover, open_interest, time, depth,num,content
    temp = json.loads(message)
    symbol= temp['stream']
    ticker = symbol[0:7].upper()
    if ticker not in shot_map:
      shot_map[ticker] = MarketSnapshot()
    topic = symbol[8:14]
    if topic == 'ticker':
        shot_map[ticker].time = int(temp['data']['E'])
        shot_map[ticker].last_trade_size = float(temp['data']['Q'])
        shot_map[ticker].last_trade = float(temp['data']['c'])
        shot_map[ticker].volume = float(temp['data']['v'])
        shot_map[ticker].turnover = float(temp['data']['q'])
    elif topic == 'depth5':
        for i in range(5):
            shot_map[ticker].bids[i] = float(temp['data']['bids'][i][0])
            shot_map[ticker].bid_sizes[i] = float(temp['data']['bids'][i][1])
            shot_map[ticker].asks[i] = float(temp['data']['asks'][i][0])
            shot_map[ticker].ask_sizes[i] = float(temp['data']['asks'][i][1])
    else:
      return
    shot_map[ticker].Show(f)
    pickle.dump(snapshot, f2, pickle.HIGHEST_PROTOCOL)

def on_error(ws,error):
    print(error)

def on_close(ws):
    ws.close()
    ws.run_forever()
    print("### closed ###")

if __name__ == "__main__":
    websocket.enableTrace(True)
    ws = websocket.WebSocketApp("wss://stream.binance.com:9443/stream?streams=ethusdt@depth5/btcusdt@depth5/xrpusdt@depth5/ltcusdt@depth5/eosusdt@depth5/ethusdt@ticker/btcusdt@ticker/xrpusdt@ticker/ltcusdt@ticker/eosusdt@ticker",
                                    on_message=on_message,
                                    on_error=on_error,
                                    on_close=on_close)
    ws.run_forever()
    f.close()
    f2.close()
