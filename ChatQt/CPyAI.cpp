/*************************************************************

    程序名称:基于Python3原生C接口的AI C++类(阻塞)
    程序版本:REV 0.1
    创建日期:20240209
    设计编写:rainhenry
    作者邮箱:rainhenry@savelife-tech.com
    开源协议:GPL

    版本修订
        REV 0.1   20240209      rainhenry    创建文档

*************************************************************/

//  包含头文件
#include <stdio.h>
#include <stdlib.h>

#include "CPyAI.h"

#include <Python.h>
#include <numpy/arrayobject.h>
#include <numpy/ndarrayobject.h>
#include <numpy/npy_3kcompat.h>

//  初始化全局变量
bool CPyAI::Py_Initialize_flag = false;

//  构造函数
CPyAI::CPyAI()
{
    //  当没有初始化过
    if(!Py_Initialize_flag)
    {
        if(!Py_IsInitialized())
        {
            Py_Initialize();
            import_array_init();
        }
        Py_Initialize_flag = true;   //  标记已经初始化
    }

    //  初始化大语言私有数据
    py_chat_module           = nullptr;
    py_chat_model_init       = nullptr;
    py_chat_tokenizer_init   = nullptr;
    py_chat_model_handle     = nullptr;
    py_chat_tokenizer_handle = nullptr;
    py_chat_ex               = nullptr;
}

//  析构函数
CPyAI::~CPyAI()
{
    //  此处不可以调用Release，因为Python环境实际运行所在线程
    //  不一定和构造该类对象是同一个线程
}

//  释放资源
//  注意！该释放必须和执行本体在同一线程中！
void CPyAI::Release(void)
{
    if(py_chat_ex != nullptr)
    {
        Py_DecRef(static_cast<PyObject*>(py_chat_ex));
    }
    if(py_chat_tokenizer_handle != nullptr)
    {
        Py_DecRef(static_cast<PyObject*>(py_chat_tokenizer_handle));
    }
    if(py_chat_model_handle != nullptr)
    {
        Py_DecRef(static_cast<PyObject*>(py_chat_model_handle));
    }
    if(py_chat_tokenizer_init != nullptr)
    {
        Py_DecRef(static_cast<PyObject*>(py_chat_tokenizer_init));
    }
    if(py_chat_model_init != nullptr)
    {
        Py_DecRef(static_cast<PyObject*>(py_chat_model_init));
    }
    if(py_chat_module != nullptr)
    {
        Py_DecRef(static_cast<PyObject*>(py_chat_module));
    }

    if(Py_Initialize_flag)
    {
        Py_Finalize();
        Py_Initialize_flag = false;   //  标记未初始化
    }
}

//  为了兼容Python C的原生API，独立封装numpy的C初始化接口
int CPyAI::import_array_init(void)
{
    import_array()
    return 0;
}

//  初始化
//  注意！该初始化必须和执行本体在同一线程中！
void CPyAI::Init(void)
{
    //  开启Python线程支持
    PyEval_InitThreads();
    PyEval_SaveThread();

    //  检测当前线程是否拥有GIL
    int ck = PyGILState_Check() ;
    if (!ck)
    {
        PyGILState_Ensure(); //  如果没有GIL，则申请获取GIL
    }

    //  构造基本Python环境
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('../ChatQt')");

    //  载入chat.py文件
    py_chat_module = static_cast<void*>(PyImport_ImportModule("chat"));

    //  检查是否成功
    if(py_chat_module == nullptr)
    {
        printf("[Error] py_chat_module == null!!");
        return;
    }

    //  初始化模型
    py_chat_model_init   = static_cast<void*>(PyObject_GetAttrString(static_cast<PyObject*>(py_chat_module), "model_init"));
    PyObject* py_chat_args = PyTuple_New(1);
    PyTuple_SetItem(py_chat_args, 0, Py_BuildValue("s", "../chatglm2-6b"));
    py_chat_model_handle = static_cast<void*>(PyObject_CallObject(static_cast<PyObject*>(py_chat_model_init), py_chat_args));

    if(py_chat_model_handle == nullptr)
    {
        printf("[Error] py_chat_model_handle == null!!");
        return;
    }

    //  初始化分词器
    py_chat_tokenizer_init   = static_cast<void*>(PyObject_GetAttrString(static_cast<PyObject*>(py_chat_module), "tokenizer_init"));
    py_chat_args = PyTuple_New(1);
    PyTuple_SetItem(py_chat_args, 0, Py_BuildValue("s", "../chatglm2-6b"));
    py_chat_tokenizer_handle = static_cast<void*>(PyObject_CallObject(static_cast<PyObject*>(py_chat_tokenizer_init), py_chat_args));

    if(py_chat_tokenizer_handle == nullptr)
    {
        printf("[Error] py_chat_tokenizer_handle == null!!");
        return;
    }

    //  载入执行聊天本体
    py_chat_ex = static_cast<void*>(PyObject_GetAttrString(static_cast<PyObject*>(py_chat_module), "chat_ex"));
}

//  执行一次对话
void CPyAI::Chat_Ex(const char* prompt)
{
    //  构造输入数据
    PyObject* py_args = PyTuple_New(3);

    //  第一个参数为关键字
    PyObject* py_prompt = Py_BuildValue("s", prompt);
    PyTuple_SetItem(py_args, 0, py_prompt);

    //  第二个参数为模型句柄
    PyTuple_SetItem(py_args, 1, static_cast<PyObject*>(py_chat_model_handle));

    //  第三个参数为分词器句柄
    PyTuple_SetItem(py_args, 2, static_cast<PyObject*>(py_chat_tokenizer_handle));

    //  执行SD
    PyObject_CallObject(static_cast<PyObject*>(py_chat_ex), py_args);

    //  释放资源
    Py_DecRef(py_prompt);
    //Py_DecRef(py_args);    //  由于其中包含了模型句柄,所以不能释放

    //  操作完成
    return;
}


