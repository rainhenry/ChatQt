/*************************************************************

    程序名称:基于Qt线程类的AI类
    程序版本:REV 0.1
    创建日期:20240209
    设计编写:rainhenry
    作者邮箱:rainhenry@savelife-tech.com
    开源协议:GPL

    版本修订
        REV 0.1   20240209      rainhenry    创建文档

*************************************************************/

//  包含头文件
#include "CQtAI.h"

//  构造函数
CQtAI::CQtAI():
    sem_cmd(0)
{
    //  初始化私有数据
    cur_st = EAISt_Ready;
    cur_cmd = EAIcmd_Null;

    //  创建Python的Chat对象
    py_chat = new CPyAI();
}

//  析构函数
CQtAI::~CQtAI()
{
    Release();   //  通知线程退出
    this->msleep(1000);
    this->quit();
    this->wait(500);

    delete py_chat;
}

//  初始化
void CQtAI::Init(void)
{
    //  暂时没有
}

void CQtAI::run()
{
    //  初始化python环境
    py_chat->Init();

    //  通知运行环境就绪
    emit send_environment_ready();

    //  现成主循环
    while(1)
    {
        //  获取信号量
        sem_cmd.acquire();

        //  获取当前命令和数据
        cmd_mutex.lock();
        EAIcmd now_cmd = this->cur_cmd;
        std::string tmp_str = this->chat_prompt.toStdString();
        cmd_mutex.unlock();

        //  当为空命令
        if(now_cmd == EAIcmd_Null)
        {
            //  释放CPU
            this->sleep(1);
        }
        //  当为退出命令
        else if(now_cmd == EAIcmd_Release)
        {
            py_chat->Release();
            qDebug("Thread is exit!!");
            return;
        }
        //  当为聊天命令
        else if(now_cmd == EAIcmd_ExChat)
        {
            //  设置忙
            cmd_mutex.lock();
            this->cur_st = EAISt_Busy;
            cmd_mutex.unlock();

            //  执行
            QElapsedTimer run_time;
            run_time.start();
            py_chat->Chat_Ex(tmp_str.c_str());
            qint64 run_time_ns = run_time.nsecsElapsed();

            //  发出操作完成
            emit send_chat_finish_all(run_time_ns);

            //  完成处理
            cmd_mutex.lock();
            this->cur_st = EAISt_Ready;
            now_cmd = EAIcmd_Null;
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

CQtAI::EAISt CQtAI::GetStatus(void)
{
    EAISt re;
    cmd_mutex.lock();
    re = this->cur_st;
    cmd_mutex.unlock();
    return re;
}

//  执行一次聊天
void CQtAI::ExChat(QString prompt)
{
    if(GetStatus() == EAISt_Busy) return;

    cmd_mutex.lock();
    this->cur_cmd = EAIcmd_ExChat;
    this->chat_prompt = prompt;
    this->cur_st = EAISt_Busy;    //  设置忙
    cmd_mutex.unlock();

    sem_cmd.release();
}

void CQtAI::Release(void)
{
    cmd_mutex.lock();
    this->cur_cmd = EAIcmd_Release;
    cmd_mutex.unlock();

    sem_cmd.release();
}
