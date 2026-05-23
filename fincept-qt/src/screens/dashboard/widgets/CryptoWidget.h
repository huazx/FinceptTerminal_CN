#pragma once
#include "screens/dashboard/widgets/QuoteTableWidget.h"
#include "ui/theme/Theme.h"
#include <QCoreApplication>

namespace fincept::screens::widgets {

inline QuoteTableWidget* create_crypto_widget(QWidget* parent = nullptr) {
    QMap<QString, QString> labels = {
        {"BTC-USD", QCoreApplication::translate("MarketSymbol", "Bitcoin")},
        {"ETH-USD", QCoreApplication::translate("MarketSymbol", "Ethereum")},
        {"BNB-USD", QCoreApplication::translate("MarketSymbol", "BNB")},
        {"SOL-USD", QCoreApplication::translate("MarketSymbol", "Solana")},
        {"XRP-USD", QCoreApplication::translate("MarketSymbol", "XRP")},
        {"ADA-USD", QCoreApplication::translate("MarketSymbol", "Cardano")},
        {"DOGE-USD", QCoreApplication::translate("MarketSymbol", "Dogecoin")},
        {"DOT-USD", QCoreApplication::translate("MarketSymbol", "Polkadot")},
        {"LTC-USD", QCoreApplication::translate("MarketSymbol", "Litecoin")},
    };
    return new QuoteTableWidget(QCoreApplication::translate("CryptoWidget", "CRYPTOCURRENCY"), 
                                services::MarketDataService::crypto_symbols(), labels, 2,
                                ui::colors::AMBER(), parent);
}

} // namespace fincept::screens::widgets
