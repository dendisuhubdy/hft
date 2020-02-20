#include "./caler.h"
#include "python_wrapper/wrapcaler.h"

CALER::CALER(std::string config_path) {
  libconfig::Config cfg;
  cfg.readFile(config_path.c_str());
  try {
    const libconfig::Setting & map = cfg.lookup("map");
    for (int i = 0; i < map.getLength(); i++) {
      const libconfig::Setting & setting = map[i];
      std::string ticker = setting["ticker"];
      min_price_move_map[ticker] = setting["min_price_move"];
      deposit_rate_map[ticker] = setting["deposit_rate"];
      contract_size_map[ticker] = setting["contract_size"];
      is_fixed_open_fee_rate_map[ticker] = setting["is_fixed_open_fee_rate"];
      if (is_fixed_open_fee_rate_map[ticker]) {
        open_fee_map[ticker] = setting["open_fee"];
      } else {
        open_fee_rate_map[ticker] = setting["open_fee_rate"];
      }   
      is_fixed_close_today_fee_rate_map[ticker] = setting["is_fixed_close_today_fee_rate"];
      if (is_fixed_close_today_fee_rate_map[ticker]) {
        close_today_fee_map[ticker] = setting["close_today_fee"];
      } else {
        close_today_fee_rate_map[ticker] = setting["close_today_fee_rate"];
      }
      is_fixed_close_fee_rate_map[ticker] = setting["is_fixed_close_fee_rate"];
      if (is_fixed_close_fee_rate_map[ticker]) {
        close_fee_map[ticker] = setting["close_fee"];
      } else {
        close_fee_rate_map[ticker] = setting["close_fee_rate"];
      }
    }
  } catch(const libconfig::SettingNotFoundException &nfex) {
    printf("Setting '%s' is missing", nfex.getPath());
    exit(1);
  } catch(const libconfig::SettingTypeException &tex) {
    printf("Setting '%s' has the wrong type", tex.getPath());
    exit(1);
  } catch (const std::exception& ex) {
    printf("EXCEPTION: %s\n", ex.what());
    exit(1);
  }
}

CALER::CALER(const std::string& config_path, const std::string &key) {
  libconfig::Config cfg;
  cfg.readFile(config_path.c_str());
  try {
    const libconfig::Setting & setting = cfg.lookup(key);
    std::string ticker = setting["ticker"];
    min_price_move_map[ticker] = setting["min_price_move"];
    deposit_rate_map[ticker] = setting["deposit_rate"];
    contract_size_map[ticker] = setting["contract_size"];
    is_fixed_open_fee_rate_map[ticker] = setting["is_fixed_open_fee_rate"];
    if (is_fixed_open_fee_rate_map[ticker]) {
      open_fee_map[ticker] = setting["open_fee"];
    } else {
      open_fee_rate_map[ticker] = setting["open_fee_rate"];
    }   
    is_fixed_close_today_fee_rate_map[ticker] = setting["is_fixed_close_today_fee_rate"];
    if (is_fixed_close_today_fee_rate_map[ticker]) {
      close_today_fee_map[ticker] = setting["close_today_fee"];
    } else {
      close_today_fee_rate_map[ticker] = setting["close_today_fee_rate"];
    }
    is_fixed_close_fee_rate_map[ticker] = setting["is_fixed_close_fee_rate"];
    if (is_fixed_close_fee_rate_map[ticker]) {
      close_fee_map[ticker] = setting["close_fee"];
    } else {
      close_fee_rate_map[ticker] = setting["close_fee_rate"];
    }
  } catch(const libconfig::SettingNotFoundException &nfex) {
    printf("Setting '%s' is missing", nfex.getPath());
    exit(1);
  } catch(const libconfig::SettingTypeException &tex) {
    printf("Setting '%s' has the wrong type", tex.getPath());
    exit(1);
  } catch (const std::exception& ex) {
    printf("EXCEPTION: %s\n", ex.what());
    exit(1);
  }
}

CALER::CALER(const libconfig::Setting & param_setting) {
  try {
    std::string ticker = param_setting["ticker"];
    min_price_move_map[ticker] = param_setting["min_price_move"];
    deposit_rate_map[ticker] = param_setting["deposit_rate"];
    contract_size_map[ticker] = param_setting["contract_size"];
    is_fixed_open_fee_rate_map[ticker] = param_setting["is_fixed_open_fee_rate"];
    if (is_fixed_open_fee_rate_map[ticker]) {
      open_fee_map[ticker] = param_setting["open_fee"];
    } else {
      open_fee_rate_map[ticker] = param_setting["open_fee_rate"];
    }   
    is_fixed_close_today_fee_rate_map[ticker] = param_setting["is_fixed_close_today_fee_rate"];
    if (is_fixed_close_today_fee_rate_map[ticker]) {
      close_today_fee_map[ticker] = param_setting["close_today_fee"];
    } else {
      close_today_fee_rate_map[ticker] = param_setting["close_today_fee_rate"];
    }
    is_fixed_close_fee_rate_map[ticker] = param_setting["is_fixed_close_fee_rate"];
    if (is_fixed_close_fee_rate_map[ticker]) {
      close_fee_map[ticker] = param_setting["close_fee"];
    } else {
      close_fee_rate_map[ticker] = param_setting["close_fee_rate"];
    }
  } catch(const libconfig::SettingNotFoundException &nfex) {
    printf("Setting '%s' is missing", nfex.getPath());
    exit(1);
  } catch(const libconfig::SettingTypeException &tex) {
    printf("Setting '%s' has the wrong type", tex.getPath());
    exit(1);
  } catch (const std::exception& ex) {
    printf("EXCEPTION: %s\n", ex.what());
    exit(1);
  }
}

CALER::CALER(Contractor& ct) {
  try {
    auto v = ct.GetTicker();
    for (auto i : v) {
      const libconfig::Setting & setting = ct.Lookup(i);
      std::string ticker = setting["ticker"];
      min_price_move_map[ticker] = setting["min_price_move"];
      deposit_rate_map[ticker] = setting["deposit_rate"];
      contract_size_map[ticker] = setting["contract_size"];
      is_fixed_open_fee_rate_map[ticker] = setting["is_fixed_open_fee_rate"];
      if (is_fixed_open_fee_rate_map[ticker]) {
        open_fee_map[ticker] = setting["open_fee"];
      } else {
        open_fee_rate_map[ticker] = setting["open_fee_rate"];
      }   
      is_fixed_close_today_fee_rate_map[ticker] = setting["is_fixed_close_today_fee_rate"];
      if (is_fixed_close_today_fee_rate_map[ticker]) {
        close_today_fee_map[ticker] = setting["close_today_fee"];
      } else {
        close_today_fee_rate_map[ticker] = setting["close_today_fee_rate"];
      }
      is_fixed_close_fee_rate_map[ticker] = setting["is_fixed_close_fee_rate"];
      if (is_fixed_close_fee_rate_map[ticker]) {
        close_fee_map[ticker] = setting["close_fee"];
      } else {
        close_fee_rate_map[ticker] = setting["close_fee_rate"];
      }
    }
  } catch(const libconfig::SettingNotFoundException &nfex) {
    printf("Setting '%s' is missing", nfex.getPath());
    exit(1);
  } catch(const libconfig::SettingTypeException &tex) {
    printf("Setting '%s' has the wrong type", tex.getPath());
    exit(1);
  } catch (const std::exception& ex) {
    printf("EXCEPTION: %s\n", ex.what());
    exit(1);
  }

}

CALER::~CALER() {
}

Fee CALER::CalFee(std::string ticker, double open_price, int open_size, double close_price, int close_size, bool no_close_today){
  Fee fee;
  std::string product = GetCon(ticker);
  // printf("product is %s\n", product.c_str());
  double open_fee, close_fee;
  if (is_fixed_open_fee_rate_map[product]) {
    open_fee = open_fee_map[product] * abs(open_size);
    // printf("openfee is %lf\n", open_fee);
  } else {
    // printf("rate is %lf contract_size_map is %d open_price is %lf, opensize if %d\n", open_fee_rate_map[product], contract_size_map[product], open_price, open_size);
    open_fee = open_fee_rate_map[product] * open_price * abs(open_size) * contract_size_map[product];
    // printf("openfee is %lf\n", open_fee);
  }
  if (is_fixed_close_today_fee_rate_map[product]) {
    if (no_close_today) {
      close_fee = close_fee_map[product] * abs(close_size);
    } else {
      close_fee = close_today_fee_map[product] * abs(close_size);
    }
    // printf("closefee is %lf\n", close_fee);
  } else {
    if (no_close_today) {
      close_fee = close_fee_rate_map[product] * close_price * abs(close_size) * contract_size_map[product];
    } else {
      close_fee = close_today_fee_rate_map[product] * close_price * abs(close_size) * contract_size_map[product];
    }
    // printf("closefee is %lf\n", close_fee);
  }
  fee.open_fee = open_fee;
  fee.close_fee = close_fee;
  return fee;
}

FeePoint CALER::CalFeePoint(std::string ticker, double open_price, int open_size, double close_price, int close_size, bool no_close_today) {
  // printf("%s open %lf@%d close %lf%d notoday %d\n", ticker.c_str(), open_price, open_size, close_price, close_size, no_close_today);
  FeePoint fee_point;
  std::string product = GetCon(ticker);
  double open_fee_point, close_fee_point;
  double d = contract_size_map[product];
  // printf("is_fixed %d %d\n", is_fixed_open_fee_rate_map[product], is_fixed_close_today_fee_rate_map[product]);
  if (is_fixed_open_fee_rate_map[product]) {
    open_fee_point = open_fee_map[product] * abs(close_size)/d;
  } else {
    open_fee_point = open_fee_rate_map[product] * open_price * abs(close_size) * contract_size_map[product]/d;
  }
  if (is_fixed_close_today_fee_rate_map[product]) {
    if (no_close_today) {
      close_fee_point = close_fee_map[product] * abs(close_size)/d;
    } else {
      close_fee_point = close_today_fee_map[product] * abs(close_size)/d;
    }
  } else {
    if (no_close_today) {
      close_fee_point = close_fee_rate_map[product] * close_price * abs(close_size) * contract_size_map[product]/d;
    } else {
      close_fee_point = close_today_fee_rate_map[product] * close_price * abs(close_size) * contract_size_map[product]/d;
    }
  }
  fee_point.open_fee_point = open_fee_point;
  fee_point.close_fee_point = close_fee_point;
  return fee_point;
}

int CALER::GetConSize(std::string ticker) {
  std::string con = GetCon(ticker);
  return contract_size_map[con];
}
double CALER::CalPnl(std::string ticker, double open_price, int open_size, double close_price, int close_size, OrderSide::Enum close_side) {
  int contract_size = GetConSize(ticker);
  if (open_size != close_size) {
    printf("opensize and closesize not equal %d %d\n", open_size, close_size);
    open_size = close_size;
  }
  switch (close_side) {
   case OrderSide::Sell:
     return (close_price * close_size - open_price * open_size) * contract_size;
     break;
   case OrderSide::Buy:
     return (open_price * open_size - close_price * close_size) * contract_size;
   default:
     printf("unknown side in pnl!\n");
     exit(1);
     return 0.0;
     break;
  }
}

double CALER::CalNetPnl(std::string ticker, float open_price, int open_size, float close_price, int close_size, OrderSide::Enum close_side, bool no_close_today) {
  if (open_size != close_size) {
    printf("opensize and closesize not equal %d %d\n", open_size, close_size);
    open_size = close_size;
  }
  double gross_pnl = CalPnl(ticker, open_price, open_size, close_price, close_size, close_side);
  Fee fee = CalFee(ticker, open_price, open_size, close_price, close_size, no_close_today);
  double total_fee = fee.open_fee + fee.close_fee;
  return gross_pnl - total_fee;
}

double CALER::GetMinPriceMove(std::string ticker) {
  std::string product = GetCon(ticker);
  return min_price_move_map[product];
}
