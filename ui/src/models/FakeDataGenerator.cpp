#include "ui/models/FakeDataGenerator.h"
#include <QStandardItem>
#include <QDate>
#include <random>
#include <map>
#include <set>
#include <vector>

namespace ui {

static int random_in_range(int a, int b) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(a, b);
    return dist(rng);
}

struct Tx {
    QString id;
    QDate date;
    bool draft = false;
};

struct BookingGroup {
    QString id;
    std::vector<Tx> transactions;
};

struct StatementData {
    QString id;
    QDate startDate;
    QDate endDate;
    std::vector<BookingGroup> booking_groups;
};

void FakeDataGenerator::populate_timeline_model(QStandardItemModel* model,
                                                int year_start,
                                                int year_count,
                                                int statements_per_year) {
    // block signals to avoid incremental view updates while populating
    bool wasBlocked = model->signalsBlocked();
    model->blockSignals(true);

    model->clear();
    model->setHorizontalHeaderLabels({"Timeline"});

    for (int y = 0; y < year_count; ++y) {
        int year = year_start + y;

        // generate statements for this year with booking groups and transactions
        std::vector<StatementData> statements;
        int day_cursor = 1; // start at Jan 1
        for (int s = 0; s < statements_per_year; ++s) {
            StatementData st;
            int stmtId = (y * statements_per_year) + s + 1;
            st.id = QStringLiteral("stmt-") + QString::number(year) + "-" + QString::number(stmtId);

            // choose a duration about 20..45 days (~1 month)
            int duration = random_in_range(20, 45);
            // small gap between statements
            int gap = random_in_range(1, 5);
            int start_day = day_cursor + gap;
            int end_day = start_day + duration;
            if (end_day > 365) end_day = 365;
            day_cursor = end_day; // next statement will start after this

            QDate startDate(year, 1, 1);
            startDate = startDate.addDays(start_day - 1);
            QDate endDate(year, 1, 1);
            endDate = endDate.addDays(end_day - 1);
            st.startDate = startDate;
            st.endDate = endDate;

            int bgCount = random_in_range(2, 6);
            for (int b = 0; b < bgCount; ++b) {
                BookingGroup bg;
                int bgId = stmtId * 100 + b + 1;
                bg.id = QStringLiteral("bg-") + QString::number(year) + "-" + QString::number(bgId);

                int txCount = random_in_range(5, 10); // reduce transactions per bg for performance
                for (int t = 0; t < txCount; ++t) {
                    Tx tx;
                    int txId = bgId * 1000 + t + 1;
                    tx.id = QStringLiteral("tx-") + QString::number(year) + "-" + QString::number(txId);

                    // assign transaction date within the statement range
                    int dayOfYear = random_in_range(start_day, end_day);
                    QDate date(year, 1, 1);
                    date = date.addDays(dayOfYear - 1);
                    tx.date = date;

                    tx.draft = (random_in_range(0, 20) == 0);

                    bg.transactions.push_back(std::move(tx));
                }

                st.booking_groups.push_back(std::move(bg));
            }

            statements.push_back(std::move(st));
        }

        // collect all unique dates that have any transaction in this year across all statements
        std::set<QDate> unique_dates;
        for (const auto& st : statements) {
            for (const auto& bg : st.booking_groups) {
                for (const auto& tx : bg.transactions) {
                    unique_dates.insert(tx.date);
                }
            }
        }

        QStandardItem* yearItem = new QStandardItem(QString::number(year));
        yearItem->setData(QStringLiteral("year-") + QString::number(year), Qt::UserRole + 1);
        yearItem->setData(QStringLiteral("year"), Qt::UserRole + 2);

        // for each date, build day node and include only statements/bg/tx relevant to that date
        for (const QDate& date : unique_dates) {
            // find the statement that covers this date (assuming each date maps to exactly one statement)
            const StatementData* covering = nullptr;
            for (const auto& st : statements) {
                if (date >= st.startDate && date <= st.endDate) { covering = &st; break; }
            }
            if (!covering) continue;

            QString dateLabel = date.toString(Qt::ISODate);
            QString monthName = date.toString("MMMM");
            QString stmtLabel = QString("%1. %2 | Bankauszug von %3 bis %4")
                                .arg(date.day())
                                .arg(monthName)
                                .arg(covering->startDate.toString("d.M.yyyy"))
                                .arg(covering->endDate.toString("d.M.yyyy"));
            QStandardItem* day = new QStandardItem(stmtLabel);
            // use the statement id as the node id so controller recognizes Statement level
            day->setData(covering->id, Qt::UserRole + 1);
            day->setData(QStringLiteral("stmt"), Qt::UserRole + 2); // day acts as statement level
            // store actual date separately
            day->setData(dateLabel, Qt::UserRole + 6);

            // within the statement/day, add the booking groups that have tx on this date
            for (const auto& bg : covering->booking_groups) {
                QStandardItem* bgItem = new QStandardItem(bg.id);
                bgItem->setData(bg.id, Qt::UserRole + 1);
                bgItem->setData(QStringLiteral("bg"), Qt::UserRole + 2);

                bool bg_has = false;
                std::set<QDate> bg_dates;
                for (const auto& tx : bg.transactions) {
                    if (tx.date == date) {
                        QStandardItem* txItem = new QStandardItem(tx.id);
                        txItem->setData(tx.id, Qt::UserRole + 1);
                        txItem->setData(QStringLiteral("tx"), Qt::UserRole + 2);
                        if (tx.draft) txItem->setData(QStringLiteral("draft"), Qt::UserRole + 3);
                        if (tx.draft) txItem->setBackground(QColor(255, 248, 220));
                        bgItem->appendRow(txItem);
                        bg_has = true;
                    }
                    bg_dates.insert(tx.date);
                }

                if (bg_has) {
                    // mark bg as multi-day if its transactions span more than one unique date
                    if (bg_dates.size() > 1) bgItem->setData(true, Qt::UserRole + 4);
                    day->appendRow(bgItem);
                }
            }

            yearItem->appendRow(day);
        }

        model->appendRow(yearItem);
    }

    // unblock signals and notify layout changed
    model->blockSignals(wasBlocked);
    QModelIndex topLeft = model->index(0,0);
    QModelIndex bottomRight = model->index(model->rowCount() - 1, 0);
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
}

}