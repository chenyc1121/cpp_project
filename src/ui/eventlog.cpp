#include "eventlog.h"
#include <QVBoxLayout>
#include <QDateTime>

EventLog::EventLog(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setMaximumHeight(200);
    m_textEdit->setPlaceholderText("游戏事件将显示在这里...");
    m_textEdit->setStyleSheet(
        "QTextEdit {"
        "  background-color: #FFFFFF;"
        "  border: 1px solid #8B1A1A;"
        "  border-radius: 6px;"
        "  padding: 6px 8px;"
        "  font-size: 12px;"
        "  color: #3D2820;"
        "}"
        "QScrollBar:vertical { width: 6px; background: #F0E8DD; border-radius: 3px; }"
        "QScrollBar::handle:vertical { background: #8B1A1A; border-radius: 3px; min-height: 16px; }");
    layout->addWidget(m_textEdit);

    setLayout(layout);
}

void EventLog::appendMessage(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_textEdit->append("[" + timestamp + "] " + message);
    // 自动滚动到底部
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_textEdit->setTextCursor(cursor);
}

void EventLog::clear() {
    m_textEdit->clear();
}
