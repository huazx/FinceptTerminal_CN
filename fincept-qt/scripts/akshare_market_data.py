#!/usr/bin/env python3
"""
AkShare Market Data Wrapper
Provides real-time market data for Chinese and global markets
Replaces yfinance for users in China who have network issues with Yahoo Finance

Supports: global indices, China indices, China A-shares, US stocks,
          forex, commodities, crypto
"""

import sys
import json
import time
import logging
from datetime import datetime, timedelta

try:
    import akshare as ak
    import pandas as pd
except ImportError as e:
    print(json.dumps({
        "success": False,
        "error": f"Missing dependency: {e}",
        "data": []
    }))
    sys.exit(1)

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s',
    stream=sys.stderr
)
logger = logging.getLogger(__name__)


def safe_call(func, *args, max_retries=2, **kwargs):
    for attempt in range(max_retries):
        try:
            result = func(*args, **kwargs)
            if isinstance(result, pd.DataFrame):
                if result.empty:
                    return None
                for col in result.columns:
                    if result[col].dtype == 'datetime64[ns]':
                        result[col] = result[col].astype(str)
                result = result.replace([float("inf"), float("-inf")], None)
                result = result.where(pd.notna(result), None)
                result = result.replace({pd.NA: None})
                return result
            elif isinstance(result, (list, dict)):
                return result
            else:
                return str(result)
        except Exception as e:
            if attempt < max_retries - 1:
                time.sleep(0.5)
                continue
            logger.error(f"safe_call failed for {func.__name__}: {e}")
            return None
    return None


def _to_float(val, default=0.0):
    if val is None or (isinstance(val, float) and (val != val)):
        return default
    try:
        return float(val)
    except (ValueError, TypeError):
        return default


def _to_str(val, default=""):
    if val is None:
        return default
    try:
        return str(val)
    except (ValueError, TypeError):
        return default


def resolve_arg(arg):
    if arg and arg.startswith("@"):
        path = arg[1:]
        try:
            with open(path, "r", encoding="utf-8") as f:
                data = f.read()
            try:
                import os
                os.remove(path)
            except Exception:
                pass
            return data
        except Exception:
            return arg
    return arg


# ==================== SYMBOL MAPPING ====================

GLOBAL_INDEX_MAP = {
    "^GSPC": {"akshare": "SPX", "cn_name": "标普500", "name": "S&P 500"},
    "^DJI": {"akshare": "DJIA", "cn_name": "道琼斯", "name": "Dow Jones"},
    "^IXIC": {"akshare": "NDX", "cn_name": "纳斯达克", "name": "NASDAQ"},
    "^RUT": {"akshare": None, "cn_name": None, "name": "Russell 2000"},
    "^VIX": {"akshare": None, "cn_name": None, "name": "VIX"},
    "^FTSE": {"akshare": "FTSE", "cn_name": "英国富时100", "name": "FTSE 100"},
    "^GDAXI": {"akshare": "GDAXI", "cn_name": "德国DAX30", "name": "DAX"},
    "^FCHI": {"akshare": "FCHI", "cn_name": "法国CAC40", "name": "CAC 40"},
    "^N225": {"akshare": "N225", "cn_name": "日经225", "name": "Nikkei 225"},
    "^HSI": {"akshare": "HSI", "cn_name": "恒生指数", "name": "Hang Seng"},
    "^AXJO": {"akshare": "AS51", "cn_name": "澳大利亚标普200", "name": "S&P/ASX 200"},
    "^BSESN": {"akshare": "SENSEX", "cn_name": "印度孟买SENSEX", "name": "BSE Sensex"},
    "^NSEI": {"akshare": None, "cn_name": None, "name": "Nifty 50"},
    "^STOXX50E": {"akshare": "SX5E", "cn_name": "欧洲斯托克50", "name": "EURO STOXX 50"},
    "^NYA": {"akshare": None, "cn_name": None, "name": "NYSE Composite"},
    "^SOX": {"akshare": None, "cn_name": None, "name": "PHLX Semiconductor"},
    "^IBEX": {"akshare": "IBEX", "cn_name": "西班牙IBEX35", "name": "IBEX 35"},
    "^AEX": {"akshare": "AEX", "cn_name": "荷兰AEX", "name": "AEX"},
    "^TNX": {"akshare": None, "cn_name": None, "name": "10-Yr Bond"},
    "^TYX": {"akshare": None, "cn_name": None, "name": "30-Yr Bond"},
    "^FVX": {"akshare": None, "cn_name": None, "name": "5-Yr Bond"},
    "^IRX": {"akshare": None, "cn_name": None, "name": "13-Week Bill"},
}

CHINA_INDEX_MAP = {
    "000001.SS": {"code": "000001", "name": "上证指数"},
    "000300.SS": {"code": "000300", "name": "沪深300"},
    "000016.SS": {"code": "000016", "name": "上证50"},
    "000905.SS": {"code": "000905", "name": "中证500"},
    "000852.SS": {"code": "000852", "name": "中证1000"},
    "399001.SZ": {"code": "399001", "name": "深证成指"},
    "399006.SZ": {"code": "399006", "name": "创业板指"},
    "000001": {"code": "000001", "name": "上证指数"},
    "000300": {"code": "000300", "name": "沪深300"},
    "000016": {"code": "000016", "name": "上证50"},
    "000905": {"code": "000905", "name": "中证500"},
    "000852": {"code": "000852", "name": "中证1000"},
    "399001": {"code": "399001", "name": "深证成指"},
    "399006": {"code": "399006", "name": "创业板指"},
}

FOREX_MAP = {
    "EURUSD=X": {"pair": "EUR/USD", "name": "欧元美元", "boc_name": "欧元"},
    "GBPUSD=X": {"pair": "GBP/USD", "name": "英镑美元", "boc_name": "英镑"},
    "USDJPY=X": {"pair": "USD/JPY", "name": "美元日元", "boc_name": "日元"},
    "USDCHF=X": {"pair": "USD/CHF", "name": "美元瑞郎", "boc_name": "瑞士法郎"},
    "USDCAD=X": {"pair": "USD/CAD", "name": "美元加元", "boc_name": "加拿大元"},
    "AUDUSD=X": {"pair": "AUD/USD", "name": "澳元美元", "boc_name": "澳大利亚元"},
    "NZDUSD=X": {"pair": "NZD/USD", "name": "纽元美元", "boc_name": "新西兰元"},
    "EURGBP=X": {"pair": "EUR/GBP", "name": "欧元英镑", "boc_name": None},
    "EURJPY=X": {"pair": "EUR/JPY", "name": "欧元日元", "boc_name": None},
    "GBPJPY=X": {"pair": "GBP/JPY", "name": "英镑日元", "boc_name": None},
    "USDCNY=X": {"pair": "USD/CNY", "name": "美元人民币", "boc_name": "美元"},
    "USDINR=X": {"pair": "USD/INR", "name": "美元卢比", "boc_name": None},
    "EURCHF=X": {"pair": "EUR/CHF", "name": "欧元瑞郎", "boc_name": None},
}

COMMODITY_MAP = {
    "GC=F": {"name": "黄金", "ak_name": "COMEX黄金"},
    "SI=F": {"name": "白银", "ak_name": "COMEX白银"},
    "CL=F": {"name": "原油", "ak_name": "NYMEX原油"},
    "BZ=F": {"name": "布伦特原油", "ak_name": "布伦特原油"},
    "NG=F": {"name": "天然气", "ak_name": "天然气"},
    "HG=F": {"name": "铜", "ak_name": "COMEX铜"},
    "PL=F": {"name": "铂金", "ak_name": "铂金"},
    "PA=F": {"name": "钯金", "ak_name": "钯金"},
}

CRYPTO_MAP = {
    "BTC-USD": {"name": "比特币", "symbol": "BTCUSDT"},
    "ETH-USD": {"name": "以太坊", "symbol": "ETHUSDT"},
    "BNB-USD": {"name": "币安币", "symbol": "BNBUSDT"},
    "SOL-USD": {"name": "Solana", "symbol": "SOLUSDT"},
    "XRP-USD": {"name": "XRP", "symbol": "XRPUSDT"},
    "ADA-USD": {"name": "Cardano", "symbol": "ADAUSDT"},
    "DOGE-USD": {"name": "狗狗币", "symbol": "DOGEUSDT"},
    "DOT-USD": {"name": "Polkadot", "symbol": "DOTUSDT"},
    "LTC-USD": {"name": "莱特币", "symbol": "LTCUSDT"},
    "LINK-USD": {"name": "Chainlink", "symbol": "LINKUSDT"},
    "AVAX-USD": {"name": "Avalanche", "symbol": "AVAXUSDT"},
    "UNI-USD": {"name": "Uniswap", "symbol": "UNIUSDT"},
    "ATOM-USD": {"name": "Cosmos", "symbol": "ATOMUSDT"},
}

ETF_MAP = {
    "SPY": {"name": "SPDR S&P 500"},
    "QQQ": {"name": "Invesco QQQ"},
    "DIA": {"name": "SPDR Dow Jones"},
    "EEM": {"name": "iShares Emerging"},
    "GLD": {"name": "SPDR Gold"},
    "XLK": {"name": "Technology Select"},
    "XLE": {"name": "Energy Select"},
    "XLF": {"name": "Financial Select"},
    "XLV": {"name": "Health Care Select"},
    "VNQ": {"name": "Vanguard Real Estate"},
    "IWM": {"name": "iShares Russell 2000"},
    "VTI": {"name": "Vanguard Total Stock"},
    "TLT": {"name": "iShares 20+ Year Treasury"},
    "IEF": {"name": "iShares 7-10 Year Treasury"},
    "SHY": {"name": "iShares 1-3 Year Treasury"},
    "BND": {"name": "Vanguard Total Bond"},
    "AGG": {"name": "iShares Core US Aggregate"},
    "LQD": {"name": "iShares Investment Grade Corporate"},
    "HYG": {"name": "iShares High Yield Corporate"},
    "JNK": {"name": "SPDR High Yield"},
}

BOND_MAP = {
    "^TNX": {"name": "10-Yr Treasury Yield"},
    "^TYX": {"name": "30-Yr Treasury Yield"},
    "^IRX": {"name": "13-Week Treasury Bill"},
    "^FVX": {"name": "5-Yr Treasury Yield"},
}

TREASURY_YIELD_COL_MAP = {
    "^IRX": "美国国债收益率2年",
    "^FVX": "美国国债收益率5年",
    "^TNX": "美国国债收益率10年",
    "^TYX": "美国国债收益率30年",
}


def classify_symbol(symbol):
    if symbol in GLOBAL_INDEX_MAP:
        if GLOBAL_INDEX_MAP[symbol].get("akshare") is None:
            if symbol in BOND_MAP:
                return "global_index"
            return "us_stock"
        return "global_index"
    if symbol in CHINA_INDEX_MAP:
        return "china_index"
    if symbol in BOND_MAP:
        return "global_index"
    if symbol.startswith("SH") or symbol.startswith("SZ"):
        return "china_stock"
    if symbol in FOREX_MAP:
        return "forex"
    if symbol in COMMODITY_MAP:
        return "commodity"
    if symbol in CRYPTO_MAP:
        return "crypto"
    if symbol == "DX-Y.NYB":
        return "forex"
    if symbol in ETF_MAP:
        return "us_stock"
    if symbol and symbol[0].isupper() and "=" not in symbol and not symbol.startswith("^"):
        return "us_stock"
    return "unknown"


def _find_col(df, candidates):
    for c in candidates:
        if c in df.columns:
            return c
    return None


def _parse_row(r, symbol, name_fallback="", exchange=""):
    display_name = name_fallback if name_fallback else _to_str(r.get('名称', symbol), symbol)
    return {
        "symbol": _to_str(symbol),
        "name": display_name,
        "price": _to_float(r.get('最新价', 0)),
        "change": _to_float(r.get('涨跌额', 0)),
        "change_percent": _to_float(r.get('涨跌幅', 0)),
        "volume": _to_float(r.get('成交量', r.get('24小时成交量', 0))),
        "high": _to_float(r.get('最高', r.get('最高价', 0))),
        "low": _to_float(r.get('最低', r.get('最低价', 0))),
        "open": _to_float(r.get('今开', r.get('开盘价', 0))),
        "previous_close": _to_float(r.get('昨收', r.get('昨收价', r.get('昨结', 0)))),
        "timestamp": int(datetime.now().timestamp()),
        "exchange": _to_str(exchange),
    }


def fetch_global_indices(symbols):
    results = []
    needed = {s: GLOBAL_INDEX_MAP[s] for s in symbols if s in GLOBAL_INDEX_MAP}
    if not needed:
        return results

    # Tier 1: Sina b_$* — reliable real-time data for most global indices.
    fetched = set()
    remaining = {s: info for s, info in needed.items()}
    b_remaining = {s: info for s, info in remaining.items() if s in SINA_B_INDEX_MAP}
    if b_remaining:
        b_results = _fetch_global_indices_sina_b(b_remaining)
        if b_results:
            results.extend(b_results)
            fetched.update(r["symbol"] for r in b_results)

    # Tier 2: Sina gb_$* for US indices — more detailed data (high/low/open/volume).
    remaining = {s: info for s, info in needed.items() if s not in fetched}
    us_remaining = {s: info for s, info in remaining.items() if s in SINA_GB_INDEX_MAP}
    if us_remaining:
        gb_results = _fetch_global_indices_sina_gb(us_remaining)
        if gb_results:
            results.extend(gb_results)
            fetched.update(r["symbol"] for r in gb_results)

    # Tier 3: AkShare (eastmoney) — fallback if Sina fails.
    remaining = {s: info for s, info in needed.items() if s not in fetched}
    if remaining:
        df = safe_call(ak.index_global_spot_em, max_retries=1)
        if df is not None and not df.empty:
            sym_col = _find_col(df, ['代码', 'symbol'])
            name_col = _find_col(df, ['名称', 'name'])
            if sym_col is not None:
                for yf_sym, info in remaining.items():
                    ak_sym = info.get('akshare')
                    if not ak_sym:
                        continue
                    row = df[df[sym_col] == ak_sym]
                    if row.empty:
                        continue
                    r = row.iloc[0]
                    try:
                        cn_name = info.get('cn_name') or info.get('name', '')
                        display_name = cn_name if name_col and name_col in r.index else info.get('name', '')
                        results.append(_parse_row(r, yf_sym, display_name, "GLOBAL"))
                        fetched.add(yf_sym)
                    except (ValueError, TypeError) as e:
                        logger.warning(f"Failed to parse global index data for {yf_sym}: {e}")
        else:
            logger.warning("Global index data unavailable from eastmoney")

    # Tier 4: Sina int_* — last resort (known to return stale data for some indices).
    remaining = {s: info for s, info in needed.items() if s not in fetched}
    if remaining:
        sina_results = _fetch_global_indices_sina(remaining)
        if sina_results:
            results.extend(sina_results)

    return results


def _fetch_global_indices_sina_b(needed):
    results = []
    import requests as _requests
    sina_syms = []
    sym_map = {}
    for yf_sym in needed:
        b_sym = SINA_B_INDEX_MAP.get(yf_sym)
        if b_sym:
            sina_syms.append(b_sym)
            sym_map[b_sym] = yf_sym
    if not sina_syms:
        return results
    try:
        url = "https://hq.sinajs.cn/list=" + ",".join(sina_syms)
        headers = {"Referer": "https://finance.sina.com.cn/"}
        r = _requests.get(url, headers=headers, timeout=10)
        if r.status_code != 200:
            logger.warning(f"Sina b_ index API returned status {r.status_code}")
            return results
        for line in r.text.strip().split("\n"):
            if "=" not in line:
                continue
            var_part = line.split("=")[0].strip()
            b_sym = var_part.split("_")[-1].strip()
            value_str = line.split('="')[1].rstrip('";')
            if not value_str:
                continue
            parts = value_str.split(",")
            if len(parts) < 4:
                continue
            lookup_key = "b_" + b_sym
            yf_sym = sym_map.get(lookup_key)
            if not yf_sym:
                continue
            info = needed.get(yf_sym, {})
            display_name = info.get("cn_name") or info.get("name", yf_sym)
            try:
                price = _to_float(parts[1])
                change = _to_float(parts[2])
                change_pct = _to_float(parts[3])
                open_price = 0
                prev_close = 0
                high = 0
                low = 0
                volume = 0
                if len(parts) >= 9:
                    open_price = _to_float(parts[8])
                    prev_close = _to_float(parts[9]) if len(parts) > 9 else 0
                    high = _to_float(parts[10]) if len(parts) > 10 else 0
                    low = _to_float(parts[11]) if len(parts) > 11 else 0
                    volume = _to_float(parts[12]) if len(parts) > 12 else 0
                results.append({
                    "symbol": _to_str(yf_sym),
                    "name": _to_str(display_name),
                    "price": price,
                    "change": change,
                    "change_percent": change_pct,
                    "volume": volume,
                    "high": high,
                    "low": low,
                    "open": open_price,
                    "previous_close": prev_close,
                    "timestamp": int(datetime.now().timestamp()),
                    "exchange": "GLOBAL",
                })
            except (ValueError, TypeError, IndexError) as e:
                logger.warning(f"Sina b_ index parse failed for {yf_sym}: {e}")
    except Exception as e:
        logger.warning(f"Sina b_ index fetch failed: {e}")
    return results


SINA_GB_INDEX_MAP = {
    "^GSPC": "gb_$inx",
    "^DJI": "gb_$dji",
    "^IXIC": "gb_$ixic",
}

SINA_B_INDEX_MAP = {
    "^GSPC": "b_SPX",
    "^FTSE": "b_FTSE",
    "^N225": "b_NKY",
    "^GDAXI": "b_DAX",
    "^FCHI": "b_CAC",
    "^HSI": "b_HSI",
    "^AXJO": "b_AS51",
    "^BSESN": "b_SENSEX",
    "^STOXX50E": "b_SX5E",
    "^IBEX": "b_IBEX",
    "^AEX": "b_AEX",
    "^VIX": "b_VIX",
}

SINA_GLOBAL_INDEX_MAP = {
    "^DJI": ("int_dji", "dji", "道琼斯"),
    "^GSPC": ("int_sp500", "sp500", "标普500"),
    "^IXIC": ("int_nasdaq", "nasdaq", "纳斯达克"),
    "^N225": ("int_nikkei", "nikkei", "日经225"),
    "^HSI": ("int_hangseng", "hangseng", "恒生指数"),
    "^FTSE": ("int_ftse", "ftse", "富时100"),
    "^GDAXI": ("int_dax", "dax", "德国DAX"),
    "^FCHI": ("int_cac", "cac", "法国CAC40"),
    "^STOXX50E": ("int_eurostox", "eurostox", "欧洲斯托克50"),
}


def _fetch_global_indices_sina_gb(needed):
    results = []
    import requests as _requests
    sina_syms = []
    sym_map = {}
    for yf_sym in needed:
        gb_sym = SINA_GB_INDEX_MAP.get(yf_sym)
        if gb_sym:
            sina_syms.append(gb_sym)
            sym_map[gb_sym] = yf_sym
    if not sina_syms:
        return results
    try:
        url = "https://hq.sinajs.cn/list=" + ",".join(sina_syms)
        headers = {"Referer": "https://finance.sina.com.cn/"}
        r = _requests.get(url, headers=headers, timeout=10)
        if r.status_code != 200:
            logger.warning(f"Sina gb_ index API returned status {r.status_code}")
            return results
        for line in r.text.strip().split("\n"):
            if "=" not in line:
                continue
            gb_sym = line.split("=")[0].split("_")[-1].strip()
            value_str = line.split('="')[1].rstrip('";')
            if not value_str:
                continue
            parts = value_str.split(",")
            if len(parts) < 5:
                continue
            lookup_key = "gb_" + gb_sym
            yf_sym = sym_map.get(lookup_key)
            if not yf_sym:
                continue
            try:
                price = _to_float(parts[1])
                change = _to_float(parts[4])
                results.append({
                    "symbol": _to_str(yf_sym),
                    "name": _to_str(parts[0]) if parts[0] else yf_sym,
                    "price": price,
                    "change": change,
                    "change_percent": _to_float(parts[2]),
                    "volume": 0,
                    "high": _to_float(parts[6]) if len(parts) > 6 else 0,
                    "low": _to_float(parts[7]) if len(parts) > 7 else 0,
                    "open": _to_float(parts[5]) if len(parts) > 5 else 0,
                    "previous_close": round(price - change, 2) if price else 0,
                    "timestamp": int(datetime.now().timestamp()),
                    "exchange": "GLOBAL",
                })
            except (ValueError, TypeError, IndexError) as e:
                logger.warning(f"Sina gb_ index parse failed for {yf_sym}: {e}")
    except Exception as e:
        logger.warning(f"Sina gb_ index fetch failed: {e}")
    return results


def _fetch_global_indices_sina(needed):
    results = []
    import requests as _requests
    sina_syms = []
    sym_map = {}
    for yf_sym, info in needed.items():
        entry = SINA_GLOBAL_INDEX_MAP.get(yf_sym)
        if entry:
            sina_syms.append(entry[0])
            sym_map[entry[1]] = (yf_sym, entry[2])
    if not sina_syms:
        return results
    try:
        url = "https://hq.sinajs.cn/list=" + ",".join(sina_syms)
        headers = {"Referer": "https://finance.sina.com.cn/"}
        r = _requests.get(url, headers=headers, timeout=10)
        if r.status_code != 200:
            logger.warning(f"Sina global index API returned status {r.status_code}")
            return results
        for line in r.text.strip().split("\n"):
            if "=" not in line:
                continue
            var_name = line.split("=")[0].split("_")[-1]
            value_str = line.split('="')[1].rstrip('";')
            if not value_str:
                continue
            parts = value_str.split(",")
            if len(parts) < 4:
                continue
            info_tuple = sym_map.get(var_name)
            if not info_tuple:
                continue
            yf_sym, display_name = info_tuple
            try:
                results.append({
                    "symbol": _to_str(yf_sym),
                    "name": display_name,
                    "price": _to_float(parts[1]),
                    "change": _to_float(parts[2]),
                    "change_percent": _to_float(parts[3]),
                    "volume": 0,
                    "high": 0,
                    "low": 0,
                    "open": 0,
                    "previous_close": 0,
                    "timestamp": int(datetime.now().timestamp()),
                    "exchange": "GLOBAL",
                })
            except (ValueError, TypeError, IndexError) as e:
                logger.warning(f"Sina global index parse failed for {yf_sym}: {e}")
    except Exception as e:
        logger.warning(f"Sina global index fetch failed: {e}")
    return results


def _fetch_treasury_yields(symbols):
    results = []
    needed = [s for s in symbols if s in TREASURY_YIELD_COL_MAP]
    if not needed:
        return results
    try:
        df = safe_call(ak.bond_zh_us_rate)
        if df is None or df.empty:
            logger.warning("Treasury yield data unavailable")
            return results
        latest = df.iloc[-1]
        prev = df.iloc[-2] if len(df) > 1 else latest
        for sym in needed:
            col_name = TREASURY_YIELD_COL_MAP[sym]
            if col_name not in df.columns:
                continue
            value = _to_float(latest.get(col_name, 0))
            prev_value = _to_float(prev.get(col_name, 0))
            change = round(value - prev_value, 4) if prev_value else 0
            results.append({
                "symbol": _to_str(sym),
                "name": BOND_MAP.get(sym, {}).get("name", sym),
                "price": round(value, 4),
                "change": change,
                "change_percent": 0,
                "volume": 0,
                "high": 0,
                "low": 0,
                "open": 0,
                "previous_close": round(prev_value, 4) if prev_value else 0,
                "timestamp": int(datetime.now().timestamp()),
                "exchange": "GLOBAL",
            })
    except Exception as e:
        logger.warning(f"Treasury yield fetch failed: {e}")
    return results


def fetch_china_indices(symbols):
    results = []
    needed = {s: CHINA_INDEX_MAP[s] for s in symbols if s in CHINA_INDEX_MAP}
    if not needed:
        return results

    df = safe_call(ak.stock_zh_index_spot_em, symbol="沪深重要指数", max_retries=1)
    if df is None or df.empty:
        logger.warning("China index data unavailable from eastmoney, trying sina fallback")
        return _fetch_china_indices_sina(needed)

    sym_col = _find_col(df, ['代码', 'symbol', '指数代码'])
    if sym_col is None:
        logger.warning(f"Cannot find symbol column in China index data. Columns: {df.columns.tolist()}")
        return _fetch_china_indices_sina(needed)

    for yf_sym, info in needed.items():
        code = info['code']
        row = df[df[sym_col].astype(str) == str(code)]
        if row.empty:
            continue
        r = row.iloc[0]
        try:
            results.append(_parse_row(r, yf_sym, info['name'], "CN"))
        except (ValueError, TypeError) as e:
            logger.warning(f"Failed to parse China index data for {yf_sym}: {e}")

    if not results:
        logger.warning("No China index results from eastmoney, trying sina fallback")
        return _fetch_china_indices_sina(needed)

    return results


SINA_CHINA_INDEX_MAP = {
    "000001.SS": "sh000001",
    "399001.SZ": "sz399001",
    "399006.SZ": "sz399006",
    "000300.SS": "sh000300",
    "000016.SS": "sh000016",
    "000905.SS": "sh000905",
    "399005.SZ": "sz399005",
    "399673.SZ": "sz399673",
}


def _fetch_china_indices_sina(needed):
    results = []
    try:
        df = safe_call(ak.stock_zh_index_spot_sina)
        if df is None or df.empty:
            logger.warning("Sina China index data also unavailable")
            return results
        sym_col = _find_col(df, ['代码', 'symbol'])
        if sym_col is None:
            return results
        for yf_sym, info in needed.items():
            sina_code = SINA_CHINA_INDEX_MAP.get(yf_sym)
            if not sina_code:
                continue
            row = df[df[sym_col].astype(str) == sina_code]
            if row.empty:
                continue
            r = row.iloc[0]
            try:
                results.append(_parse_row(r, yf_sym, info['name'], "CN"))
            except (ValueError, TypeError) as e:
                logger.warning(f"Sina fallback failed for China index {yf_sym}: {e}")
    except Exception as e:
        logger.warning(f"Sina China index fetch failed: {e}")
    return results


def fetch_china_stocks(symbols):
    results = []
    if not symbols:
        return results

    df = safe_call(ak.stock_zh_a_spot_em)
    if df is None or df.empty:
        logger.warning("China A-share data unavailable from AkShare")
        return results

    for sym in symbols:
        code = sym[2:] if sym.startswith("SH") or sym.startswith("SZ") else sym
        row = df[df['代码'] == code]
        if row.empty:
            continue
        r = row.iloc[0]
        try:
            results.append(_parse_row(r, sym, "", "CN"))
        except (ValueError, TypeError) as e:
            logger.warning(f"Failed to parse China stock data for {sym}: {e}")

    return results


SINA_US_SYMBOL_MAP = {
    "^VIX": "gb_vixy",
}

SINA_US_NAME_MAP = {
    "gb_vixy": "VIX Fear Gauge",
}


def _fetch_us_stocks_sina(symbols):
    results = []
    import requests as _requests

    sina_syms = []
    sym_map = {}
    for sym in symbols:
        gb_sym = SINA_US_SYMBOL_MAP.get(sym, f"gb_{sym.lower()}")
        sina_syms.append(gb_sym)
        sym_map[gb_sym] = sym

    if not sina_syms:
        return results

    try:
        url = "https://hq.sinajs.cn/list=" + ",".join(sina_syms)
        headers = {"Referer": "https://finance.sina.com.cn/"}
        r = _requests.get(url, headers=headers, timeout=10)
        if r.status_code != 200:
            logger.warning(f"Sina US stock API returned status {r.status_code}")
            return results

        for line in r.text.strip().split("\n"):
            if "=" not in line:
                continue
            var_name = line.split("=")[0].split("hq_str_")[-1]
            value_str = line.split('="')[1].rstrip('";') if '="' in line else ""
            if not value_str:
                continue
            parts = value_str.split(",")
            if len(parts) < 2:
                continue

            yf_sym = sym_map.get(var_name)
            if not yf_sym:
                continue

            try:
                name = SINA_US_NAME_MAP.get(var_name, parts[0]) if parts[0] else yf_sym
                price = _to_float(parts[1])
                change = _to_float(parts[4]) if len(parts) > 4 else 0.0
                change_pct = _to_float(parts[2]) if len(parts) > 2 else 0.0
                high = _to_float(parts[6]) if len(parts) > 6 else 0.0
                low = _to_float(parts[7]) if len(parts) > 7 else 0.0
                open_price = _to_float(parts[5]) if len(parts) > 5 else 0.0
                prev_close = _to_float(parts[3]) if len(parts) > 3 else 0.0

                results.append({
                    "symbol": _to_str(yf_sym),
                    "name": name,
                    "price": price,
                    "change": change,
                    "change_percent": change_pct,
                    "volume": 0,
                    "high": high,
                    "low": low,
                    "open": open_price,
                    "previous_close": prev_close,
                    "timestamp": int(datetime.now().timestamp()),
                    "exchange": "US",
                })
            except (ValueError, TypeError, IndexError) as e:
                logger.warning(f"Sina US stock parse failed for {yf_sym}: {e}")
    except Exception as e:
        logger.warning(f"Sina US stock fetch failed: {e}")

    return results


def _fetch_us_stocks_daily_fallback(symbols):
    results = []
    for sym in symbols:
        try:
            stock_df = safe_call(ak.stock_us_daily, symbol=sym, adjust="qfq")
            if stock_df is not None and not stock_df.empty:
                latest = stock_df.iloc[-1]
                prev = stock_df.iloc[-2] if len(stock_df) > 1 else latest
                price = _to_float(latest.get('close', latest.get('收盘', 0)))
                prev_close = _to_float(prev.get('close', prev.get('收盘', 0)))
                change = price - prev_close if prev_close > 0 else 0.0
                change_pct = (change / prev_close * 100.0) if prev_close > 0 else 0.0
                results.append({
                    "symbol": _to_str(sym),
                    "name": ETF_MAP.get(sym, {}).get('name', sym),
                    "price": price,
                    "change": change,
                    "change_percent": change_pct,
                    "volume": _to_float(latest.get('volume', latest.get('成交量', 0))),
                    "high": _to_float(latest.get('high', latest.get('最高', 0))),
                    "low": _to_float(latest.get('low', latest.get('最低', 0))),
                    "open": _to_float(latest.get('open', latest.get('开盘', 0))),
                    "previous_close": prev_close,
                    "timestamp": int(datetime.now().timestamp()),
                    "exchange": "US",
                })
        except Exception as e:
            logger.warning(f"Per-stock daily fallback failed for {sym}: {e}")
    return results


def fetch_us_stocks(symbols):
    results = []
    if not symbols:
        return results

    # Sina batch first — sub-second, works outside China.
    results = _fetch_us_stocks_sina(symbols)
    fetched = {r["symbol"] for r in results}
    remaining = [s for s in symbols if s not in fetched]

    if not remaining:
        return results

    # AkShare as fallback for symbols Sina didn't cover.
    us_func = getattr(ak, 'stock_us_spot_em', None) or getattr(ak, 'stock_us_spot', None)
    if us_func is not None:
        df = safe_call(us_func, max_retries=1)
        if df is not None and not df.empty:
            sym_col = _find_col(df, ['代码', 'symbol'])
            if sym_col is not None:
                for sym in remaining:
                    row = df[df[sym_col] == sym]
                    if row.empty:
                        row = df[df[sym_col].astype(str).str.endswith(f".{sym}")]
                    if row.empty:
                        continue
                    r = row.iloc[0]
                    try:
                        results.append(_parse_row(r, sym, ETF_MAP.get(sym, {}).get('name', ''), "US"))
                    except (ValueError, TypeError) as e:
                        logger.warning(f"Failed to parse US stock data for {sym}: {e}")

    still_remaining = [s for s in remaining if s not in {r["symbol"] for r in results}]
    if still_remaining:
        logger.info(f"Trying per-stock daily fallback for {len(still_remaining)} symbols")
        results.extend(_fetch_us_stocks_daily_fallback(still_remaining))

    return results


def fetch_forex(symbols):
    results = []
    needed = [s for s in symbols if s in FOREX_MAP]
    dx_needed = "DX-Y.NYB" in symbols
    if not needed and not dx_needed:
        return results

    import requests as _requests
    sina_headers = {"Referer": "https://finance.sina.com.cn/"}

    if needed:
        sina_pairs = []
        for sym in needed:
            info = FOREX_MAP[sym]
            pair = info['pair'].replace('/', '')
            sina_pairs.append(pair)

        try:
            url = "https://hq.sinajs.cn/list=" + ",".join(sina_pairs)
            r = _requests.get(url, headers=sina_headers, timeout=10)
            if r.status_code == 200:
                for line in r.text.strip().split('\n'):
                    if '="' not in line:
                        continue
                    var_part, data = line.split('="', 1)
                    data = data.rstrip('";')
                    if not data:
                        continue
                    parts = data.split(',')
                    if len(parts) < 11:
                        continue
                    pair_name = var_part.split('_')[-1].strip() if '_' in var_part else var_part.strip()
                    yf_sym = None
                    for sym in needed:
                        if FOREX_MAP[sym]['pair'].replace('/', '') == pair_name:
                            yf_sym = sym
                            break
                    if not yf_sym:
                        continue
                    price = _to_float(parts[1])
                    prev_close = _to_float(parts[5])
                    high = _to_float(parts[6])
                    low = _to_float(parts[7])
                    open_p = _to_float(parts[8])
                    cn_name = parts[9] if len(parts) > 9 else FOREX_MAP[yf_sym]['name']
                    change = round(price - prev_close, 6) if price and prev_close else 0
                    change_pct = round(change / prev_close * 100, 4) if prev_close else 0
                    results.append({
                        "symbol": yf_sym,
                        "name": cn_name or FOREX_MAP[yf_sym]['name'],
                        "price": price,
                        "change": change,
                        "change_percent": change_pct,
                        "volume": _to_float(parts[4]) if parts[4] else 0,
                        "high": high,
                        "low": low,
                        "open": open_p,
                        "previous_close": prev_close,
                        "timestamp": int(datetime.now().timestamp()),
                        "exchange": "FOREX",
                    })
                logger.info(f"Sina forex: fetched {len(results)}/{len(needed)} pairs")
        except Exception as e:
            logger.warning(f"Sina forex fetch failed: {e}")

    if dx_needed:
        try:
            url = "https://hq.sinajs.cn/list=DINIW"
            r = _requests.get(url, headers=sina_headers, timeout=10)
            if r.status_code == 200:
                text = r.text.strip()
                if '="' in text:
                    parts = text.split('="')[1].rstrip('";').split(",")
                    if len(parts) >= 8:
                        price = _to_float(parts[1])
                        prev_close = _to_float(parts[5])
                        results.append({
                            "symbol": "DX-Y.NYB",
                            "name": "美元指数",
                            "price": price,
                            "change": round(price - prev_close, 4) if price and prev_close else 0,
                            "change_percent": round((price - prev_close) / prev_close * 100, 4) if prev_close else 0,
                            "volume": _to_float(parts[4]) if parts[4] else 0,
                            "high": _to_float(parts[6]) if parts[6] else 0,
                            "low": _to_float(parts[7]) if parts[7] else 0,
                            "open": _to_float(parts[3]) if parts[3] else 0,
                            "previous_close": prev_close,
                            "timestamp": int(datetime.now().timestamp()),
                            "exchange": "FOREX",
                        })
        except Exception as e:
            logger.warning(f"DX-Y.NYB fetch failed: {e}")

    return results


def fetch_commodities(symbols):
    results = []
    needed = [s for s in symbols if s in COMMODITY_MAP]
    if not needed:
        return results

    try:
        futures_func = getattr(ak, 'futures_global_spot_em', None)
        if futures_func is None:
            logger.warning("futures_global_spot_em not available in AkShare")
            return results

        df = safe_call(futures_func, max_retries=1)
        if df is None or df.empty:
            logger.warning("Commodity data unavailable from AkShare")
            return results

        name_col = _find_col(df, ['名称', 'name'])
        if name_col is None:
            logger.warning(f"Cannot find name column in commodity data. Columns: {df.columns.tolist()}")
            return results

        for sym in needed:
            info = COMMODITY_MAP[sym]
            ak_name = info['ak_name']
            matched = df[df[name_col].astype(str).str.contains(ak_name, case=False, na=False)]
            if matched.empty:
                continue
            exact = matched[matched[name_col].astype(str) == ak_name]
            r = exact.iloc[0] if not exact.empty else matched.iloc[0]
            try:
                results.append(_parse_row(r, sym, info['name'], "FUTURES"))
            except (ValueError, TypeError) as e:
                logger.warning(f"Failed to parse commodity data for {sym}: {e}")
    except (AttributeError, TypeError) as e:
        logger.warning(f"Commodity data fetch failed: {e}")

    if not results:
        logger.warning("No commodity results from eastmoney, trying sina fallback")
        results = _fetch_commodities_sina(needed)

    return results


SINA_COMMODITY_MAP = {
    "GC=F": ("hf_GC", "gc", "纽约黄金"),
    "SI=F": ("hf_SI", "si", "纽约白银"),
    "CL=F": ("hf_CL", "cl", "纽约原油"),
    "HG=F": ("hf_HG", "hg", "纽约铜"),
    "NG=F": ("hf_NG", "ng", "纽约天然气"),
}


def _fetch_commodities_sina(needed):
    results = []
    import requests as _requests
    sina_syms = []
    sym_map = {}
    for sym in needed:
        entry = SINA_COMMODITY_MAP.get(sym)
        if entry:
            sina_syms.append(entry[0])
            sym_map[entry[1]] = (sym, entry[2])
    if not sina_syms:
        return results
    try:
        url = "https://hq.sinajs.cn/list=" + ",".join(sina_syms)
        headers = {"Referer": "https://finance.sina.com.cn/futures/"}
        r = _requests.get(url, headers=headers, timeout=10)
        if r.status_code != 200:
            return results
        for line in r.text.strip().split("\n"):
            if "=" not in line:
                continue
            var_name = line.split("=")[0].split("_")[-1]
            value_str = line.split('="')[1].rstrip('";')
            if not value_str:
                continue
            parts = value_str.split(",")
            if len(parts) < 8:
                continue
            info_tuple = sym_map.get(var_name)
            if not info_tuple:
                continue
            yf_sym, display_name = info_tuple
            try:
                results.append({
                    "symbol": _to_str(yf_sym),
                    "name": display_name,
                    "price": _to_float(parts[0]),
                    "change": 0,
                    "change_percent": 0,
                    "volume": 0,
                    "high": _to_float(parts[4]),
                    "low": _to_float(parts[5]),
                    "open": _to_float(parts[2]),
                    "previous_close": _to_float(parts[7]),
                    "timestamp": int(datetime.now().timestamp()),
                    "exchange": "FUTURES",
                })
            except (ValueError, TypeError, IndexError) as e:
                logger.warning(f"Sina commodity parse failed for {yf_sym}: {e}")
    except Exception as e:
        logger.warning(f"Sina commodity fetch failed: {e}")
    return results


def fetch_crypto(symbols):
    results = []
    needed = [s for s in symbols if s in CRYPTO_MAP]
    if not needed:
        return results

    try:
        crypto_func = getattr(ak, 'crypto_js_spot', None)
        if crypto_func is None:
            logger.warning("crypto_js_spot not available in AkShare")
            return results

        df = safe_call(crypto_func, max_retries=1)
        if df is None or df.empty:
            logger.warning("Crypto data unavailable from AkShare")
            return results

        sym_col = _find_col(df, ['交易品种', 'symbol', '币种'])
        if sym_col is None:
            logger.warning(f"Cannot find symbol column in crypto data. Columns: {df.columns.tolist()}")
            return results

        for sym in needed:
            info = CRYPTO_MAP[sym]
            ak_sym = info['symbol']
            base = ak_sym.replace('USDT', '').replace('USD', '')
            matched = df[df[sym_col].astype(str).str.contains(base, case=False, na=False)]
            if matched.empty:
                continue
            usd_rows = matched[matched[sym_col].astype(str).str.contains('USD', case=False, na=False)]
            r = usd_rows.iloc[0] if not usd_rows.empty else matched.iloc[0]
            try:
                results.append({
                    "symbol": sym,
                    "name": info['name'],
                    "price": _to_float(r.get('最近报价', 0)),
                    "change": _to_float(r.get('涨跌额', 0)),
                    "change_percent": _to_float(r.get('涨跌幅', 0)),
                    "volume": _to_float(r.get('24小时成交量', 0)),
                    "high": _to_float(r.get('24小时最高', 0)),
                    "low": _to_float(r.get('24小时最低', 0)),
                    "open": 0,
                    "previous_close": 0,
                    "timestamp": int(datetime.now().timestamp()),
                    "exchange": "CRYPTO",
                })
            except (ValueError, TypeError) as e:
                logger.warning(f"Failed to parse crypto data for {sym}: {e}")
    except (AttributeError, TypeError) as e:
        logger.warning(f"Crypto data fetch failed: {e}")

    return results


def get_batch_quotes(symbols):
    logger.info(f"Fetching quotes for {len(symbols)} symbols using AkShare")
    results = []

    groups = {
        "global_index": [],
        "china_index": [],
        "china_stock": [],
        "us_stock": [],
        "forex": [],
        "commodity": [],
        "crypto": [],
        "unknown": [],
    }
    for sym in symbols:
        cat = classify_symbol(sym)
        groups.setdefault(cat, []).append(sym)

    if groups["global_index"]:
        try:
            results.extend(fetch_global_indices(groups["global_index"]))
        except Exception as e:
            logger.error(f"fetch_global_indices failed: {e}")
        fetched_syms = {r["symbol"] for r in results}
        treasury_syms = [s for s in groups["global_index"] if s in BOND_MAP and s not in fetched_syms]
        if treasury_syms:
            try:
                results.extend(_fetch_treasury_yields(treasury_syms))
            except Exception as e:
                logger.error(f"_fetch_treasury_yields failed: {e}")

    if groups["china_index"]:
        try:
            results.extend(fetch_china_indices(groups["china_index"]))
        except Exception as e:
            logger.error(f"fetch_china_indices failed: {e}")

    if groups["china_stock"]:
        try:
            results.extend(fetch_china_stocks(groups["china_stock"]))
        except Exception as e:
            logger.error(f"fetch_china_stocks failed: {e}")

    if groups["us_stock"]:
        try:
            results.extend(fetch_us_stocks(groups["us_stock"]))
        except Exception as e:
            logger.error(f"fetch_us_stocks failed: {e}")

    if groups["forex"]:
        try:
            results.extend(fetch_forex(groups["forex"]))
        except Exception as e:
            logger.error(f"fetch_forex failed: {e}")

    if groups["commodity"]:
        try:
            results.extend(fetch_commodities(groups["commodity"]))
        except Exception as e:
            logger.error(f"fetch_commodities failed: {e}")

    if groups["crypto"]:
        try:
            results.extend(fetch_crypto(groups["crypto"]))
        except Exception as e:
            logger.error(f"fetch_crypto failed: {e}")

    if groups["unknown"]:
        for sym in groups["unknown"]:
            logger.debug(f"Unknown symbol type, skipping: {sym}")

    logger.info(f"Successfully fetched {len(results)}/{len(symbols)} quotes from AkShare")
    return results


def get_batch_sparklines(symbols, period="5d"):
    logger.info(f"Fetching sparklines for {len(symbols)} symbols")
    results = {}

    groups = {
        "global_index": [],
        "china_index": [],
        "china_stock": [],
        "us_stock": [],
        "forex": [],
        "commodity": [],
        "crypto": [],
        "unknown": [],
    }
    for sym in symbols:
        cat = classify_symbol(sym)
        groups.setdefault(cat, []).append(sym)

    end_date = datetime.now().strftime("%Y%m%d")
    start_date = (datetime.now() - timedelta(days=7)).strftime("%Y%m%d")

    for sym in groups.get("china_index", []):
        info = CHINA_INDEX_MAP.get(sym, {})
        code = info.get('code', sym)
        try:
            df = safe_call(ak.index_zh_a_hist, symbol=code, period="daily",
                           start_date=start_date, end_date=end_date)
            if df is not None and not df.empty:
                close_col = _find_col(df, ['收盘', 'close'])
                if close_col:
                    results[sym] = df[close_col].tolist()[-50:]
        except Exception as e:
            logger.warning(f"Sparkline fetch failed for China index {sym}: {e}")

    for sym in groups.get("china_stock", []):
        code = sym[2:] if sym.startswith("SH") or sym.startswith("SZ") else sym
        try:
            df = safe_call(ak.stock_zh_a_hist, symbol=code, period="daily",
                           start_date=start_date, end_date=end_date, adjust="qfq")
            if df is not None and not df.empty:
                close_col = _find_col(df, ['收盘', 'close'])
                if close_col:
                    results[sym] = df[close_col].tolist()[-50:]
        except Exception as e:
            logger.warning(f"Sparkline fetch failed for China stock {sym}: {e}")

    for sym in groups.get("global_index", []):
        info = GLOBAL_INDEX_MAP.get(sym, {})
        cn_name = info.get('cn_name')
        if not cn_name:
            continue
        try:
            df = safe_call(ak.index_global_hist_em, symbol=cn_name)
            if df is not None and not df.empty:
                close_col = _find_col(df, ['最新价', '收盘', 'close'])
                if close_col:
                    results[sym] = df[close_col].tolist()[-50:]
        except Exception as e:
            logger.warning(f"Sparkline fetch failed for global index {sym}: {e}")

    for sym in groups.get("us_stock", []):
        try:
            df = safe_call(ak.stock_us_daily, symbol=sym, adjust="qfq")
            if df is not None and not df.empty:
                close_col = _find_col(df, ['close', '收盘'])
                if close_col:
                    recent = df.tail(50)
                    results[sym] = recent[close_col].tolist()
        except Exception as e:
            logger.warning(f"Sparkline fetch failed for US stock {sym}: {e}")

    logger.info(f"Successfully fetched {len(results)}/{len(symbols)} sparklines")
    return results


def get_batch_all(payload):
    logger.info(f"get_batch_all called with payload keys: {list(payload.keys())}")
    out = {}

    quote_syms = payload.get("quotes") or []
    if quote_syms:
        logger.info(f"Fetching {len(quote_syms)} quotes")
        try:
            out["quotes"] = get_batch_quotes(quote_syms)
        except Exception as e:
            logger.error(f"Quotes fetch failed: {e}")
            out["quotes"] = []

    spark_syms = payload.get("sparklines") or []
    if spark_syms:
        logger.info(f"Fetching {len(spark_syms)} sparklines")
        try:
            out["sparklines"] = get_batch_sparklines(spark_syms)
        except Exception as e:
            logger.error(f"Sparklines fetch failed: {e}")
            out["sparklines"] = {}

    hist_reqs = payload.get("histories") or []
    if hist_reqs:
        try:
            out["histories"] = _fetch_histories(hist_reqs)
        except Exception as e:
            logger.error(f"Histories fetch failed: {e}")
            out["histories"] = []

    logger.info(f"get_batch_all complete: {len(out.get('quotes', []))} quotes, "
                f"{len(out.get('sparklines', {}))} sparklines, "
                f"{len(out.get('histories', []))} histories")
    return out


def _fetch_histories(hist_reqs):
    results = []
    for req in hist_reqs:
        symbol = req.get("symbol", "")
        period = req.get("period", "1mo")
        interval = req.get("interval", "1d")

        cat = classify_symbol(symbol)
        df = None

        try:
            if cat == "china_index":
                info = CHINA_INDEX_MAP.get(symbol, {})
                code = info.get('code', symbol)
                end_date = datetime.now().strftime("%Y%m%d")
                start_date = _period_to_start(period)
                df = safe_call(ak.index_zh_a_hist, symbol=code, period=interval,
                               start_date=start_date, end_date=end_date)

            elif cat == "china_stock":
                code = symbol[2:] if symbol.startswith("SH") or symbol.startswith("SZ") else symbol
                end_date = datetime.now().strftime("%Y%m%d")
                start_date = _period_to_start(period)
                df = safe_call(ak.stock_zh_a_hist, symbol=code, period=interval,
                               start_date=start_date, end_date=end_date, adjust="qfq")

            elif cat == "global_index":
                info = GLOBAL_INDEX_MAP.get(symbol, {})
                cn_name = info.get('cn_name')
                if cn_name:
                    df = safe_call(ak.index_global_hist_em, symbol=cn_name)
                    if df is not None and not df.empty:
                        cutoff = _period_to_datetime(period)
                        date_col = _find_col(df, ['日期', 'date'])
                        if date_col:
                            df[date_col] = pd.to_datetime(df[date_col], errors='coerce')
                            df = df[df[date_col] >= cutoff]

            elif cat == "us_stock":
                df = safe_call(ak.stock_us_daily, symbol=symbol, adjust="qfq")
                if df is not None and not df.empty:
                    cutoff = _period_to_datetime(period)
                    date_col = _find_col(df, ['date', '日期'])
                    if date_col:
                        df[date_col] = pd.to_datetime(df[date_col])
                        df = df[df[date_col] >= cutoff]

        except Exception as e:
            logger.warning(f"History fetch failed for {symbol}: {e}")
            results.append({"symbol": symbol, "period": period, "interval": interval,
                            "error": str(e)})
            continue

        if df is None or df.empty:
            results.append({"symbol": symbol, "period": period, "interval": interval,
                            "error": "No data"})
            continue

        points = _df_to_history_points(df)
        results.append({"symbol": symbol, "period": period, "interval": interval,
                        "points": points})

    return results


def _period_to_start(period):
    mapping = {
        "1mo": 30, "3mo": 90, "6mo": 180, "1y": 365,
        "2y": 730, "5y": 1825, "10y": 3650,
    }
    days = mapping.get(period, 30)
    return (datetime.now() - timedelta(days=days)).strftime("%Y%m%d")


def _period_to_datetime(period):
    mapping = {
        "1mo": 30, "3mo": 90, "6mo": 180, "1y": 365,
        "2y": 730, "5y": 1825, "10y": 3650,
    }
    days = mapping.get(period, 30)
    return datetime.now() - timedelta(days=days)


def _df_to_history_points(df):
    points = []
    date_col = _find_col(df, ['日期', 'date'])
    open_col = _find_col(df, ['开盘', '开盘价', 'open'])
    high_col = _find_col(df, ['最高', '最高价', 'high'])
    low_col = _find_col(df, ['最低', '最低价', 'low'])
    close_col = _find_col(df, ['收盘', '最新价', 'close'])
    vol_col = _find_col(df, ['成交量', 'volume'])

    if not all([date_col, open_col, high_col, low_col, close_col]):
        return points

    for _, row in df.iterrows():
        try:
            dt = pd.to_datetime(row[date_col])
            ts = int(dt.timestamp())
            points.append({
                "timestamp": ts,
                "open": _to_float(row[open_col]),
                "high": _to_float(row[high_col]),
                "low": _to_float(row[low_col]),
                "close": _to_float(row[close_col]),
                "volume": int(_to_float(row[vol_col], 0)),
            })
        except Exception:
            continue

    return points


def get_all_endpoints():
    return {
        "success": True,
        "endpoints": [
            {"name": "get_batch_quotes", "description": "Get quotes for multiple symbols"},
            {"name": "get_batch_sparklines", "description": "Get sparkline data"},
            {"name": "get_batch_all", "description": "Unified batch fetcher"},
        ]
    }


def _daemon_read_frame(stream):
    header = b""
    while len(header) < 4:
        chunk = stream.read(4 - len(header))
        if not chunk:
            return None
        header += chunk
    n = int.from_bytes(header, byteorder="big", signed=False)
    if n == 0 or n > 64 * 1024 * 1024:
        return None
    buf = b""
    while len(buf) < n:
        chunk = stream.read(n - len(buf))
        if not chunk:
            return None
        buf += chunk
    return buf


def _daemon_write_frame(stream, data_bytes):
    n = len(data_bytes)
    stream.write(n.to_bytes(4, byteorder="big", signed=False))
    stream.write(data_bytes)
    stream.flush()


def _daemon_dispatch(action, payload):
    if action == "batch_all":
        return get_batch_all(payload or {})
    if action == "batch_quotes":
        syms = (payload or {}).get("symbols") or []
        return get_batch_quotes(syms)
    if action == "batch_sparklines":
        syms = (payload or {}).get("symbols") or []
        return get_batch_sparklines(syms)
    return {"error": f"Unknown action: {action}"}


def run_daemon():
    stdin = sys.stdin.buffer
    stdout = sys.stdout.buffer
    ready = json.dumps({"ready": True, "pid": __import__("os").getpid()}).encode("utf-8")
    _daemon_write_frame(stdout, ready)
    while True:
        frame = _daemon_read_frame(stdin)
        if frame is None:
            break
        try:
            req = json.loads(frame.decode("utf-8"))
        except Exception as e:
            err = {"id": 0, "ok": False, "error": f"bad request JSON: {e}"}
            _daemon_write_frame(stdout, json.dumps(err).encode("utf-8"))
            continue
        req_id = req.get("id", 0)
        action = req.get("action", "")
        if action == "shutdown":
            resp = {"id": req_id, "ok": True, "result": {"shutdown": True}}
            _daemon_write_frame(stdout, json.dumps(resp).encode("utf-8"))
            break
        try:
            result = _daemon_dispatch(action, req.get("payload"))
            resp = {"id": req_id, "ok": True, "result": result}
        except Exception as e:
            resp = {"id": req_id, "ok": False, "error": str(e)}
        try:
            _daemon_write_frame(stdout, json.dumps(resp, ensure_ascii=False, default=str).encode("utf-8"))
        except Exception:
            break


def main(args=None):
    if args is None:
        args = sys.argv[1:]

    if not args:
        print(json.dumps({"error": "No command specified"}))
        return

    command = args[0]

    try:
        if command == "get_all_endpoints":
            result = get_all_endpoints()
        elif command == "batch_all":
            if len(args) < 2:
                print(json.dumps({"error": "Missing payload JSON"}))
                return
            try:
                payload = json.loads(resolve_arg(args[1]))
            except json.JSONDecodeError as e:
                result = {"error": f"Invalid JSON: {e}"}
                print(json.dumps(result, ensure_ascii=False))
                return
            result = get_batch_all(payload)
        elif command == "batch_quotes":
            if len(args) < 2:
                print(json.dumps({"error": "Missing symbols"}))
                return
            symbols = args[1].split(",") if "," in args[1] else args[1:]
            result = get_batch_quotes(symbols)
        else:
            result = {"error": f"Unknown command: {command}"}
    except Exception as e:
        logger.error(f"Unhandled exception in main: {e}")
        result = {"error": f"Internal error: {e}"}

    try:
        print(json.dumps(result, ensure_ascii=False, default=str))
    except (TypeError, ValueError) as e:
        logger.error(f"JSON serialization failed: {e}")
        print(json.dumps({"error": f"Serialization error: {e}"}))


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--daemon":
        run_daemon()
    else:
        main()
