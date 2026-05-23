#pragma once
#include "screens/dashboard/widgets/QuoteTableWidget.h"
#include <QCoreApplication>

namespace fincept::screens::widgets {

inline QuoteTableWidget* create_indices_widget(QWidget* parent = nullptr) {
    QMap<QString, QString> labels = {
        {"^GSPC",      QCoreApplication::translate("MarketSymbol", "S&P 500")},
        {"^DJI",       QCoreApplication::translate("MarketSymbol", "Dow Jones")},
        {"^IXIC",      QCoreApplication::translate("MarketSymbol", "NASDAQ")},
        {"^RUT",       QCoreApplication::translate("MarketSymbol", "Russell 2000")},
        {"^FTSE",      QCoreApplication::translate("MarketSymbol", "FTSE 100")},
        {"^GDAXI",     QCoreApplication::translate("MarketSymbol", "DAX")},
        {"^FCHI",      QCoreApplication::translate("MarketSymbol", "CAC 40")},
        {"^N225",      QCoreApplication::translate("MarketSymbol", "Nikkei 225")},
        {"^HSI",       QCoreApplication::translate("MarketSymbol", "Hang Seng")},
        {"000001.SS",  QCoreApplication::translate("MarketSymbol", "SSE Composite")},
        {"^BSESN",     QCoreApplication::translate("MarketSymbol", "BSE Sensex")},
        {"^NSEI",      QCoreApplication::translate("MarketSymbol", "NSE Nifty")},
    };
    return new QuoteTableWidget(QCoreApplication::translate("IndicesWidget", "GLOBAL INDICES"), 
                                services::MarketDataService::indices_symbols(), labels, 2, {},
                                parent);
}

} // namespace fincept::screens::widgets
