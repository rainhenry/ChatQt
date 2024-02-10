#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "CQtAI.h"
#include "CQtAsyncFileRead.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    //  Chat对象
    CQtAI chat;

    //  定时器
    QTimer* timer;

    //  当前忙
    bool is_cur_busy;

    //  累计输出文本
    QString acc_out_text;

    //  异步读取文件类
    CQtAsyncFileRead fileread;

    //  本次输出对话的文本
    QByteArray cur_out_text;


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    //  当环境就绪
    void slot_OnEnvReady(void);

    //  定时器
    void slot_timeout();

    //  聊天一次处理完成
    void slot_OnChatFinish(qint64 run_time_ns);

    //  当接收到文字
    void slot_OnRecvWords(QByteArray dat);


    //  单击发送消息按钮
    void on_pushButton_send_clicked();

    //  单击清除按钮
    void on_pushButton_clear_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
