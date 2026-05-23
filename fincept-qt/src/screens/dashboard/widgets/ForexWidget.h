#pragma once
#include "screens/dashboard/widgets/QuoteTableWidget.h"
#include <QCoreApplication>

namespace fincept::screens::widgets {

inline QuoteTableWidget* create_forex_widget(QWidget* parent = nullptr) {
    QMap<QString, QString> labels = {
        {"EURUSD=X", QCoreApplication::translate("ForexWidget", "EUR/USD")},
        {"GBPUSD=X", QCoreApplication::translate("ForexWidget", "GBP/USD")},
        {"USDJPY=X", QCoreApplication::translate("ForexWidget", "USD/JPY")},
        {"AUDUSD=X", QCoreApplication::translate("ForexWidget", "AUD/USD")},
        {"USDCAD=X", QCoreApplication::translate("ForexWidget", "USD/CAD")},
        {"USDCHF=X", QCoreApplication::translate("ForexWidget", "USD/CHF")},
        {"NZDUSD=X", QCoreApplication::translate("ForexWidget", "NZD/USD")},
        {"EURCHF=X", QCoreApplication::translate("ForexWidget", "EUR/CHF")},
    };
    return new QuoteTableWidget(QCoreApplication::translate("ForexWidget", "FOREX - MAJOR PAIRS"),
                                services::MarketDataService::forex_symbols(), labels, 4,
                                "#9D4EDD", parent);
}

} // namespace fincept::screens::widgets
