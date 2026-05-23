#include "screens/devtools/DataHubInspector.h"

#include "datahub/DataHub.h"

#include <QDateTime>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>

namespace fincept::screens::devtools {

namespace {
QString format_age(qint64 ms_since_epoch) {
    if (ms_since_epoch <= 0) return QStringLiteral("—");
    const qint64 age = QDateTime::currentMSecsSinceEpoch() - ms_since_epoch;
    if (age < 1000) return QStringLiteral("%1 ms").arg(age);
    if (age < 60000) return QStringLiteral("%1 s").arg(age / 1000);
    return QStringLiteral("%1 m").arg(age / 60000);
}
} // namespace

DataHubInspector::DataHubInspector(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    table_ = new QTableWidget(this);
    table_->setColumnCount(6);
    table_->setHorizontalHeaderLabels(
        {"Topic", "Subs", "Publishes", "Last Publish", "Last Refresh", "State"});
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(table_);

    refresh_timer_.setInterval(1000);
    connect(&refresh_timer_, &QTimer::timeout, this, &DataHubInspector::refresh);
    // Per CLAUDE.md P3: start/stop the timer in show/hide, not the ctor.
}

void DataHubInspector::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
    QTimer::singleShot(0, this, [this]() {
        refresh();
        refresh_timer_.start();
    });
}

void DataHubInspector::hideEvent(QHideEvent* e) {
    QWidget::hideEvent(e);
    refresh_timer_.stop();
}

void DataHubInspector::refresh() {
    const auto stats = datahub::DataHub::instance().stats();
    table_->setUpdatesEnabled(false);
    if (table_->rowCount() != stats.size())
        table_->setRowCount(stats.size());
    for (int row = 0; row < stats.size(); ++row) {
        const auto& s = stats[row];
        auto set_or_create = [&](int col, const QString& text) {
            auto* item = table_->item(row, col);
            if (!item) {
                item = new QTableWidgetItem(text);
                table_->setItem(row, col, item);
            } else {
                item->setText(text);
            }
        };
        set_or_create(0, s.topic);
        set_or_create(1, QString::number(s.subscriber_count));
        set_or_create(2, QString::number(s.total_publishes));
        set_or_create(3, format_age(s.last_publish_ms));
        set_or_create(4, format_age(s.last_refresh_request_ms));
        QString state_label;
        if (s.push_only) state_label = QStringLiteral("push");
        else if (s.in_flight) state_label = QStringLiteral("in-flight");
        else state_label = QStringLiteral("idle");
        set_or_create(5, state_label);
    }
    table_->setUpdatesEnabled(true);
}

} // namespace fincept::screens::devtools
