#pragma once
#include "services/news/NewsClusterService.h"
#include "services/news/NewsMonitorService.h"
#include "services/news/NewsService.h"

#include <QAbstractListModel>
#include <QHash>
#include <QSet>
#include <QTimer>
#include <QVector>

namespace fincept::screens {

enum NewsFeedRole {
    ArticleRole = Qt::UserRole + 1,
    ClusterRole,
    ViewModeRole,
    IsNewRole,
    MonitorColorRole,
    IsSelectedRole,
    SourceTierRole,
    VelocityTextRole,
    ThreatLevelRole,
    SourceFlagRole,
    LanguageRole,
    HasGeoRole,
    PulsePhaseRole,
    HeadlineZhRole,
    SummaryZhRole,
    FormattedSourceRole,
    FormattedLangRole,
    FormattedThreatRole,
    FormattedTickersRole,
    ThreatColorRole,
    PriorityColorRole,
};

class NewsFeedModel : public QAbstractListModel {
    Q_OBJECT
  public:
    explicit NewsFeedModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void set_wire_articles(const QVector<services::NewsArticle>& articles);
    void set_clusters(const QVector<services::NewsCluster>& clusters);
    void set_view_mode(const QString& mode);
    void set_selected_id(const QString& article_id);
    void set_monitor_matches(const QMap<QString, QVector<services::NewsArticle>>& matches,
                              const QVector<services::NewsMonitor>& monitors);

    void mark_all_seen();
    void mark_seen(const QString& article_id);
    int unseen_count() const;

    void set_geo_articles(const QSet<QString>& geolocated_ids);
    void advance_pulse();

    QModelIndex index_for_article(const QString& article_id) const;
    services::NewsArticle article_at(int row) const;
    services::NewsCluster cluster_at(int row) const;
    QString view_mode() const { return view_mode_; }

    void translate_headlines();

  private:
    QString monitor_color_for(const QString& article_id) const;
    void translate_next_batch();

    QVector<services::NewsArticle> articles_;
    QVector<services::NewsCluster> clusters_;
    QString view_mode_ = "WIRE";
    QString selected_id_;
    QSet<QString> seen_ids_;
    QHash<QString, int> article_id_to_row_;
    QHash<QString, int> cluster_id_to_row_;
    QHash<QString, QString> article_monitor_color_;
    QSet<QString> geo_article_ids_;
    int pulse_phase_ = 0;
    int unseen_count_ = 0;

    struct FormattedRow {
        QString source;
        QString lang;
        QString threat;
        QString tickers;
        QString threat_color;
        QString priority_color;
    };
    QVector<FormattedRow> formatted_rows_;

    static constexpr int kMaxCacheSize = 1000;
    QHash<QString, QString> headline_zh_cache_;
    QHash<QString, QString> summary_zh_cache_;
    int translate_batch_idx_ = 0;
    bool translate_in_progress_ = false;
    QTimer* translate_timer_ = nullptr;
    static constexpr int kTranslateBatchSize = 6;
    static constexpr int kTranslateDelayMs = 1000;
};

} // namespace fincept::screens
