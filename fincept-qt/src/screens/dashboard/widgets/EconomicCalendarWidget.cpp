#include "screens/dashboard/widgets/EconomicCalendarWidget.h"

#include "datahub/DataHub.h"
#include "services/news/NewsNlpService.h"
#include "ui/theme/Theme.h"
#include <QCoreApplication>

#include <QFrame>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QPointer>

namespace {
constexpr const char* kTopic = "econ:fincept:upcoming_events";

QString translate_event(const QString& name) {
    static const QHash<QString, QString> kMap = {
        {"Non Farm Payrolls", "非农就业"},
        {"Nonfarm Payrolls", "非农就业"},
        {"Nonfarm Payrolls Change", "非农就业变化"},
        {"CPI", "CPI"},
        {"CPI y/y", "CPI同比"},
        {"CPI m/m", "CPI环比"},
        {"CPI YoY", "CPI同比"},
        {"CPI MoM", "CPI环比"},
        {"Core CPI", "核心CPI"},
        {"Core CPI m/m", "核心CPI环比"},
        {"Core CPI MoM", "核心CPI环比"},
        {"Core CPI y/y", "核心CPI同比"},
        {"Core CPI YoY", "核心CPI同比"},
        {"PPI", "PPI"},
        {"PPI m/m", "PPI环比"},
        {"PPI MoM", "PPI环比"},
        {"PPI y/y", "PPI同比"},
        {"PPI YoY", "PPI同比"},
        {"Core PPI", "核心PPI"},
        {"PCE Price Index", "PCE物价指数"},
        {"Core PCE Price Index", "核心PCE物价指数"},
        {"Core PCE m/m", "核心PCE环比"},
        {"Core PCE MoM", "核心PCE环比"},
        {"Core PCE y/y", "核心PCE同比"},
        {"Core PCE YoY", "核心PCE同比"},
        {"GDP", "GDP"},
        {"GDP q/q", "GDP环比"},
        {"GDP QoQ", "GDP环比"},
        {"GDP y/y", "GDP同比"},
        {"GDP YoY", "GDP同比"},
        {"GDP Growth Rate", "GDP增长率"},
        {"Unemployment Rate", "失业率"},
        {"Initial Jobless Claims", "初请失业金"},
        {"Jobless Claims", "初请失业金"},
        {"Continuing Jobless Claims", "续请失业金"},
        {"Interest Rate Decision", "利率决议"},
        {"Interest Rate", "利率"},
        {"Fed Interest Rate Decision", "美联储利率决议"},
        {"FOMC Statement", "美联储声明"},
        {"FOMC Minutes", "美联储会议纪要"},
        {"Industrial Production", "工业产出"},
        {"Industrial Production m/m", "工业产出环比"},
        {"Industrial Production MoM", "工业产出环比"},
        {"Retail Sales", "零售销售"},
        {"Retail Sales m/m", "零售销售环比"},
        {"Retail Sales MoM", "零售销售环比"},
        {"Consumer Confidence", "消费者信心"},
        {"Consumer Sentiment", "消费者信心"},
        {"Michigan Consumer Sentiment", "密歇根消费者信心"},
        {"U. of Michigan Consumer Sentiment", "密歇根消费者信心"},
        {"Manufacturing PMI", "制造业PMI"},
        {"Services PMI", "服务业PMI"},
        {"Composite PMI", "综合PMI"},
        {"PMI", "PMI"},
        {"ISM Manufacturing PMI", "ISM制造业PMI"},
        {"ISM Services PMI", "ISM服务业PMI"},
        {"Building Permits", "建筑许可"},
        {"Housing Starts", "新屋开工"},
        {"Existing Home Sales", "成屋销售"},
        {"New Home Sales", "新屋销售"},
        {"Trade Balance", "贸易帐"},
        {"Durable Goods Orders", "耐用品订单"},
        {"Core Durable Goods Orders", "核心耐用品订单"},
        {"Factory Orders", "工厂订单"},
        {"Business Inventories", "商业库存"},
        {"Consumer Spending", "消费者支出"},
        {"Personal Spending", "个人支出"},
        {"Personal Income", "个人收入"},
        {"Import Prices", "进口价格"},
        {"Export Prices", "出口价格"},
        {"Average Hourly Earnings", "平均时薪"},
        {"Employment Change", "就业变化"},
        {"Capacity Utilization", "产能利用率"},
        {"Labor Force Participation Rate", "劳动参与率"},
        {"ADP Employment Change", "ADP就业变化"},
        {"ADP Nonfarm Employment Change", "ADP非农就业变化"},
        {"JOLTS Job Openings", "JOLTS职位空缺"},
        {"Job Openings", "职位空缺"},
        {"Wholesale Inventories", "批发库存"},
        {"Philadelphia Fed Manufacturing Index", "费城联储制造业指数"},
        {"Empire State Manufacturing Index", "纽约联储制造业指数"},
        {"Chicago PMI", "芝加哥PMI"},
        {"NAHB Housing Market Index", "NAHB房地产市场指数"},
        {"MBA Mortgage Applications", "MBA抵押贷款申请"},
        {"Crude Oil Inventories", "原油库存"},
        {"EIA Crude Oil Stocks", "EIA原油库存"},
        {"Baker Hughes Oil Rig Count", "贝克休斯石油钻井数"},
        {"10-Year Note Auction", "10年期国债拍卖"},
        {"5-Year Note Auction", "5年期国债拍卖"},
        {"2-Year Note Auction", "2年期国债拍卖"},
        {"30-Year Bond Auction", "30年期国债拍卖"},
        {"Current Account", "经常帐"},
        {"Federal Budget Balance", "联邦预算余额"},
        {"Consumer Credit", "消费者信贷"},
        {"Leading Economic Index", "领先经济指标"},
        {"Unit Labor Costs", "单位劳动成本"},
        {"Nonfarm Productivity", "非农生产力"},
        {"ECB Interest Rate Decision", "欧洲央行利率决议"},
        {"ECB Press Conference", "欧洲央行新闻发布会"},
        {"BOE Interest Rate Decision", "英国央行利率决议"},
        {"BOJ Interest Rate Decision", "日本央行利率决议"},
        {"PBOC Interest Rate Decision", "中国央行利率决议"},
        {"Caixin Manufacturing PMI", "财新制造业PMI"},
        {"Caixin Services PMI", "财新服务业PMI"},
        {"RBA Interest Rate Decision", "澳洲联储利率决议"},
        {"RBNZ Interest Rate Decision", "新西兰联储利率决议"},
        {"SNB Interest Rate Decision", "瑞士央行利率决议"},
        {"RBI Interest Rate Decision", "印度央行利率决议"},
        {"China GDP", "中国GDP"},
        {"China CPI", "中国CPI"},
        {"China PPI", "中国PPI"},
        {"China Industrial Production", "中国工业产出"},
        {"China Retail Sales", "中国零售销售"},
        {"China Trade Balance", "中国贸易帐"},
        {"LPR", "贷款市场报价利率"},
        {"Loan Prime Rate", "贷款市场报价利率"},
        {"1-Year LPR", "1年期LPR"},
        {"5-Year LPR", "5年期LPR"},
        {"Fixed Asset Investment", "固定资产投资"},
    };
    auto it = kMap.find(name);
    if (it != kMap.end())
        return it.value();
    return name;
}

} // namespace

namespace fincept::screens::widgets {

EconomicCalendarWidget::EconomicCalendarWidget(QWidget* parent)
    : BaseWidget(QCoreApplication::translate("EconomicCalendarWidget", "ECONOMIC CALENDAR"), parent, ui::colors::CYAN()) {
    auto* vl = content_layout();
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);

    // Column headers
    header_widget_ = new QWidget(this);
    auto* hl = new QHBoxLayout(header_widget_);
    hl->setContentsMargins(8, 4, 8, 4);

    auto make_hdr = [&](const QString& text, int stretch, Qt::Alignment align = Qt::AlignLeft) {
        auto* lbl = new QLabel(text);
        lbl->setAlignment(align);
        header_labels_.append(lbl);
        hl->addWidget(lbl, stretch);
    };
    make_hdr(QCoreApplication::translate("EconomicCalendarWidget", "EVENT"), 4);
    make_hdr(QCoreApplication::translate("EconomicCalendarWidget", "CTY"), 1);
    make_hdr(QCoreApplication::translate("EconomicCalendarWidget", "DATE"), 2);
    make_hdr(QCoreApplication::translate("EconomicCalendarWidget", "ACT"), 1, Qt::AlignRight);
    make_hdr(QCoreApplication::translate("EconomicCalendarWidget", "FCST"), 1, Qt::AlignRight);
    make_hdr(QCoreApplication::translate("EconomicCalendarWidget", "IMP"), 1, Qt::AlignRight);
    vl->addWidget(header_widget_);

    header_sep_ = new QFrame;
    header_sep_->setFixedHeight(1);
    vl->addWidget(header_sep_);

    // Scrollable list
    scroll_area_ = new QScrollArea;
    scroll_area_->setWidgetResizable(true);

    auto* list_widget = new QWidget(this);
    list_widget->setStyleSheet("background: transparent;");
    list_layout_ = new QVBoxLayout(list_widget);
    list_layout_->setContentsMargins(0, 0, 0, 0);
    list_layout_->setSpacing(0);

    status_label_ = new QLabel(QCoreApplication::translate("EconomicCalendarWidget", "Loading..."));
    status_label_->setAlignment(Qt::AlignCenter);
    list_layout_->addWidget(status_label_);
    list_layout_->addStretch();

    scroll_area_->setWidget(list_widget);
    vl->addWidget(scroll_area_, 1);

    // User-driven refresh button on the BaseWidget title bar — force the
    // hub to refresh the topic. Per-producer rate limit (2/sec) still
    // applies, so rage-clicking can't hammer api.fincept.in.
    connect(this, &BaseWidget::refresh_requested, this, []() {
        datahub::DataHub::instance().request(QString::fromLatin1(kTopic), /*force=*/true);
    });

    translation_timer_ = new QTimer(this);
    translation_timer_->setInterval(300);
    translation_timer_->setSingleShot(true);
    connect(translation_timer_, &QTimer::timeout, this, [this]() {
        if (!translation_queue_.isEmpty()) {
            auto next = translation_queue_.takeFirst();
            request_online_translation(next);
        } else {
            translation_in_progress_ = false;
        }
    });

    apply_styles();
    set_loading(true);
}

void EconomicCalendarWidget::apply_styles() {
    header_widget_->setStyleSheet(QString("background: %1;").arg(ui::colors::BG_RAISED()));
    for (auto* lbl : header_labels_)
        lbl->setStyleSheet(QString("color: %1; font-size: 9px; font-weight: bold; background: transparent;")
                               .arg(ui::colors::TEXT_TERTIARY()));
    header_sep_->setStyleSheet(QString("background: %1;").arg(ui::colors::BORDER_DIM()));
    scroll_area_->setStyleSheet(
        QString("QScrollArea { border: none; background: transparent; }"
                "QScrollBar:vertical { width: 4px; background: transparent; }"
                "QScrollBar::handle:vertical { background: %1; border-radius: 2px; min-height: 20px; }"
                "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }")
            .arg(ui::colors::BORDER_MED()));
    status_label_->setStyleSheet(QString("color: %1; font-size: 10px; padding: 16px; background: transparent;")
                                     .arg(ui::colors::TEXT_TERTIARY()));
}

void EconomicCalendarWidget::on_theme_changed() {
    apply_styles();
    if (!last_events_.isEmpty())
        populate(last_events_);
}

void EconomicCalendarWidget::clear_list() {
    while (list_layout_->count() > 0) {
        auto* item = list_layout_->takeAt(0);
        if (auto* widget = item->widget()) {
            if (widget == status_label_) {
                status_label_->hide();
            } else {
                widget->deleteLater();
            }
        }
        delete item;
    }
}

void EconomicCalendarWidget::show_status(const QString& text) {
    clear_list();
    status_label_->setText(text);
    status_label_->setVisible(true);
    list_layout_->addWidget(status_label_);
    list_layout_->addStretch();
}

void EconomicCalendarWidget::showEvent(QShowEvent* e) {
    BaseWidget::showEvent(e);
    if (!hub_active_)
        hub_subscribe();
}

void EconomicCalendarWidget::hideEvent(QHideEvent* e) {
    BaseWidget::hideEvent(e);
    if (hub_active_)
        hub_unsubscribe();
}

void EconomicCalendarWidget::hub_subscribe() {
    auto& hub = datahub::DataHub::instance();
    hub.subscribe(this, QString::fromLatin1(kTopic), [this](const QVariant& v) {
        set_loading(false);
        QJsonArray events;
        if (v.canConvert<QJsonArray>())
            events = v.value<QJsonArray>();
        if (events.isEmpty()) {
            show_status(QCoreApplication::translate("EconomicCalendarWidget", "No events available"));
            return;
        }
        populate(events);
    });
    // Per-topic error subscription — fires only when *our* topic errors,
    // unlike the global topic_error signal which fans every error to every
    // listener. Keeps the widget out of the global error broadcast path.
    hub.subscribe_errors(this, QString::fromLatin1(kTopic),
        [this](const QString& /*error*/) {
            set_loading(false);
            show_status(QCoreApplication::translate("EconomicCalendarWidget", "Failed to load calendar"));
        });
    hub_active_ = true;
    // Cold-cache fallback: if the producer warmed the topic earlier, paint
    // it now even if it's slightly stale — beats a blank panel.
    QVariant cached = hub.peek_raw(QString::fromLatin1(kTopic));
    if (cached.canConvert<QJsonArray>()) {
        const auto events = cached.value<QJsonArray>();
        if (!events.isEmpty()) {
            set_loading(false);
            populate(events);
        }
    }
}

void EconomicCalendarWidget::hub_unsubscribe() {
    auto& hub = datahub::DataHub::instance();
    hub.unsubscribe(this, QString::fromLatin1(kTopic));
    hub.unsubscribe_errors(this, QString::fromLatin1(kTopic));
    hub_active_ = false;
}

void EconomicCalendarWidget::populate(const QJsonArray& events) {
    last_events_ = events;

    clear_list();

    bool alt = false;
    int count = 0;

    for (const auto& v : events) {
        if (count >= 25)
            break;
        auto e = v.toObject();

        // Real fields: event, country, date, time, importance, actual, forecast, previous
        QString event_name = e["event"].toString().trimmed();
        if (event_name.isEmpty())
            continue;

        QString country = e["country"].toString().toUpper();
        QString date = e["date"].toString();
        QString time_str = e["time"].toString().trimmed();
        QString actual = e["actual"].toString().trimmed();
        QString forecast = e["forecast"].toString().trimmed();
        int imp_int = e["importance"].toInt(0);

        // Date: show as MM/DD
        QString date_display = date;
        if (date.length() == 10) {
            QStringList parts = date.split('-');
            if (parts.size() == 3)
                date_display = QString("%1/%2").arg(parts[1]).arg(parts[2]);
        }
        if (!time_str.isEmpty())
            date_display += " " + time_str.left(5);

        // Importance color: 0=dim, 1=low/dim, 2=medium/amber, 3=high/red
        QString imp_color = imp_int >= 3   ? ui::colors::NEGATIVE()
                            : imp_int == 2 ? ui::colors::WARNING()
                                           : ui::colors::TEXT_TERTIARY();
        QString imp_text = imp_int >= 3 ? QCoreApplication::translate("EconomicCalendarWidget", "HIGH") : imp_int == 2 ? QCoreApplication::translate("EconomicCalendarWidget", "MED") : imp_int == 1 ? QCoreApplication::translate("EconomicCalendarWidget", "LOW") : QStringLiteral("--");

        auto* row = new QWidget(this);
        row->setStyleSheet(QString("background: %1;").arg(alt ? ui::colors::BG_RAISED() : "transparent"));
        auto* rl = new QHBoxLayout(row);
        rl->setContentsMargins(8, 4, 8, 4);

        // Event name — check cache, static map, or queue for online translation
        QString display_name;
        auto cached_it = translation_cache_.find(event_name);
        if (cached_it != translation_cache_.end()) {
            display_name = cached_it.value();
        } else {
            display_name = translate_event(event_name);
            if (display_name == event_name && !event_name.isEmpty()) {
                translation_queue_.append(event_name);
                if (!translation_in_progress_ && translation_timer_)
                    translation_timer_->start();
            }
        }
        if (display_name.length() > 28)
            display_name = display_name.left(26) + QStringLiteral("\342\200\246");
        auto* ev_lbl = new QLabel(display_name);
        ev_lbl->setToolTip(display_name);
        ev_lbl->setStyleSheet(
            QString("color: %1; font-size: 10px; background: transparent;").arg(ui::colors::TEXT_PRIMARY()));
        rl->addWidget(ev_lbl, 4);

        auto* cty_lbl = new QLabel(country);
        cty_lbl->setStyleSheet(QString("color: %1; font-size: 9px; background: transparent;").arg(ui::colors::CYAN()));
        rl->addWidget(cty_lbl, 1);

        auto* date_lbl = new QLabel(date_display);
        date_lbl->setStyleSheet(
            QString("color: %1; font-size: 9px; background: transparent;").arg(ui::colors::TEXT_SECONDARY()));
        rl->addWidget(date_lbl, 2);

        auto* act_lbl = new QLabel(actual.isEmpty() ? "--" : actual);
        act_lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        act_lbl->setStyleSheet(QString("color: %1; font-size: 10px; font-weight: bold; background: transparent;")
                                   .arg(actual.isEmpty() ? ui::colors::TEXT_TERTIARY() : ui::colors::TEXT_PRIMARY()));
        rl->addWidget(act_lbl, 1);

        auto* fcst_lbl = new QLabel(forecast.isEmpty() ? "--" : forecast);
        fcst_lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        fcst_lbl->setStyleSheet(
            QString("color: %1; font-size: 9px; background: transparent;").arg(ui::colors::TEXT_SECONDARY()));
        rl->addWidget(fcst_lbl, 1);

        auto* imp_lbl = new QLabel(imp_text);
        imp_lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        imp_lbl->setStyleSheet(
            QString("color: %1; font-size: 9px; font-weight: bold; background: transparent;").arg(imp_color));
        rl->addWidget(imp_lbl, 1);

        list_layout_->addWidget(row);
        alt = !alt;
        ++count;
    }

    if (count == 0) {
        show_status(QCoreApplication::translate("EconomicCalendarWidget", "No events available"));
        return;
    }

    list_layout_->addStretch();
}

void EconomicCalendarWidget::request_online_translation(const QString& original) {
    translation_in_progress_ = true;

    QPointer<EconomicCalendarWidget> guard = this;
    services::NewsNlpService::instance().translate_text(
        original, "zh",
        [guard, original](bool ok, const QString& translated, const QString& /*detected_lang*/) {
            if (!guard)
                return;
            if (ok && !translated.isEmpty() && translated != original) {
                guard->on_translation_received(original, translated);
            } else if (!guard->translation_queue_.isEmpty()) {
                guard->translation_timer_->start();
            } else {
                guard->translation_in_progress_ = false;
            }
        });
}

void EconomicCalendarWidget::on_translation_received(const QString& original, const QString& translated) {
    translation_cache_[original] = translated;
    translation_queue_.removeAll(original);

    if (!translation_queue_.isEmpty()) {
        translation_timer_->start();
    } else {
        translation_in_progress_ = false;
    }
    refresh_translated_labels();
}

void EconomicCalendarWidget::refresh_translated_labels() {
    if (last_events_.isEmpty())
        return;
    populate(last_events_);
}

} // namespace fincept::screens::widgets
