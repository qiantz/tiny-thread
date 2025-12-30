#ifndef CQTZ_THREAD_WINDOWS_INCLUDE_
#define CQTZ_THREAD_WINDOWS_INCLUDE_
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) || defined(_WINDOWS) || defined(_WINDOWS_)
#include <assert.h>
#include <process.h>
#include <functional>

using TinyThreadHandle = std::function<void(void)>;

template<typename _Tp>
using ArgsThreadHandle = std::function<void(_Tp thr_args)>;


template<typename _Tp>
struct __ThreadCtx
{
    explicit __ThreadCtx(ArgsThreadHandle<_Tp>&& thr_handle, _Tp&& thr_args)
        : thread_handle(thr_handle)
        , thread_args(thr_args)
    {

    }
    ArgsThreadHandle<_Tp> thread_handle;
    _Tp                   thread_args;
};

static unsigned int __stdcall GlobalWinThreadFunc(void* pArguments)
{
    TinyThreadHandle* thread_handle = reinterpret_cast<TinyThreadHandle*>(pArguments);
    if (thread_handle)
    {
        (*thread_handle)();
        delete thread_handle;
    }
    _endthreadex(0);
    return 0;
}

template<typename _Tp>
static unsigned int __stdcall GlobalWinThreadFunc_Args(void* pArguments)
{
    __ThreadCtx<_Tp>* thread_ctx = reinterpret_cast<__ThreadCtx<_Tp>*>(pArguments);
    if (thread_ctx)
    {
        if (thread_ctx->thread_handle)
        {
            thread_ctx->thread_handle(thread_ctx->thread_args);
        }
        delete thread_ctx;
    }
    _endthreadex(0);
    return 0;
}

class CQTZ_Thread
{
public:
    CQTZ_Thread(void) noexcept
        : m_threadID(0)
        , m_hThread(nullptr)
    {

    }
    // 无参数线程
    CQTZ_Thread(TinyThreadHandle handle, int stack_size = 0) noexcept
        : m_threadID(0)
        , m_hThread(nullptr)
    {
        TinyThreadHandle* thread_handle = new TinyThreadHandle(std::forward<TinyThreadHandle>(handle));
        m_hThread = (HANDLE)_beginthreadex(nullptr, (unsigned)stack_size, &GlobalWinThreadFunc, thread_handle, 0, &m_threadID);
        if (!m_hThread)
        {
            delete thread_handle;
        }
    }
    // 带参数线程
    template<typename _Tp>
    CQTZ_Thread(ArgsThreadHandle<_Tp> handle, _Tp args, int stack_size = 0) noexcept
        : m_threadID(0)
        , m_hThread(nullptr)
    {
        __ThreadCtx<_Tp>* thread_ctx = new __ThreadCtx<_Tp>(std::forward<ArgsThreadHandle<_Tp>>(handle), std::forward<_Tp>(args));
        m_hThread = (HANDLE)_beginthreadex(nullptr, (unsigned)stack_size, &GlobalWinThreadFunc_Args<_Tp>, thread_ctx, 0, &m_threadID);
        if (!m_hThread)
        {
            delete thread_ctx;
        }
    }

    ~CQTZ_Thread(void)
    {
        #ifdef _DEBUG
        assert(!joinable());
        #endif // _DEBUG
    }
    CQTZ_Thread(const CQTZ_Thread&) = delete;
    CQTZ_Thread(CQTZ_Thread&& _Other) noexcept : m_threadID(_Other.m_threadID), m_hThread(_Other.m_hThread)
    {
        _Other.m_threadID = 0;
        _Other.m_hThread  = nullptr;
    }
    CQTZ_Thread& operator=(const CQTZ_Thread&) = delete;
    CQTZ_Thread& operator=(CQTZ_Thread&& _Other) noexcept
    {
        if (this != &_Other)
        {
            #ifdef _DEBUG
            assert(!joinable());
            #endif
            if (nullptr != _Other.m_hThread)
            {
                m_threadID = _Other.m_threadID;
                m_hThread  = _Other.m_hThread;
                _Other.m_threadID = 0;
                _Other.m_hThread  = nullptr;
            }
        }
        return *this;
    }
    inline size_t get_id(void) const noexcept
    {
        return (size_t)(m_threadID);
    }
    inline bool joinable() const noexcept
    {
        return nullptr != m_hThread;
    }
    void join()
    {
        if (joinable())
        {
            ::WaitForSingleObject(m_hThread, INFINITE);
            ::CloseHandle(m_hThread);
            m_threadID = 0;
            m_hThread  = nullptr;
        }
    }
    inline void swap(CQTZ_Thread& _Other) noexcept
    {
        std::swap(m_threadID, _Other.m_threadID);
        std::swap(m_hThread,  _Other.m_hThread);
    }
private:
    unsigned int    m_threadID;
    HANDLE          m_hThread;
};
#endif
#endif // CQTZ_THREAD_WINDOWS_INCLUDE_
