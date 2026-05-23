#pragma once
#include "screens/dashboard/widgets/QuoteTableWidget.h"
#include "ui/theme/Theme.h"
#include <QCoreApplication>

namespace fincept::screens::widgets {

inline QuoteTableWidget* create_commodities_widget(QWidget* parent = nullptr) {
    QMap<QString, QString> labels = {
        {"GC=F", QCoreApplication::translate("MarketSymbol", "Gold")},
        {"SI=F", QCoreApplication::translate("MarketSymbol", "Silver")},
        {"CL=F", QCoreApplication::translate("MarketSymbol", "WTI Crude")},
        {"BZ=F", QCoreApplication::translate("MarketSymbol", "Brent Crude")},
        {"NG=F", QCoreApplication::translate("MarketSymbol", "Natural Gas")},
        {"HG=F", QCoreApplication::translate("MarketSymbol", "Copper")},
        {"PL=F", QCoreApplication::translate("MarketSymbol", "Platinum")},
        {"PA=F", QCoreApplication::translate("MarketSymbol", "Palladium")},
    };
    return new QuoteTableWidget(QCoreApplication::translate("CommoditiesWidget", "COMMODITIES"), 
                                services::MarketDataService::commodity_symbols(), labels, 2,
                                ui::colors::WARNING(), parent);
}

} // namespace fincept::screens::widgets
