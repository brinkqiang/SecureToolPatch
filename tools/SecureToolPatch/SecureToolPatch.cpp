
#include "execute.h"
#include "dmutil.h"
#include "dmlog.hpp"
#include "dmstrtk.hpp"
#include "dmflags.h"

#include "dmutil.h"
#include "dmtimermodule.h"
#include "dmsingleton.h"
#include "dmthread.h"
#include "dmconsole.h"
#include "dmtypes.h"

class CMain : public IDMConsoleSink,
    public IDMThread,
    public CDMThreadCtrl,
    public CDMTimerNode,
    public TSingleton<CMain>
{
    friend class TSingleton<CMain>;

    typedef enum
    {
        eTimerID_UUID = 0,
        eTimerID_CRON = 1,
        eTimerID_STOP,
    } ETimerID;

    typedef enum
    {
        eTimerTime_UUID = 1000,
    } ETimerTime;

public:
    virtual void ThrdProc()
    {
        m_execute = executeGetModule();

        if (NULL != m_execute)
        {
            std::string strSecureToolPathFmt =
                R"({}:\Users\{}\AppData\Roaming\VanDyke\Config\Sessions)";

            std::string strUserName = getenv("username");
            std::string strSecureToolPath;

            for (char a = 'C'; a <= 'Z'; a++)
            {
                auto path = fmt::format(strSecureToolPathFmt, a, strUserName);

                if (DMIsDirectory(path.c_str()))
                {
                    strSecureToolPath = path;
                    break;
                }
            }

            if (strSecureToolPath.empty())
            {
                fmt::print("{} {}", "cann't find", strSecureToolPathFmt);
                return;
            }

            std::string strCmd = fmt::format(R"(sed -i "s/\"Filenames Always Use UTF8\"=00000000/\"Filenames Always Use UTF8\"=00000001/g" {}\*.ini)", strSecureToolPath);
            std::string strRet = m_execute->exec(strCmd);

            fmt::print("Done");
        }
    }

    virtual void Terminate()
    {
        m_bStop = true;
    }

    virtual void OnCloseEvent()
    {
        Stop();
    }

    virtual void OnTimer(uint64_t qwIDEvent, dm::any& oAny)
    {
        switch (qwIDEvent)
        {
        case eTimerID_STOP:
        {
            KillTimer(qwIDEvent);
            Stop();
        }
        break;

        default:
            break;
        }
    }

private:
    CMain()
        : m_bStop(false), m_execute(nullptr), m_bVsp(false)
    {
        HDMConsoleMgr::Instance()->SetHandlerHook(this);
    }

    virtual ~CMain()
    {
        if (m_execute)
        {
            m_execute->Release();
        }
    }

private:
    bool __Run()
    {
        return false;
    }

private:
    volatile bool m_bStop;
    volatile bool m_bVsp;
    Iexecute* m_execute;
};

int main(int argc, char* argv[])
{
    DMFLAGS_INIT(argc, argv);
    CMain::Instance()->Start(CMain::Instance());
    CMain::Instance()->WaitFor();
    return 0;
}
