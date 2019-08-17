from caler import *

cl = CALER("/root/hft/config/backtest/contract.config")
print(cl.GetMinPriceMove("ni1904"))
print(cl.CalNetPnl("ni1904", 100.0, 1, 99.0, 1,OrderSide.Buy))
print(cl.CalFee("ni1904", 100.0, 1, 99.0, 1).open_fee)
