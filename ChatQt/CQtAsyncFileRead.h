/*************************************************************

    程序名称:基于Qt的异步文件读取类
    程序版本:REV 0.1
    创建日期:20240209
    设计编写:rainhenry
    作者邮箱:rainhenry@savelife-tech.com
    开源协议:GPL

    版本修订
        REV 0.1   20240209      rainhenry    创建文档

*************************************************************/
//------------------------------------------------------------
//  重定义保护
#ifndef __CQTASYNCFILEREAD_H__
#define __CQTASYNCFILEREAD_H__

//------------------------------------------------------------
//  包含头文件
#include <QString>
#include <QVariant>
#include <QThread>
#include <QSemaphore>
#include <QMutex>
#include <QElapsedTimer>
#include <QByteArray>

//------------------------------------------------------------
//  类定义
class CQtAsyncFileRead: public QThread
{
    Q_OBJECT

public:
    //  状态枚举
    typedef enum
    {
        EQAFRSt_Ready = 0,
        EQAFRSt_Busy,
        EQAFRSt_Error
    }EQAFRSt;

    //  命令枚举
    typedef enum
    {
        EQAFRcmd_Null = 0,
        EQAFRcmd_ExRead,
        EQAFRcmd_Release,
    }EQAFRcmd;

    //  构造和析构函数
    CQtAsyncFileRead();
    ~CQtAsyncFileRead() override;

    //  线程运行本体
    void run() override;

    //  初始化
    void Init(void);

    //  执行读取文件
    void ExRead(QString filename);

    //  得到当前状态
    EQAFRSt GetStatus(void);

    //  释放资源
    void Release(void);

signals:
    //  当读取到数据
    void send_read_data(QByteArray dat);

    //  读取结束
    void send_read_finish(void);

private:
    //  内部私有变量
    QSemaphore sem_cmd;           //  命令信号量
    QMutex cmd_mutex;             //  命令互斥量
    EQAFRSt cur_st;               //  当前状态
    EQAFRcmd cur_cmd;             //  当前命令
    QString filename;             //  文件名
};

//------------------------------------------------------------
#endif  //  __CQTASYNCFILEREAD_H__


