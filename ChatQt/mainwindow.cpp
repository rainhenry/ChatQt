#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //  提示
    ui->progressBar->setFormat("Initializing environment, please wait...");

    //  关闭控件
    ui->pushButton_send ->setEnabled(false);
    ui->pushButton_clear->setEnabled(false);

    //  设置当前忙状态
    is_cur_busy = true;

    //  初始化运行环境
    system("rm -rf chat_out");
    system("mkfifo chat_out");

    //  初始化Chat环境
    connect(&chat, &CQtAI::send_environment_ready, this, &MainWindow::slot_OnEnvReady);
    connect(&chat, &CQtAI::send_chat_finish_all,   this, &MainWindow::slot_OnChatFinish);
    chat.Init();
    chat.start();

    //  初始化文件读取
    connect(&fileread, &CQtAsyncFileRead::send_read_data, this, &MainWindow::slot_OnRecvWords);
    fileread.Init();
    fileread.start();

    //  定时器初始化
    timer = new QTimer(this);
    timer->setInterval(30);
    connect(timer, SIGNAL(timeout()), this, SLOT(slot_timeout()));
    timer->start();

}

MainWindow::~MainWindow()
{
    //  释放定时器
    if(timer != nullptr)
    {
        timer->stop();
        delete timer;
        timer = nullptr;
    }

    //  释放资源
    fileread.Release();
    chat.Release();

    //  释放UI资源
    delete ui;
}

//  当OCR环境就绪
void MainWindow::slot_OnEnvReady(void)
{
    //  开启UI
    ui->pushButton_send ->setEnabled(true);
    ui->pushButton_clear->setEnabled(true);

    //  提示
    is_cur_busy = false;
    ui->progressBar->setFormat("Ready!!");
}

//  定时器
void MainWindow::slot_timeout()
{
    //  进度变量
    static int cur_proc = 100;

    //  当忙
    if(is_cur_busy)
    {
        if(cur_proc >= 100) cur_proc = 0;
        else                cur_proc = cur_proc + 1;
    }
    //  当忙完
    else
    {
        cur_proc = 100;
    }

    //  更新到控件
    ui->progressBar->setValue(cur_proc);
}

//  单击发送消息按钮
void MainWindow::on_pushButton_send_clicked()
{
    //  检查输入内容
    QString in_text = ui->textEdit_in->toPlainText();

    //  当不合法
    if(in_text.isEmpty())
    {
        //  提示
        ui->progressBar->setFormat("Input Text is NULL!");
        return;
    }

    //  清除
    cur_out_text.clear();

    //  设置前置词语
    acc_out_text += "\r\n人脑: " + in_text + "\r\n电脑: \r\n";
    ui->textEdit_out->setText(acc_out_text);
    ui->textEdit_out->moveCursor(QTextCursor::End);

    //  开始读文件
    fileread.ExRead("chat_out");

    //  启动
    chat.ExChat(in_text);

    //  设置忙
    is_cur_busy = false;
    ui->progressBar->setFormat("Processing...");

    //  关闭控件
    ui->pushButton_send ->setEnabled(false);
    ui->pushButton_clear->setEnabled(false);
}

//  单击清除按钮
void MainWindow::on_pushButton_clear_clicked()
{
    ui->textEdit_in ->setText("");
    ui->textEdit_out->setText("");
    acc_out_text = "";
    cur_out_text.clear();
}

//  聊天一次处理完成
void MainWindow::slot_OnChatFinish(qint64 run_time_ns)
{
    //  设置结束语
    acc_out_text += cur_out_text + "\r\n";
    cur_out_text.clear();
    ui->textEdit_out->setText(acc_out_text);
    ui->textEdit_out->moveCursor(QTextCursor::End);

    //  设置不忙
    is_cur_busy = false;

    //  设置状态
    long long ms = run_time_ns / 1000000LL;
    int sec = static_cast<int>(ms / 1000);
    int min = sec / 60;
    int hour = min / 60;
    QString str = QString::asprintf("Finish (%d:%02d:%02d.%03lld)",
                                    hour, min % 60, sec % 60, ms % 1000LL
                                    );
    ui->progressBar->setFormat(str);

    //  打印时间
    acc_out_text += "===================";
    acc_out_text += str;
    acc_out_text += "===================\r\n";
    ui->textEdit_out->setText(acc_out_text);
    ui->textEdit_out->moveCursor(QTextCursor::End);

    //  打开控件
    ui->pushButton_send ->setEnabled(true);
    ui->pushButton_clear->setEnabled(true);
}

//  当接收到文字
void MainWindow::slot_OnRecvWords(QByteArray dat)
{
    cur_out_text += dat;
    ui->textEdit_out->setText(acc_out_text + QString(cur_out_text));
    ui->textEdit_out->moveCursor(QTextCursor::End);
}
