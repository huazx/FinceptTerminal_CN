#pragma once
#include "screens/dashboard/widgets/BaseWidget.h"

#include <QHash>
#include <QHideEvent>
#include <QJsonArray>
#include <QLabel>
#include <QScrollArea>
#include <QShowEvent>
#include <QStringList>
#include <QTimer>
#include <QVBoxLayout>

namespace fincept::screens::widgets {

/// Economic Calendar Widget — consumes the DataHub topic
/// `econ:fincept:upcoming_events` (HTTP-backed by `MacroCalendarService`).
/// All cadence is owned by the hub scheduler.
class EconomicCalendarWidget : public BaseWidget {
    Q_OBJECT
  public:
    explicit EconomicCalendarWidget(QWidget* parent = nullptr);

  protected:
    void on_theme_changed() override;
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;

  private:
    void apply_styles();
    void clear_list();
    void show_status(const QString& text);
    void hub_subscribe();
    void hub_unsubscribe();
    void populate(const QJsonArray& events);
    void request_online_translation(const QString& original);
    void on_translation_received(const QString& original, const QString& translated);
    void refresh_translated_labels();
    QWidget* header_widget_ = nullptr;
    QFrame* header_sep_ = nullptr;
    QScrollArea* scroll_area_ = nullptr;
    QVBoxLayout* list_layout_ = nullptr;
    QLabel* status_label_ = nullptr;
    QVector<QLabel*> header_labels_;
    QJsonArray last_events_; // cached for theme-change re-populate
    QHash<QString, QString> translation_cache_;
    QStringList translation_queue_;
    QTimer* translation_timer_ = nullptr;
    bool translation_in_progress_ = false;
    bool hub_active_ = false;
};

} // namespace fincept::screens::widgets
