#include "OnTimerTest.h"
#include "ZIPC/base/time/OnTimer.h"

std::atomic<uint32_t> g_receive = { 0 };
std::atomic<uint32_t> g_maxdiff = { 0 };
bool IPCOnTimerTest() {
    {
        std::chrono::milliseconds dura(10);
        auto pOnTimer = new zilliz::lib::OnTimer(1);
        if (!pOnTimer->InitTimer())
            return false;

        g_receive.store(0);
        g_maxdiff.store(0);
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 10; j++) {
                intptr_t cp = (intptr_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                int32_t nUseTime = 10 * j;
                pOnTimer->AddTimeOut([cp, nUseTime](intptr_t nKey) {
                    intptr_t now = (intptr_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                    auto diff = abs(abs(now - cp) - nUseTime);
                    if (diff > g_maxdiff)
                        g_maxdiff = diff;
                    g_receive.fetch_add(1);
                }, nUseTime);
            }
            std::this_thread::sleep_for(dura);
        }
        
        while (g_receive != 1000){
            std::this_thread::sleep_for(dura);
        }
        printf("Max diff %d\n", g_maxdiff.load());
        if (g_maxdiff.load() >= 10){
            printf("OnTimer 1 precise diff(%d) > 10 \n", g_maxdiff.load());
            delete pOnTimer;
            return false;
        }
        delete pOnTimer;
    }
    {
        std::chrono::milliseconds dura(10);
        auto pOnTimer = new zilliz::lib::OnTimer(10);
        if (!pOnTimer->InitTimer())
            return false;

        g_receive.store(0);
        g_maxdiff.store(0);
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 10; j++) {
                intptr_t cp = (intptr_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                int32_t nUseTime = 100 * (j + 1);
                pOnTimer->AddTimeOut([cp, nUseTime](intptr_t nKey) {
                    intptr_t now = (intptr_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                    auto diff = abs(abs(now - cp) - nUseTime);
                    if (diff > 10)
                        g_maxdiff = diff;
                    if (diff > g_maxdiff)
                        g_maxdiff = diff;
                    
                    g_receive.fetch_add(1);
                }, nUseTime);
            }
            std::this_thread::sleep_for(dura);
        }

        while (g_receive != 1000) {
            std::this_thread::sleep_for(dura);
        }
        printf("Max diff %d\n", g_maxdiff.load());
        if (g_maxdiff.load() >= 20) {
            printf("OnTimer 10 precise diff(%d) > 20 \n", g_maxdiff.load());
            delete pOnTimer;
            return false;
        }
        delete pOnTimer;
    }
    {
        std::chrono::milliseconds dura(10);
        auto pOnTimer = new zilliz::lib::OnTimer(10);
        if (!pOnTimer->InitTimer())
            return false;

        g_receive.store(0);
        g_maxdiff.store(0);
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 10; j++) {
                intptr_t cp = (intptr_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                int32_t nUseTime = 1000 * (j + 1);
                pOnTimer->AddTimeOut([cp, nUseTime](intptr_t nKey) {
                    intptr_t now = (intptr_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                    auto diff = abs(abs(now - cp) - nUseTime);
                    if (diff > 10)
                        g_maxdiff = diff;
                    if (diff > g_maxdiff)
                        g_maxdiff = diff;

                    g_receive.fetch_add(1);
                }, 1000 * (j + 1));
            }
            std::this_thread::sleep_for(dura);
        }

        while (g_receive != 1000) {
            std::this_thread::sleep_for(dura);
        }
        printf("Max diff %d\n", g_maxdiff.load());
        if (g_maxdiff.load() >= 20) {
            printf("OnTimer 10 precise diff(%d) > 20 \n", g_maxdiff.load());
            delete pOnTimer;
            return false;
        }
        delete pOnTimer;
    }
    
    return true;
}