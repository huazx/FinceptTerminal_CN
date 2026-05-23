#include "ui/widgets/CustomTooltip.h"
#include "ui/theme/Theme.h"

#include <QApplication>
#include <QPainter>
#include <QScreen>

namespace fincept::ui {

CustomTooltip::CustomTooltip(QWidget* parent)
    : QFrame(parent) {
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::NoFocus);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(0);

    label_ = new QLabel(this);
    label_->setWordWrap(true);
    label_->setTextInteractionFlags(Qt::NoTextInteraction);
    label_->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(label_);

    apply_theme();
}

void CustomTooltip::set_text(const QString& text) {
    label_->setText(text);
    adjustSize();
}

void CustomTooltip::show_at(const QPoint& global_pos) {
    QPoint pos = global_pos + QPoint(12, 12);

    QRect screen_rect;
    if (auto* screen = QGuiApplication::screenAt(global_pos)) {
        screen_rect = screen->availableGeometry();
    } else {
        screen_rect = QGuiApplication::primaryScreen()->availableGeometry();
    }

    adjustSize();
    int w = width();
    int h = height();

    if (pos.x() + w > screen_rect.right())
        pos.setX(global_pos.x() - w - 8);
    if (pos.y() + h > screen_rect.bottom())
        pos.setY(global_pos.y() - h - 8);
    if (pos.x() < screen_rect.left())
        pos.setX(screen_rect.left());
    if (pos.y() < screen_rect.top())
        pos.setY(screen_rect.top());

    move(pos);
    show();
}

void CustomTooltip::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QColor bg(colors::BG_RAISED);
    QColor border(colors::BORDER_MED);

    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawRoundedRect(rect(), 4, 4);

    p.setPen(border);
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 4, 4);
}

void CustomTooltip::apply_theme() {
    QColor text_color(colors::TEXT_PRIMARY);

    setStyleSheet(QString());
    label_->setStyleSheet(
        QString("QLabel { color: %1; background: transparent; font-size: 11px; }")
            .arg(text_color.name()));

    setFixedWidth(360);
}

} // namespace fincept::ui
