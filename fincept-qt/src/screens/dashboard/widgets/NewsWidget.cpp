#include "screens/dashboard/widgets/NewsWidget.h"

#include "datahub/DataHub.h"
#include "datahub/DataHubMetaTypes.h"
#include "services/news/NewsNlpService.h"
#include "ui/theme/Theme.h"
#include "ui/widgets/CustomTooltip.h"
#include <QCoreApplication>

#include <QDateTime>
#include <QLabel>
#include <QHoverEvent>
#include <QRegularExpression>

namespace fincept::screens::widgets {

namespace {
constexpr const char* kTopic = "news:general";
constexpr int kMaxArticles = 30;
constexpr int kTranslationDelayMs = 300;

QString translate_headline_fallback(const QString& headline) {
    static const QVector<QPair<QString, QString>> kPatterns = {
        {"\\binterest rates?\\b", "利率"},
        {"\\bcentral bank\\b", "央行"},
        {"\\bFederal Reserve\\b", "美联储"},
        {"\\bFed\\b", "美联储"},
        {"\\bECB\\b", "欧洲央行"},
        {"\\bBOE\\b", "英国央行"},
        {"\\bBOJ\\b", "日本央行"},
        {"\\bPBOC\\b", "中国央行"},
        {"\\bRBI\\b", "印度央行"},
        {"\\bRBA\\b", "澳洲央行"},
        {"\\bBank of Japan\\b", "日本央行"},
        {"\\bBank of England\\b", "英国央行"},
        {"\\binflation\\b", "通胀"},
        {"\\bdeflation\\b", "通缩"},
        {"\\brecession\\b", "衰退"},
        {"\\bdepression\\b", "萧条"},
        {"\\bunemployment\\b", "失业"},
        {"\\bjobless\\b", "失业"},
        {"\\bemployment\\b", "就业"},
        {"\\bstock market\\b", "股市"},
        {"\\bstocks?\\b", "股票"},
        {"\\bbonds?\\b", "债券"},
        {"\\byields?\\b", "收益率"},
        {"\\btreasury\\b", "国债"},
        {"\\bTreasuries\\b", "国债"},
        {"\\bcrude oil\\b", "原油"},
        {"\\boil prices?\\b", "油价"},
        {"\\bgold\\b", "黄金"},
        {"\\bsilver\\b", "白银"},
        {"\\bcopper\\b", "铜"},
        {"\\bnatural gas\\b", "天然气"},
        {"\\bcommodit\\w+\\b", "大宗商品"},
        {"\\bforex\\b", "外汇"},
        {"\\bcurrenc\\w+\\b", "货币"},
        {"\\bdollar\\b", "美元"},
        {"\\beuro\\b", "欧元"},
        {"\\byen\\b", "日元"},
        {"\\byuan\\b", "人民币"},
        {"\\bpound\\b", "英镑"},
        {"\\bfranc\\b", "瑞郎"},
        {"\\btrade war\\b", "贸易战"},
        {"\\btariffs?\\b", "关税"},
        {"\\bsanctions\\b", "制裁"},
        {"\\bretail sales\\b", "零售销售"},
        {"\\bconsumer confidence\\b", "消费者信心"},
        {"\\bhousing market\\b", "房地产市场"},
        {"\\bmanufacturing\\b", "制造业"},
        {"\\bjobless claims\\b", "初请失业金"},
        {"\\bnon-?farm\\b", "非农"},
        {"\\brate hike\\b", "加息"},
        {"\\brate cut\\b", "降息"},
        {"\\brate decision\\b", "利率决议"},
        {"\\bmonetary policy\\b", "货币政策"},
        {"\\bfiscal policy\\b", "财政政策"},
        {"\\bquantitative easing\\b", "量化宽松"},
        {"\\bQE\\b", "量化宽松"},
        {"\\bstimulus\\b", "刺激措施"},
        {"\\bdefault\\b", "违约"},
        {"\\bdebt ceiling\\b", "债务上限"},
        {"\\bdebt crisis\\b", "债务危机"},
        {"\\bmerger\\b", "并购"},
        {"\\bacquisition\\b", "收购"},
        {"\\bprofit\\b", "利润"},
        {"\\brevenue\\b", "营收"},
        {"\\bearnings\\b", "财报"},
        {"\\bdividend\\b", "股息"},
        {"\\bbuyback\\b", "回购"},
        {"\\bbull market\\b", "牛市"},
        {"\\bbear market\\b", "熊市"},
        {"\\brally\\b", "反弹"},
        {"\\bsell-?off\\b", "抛售"},
        {"\\bcorrection\\b", "回调"},
        {"\\bcrash\\b", "暴跌"},
        {"\\bsurge\\b", "飙升"},
        {"\\bplunge\\b", "跳水"},
        {"\\bdrops?\\b", "下跌"},
        {"\\bfalls?\\b", "下跌"},
        {"\\brises?\\b", "上涨"},
        {"\\bgains?\\b", "上涨"},
        {"\\bclimbs?\\b", "攀升"},
        {"\\bslumps?\\b", "暴跌"},
        {"\\bdips?\\b", "回落"},
        {"\\bsoars?\\b", "飙升"},
        {"\\btumbles?\\b", "大跌"},
        {"\\bslides?\\b", "下滑"},
        {"\\bjumps?\\b", "跳涨"},
        {"\\bsinks?\\b", "下挫"},
        {"\\bvolatil\\w+\\b", "波动"},
        {"\\bBitcoin\\b", "比特币"},
        {"\\bEthereum\\b", "以太坊"},
        {"\\bcryptocurrenc\\w+\\b", "加密货币"},
        {"\\bblockchain\\b", "区块链"},
        {"\\bartificial intelligence\\b", "人工智能"},
        {"\\bsemiconductor\\b", "半导体"},
        {"\\bS&P 500\\b", "标普500"},
        {"\\bNasdaq\\b", "纳斯达克"},
        {"\\bDow Jones\\b", "道琼斯"},
        {"\\bNikkei\\b", "日经"},
        {"\\bFTSE\\b", "富时"},
        {"\\bHang Seng\\b", "恒生"},
        {"\\bWall Street\\b", "华尔街"},
        {"\\bWhite House\\b", "白宫"},
        {"\\bCongress\\b", "国会"},
        {"\\bSenate\\b", "参议院"},
        {"\\bUkraine\\b", "乌克兰"},
        {"\\bRussi\\w+\\b", "俄罗斯"},
        {"\\bChin\\w+\\b", "中国"},
        {"\\bJapan\\w*\\b", "日本"},
        {"\\bEurope\\w*\\b", "欧洲"},
        {"\\bGerman\\w*\\b", "德国"},
        {"\\bBrit\\w+\\b", "英国"},
        {"\\bSouth Korea\\b", "韩国"},
        {"\\bMiddle East\\b", "中东"},
        {"\\bIsrael\\b", "以色列"},
        {"\\bIran\\b", "伊朗"},
        {"\\bTaiwan\\b", "台湾"},
        {"\\bHong Kong\\b", "香港"},
        {"\\bNATO\\b", "北约"},
        {"\\bOPEC\\b", "OPEC"},
        {"\\bIMF\\b", "IMF"},
        {"\\bWorld Bank\\b", "世界银行"},
        {"\\bSEC\\b", "美国证交会"},
        {"\\bmarket cap\\b", "市值"},
        {"\\bvaluation\\b", "估值"},
        {"\\bportfolio\\b", "投资组合"},
        {"\\bhedge fund\\b", "对冲基金"},
        {"\\blayoffs?\\b", "裁员"},
        {"\\bjob cuts\\b", "裁员"},
        {"\\bsupply chain\\b", "供应链"},
        {"\\brecovery\\b", "复苏"},
        {"\\bbubble\\b", "泡沫"},
        {"\\breal estate\\b", "房地产"},
        {"\\bmortgage\\b", "抵押贷款"},
        {"\\bbankruptcy\\b", "破产"},
        {"\\bregulation\\b", "监管"},
        {"\\bantitrust\\b", "反垄断"},
        {"\\bclimate change\\b", "气候变化"},
        {"\\brenewable\\b", "可再生能源"},
        {"\\belectric vehicle\\w*\\b", "电动汽车"},
        {"\\bcyber\\w*\\b", "网络"},
        {"\\bdata breach\\b", "数据泄露"},
        {"\\bbreaking\\b", "突发"},
        {"\\bexclusive\\b", "独家"},
        {"\\boutlook\\b", "展望"},
        {"\\bforecast\\b", "预测"},
        {"\\bupgrade\\b", "上调"},
        {"\\bdowngrade\\b", "下调"},
        {"\\bbullish\\b", "看涨"},
        {"\\bbearish\\b", "看跌"},
        {"\\bwarning\\b", "警告"},
        {"\\brisks?\\b", "风险"},
        {"\\bcrisis\\b", "危机"},
        {"\\bgrowth\\b", "增长"},
        {"\\bslowdown\\b", "放缓"},
        {"\\bsoft landing\\b", "软着陆"},
        {"\\bhard landing\\b", "硬着陆"},
        {"\\bquarter\\b", "季度"},
        {"\\bfiscal year\\b", "财年"},
    };

    QString result = headline;
    for (const auto& p : kPatterns) {
        QRegularExpression re(p.first);
        result.replace(re, p.second);
    }
    return result;
}

QString translate_source(const QString& source) {
    static const QVector<QPair<QString, QString>> kSourceCn = {
        {"AP", "美联社"},     {"BLOOMBERG", "彭博"},     {"WSJ", "华尔街日报"},
        {"MARKETWATCH", "MarketWatch"}, {"CNBC", "CNBC"}, {"SEEKING ALPHA", "Seeking Alpha"},
        {"BBC", "BBC"},       {"AL JAZEERA", "半岛电视台"}, {"NYT", "纽约时报"},
        {"GUARDIAN", "卫报"}, {"FRANCE 24", "法国24"},   {"FOREIGN POLICY", "外交政策"},
        {"OILPRICE", "油价网"}, {"TECHCRUNCH", "TechCrunch"}, {"WIRED", "连线"},
        {"FXSTREET", "FXStreet"}, {"SCMP", "南华早报"}, {"NIKKEI ASIA", "日经亚洲"},
        {"THE HINDU", "印度教徒报"}, {"MIDDLE EAST EYE", "中东之眼"},
        {"INVESTING.COM", "Investing.com"}, {"ECONOMIST", "经济学人"},
        {"COINDESK", "CoinDesk"}, {"COINTELEGRAPH", "CoinTelegraph"},
        {"THE BLOCK", "The Block"}, {"DECRYPT", "Decrypt"},
        {"ECB", "欧洲央行"},   {"BOE", "英国央行"},     {"FEDERAL RESERVE", "美联储"},
        {"SEC", "美国证交会"}, {"BENZINGA", "Benzinga"}, {"DW", "德国之声"},
        {"LIVEMINT", "LiveMint"}, {"ECONOMIC TIMES", "经济时报"},
        {"MONEYCONTROL", "MoneyControl"}, {"CNA", "亚洲新闻台"},
        {"FINEXTRA", "Finextra"}, {"ZEROHEDGE", "零对冲"},
        {"CALCULATED RISK", "Calculated Risk"}, {"WOLF STREET", "Wolf Street"},
        {"BELLINGCAT", "Bellingcat"}, {"ARS TECHNICA", "Ars Technica"},
        {"THE VERGE", "The Verge"}, {"MIT TECH REVIEW", "MIT科技评论"},
        {"CARBON BRIEF", "Carbon Brief"}, {"HACKER NEWS", "黑客新闻"},
        {"ABNORMAL RETURNS", "Abnormal Returns"}, {"MARGINAL REVOLUTION", "边际革命"},
        {"MINING.COM", "Mining.com"}, {"UN", "联合国"},
    };
    for (const auto& p : kSourceCn) {
        if (source.compare(p.first, Qt::CaseInsensitive) == 0)
            return p.second;
    }
    return source;
}

bool is_mostly_chinese(const QString& text) {
    int cn = 0;
    int total = 0;
    for (const QChar& ch : text) {
        if (ch.script() == QChar::Script_Han) {
            ++cn;
            ++total;
        } else if (ch.isLetter()) {
            ++total;
        }
    }
    return total > 0 && cn * 100 / total > 40;
}

} // namespace

NewsWidget::NewsWidget(QWidget* parent) : BaseWidget(QCoreApplication::translate("NewsWidget", "MARKET NEWS"), parent, ui::colors::CYAN) {
    scroll_area_ = new QScrollArea;
    scroll_area_->setWidgetResizable(true);

    auto* container = new QWidget(this);
    news_layout_ = new QVBoxLayout(container);
    news_layout_->setContentsMargins(4, 4, 4, 4);
    news_layout_->setSpacing(0);
    news_layout_->addStretch();

    scroll_area_->setWidget(container);
    content_layout()->addWidget(scroll_area_);

    translation_timer_ = new QTimer(this);
    translation_timer_->setSingleShot(true);
    connect(translation_timer_, &QTimer::timeout, this, [this]() {
        if (!translation_queue_.isEmpty()) {
            QString next = translation_queue_.takeFirst();
            request_online_translation(next);
        } else {
            translation_in_progress_ = false;
        }
    });

    connect(this, &BaseWidget::refresh_requested, this, []() {
        datahub::DataHub::instance().request(QString::fromLatin1(kTopic), /*force=*/true);
    });

    apply_styles();
    set_loading(true);
}

void NewsWidget::apply_styles() {
    scroll_area_->setStyleSheet(QString("QScrollArea { border: none; background: transparent; }"
                                        "QScrollBar:vertical { width: 6px; background: transparent; }"
                                        "QScrollBar::handle:vertical { background: %1; border-radius: 3px; }"
                                        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }")
                                    .arg(ui::colors::BORDER_MED()));
}

void NewsWidget::on_theme_changed() {
    apply_styles();
    if (!last_articles_.isEmpty())
        populate(last_articles_);
}

void NewsWidget::showEvent(QShowEvent* e) {
    BaseWidget::showEvent(e);
    if (!hub_active_)
        hub_subscribe();
}

void NewsWidget::hideEvent(QHideEvent* e) {
    BaseWidget::hideEvent(e);
    if (hub_active_)
        hub_unsubscribe();
}

void NewsWidget::hub_subscribe() {
    auto& hub = datahub::DataHub::instance();
    hub.subscribe(this, QString::fromLatin1(kTopic), [this](const QVariant& v) {
        if (!v.canConvert<QVector<services::NewsArticle>>())
            return;
        set_loading(false);
        populate(v.value<QVector<services::NewsArticle>>());
    });
    hub_active_ = true;
    QVariant cached = hub.peek_raw(QString::fromLatin1(kTopic));
    if (cached.canConvert<QVector<services::NewsArticle>>()) {
        set_loading(false);
        populate(cached.value<QVector<services::NewsArticle>>());
    }
}

void NewsWidget::hub_unsubscribe() {
    datahub::DataHub::instance().unsubscribe(this);
    hub_active_ = false;
}

void NewsWidget::request_online_translation(const QString& original) {
    translation_in_progress_ = true;

    auto* self = this;
    services::NewsNlpService::instance().translate_text(
        original, "zh",
        [self, original](bool ok, const QString& translated, const QString& /*detected_lang*/) {
            if (!self)
                return;
            if (ok && !translated.isEmpty() && translated != original && !is_mostly_chinese(original)) {
                self->on_translation_received(original, translated);
                return;
            }
            self->translation_in_progress_ = false;
        });
}

void NewsWidget::on_translation_received(const QString& original, const QString& translated) {
    translation_cache_[original] = translated;
    refresh_display();

    if (!translation_queue_.isEmpty()) {
        translation_timer_->start(kTranslationDelayMs);
    } else {
        translation_in_progress_ = false;
    }
}

void NewsWidget::refresh_display() {
    if (last_articles_.isEmpty())
        return;

    const int count = news_layout_->count();
    for (int i = 0; i < count - 1; ++i) {
        auto* item = news_layout_->itemAt(i);
        if (!item || !item->widget())
            continue;
        auto* row = item->widget();
        auto labels = row->findChildren<QLabel*>();
        for (auto* lbl : labels) {
            QString tooltip = lbl->property("tooltip_text").toString();
            if (tooltip.isEmpty())
                continue;
            auto it = translation_cache_.find(tooltip);
            if (it != translation_cache_.end() && lbl->text() != it.value()) {
                lbl->setText(it.value());
            }
        }
    }
}

void NewsWidget::populate(const QVector<services::NewsArticle>& articles) {
    last_articles_ = articles;

    while (news_layout_->count() > 1) {
        auto* item = news_layout_->takeAt(0);
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    if (articles.isEmpty()) {
        auto* empty = new QLabel(QStringLiteral("No news available."));
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet(QString("color: %1; font-size: 11px; padding: 12px; background: transparent;")
                                 .arg(ui::colors::TEXT_TERTIARY()));
        news_layout_->insertWidget(0, empty);
        return;
    }

    translation_queue_.clear();

    int rendered = 0;
    for (const auto& article : articles) {
        if (rendered >= kMaxArticles)
            break;
        if (article.headline.isEmpty())
            continue;

        QString time_str = article.time.left(5);
        if (time_str.isEmpty() && article.sort_ts > 0) {
            time_str = QDateTime::fromSecsSinceEpoch(article.sort_ts).toString(QStringLiteral("HH:mm"));
        }

        auto* row = new QWidget(this);
        row->setStyleSheet(QString("border-bottom: 1px solid %1;").arg(ui::colors::BORDER_DIM()));
        auto* rl = new QHBoxLayout(row);
        rl->setContentsMargins(4, 4, 4, 4);
        rl->setSpacing(8);

        if (!time_str.isEmpty()) {
            auto* time_lbl = new QLabel(time_str);
            time_lbl->setFixedWidth(36);
            time_lbl->setStyleSheet(
                QString("color: %1; font-size: 9px; background: transparent;").arg(ui::colors::CYAN()));
            rl->addWidget(time_lbl);
        }

        QString display_text;
        auto cached_it = translation_cache_.find(article.headline);
        if (cached_it != translation_cache_.end()) {
            display_text = cached_it.value();
        } else if (is_mostly_chinese(article.headline)) {
            display_text = article.headline;
        } else {
            display_text = translate_headline_fallback(article.headline);
            if (!is_mostly_chinese(display_text)) {
                translation_queue_.append(article.headline);
            }
        }

        auto* headline = new QLabel(display_text);
        headline->setWordWrap(true);
        headline->setProperty("tooltip_text", article.headline);
        headline->installEventFilter(this);
        headline->setMouseTracking(true);
        headline->setStyleSheet(
            QString("color: %1; font-size: 11px; background: transparent;").arg(ui::colors::TEXT_PRIMARY()));
        rl->addWidget(headline, 1);

        if (!article.source.isEmpty()) {
            auto* src = new QLabel(translate_source(article.source));
            src->setStyleSheet(
                QString("color: %1; font-size: 9px; background: transparent;").arg(ui::colors::TEXT_TERTIARY()));
            rl->addWidget(src);
        }

        news_layout_->insertWidget(news_layout_->count() - 1, row);
        ++rendered;
    }

    if (!translation_queue_.isEmpty() && !translation_in_progress_) {
        QString first = translation_queue_.takeFirst();
        request_online_translation(first);
    }
}

bool NewsWidget::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::Enter) {
        auto* lbl = qobject_cast<QLabel*>(watched);
        if (lbl) {
            QString tip = lbl->property("tooltip_text").toString();
            if (!tip.isEmpty()) {
                QPoint global_pos = lbl->mapToGlobal(QPoint(0, lbl->height()));
                show_custom_tooltip(tip, global_pos);
            }
        }
    } else if (event->type() == QEvent::Leave) {
        hide_custom_tooltip();
    } else if (event->type() == QEvent::Destroy) {
        watched->removeEventFilter(this);
    }
    return BaseWidget::eventFilter(watched, event);
}

void NewsWidget::show_custom_tooltip(const QString& text, const QPoint& global_pos) {
    if (!custom_tooltip_) {
        custom_tooltip_ = new ui::CustomTooltip();
    }
    custom_tooltip_->set_text(text);
    custom_tooltip_->show_at(global_pos);
}

void NewsWidget::hide_custom_tooltip() {
    if (custom_tooltip_) {
        custom_tooltip_->hide();
    }
}

} // namespace fincept::screens::widgets
