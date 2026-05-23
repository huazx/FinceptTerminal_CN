#include "services/economics/MacroCalendarService.h"

#include "core/logging/Logger.h"
#include "datahub/DataHub.h"
#include "datahub/TopicPolicy.h"
#include "network/http/HttpClient.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QPointer>

namespace fincept::services {

namespace {
constexpr const char* kTopic = "econ:fincept:upcoming_events";
constexpr const char* kUrl = "https://api.fincept.in/macro/upcoming-events?limit=25";
constexpr const char* kFallbackUrl = "https://nfs.faireconomy.media/ff_calendar_thisweek.json";

QJsonArray parse_events(const QJsonDocument& doc) {
    if (doc.isArray())
        return doc.array();
    if (!doc.isObject())
        return {};

    const auto root = doc.object();
    if (root.contains(QStringLiteral("data")) && root.value(QStringLiteral("data")).isObject()) {
        const auto data = root.value(QStringLiteral("data")).toObject();
        if (data.contains(QStringLiteral("events")) && data.value(QStringLiteral("events")).isArray())
            return data.value(QStringLiteral("events")).toArray();
    }
    if (root.contains(QStringLiteral("data")) && root.value(QStringLiteral("data")).isArray())
        return root.value(QStringLiteral("data")).toArray();
    if (root.contains(QStringLiteral("events")) && root.value(QStringLiteral("events")).isArray())
        return root.value(QStringLiteral("events")).toArray();
    return {};
}

QJsonArray normalize_ff_events(const QJsonArray& raw) {
    static const QHash<QString, QString> kEventCn = {
        {"1-y Loan Prime Rate", "1年期贷款市场报价利率"},
        {"5-y Loan Prime Rate", "5年期贷款市场报价利率"},
        {"10-y Bond Auction", "10年期国债拍卖"},
        {"ADP Weekly Employment Change", "ADP周度就业变动"},
        {"API Weekly Statistical Bulletin", "API周度统计公报"},
        {"Average Earnings Index 3m/y", "平均薪资指数3个月/年"},
        {"BOE Gov Bailey Speaks", "英央行行长贝利讲话"},
        {"Bank Holiday", "银行假日"},
        {"Belgian NBB Business Climate", "比利时NBB商业景气指数"},
        {"Building Permits", "建筑许可"},
        {"Building Permits m/m", "建筑许可环比"},
        {"BusinessNZ Services Index", "新西兰服务业指数"},
        {"CB Leading Index m/m", "世企联领先指标环比"},
        {"CBI Industrial Order Expectations", "CBI工业订单预期"},
        {"CPI m/m", "CPI环比"},
        {"CPI y/y", "CPI同比"},
        {"Claimant Count Change", "申请失业金人数变动"},
        {"Common CPI y/y", "调和CPI同比"},
        {"Consumer Confidence", "消费者信心"},
        {"Core CPI m/m", "核心CPI环比"},
        {"Core CPI y/y", "核心CPI同比"},
        {"Core Machinery Orders m/m", "核心机械订单环比"},
        {"Core Retail Sales m/m", "核心零售销售环比"},
        {"Core Retail Sales q/q", "核心零售销售季环比"},
        {"Credit Card Spending y/y", "信用卡消费同比"},
        {"Crude Oil Inventories", "原油库存"},
        {"Current Account", "经常账户"},
        {"ECOFIN Meetings", "欧盟财长会议"},
        {"EU Economic Forecasts", "欧盟经济预测"},
        {"Employment Change", "就业变动"},
        {"Eurogroup Meetings", "欧元集团会议"},
        {"FOMC Meeting Minutes", "FOMC会议纪要"},
        {"Final CPI y/y", "终值CPI同比"},
        {"Final Core CPI y/y", "终值核心CPI同比"},
        {"Fixed Asset Investment ytd/y", "固定资产投资累计同比"},
        {"Flash Manufacturing PMI", "制造业PMI初值"},
        {"Flash Services PMI", "服务业PMI初值"},
        {"Gasoline Inventories", "汽油库存"},
        {"GDP m/m", "GDP环比"},
        {"GDP q/q", "GDP季环比"},
        {"GDP y/y", "GDP同比"},
        {"German Buba Monthly Report", "德国央行月报"},
        {"German CPI m/m", "德国CPI环比"},
        {"German CPI y/y", "德国CPI同比"},
        {"German Final CPI m/m", "德国终值CPI环比"},
        {"German Final CPI y/y", "德国终值CPI同比"},
        {"German Import Prices m/m", "德国进口价格环比"},
        {"German PPI m/m", "德国PPI环比"},
        {"German ZEW Economic Sentiment", "德国ZEW经济景气指数"},
        {"HPI m/m", "房价指数环比"},
        {"Industrial Production m/m", "工业产出环比"},
        {"Industrial Production y/y", "工业产出同比"},
        {"ISM Manufacturing PMI", "ISM制造业PMI"},
        {"ISM Non-Manufacturing PMI", "ISM非制造业PMI"},
        {"ISM Services PMI", "ISM服务业PMI"},
        {"Jobless Claims", "初请失业金人数"},
        {"JOLTS Job Openings", "JOLTS职位空缺"},
        {"Manufacturing PMI", "制造业PMI"},
        {"Manufacturing Production m/m", "制造业产出环比"},
        {"Michigan Consumer Sentiment", "密歇根消费者信心"},
        {"Natural Gas Storage", "天然气库存"},
        {"New Home Sales", "新屋销售"},
        {"Non-Farm Employment Change", "非农就业变动"},
        {"NZ Business NZ PMI", "新西兰商业PMI"},
        {"PPI m/m", "PPI环比"},
        {"PPI y/y", "PPI同比"},
        {"PPI Input m/m", "PPI投入环比"},
        {"Retail Sales m/m", "零售销售环比"},
        {"Retail Sales y/y", "零售销售同比"},
        {"Services PMI", "服务业PMI"},
        {"Trade Balance", "贸易差额"},
        {"Unemployment Claims", "失业金申请"},
        {"Unemployment Rate", "失业率"},
        {"Wage Price Index q/q", "工资价格指数季环比"},
        {"Wholesale Inventories m/m", "批发库存环比"},
    };
    static const QHash<QString, QString> kCountryCn = {
        {"USD", "美国"}, {"EUR", "欧元区"}, {"GBP", "英国"}, {"JPY", "日本"},
        {"CNY", "中国"}, {"AUD", "澳大利亚"}, {"CAD", "加拿大"}, {"CHF", "瑞士"},
        {"NZD", "新西兰"}, {"CNY", "中国"}, {"KRW", "韩国"}, {"SGD", "新加坡"},
        {"HKD", "香港"}, {"TWD", "台湾"}, {"INR", "印度"}, {"RUB", "俄罗斯"},
        {"BRL", "巴西"}, {"MXN", "墨西哥"}, {"ZAR", "南非"}, {"TRY", "土耳其"},
        {"SEK", "瑞典"}, {"NOK", "挪威"}, {"DKK", "丹麦"}, {"PLN", "波兰"},
        {"THB", "泰国"}, {"IDR", "印尼"}, {"MYR", "马来西亚"}, {"PHP", "菲律宾"},
    };

    QJsonArray out;
    for (const auto& v : raw) {
        auto obj = v.toObject();
        QJsonObject ev;
        const QString title = obj.value("title").toString();
        const QString cn_event = kEventCn.value(title, title);
        ev["event"] = cn_event;
        const QString country = obj.value("country").toString().toUpper();
        ev["country"] = kCountryCn.value(country, country);
        ev["date"] = obj.value("date").toString().left(10);
        ev["time"] = obj.value("date").toString().mid(11, 5);
        ev["actual"] = obj.value("actual").toString();
        ev["forecast"] = obj.value("forecast").toString();
        ev["previous"] = obj.value("previous").toString();
        const QString impact = obj.value("impact").toString().toLower();
        int imp = 0;
        if (impact == "high")
            imp = 3;
        else if (impact == "medium")
            imp = 2;
        else if (impact == "low")
            imp = 1;
        ev["importance"] = imp;
        out.append(ev);
    }
    return out;
}

void publish_events(const QJsonArray& events) {
    auto& hub = fincept::datahub::DataHub::instance();
    if (events.isEmpty()) {
        hub.publish_error(QString::fromLatin1(kTopic),
                          QStringLiteral("No economic events available"));
        return;
    }
    hub.publish(QString::fromLatin1(kTopic), QVariant::fromValue(events));
}

void fetch_fallback(QPointer<MacroCalendarService> self) {
    if (!self)
        return;
    fincept::HttpClient::instance().get(QString::fromLatin1(kFallbackUrl),
        [self](fincept::Result<QJsonDocument> result) {
            if (!self)
                return;
            if (!result.is_ok()) {
                LOG_WARN("MacroCalendarService",
                         "Fallback ForexFactory fetch failed: " + QString::fromStdString(result.error()));
                publish_events({});
                return;
            }
            const QJsonArray raw = parse_events(result.value());
            const QJsonArray events = normalize_ff_events(raw);
            LOG_INFO("MacroCalendarService",
                     QString("Loaded %1 events from ForexFactory fallback").arg(events.size()));
            publish_events(events);
        });
}

} // namespace

MacroCalendarService& MacroCalendarService::instance() {
    static MacroCalendarService s;
    return s;
}

MacroCalendarService::MacroCalendarService(QObject* parent) : QObject(parent) {}

void MacroCalendarService::ensure_registered_with_hub() {
    if (hub_registered_)
        return;
    auto& hub = fincept::datahub::DataHub::instance();
    hub.register_producer(this);

    fincept::datahub::TopicPolicy policy;
    policy.ttl_ms = 5 * 60 * 1000;
    policy.min_interval_ms = 60 * 1000;
    policy.refresh_timeout_ms = 30 * 1000;
    hub.set_policy(QString::fromLatin1(kTopic), policy);

    hub_registered_ = true;
    LOG_INFO("MacroCalendarService", "Registered with DataHub (econ:fincept:upcoming_events)");
}

QStringList MacroCalendarService::topic_patterns() const {
    return {QString::fromLatin1(kTopic)};
}

void MacroCalendarService::refresh(const QStringList& topics) {
    if (!topics.contains(QString::fromLatin1(kTopic)))
        return;

    QPointer<MacroCalendarService> self = this;
    fincept::HttpClient::instance().get(QString::fromLatin1(kUrl),
        [self](fincept::Result<QJsonDocument> result) {
            if (!self)
                return;
            if (!result.is_ok()) {
                LOG_WARN("MacroCalendarService",
                         "Fincept API failed, trying ForexFactory fallback: "
                             + QString::fromStdString(result.error()));
                fetch_fallback(self);
                return;
            }
            const QJsonArray events = parse_events(result.value());
            if (events.isEmpty()) {
                LOG_INFO("MacroCalendarService", "Fincept API returned empty, trying ForexFactory fallback");
                fetch_fallback(self);
                return;
            }
            LOG_INFO("MacroCalendarService",
                     QString("Loaded %1 events from Fincept API").arg(events.size()));
            publish_events(events);
        });
}

} // namespace fincept::services
