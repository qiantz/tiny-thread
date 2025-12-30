#ifndef CQTZ_THREAD_LINUX_INCLUDE_
#define CQTZ_THREAD_LINUX_INCLUDE_
#if !(defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) || defined(_WINDOWS) || defined(_WINDOWS_))
#include <assert.h>
#include <functional>
#include <pthread.h>

using TinyThreadHandle = std::function<void(void)>;
template<typename _Tp>
using ArgsThreadHandle = std::function<void(_Tp thr_args)>;

const pthread_t CONST_INVALID_PTHREAD = -1;

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

static void* GlobalLinuxThreadFunc(void* pArguments)
{
    TinyThreadHandle* thread_handle = reinterpret_cast<TinyThreadHandle*>(pArguments);
    if (thread_handle)
    {
        (*thread_handle)();
        delete thread_handle;
    }
    pthread_exit(nullptr);
    return nullptr;
}

template<typename _Tp>
static void* GlobalLinuxThreadFunc_Args(void* pArguments)
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
    pthread_exit(nullptr);
    return nullptr;
}

class CQTZ_Thread
{
public:
    CQTZ_Thread(void) noexcept
    : m_hThread(CONST_INVALID_PTHREAD)
    , m_thrAttr(nullptr)
    {

    }
    // 无参线程
    CQTZ_Thread(TinyThreadHandle handle, int stack_size = 0) noexcept
    : m_hThread(CONST_INVALID_PTHREAD)
    , m_thrAttr(nullptr)
    {
        if(stack_size)
        {
            m_thrAttr = new pthread_attr_t();
            if(m_thrAttr)
            {
                if(0 != pthread_attr_init(m_thrAttr))
                {
                   delete m_thrAttr;
                   m_thrAttr = nullptr;
                }
                else
                {
                    pthread_attr_setstacksize(m_thrAttr, stack_size);
                }
            }
            if(!m_thrAttr) return;
        }

        TinyThreadHandle* thread_hadle = new TinyThreadHandle(std::forward<TinyThreadHandle>(handle));
        if (0 != pthread_create(&m_hThread, m_thrAttr, GlobalLinuxThreadFunc, thread_hadle))
        {
           delete thread_hadle;
           free_attr();
        }
    }
    // 带参数线程
    template<typename _Tp>
    CQTZ_Thread(ArgsThreadHandle<_Tp> handle, _Tp args, int stack_size = 0) noexcept
    : m_hThread(CONST_INVALID_PTHREAD)
    , m_thrAttr(nullptr)
    {
        if(stack_size)
        {
            m_thrAttr = new pthread_attr_t();
            if(m_thrAttr)
            {
                if(0 != pthread_attr_init(m_thrAttr))
                {
                   delete m_thrAttr;
                   m_thrAttr = nullptr;
                }
                else
                {
                    pthread_attr_setstacksize(m_thrAttr, stack_size);
                }
            }
            if(!m_thrAttr) return;
        }

        __ThreadCtx<_Tp>* thread_ctx = new __ThreadCtx<_Tp>(std::forward<ArgsThreadHandle<_Tp>>(handle), std::forward<_Tp>(args));
        if (0 != pthread_create(&m_hThread, m_thrAttr, GlobalLinuxThreadFunc_Args<_Tp>, thread_ctx))
        {
           delete thread_ctx;
           free_attr();
        }
    }

    ~CQTZ_Thread(void)
    {
       #ifdef _DEBUG
       assert(!joinable());
       #endif // _DEBUG
    }
    CQTZ_Thread(const CQTZ_Thread&) = delete;
    CQTZ_Thread(CQTZ_Thread&& _Other) noexcept : m_hThread(_Other.m_hThread),  m_thrAttr(_Other.m_thrAttr)
    {
        _Other.m_hThread = CONST_INVALID_PTHREAD;
        _Other.m_thrAttr = nullptr;
    }
    CQTZ_Thread& operator=(const CQTZ_Thread&) = delete;
    CQTZ_Thread& operator=(CQTZ_Thread&& _Other) noexcept
    {
        if (this != &_Other)
        {
            #ifdef _DEBUG
            assert(!joinable());
            #endif
            if (CONST_INVALID_PTHREAD != _Other.m_hThread)
            {
                m_hThread       = _Other.m_hThread;
                m_thrAttr       = _Other.m_thrAttr;
               _Other.m_hThread = CONST_INVALID_PTHREAD;
               _Other.m_thrAttr = nullptr;
            }
        }
        return *this;
    }
    inline size_t get_id(void) const noexcept
    {
        return (size_t)(m_hThread);
    }
    inline bool joinable() const noexcept
    {
        return CONST_INVALID_PTHREAD != m_hThread;
    }
    void join()
    {
        if (joinable())
        {
            pthread_join(m_hThread, nullptr);
            m_hThread = CONST_INVALID_PTHREAD;
            free_attr();
        }
    }
    inline void swap(CQTZ_Thread& _Other) noexcept
    {
        std::swap(m_hThread, _Other.m_hThread);
        std::swap(m_thrAttr, _Other.m_thrAttr);
    }
    inline void free_attr(void)
    {
        if(m_thrAttr)
        {
            pthread_attr_destroy(m_thrAttr);
            delete m_thrAttr;
            m_thrAttr = nullptr;
        }
    }
private:
    pthread_t       m_hThread;
    pthread_attr_t* m_thrAttr;
};
#endif
#endif // CQTZ_THREAD_LINUX_INCLUDE_
