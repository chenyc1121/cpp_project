#ifndef EVENTLOG_H
#define EVENTLOG_H

#include <QWidget>
#include <QTextEdit>

// ==================== 事件日志组件 ====================
// 滚动显示游戏中的所有事件消息
class EventLog : public QWidget {
    Q_OBJECT
public:
    explicit EventLog(QWidget* parent = nullptr);

public slots:
    // 追加一条事件消息
    void appendMessage(const QString& message);
    // 清空日志
    void clear();

private:
    QTextEdit* m_textEdit;
};

#endif // EVENTLOG_H
