#pragma once
#include <QFrame>
#include <QLabel>
#include <QPaintEvent>
#include <QVBoxLayout>

namespace fincept::ui {

class CustomTooltip : public QFrame {
    Q_OBJECT
  public:
    explicit CustomTooltip(QWidget* parent = nullptr);

    void set_text(const QString& text);
    void show_at(const QPoint& global_pos);

  protected:
    void paintEvent(QPaintEvent* event) override;

  private:
    void apply_theme();

    QLabel* label_ = nullptr;
};

} // namespace fincept::ui
