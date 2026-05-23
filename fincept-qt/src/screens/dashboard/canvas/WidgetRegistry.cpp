#include "screens/dashboard/canvas/WidgetRegistry.h"

#include "screens/dashboard/widgets/AgentErrorsWidget.h"
#include "screens/dashboard/widgets/BrokerHoldingsWidget.h"
#include "screens/dashboard/widgets/CommoditiesWidget.h"
#include "screens/dashboard/widgets/CryptoTickerWidget.h"
#include "screens/dashboard/widgets/CryptoWidget.h"
#include "screens/dashboard/widgets/EconomicCalendarWidget.h"
#include "screens/dashboard/widgets/ForexWidget.h"
#include "screens/dashboard/widgets/GeopoliticsEventsWidget.h"
#include "screens/dashboard/widgets/IndicesWidget.h"
#include "screens/dashboard/widgets/MarginUsageWidget.h"
#include "screens/dashboard/widgets/MaritimeVesselsWidget.h"
#include "screens/dashboard/widgets/MarketQuoteStripWidget.h"
#include "screens/dashboard/widgets/MarketSentimentWidget.h"
#include "screens/dashboard/widgets/NewsCategoryWidget.h"
#include "screens/dashboard/widgets/NewsWidget.h"
#include "screens/dashboard/widgets/NotesWidget.h"
#include "screens/dashboard/widgets/OpenPositionsWidget.h"
#include "screens/dashboard/widgets/OrderBookMiniWidget.h"
#include "screens/dashboard/widgets/PerformanceWidget.h"
#include "screens/dashboard/widgets/PolymarketPriceWidget.h"
#include "screens/dashboard/widgets/PortfolioSummaryWidget.h"
#include "screens/dashboard/widgets/QuickTradeWidget.h"
#include "screens/dashboard/widgets/RecentFilesWidget.h"
#include "screens/dashboard/widgets/RiskMetricsWidget.h"
#include "screens/dashboard/widgets/ScreenerWidget.h"
#include "screens/dashboard/widgets/SectorHeatmapWidget.h"
#include "screens/dashboard/widgets/SparklineStripWidget.h"
#include "screens/dashboard/widgets/StockQuoteWidget.h"
#include "screens/dashboard/widgets/TodayPnLWidget.h"
#include "screens/dashboard/widgets/TopMoversWidget.h"
#include "screens/dashboard/widgets/TradeTapeWidget.h"
#include "screens/dashboard/widgets/VideoPlayerWidget.h"
#include "screens/dashboard/widgets/WatchlistWidget.h"
#include "screens/dashboard/widgets/WebScraperWidget.h"

#include <QCoreApplication>

namespace fincept::screens {

namespace {
#define TR(x) QCoreApplication::translate("WidgetRegistry", x)
}

WidgetRegistry& WidgetRegistry::instance() {
    static WidgetRegistry inst;
    return inst;
}

WidgetRegistry::WidgetRegistry() {
    // 12-column grid: default_w, default_h, min_w, min_h
    // Factory receives per-instance config (empty QJsonObject for fresh widgets).
    // Existing widgets ignore it until they opt into configurable behaviour.

    // ── Markets ───────────────────────────────────────────────────────────────
    register_widget({"indices", TR("全球指数"), TR("市场"), TR("全球主要指数 — SPY, QQQ, DIA, IWM"), 4, 5, 3, 4,
                     [](const QJsonObject&) { return widgets::create_indices_widget(); }});

    register_widget({"forex", TR("外汇"), TR("市场"), TR("主要货币对 — EURUSD, GBPUSD, USDJPY"), 4, 4, 2, 3,
                     [](const QJsonObject&) { return widgets::create_forex_widget(); }});

    register_widget({"crypto", TR("加密货币"), TR("市场"), TR("主流加密货币 — BTC, ETH, BNB, SOL"), 4, 4, 2, 3,
                     [](const QJsonObject&) { return widgets::create_crypto_widget(); }});

    register_widget({"commodities", TR("大宗商品"), TR("市场"), TR("黄金、原油、天然气、铜"), 4, 4, 2, 3,
                     [](const QJsonObject&) { return widgets::create_commodities_widget(); }});

    register_widget({"sector_heatmap", TR("行业热力图"), TR("市场"), TR("标普500行业表现热力图"), 6, 5, 3, 4,
                     [](const QJsonObject&) { return new widgets::SectorHeatmapWidget; }});

    register_widget({"top_movers", TR("涨跌排行"), TR("市场"), TR("今日最大涨跌幅个股"), 6, 5, 3, 4,
                     [](const QJsonObject&) { return new widgets::TopMoversWidget; }});

    register_widget({"sentiment", TR("市场情绪"), TR("市场"), TR("恐惧与贪婪指数、多空指标"), 4, 4, 2, 3,
                     [](const QJsonObject&) { return new widgets::MarketSentimentWidget; }});

    // ── Research ──────────────────────────────────────────────────────────────
    register_widget({"news", TR("新闻资讯"), TR("研究"), TR("最新财经新闻标题"), 8, 4, 3, 3,
                     [](const QJsonObject&) { return new widgets::NewsWidget; }});

    register_widget({"stock_quote", TR("个股行情"), TR("研究"), TR("单只股票详情 — 价格、成交量、图表"), 4, 5, 2, 3,
                     [](const QJsonObject& cfg) {
                         const QString sym = cfg.value("symbol").toString("AAPL");
                         return new widgets::StockQuoteWidget(sym);
                     }});

    register_widget({"screener", TR("选股器"), TR("研究"), TR("按基本面和技术面筛选股票"), 6, 5, 3,
                     4, [](const QJsonObject&) { return new widgets::ScreenerWidget; }});

    register_widget({"econ_calendar", TR("经济日历"), TR("研究"), TR("即将发布的宏观经济数据和事件"), 4, 4, 2, 3,
                     [](const QJsonObject&) { return new widgets::EconomicCalendarWidget; }});

    // ── Portfolio ─────────────────────────────────────────────────────────────
    register_widget({"watchlist", TR("自选股"), TR("投资组合"), TR("已保存的股票列表及实时行情"), 6, 4, 2, 3,
                     [](const QJsonObject&) { return new widgets::WatchlistWidget; }});

    register_widget({"performance", TR("表现追踪"), TR("投资组合"), TR("投资组合盈亏 — 今日、本周、本月、年初至今"), 4, 5, 3, 4,
                     [](const QJsonObject&) { return new widgets::PerformanceWidget; }});

    register_widget({"portfolio_summary", TR("投资组合概览"), TR("投资组合"),
                     TR("持仓概览及资产配置明细"), 6, 4, 2, 3,
                     [](const QJsonObject&) { return new widgets::PortfolioSummaryWidget; }});

    register_widget({"risk_metrics", TR("风险指标"), TR("投资组合"), TR("波动率、Beta、最大回撤、夏普比率"), 4, 5, 3,
                     4, [](const QJsonObject&) { return new widgets::RiskMetricsWidget; }});

    // ── Trading ───────────────────────────────────────────────────────────────
    register_widget({"quick_trade", TR("快捷交易"), TR("交易"), TR("快速下单 — 加密货币和股票"), 4, 5, 2, 3,
                     [](const QJsonObject&) { return new widgets::QuickTradeWidget; }});

    register_widget({"open_positions", TR("持仓明细"), TR("交易"),
                     TR("券商账户实时持仓 — 点击齿轮图标选择账户"), 6, 5, 3, 3,
                     [](const QJsonObject& cfg) { return new widgets::OpenPositionsWidget(cfg); }});

    register_widget({"working_orders", TR("委托订单"), TR("交易"),
                     TR("券商账户待成交订单 — 点击×取消订单"), 6, 5, 3, 3,
                     [](const QJsonObject& cfg) { return new widgets::OrderBookMiniWidget(cfg); }});

    register_widget({"margin_usage", TR("保证金使用"), TR("交易"),
                     TR("券商账户资金 — 可用、已用保证金、总额、使用率"), 3, 4, 2, 3,
                     [](const QJsonObject& cfg) { return new widgets::MarginUsageWidget(cfg); }});

    register_widget({"today_pnl", TR("今日盈亏"), TR("交易"),
                     TR("券商账户汇总盈亏 — 总计、当日、已实现、持仓"), 3, 4, 2, 3,
                     [](const QJsonObject& cfg) { return new widgets::TodayPnLWidget(cfg); }});

    register_widget({"holdings", TR("长期持仓"), TR("投资组合"),
                     TR("券商长期持仓 — 成本价、现价、盈亏%"), 6, 5, 3, 3,
                     [](const QJsonObject& cfg) { return new widgets::BrokerHoldingsWidget(cfg); }});

    // ── Tools ────────────────────────────────────────────────────────────────
    register_widget({"video_player", TR("直播/视频"), TR("工具"),
                     TR("财经电视 — 主流财经频道及自定义直播源"), 4, 5, 3, 4,
                     [](const QJsonObject&) { return widgets::create_video_player_widget(); }});

    register_widget({"recent_files", TR("最近文件"), TR("工具"),
                     TR("最近保存的文件 — 导出、报告、笔记本等"), 4, 4, 2, 3,
                     [](const QJsonObject&) { return new widgets::RecentFilesWidget; }});

    register_widget({"quote_strip", TR("行情条"), TR("市场"),
                     TR("可配置的实时行情列表 — 点击齿轮图标选择标的"), 3, 5, 2, 3,
                     [](const QJsonObject& cfg) { return new widgets::MarketQuoteStripWidget(cfg); }});

    register_widget({"crypto_ticker", TR("加密行情条"), TR("市场"),
                     TR("Kraken/HyperLiquid实时行情条 — 可配置交易对列表"), 3, 5, 2, 3,
                     [](const QJsonObject& cfg) { return new widgets::CryptoTickerWidget(cfg); }});

    register_widget({"polymarket_prices", TR("预测市场"), TR("市场"),
                     TR("实时预测市场价格 — 可配置资产列表"), 3, 5, 2, 3,
                     [](const QJsonObject& cfg) { return new widgets::PolymarketPriceWidget(cfg); }});

    register_widget({"agent_errors", TR("智能体错误"), TR("工具"),
                     TR("最近的智能体执行失败记录 — 订阅 agent:error:*"), 5, 4, 3, 3,
                     [](const QJsonObject& cfg) { return new widgets::AgentErrorsWidget(cfg); }});

    register_widget({"sparklines", TR("迷你走势图"), TR("市场"),
                     TR("可配置的迷你走势图 — 订阅 market:sparkline:*"), 4, 5, 3, 3,
                     [](const QJsonObject& cfg) { return new widgets::SparklineStripWidget(cfg); }});

    register_widget({"trade_tape", TR("成交明细"), TR("市场"),
                     TR("加密货币实时成交记录 — ws:<exchange>:trades:<pair>"), 4, 5, 3, 4,
                     [](const QJsonObject& cfg) { return new widgets::TradeTapeWidget(cfg); }});

    register_widget({"news_category", TR("分类新闻"), TR("研究"),
                     TR("按类别筛选的新闻标题 — news:category:<category>"), 5, 5, 3, 3,
                     [](const QJsonObject& cfg) { return new widgets::NewsCategoryWidget(cfg); }});

    register_widget({"web_scraper", TR("网页抓取"), TR("工具"),
                     TR("从任意URL抓取表格 — 自动识别HTML、JSON、CSV、XML/RSS"), 6, 5, 3,
                     3, [](const QJsonObject& cfg) { return new widgets::WebScraperWidget(cfg); }});

    // ── Geopolitics ──────────────────────────────────────────────────────────
    register_widget({"geopolitics_events", TR("地缘政治事件"), TR("地缘政治"),
                     TR("实时冲突/政治事件 — 订阅 geopolitics:events"), 6, 5, 3, 3,
                     [](const QJsonObject& cfg) { return new widgets::GeopoliticsEventsWidget(cfg); }});

    register_widget({"maritime_vessels", TR("海事船舶"), TR("地缘政治"),
                     TR("实时船舶位置 — 可配置IMO列表，订阅 maritime:vessel:*"), 5, 5, 3, 3,
                     [](const QJsonObject& cfg) { return new widgets::MaritimeVesselsWidget(cfg); }});

    register_widget({"notes", TR("笔记"), TR("工具"),
                     TR("最近/收藏的财经笔记 — 点击打开笔记界面"), 4, 5, 2, 3,
                     [](const QJsonObject& cfg) { return new widgets::NotesWidget(cfg); }});
}

namespace {
#undef TR
}

void WidgetRegistry::register_widget(WidgetMeta meta) {
    registry_.insert(meta.type_id, std::move(meta));
}

const WidgetMeta* WidgetRegistry::find(const QString& type_id) const {
    auto it = registry_.find(type_id);
    return (it != registry_.end()) ? &it.value() : nullptr;
}

QVector<WidgetMeta> WidgetRegistry::all() const {
    QVector<WidgetMeta> result;
    result.reserve(registry_.size());
    for (auto it = registry_.cbegin(); it != registry_.cend(); ++it)
        result.append(it.value());
    return result;
}

QVector<WidgetMeta> WidgetRegistry::by_category(const QString& category) const {
    QVector<WidgetMeta> result;
    for (const auto& m : registry_) {
        if (category.isEmpty() || m.category == category)
            result.append(m);
    }
    return result;
}

} // namespace fincept::screens
