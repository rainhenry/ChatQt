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

//  包含头文件
#include "CQtAsyncFileRead.h"
#include <cstdio>
#include <cstring>

//  构造函数
CQtAsyncFileRead::CQtAsyncFileRead():
    sem_cmd(0)
{
    //  初始化私有数据
    cur_st = EQAFRSt_Ready;
    cur_cmd = EQAFRcmd_Null;
}

//  析构函数
CQtAsyncFileRead::~CQtAsyncFileRead()
{
    Release();   //  通知线程退出
    this->msleep(1000);
    this->quit();
    this->wait(500);
}

//  初始化
void CQtAsyncFileRead::Init(void)
{
    //  暂时没有
}

void CQtAsyncFileRead::run()
{
    //  线程主循环
    while(1)
    {
        //  获取信号量
        sem_cmd.acquire();

        //  获取当前命令和数据
        cmd_mutex.lock();
        EQAFRcmd now_cmd = this->cur_cmd;
        std::string tmp_str = this->filename.toStdString();
        cmd_mutex.unlock();

        //  当为空命令
        if(now_cmd == EQAFRcmd_Null)
        {
            //  释放CPU
            this->sleep(1);
        }
        //  当为退出命令
        else if(now_cmd == EQAFRcmd_Release)
        {
            qDebug("Read File Thread is exit!!");
            return;
        }
        //  当为读取文件命令
        else if(now_cmd == EQAFRcmd_ExRead)
        {
            //  设置忙
            cmd_mutex.lock();
            this->cur_st = EQAFRSt_Busy;
            cmd_mutex.unlock();

            //  打开被读取的文件
            FILE* fp = fopen(tmp_str.c_str(), "r");

            //  当错误
            if(fp == nullptr)
            {
                qDebug("[Error] fopen error!!");
                cmd_mutex.lock();
                this->cur_st = EQAFRSt_Ready;
                now_cmd = EQAFRcmd_Null;
                cmd_mutex.unlock();

                //  发出操作完成
                emit send_read_finish();
                continue;
            }

            //  循环读取文件
            size_t re = 0;
            while(1)
            {
                //  尝试读取指定的字节
                char tmp[1];
                memset(tmp, 0, sizeof(tmp));
                re = fread(&tmp, 1, sizeof(tmp), fp);

                //  当读取OK，并且文件没有结束
                if((re > 0) && (!feof(fp)))
                {
                    //  发出该数据
                    QByteArray dat(tmp, static_cast<int>(re));
                    send_read_data(dat);
                }
                //  出错或者达到文件末尾
                else
                {
                    break;
                }
            }            

            //  关闭文件
            fclose(fp);

            //  发出操作完成
            emit send_read_finish();

            //  完成处理
            cmd_mutex.lock();
            this->cur_st = EQAFRSt_Ready;
            now_cmd = EQAFRcmd_Null;
            cmd_mutex.unlock();
        }
        //  非法命令
        else
        {
            //  释放CPU
            QThread::sleep(1);
            qDebug("Unknow cmd code!!");
        }
    }
}

CQtAsyncFileRead::EQAFRSt CQtAsyncFileRead::GetStatus(void)
{
    EQAFRSt re;
    cmd_mutex.lock();
    re = this->cur_st;
    cmd_mutex.unlock();
    return re;
}

//  执行读取文件
void CQtAsyncFileRead::ExRead(QString filename)
{
    if(GetStatus() == EQAFRSt_Busy) return;

    cmd_mutex.lock();
    this->cur_cmd = EQAFRcmd_ExRead;
    this->filename = filename;
    this->cur_st = EQAFRSt_Busy;    //  设置忙
    cmd_mutex.unlock();

    sem_cmd.release();
}

void CQtAsyncFileRead::Release(void)
{
    cmd_mutex.lock();
    this->cur_cmd = EQAFRcmd_Release;
    cmd_mutex.unlock();

    sem_cmd.release();
}

