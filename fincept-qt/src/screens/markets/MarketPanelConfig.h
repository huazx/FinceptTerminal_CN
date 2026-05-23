#pragma once
#include <QStringList>
#include <QString>
#include <QHash>
#include <QLatin1String>
#include <QUuid>
#include <QCoreApplication>

namespace fincept::screens {

inline QStringList all_market_columns() {
    return {"SYMBOL", "LAST", "CHG", "CHG%", "HIGH", "LOW", "VOL", "BID", "ASK", "OPEN", "NAME"};
}

inline QStringList default_market_columns() {
    return {"SYMBOL", "LAST", "CHG", "CHG%", "HIGH", "LOW"};
}

inline QString market_column_display_name(const QString& col_id) {
    if (col_id == "SYMBOL") return QCoreApplication::translate("MarketPanel", "SYMBOL");
    if (col_id == "LAST")   return QCoreApplication::translate("MarketPanel", "LAST");
    if (col_id == "CHG")    return QCoreApplication::translate("MarketPanel", "CHG");
    if (col_id == "CHG%")   return QCoreApplication::translate("MarketPanel", "CHG%");
    if (col_id == "HIGH")   return QCoreApplication::translate("MarketPanel", "HIGH");
    if (col_id == "LOW")    return QCoreApplication::translate("MarketPanel", "LOW");
    if (col_id == "VOL")    return QCoreApplication::translate("MarketPanel", "VOL");
    if (col_id == "BID")    return QCoreApplication::translate("MarketPanel", "BID");
    if (col_id == "ASK")    return QCoreApplication::translate("MarketPanel", "ASK");
    if (col_id == "OPEN")   return QCoreApplication::translate("MarketPanel", "OPEN");
    if (col_id == "NAME")   return QCoreApplication::translate("MarketPanel", "NAME");
    return col_id;
}

inline QString market_symbol_display_name(const QString& symbol) {
    using S = QLatin1String;
    static const QHash<QLatin1String, QLatin1String> kMap = {
        {S("^GSPC"),      S("S&P 500")},
        {S("^IXIC"),      S("NASDAQ")},
        {S("^DJI"),       S("Dow Jones")},
        {S("^RUT"),       S("Russell 2000")},
        {S("^VIX"),       S("VIX")},
        {S("^FTSE"),      S("FTSE 100")},
        {S("^GDAXI"),     S("DAX")},
        {S("^N225"),      S("Nikkei 225")},
        {S("^FCHI"),      S("CAC 40")},
        {S("^HSI"),       S("Hang Seng")},
        {S("^AXJO"),      S("ASX 200")},
        {S("^BSESN"),     S("BSE Sensex")},
        {S("^NSEI"),      S("NSE Nifty")},
        {S("^STOXX50E"),  S("EURO STOXX 50")},
        {S("^NYA"),       S("NYSE Composite")},
        {S("^SOX"),       S("PHLX Semiconductor")},
        {S("^IBEX"),      S("IBEX 35")},
        {S("^AEX"),       S("AEX")},
        {S("000001"),     S("SSE Composite")},
        {S("000300"),     S("CSI 300")},
        {S("399001"),     S("SZSE Component")},
        {S("399006"),     S("ChiNext")},
        {S("EURUSD=X"),   S("EUR/USD")},
        {S("GBPUSD=X"),   S("GBP/USD")},
        {S("USDJPY=X"),   S("USD/JPY")},
        {S("USDCHF=X"),   S("USD/CHF")},
        {S("USDCAD=X"),   S("USD/CAD")},
        {S("AUDUSD=X"),   S("AUD/USD")},
        {S("NZDUSD=X"),   S("NZD/USD")},
        {S("EURGBP=X"),   S("EUR/GBP")},
        {S("EURJPY=X"),   S("EUR/JPY")},
        {S("GBPJPY=X"),   S("GBP/JPY")},
        {S("USDCNY=X"),   S("USD/CNY")},
        {S("USDINR=X"),   S("USD/INR")},
        {S("EURCHF=X"),   S("EUR/CHF")},
        {S("GC=F"),       S("Gold")},
        {S("SI=F"),       S("Silver")},
        {S("PL=F"),       S("Platinum")},
        {S("PA=F"),       S("Palladium")},
        {S("HG=F"),       S("Copper")},
        {S("CL=F"),       S("WTI Crude")},
        {S("BZ=F"),       S("Brent Crude")},
        {S("NG=F"),       S("Natural Gas")},
        {S("RB=F"),       S("RBOB Gasoline")},
        {S("HO=F"),       S("Heating Oil")},
        {S("ZC=F"),       S("Corn")},
        {S("ZW=F"),       S("Wheat")},
        {S("ZS=F"),       S("Soybeans")},
        {S("KC=F"),       S("Coffee")},
        {S("CT=F"),       S("Cotton")},
        {S("SB=F"),       S("Sugar")},
        {S("CC=F"),       S("Cocoa")},
        {S("LBS=F"),      S("Lumber")},
        {S("^TNX"),       S("10Y Treasury")},
        {S("^TYX"),       S("30Y Treasury")},
        {S("^IRX"),       S("13W T-Bill")},
        {S("^FVX"),       S("5Y Treasury")},
        {S("TLT"),        S("TLT")},
        {S("IEF"),        S("IEF")},
        {S("SHY"),        S("SHY")},
        {S("BND"),        S("BND")},
        {S("AGG"),        S("AGG")},
        {S("LQD"),        S("LQD")},
        {S("HYG"),        S("HYG")},
        {S("JNK"),        S("JNK")},
        {S("SPY"),        S("SPY")},
        {S("QQQ"),        S("QQQ")},
        {S("DIA"),        S("DIA")},
        {S("EEM"),        S("EEM")},
        {S("GLD"),        S("GLD")},
        {S("XLK"),        S("XLK")},
        {S("XLE"),        S("XLE")},
        {S("XLF"),        S("XLF")},
        {S("XLV"),        S("XLV")},
        {S("VNQ"),        S("VNQ")},
        {S("IWM"),        S("IWM")},
        {S("VTI"),        S("VTI")},
        {S("BTC-USD"),    S("Bitcoin")},
        {S("ETH-USD"),    S("Ethereum")},
        {S("BNB-USD"),    S("BNB")},
        {S("SOL-USD"),    S("Solana")},
        {S("XRP-USD"),    S("XRP")},
        {S("ADA-USD"),    S("Cardano")},
        {S("DOGE-USD"),   S("Dogecoin")},
        {S("LINK-USD"),   S("Chainlink")},
        {S("DOT-USD"),    S("Polkadot")},
        {S("AVAX-USD"),   S("Avalanche")},
        {S("UNI-USD"),    S("Uniswap")},
        {S("ATOM-USD"),   S("Cosmos")},
        {S("LTC-USD"),    S("Litecoin")},
        {S("DX-Y.NYB"),   S("US Dollar Index")},
        {S("RELIANCE.NS"),S("Reliance")},
        {S("TCS.NS"),     S("TCS")},
        {S("HDFCBANK.NS"),S("HDFC Bank")},
        {S("INFY.NS"),    S("Infosys")},
        {S("HINDUNILVR.NS"),S("Hindustan Unilever")},
        {S("ICICIBANK.NS"),S("ICICI Bank")},
        {S("SBIN.NS"),    S("SBI")},
        {S("BHARTIARTL.NS"),S("Bharti Airtel")},
        {S("ITC.NS"),     S("ITC")},
        {S("KOTAKBANK.NS"),S("Kotak Bank")},
        {S("LT.NS"),      S("L&T")},
        {S("WIPRO.NS"),   S("Wipro")},
        {S("BABA"),       S("Alibaba")},
        {S("PDD"),        S("PDD")},
        {S("JD"),         S("JD.com")},
        {S("BIDU"),       S("Baidu")},
        {S("NIO"),        S("NIO")},
        {S("LI"),         S("Li Auto")},
        {S("XPEV"),       S("XPeng")},
        {S("BILI"),       S("Bilibili")},
        {S("NTES"),       S("NetEase")},
        {S("ZTO"),        S("ZTO Express")},
        {S("VNET"),       S("VNET")},
        {S("TAL"),        S("TAL Education")},
        {S("AAPL"),       S("Apple")},
        {S("MSFT"),       S("Microsoft")},
        {S("GOOGL"),      S("Alphabet")},
        {S("AMZN"),       S("Amazon")},
        {S("NVDA"),       S("NVIDIA")},
        {S("META"),       S("Meta")},
        {S("TSLA"),       S("Tesla")},
        {S("JPM"),        S("JPMorgan")},
        {S("V"),          S("Visa")},
        {S("WMT"),        S("Walmart")},
        {S("UNH"),        S("UnitedHealth")},
        {S("MA"),         S("Mastercard")},
        {S("PLTR"),       S("Palantir")},
        {S("COIN"),       S("Coinbase")},
        {S("AMD"),        S("AMD")},
        {S("NFLX"),       S("Netflix")},
        {S("INTC"),       S("Intel")},
        {S("QCOM"),       S("Qualcomm")},
        {S("ADBE"),       S("Adobe")},
        {S("CSCO"),       S("Cisco")},
        {S("ORCL"),       S("Oracle")},
        {S("CRM"),        S("Salesforce")},
        {S("AVGO"),       S("Broadcom")},
        {S("TXN"),        S("Texas Instruments")},
        {S("BRK-B"),      S("Berkshire Hathaway")},
        {S("XOM"),        S("ExxonMobil")},
        {S("LLY"),        S("Eli Lilly")},
        {S("JNJ"),        S("Johnson & Johnson")},
        {S("PG"),         S("Procter & Gamble")},
        {S("HD"),         S("Home Depot")},
        {S("CVX"),        S("Chevron")},
        {S("MRK"),        S("Merck")},
        {S("GS"),         S("Goldman Sachs")},
        {S("BAC"),        S("Bank of America")},
        {S("WFC"),        S("Wells Fargo")},
        {S("C"),          S("Citigroup")},
        {S("MS"),         S("Morgan Stanley")},
        {S("BLK"),        S("BlackRock")},
        {S("AXP"),        S("American Express")},
        {S("CAT"),        S("Caterpillar")},
        {S("BA"),         S("Boeing")},
        {S("GE"),         S("GE")},
        {S("DIS"),        S("Disney")},
        {S("NKE"),        S("Nike")},
        {S("KO"),         S("Coca-Cola")},
        {S("PEP"),        S("PepsiCo")},
        {S("MCD"),        S("McDonald's")},
        {S("PFE"),        S("Pfizer")},
        {S("ABT"),        S("Abbott Labs")},
        {S("TMO"),        S("Thermo Fisher")},
        {S("UPS"),        S("UPS")},
        {S("FDX"),        S("FedEx")},
    };
    auto it = kMap.find(QLatin1String(symbol.toUtf8().constData()));
    if (it != kMap.end())
        return QCoreApplication::translate("MarketSymbol", it.value().latin1());
    return symbol;
}

struct MarketPanelConfig {
    QString     id;
    QString     title;
    QStringList symbols;
    bool        show_name      = false;         // legacy — kept for JSON compat
    QStringList column_order;                   // display column order; empty = use default_market_columns()
    int         column_index   = 0;             // which horizontal splitter column (0-based)
    int         splitter_index = 0;             // position within that column's vertical splitter

    static MarketPanelConfig make(const QString& title, const QStringList& symbols, bool /*show_name*/ = false) {
        return { QUuid::createUuid().toString(QUuid::WithoutBraces), title, symbols,
                 false, default_market_columns(), 0, 0 };
    }
};

} // namespace fincept::screens
