/***********************************************************************************************
// filename : 	OnTimer.h
// describ:		ontimer
// version:   	1.0V
************************************************************************************************/
#pragma once


#include "../inc/IPCDefine.h"

__NS_ZILLIZ_IPC_START
//////////////////////////////////////////////////////////////////////////
//Timing-Wheel 
//define the ontimer, timeout, 

#include <stdint.h>
#include <unordered_map>
#include <thread>
#include <mutex>
///////////////////////////////////////////////////////////////////////////////////////////
#define TIME_NEAR_SHIFT 8
#define TIME_NEAR (1 << TIME_NEAR_SHIFT)
#define TIME_LEVEL_SHIFT 6
#define TIME_LEVEL (1 << TIME_LEVEL_SHIFT)
#define TIME_NEAR_MASK (TIME_NEAR-1)
#define TIME_LEVEL_MASK (TIME_LEVEL-1)
struct timer_node {
	timer_node*			            m_next;
	uint32_t 			            m_expire;
	uint32_t			            m_nRepeatTime;
	uint8_t				            m_bIsValid;
    intptr_t                        m_nKey;
    std::function<void(intptr_t)>   m_callback;
};

struct link_list {
	timer_node	m_head;
	timer_node*	m_tail;
};
///////////////////////////////////////////////////////////////////////////////////////////
class OnTimer{
public:
    /*	precise, 1 = 1ms  10 = 10ms 100 = 100ms 1000 = 1s
    // the less value the more cpu used
    */
    OnTimer(uint16_t precise = 10);
	virtual ~OnTimer();

	bool InitTimer();
	void CloseTimer();

    intptr_t AddTimeOut(const std::function<void(intptr_t)>& func, int32_t nTimes);
    intptr_t AddOnTimer(const std::function<void(intptr_t)>& func, int32_t nTimes);
	void DelTimer(intptr_t nKey);

    //! addtime util true
    bool AddOnTimerUtil(const std::function<bool()>& func, int32_t nTimes);
protected:
	void TimerRun();

	void TimerUpdate();
protected:
    intptr_t timer_add(const std::function<void(intptr_t)>& func, int32_t time, int bRepeat);
	void timer_del(intptr_t nKey);

	timer_node* link_clear(link_list *list);
	void timer_execute(std::unique_lock<std::mutex>& lock);
	void timer_shift();
	void move_list(int level, int idx);
	void dispatch_list(struct timer_node *current);
	void add_node(timer_node *node);
	void linklist(link_list *list, timer_node *node);
protected:
    volatile bool                               m_bTimerExit;
    std::shared_ptr<std::thread>                m_thread_worker_ptr;
    link_list                                   m_near[TIME_NEAR];
    link_list                                   m_t[4][TIME_LEVEL];
    std::mutex                                  m_lock;
    uint32_t                                    m_time;
    uint64_t                                    m_current_point;
    std::unordered_map<intptr_t, timer_node*>   m_mapNode;

    std::chrono::milliseconds                   m_dura;
};

__NS_ZILLIZ_IPC_END
