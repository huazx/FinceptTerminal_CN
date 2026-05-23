#pragma once
#include "screens/dashboard/widgets/BaseWidget.h"
#include "services/news/NewsService.h"

#include <QHideEvent>
#include <QHash>
#include <QScrollArea>
#include <QShowEvent>
#include <QTimer>
#include <QVector>

namespace fincept::ui {
class CustomTooltip;
}

namespace fincept::screens::widgets {

class NewsWidget : public BaseWidget {
    Q_OBJECT
  public:
    explicit NewsWidget(QWidget* parent = nullptr);

  protected:
    void on_theme_changed() override;
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

  private:
    void apply_styles();
    void hub_subscribe();
    void hub_unsubscribe();
    void populate(const QVector<services::NewsArticle>& articles);
    void request_online_translation(const QString& original);
    void on_translation_received(const QString& original, const QString& translated);
    void refresh_display();
    void show_custom_tooltip(const QString& text, const QPoint& global_pos);
    void hide_custom_tooltip();

    QScrollArea* scroll_area_ = nullptr;
    QVBoxLayout* news_layout_ = nullptr;
    QVector<services::NewsArticle> last_articles_;
    QHash<QString, QString> translation_cache_;
    QStringList translation_queue_;
    bool hub_active_ = false;
    bool translation_in_progress_ = false;
    QTimer* translation_timer_ = nullptr;
    ui::CustomTooltip* custom_tooltip_ = nullptr;
};

} // namespace fincept::screens::widgets
