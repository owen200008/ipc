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
typedef void(*pOnTimerCallback)(intptr_t, intptr_t pParam1);
///////////////////////////////////////////////////////////////////////////////////////////
#define TIME_NEAR_SHIFT 8
#define TIME_NEAR (1 << TIME_NEAR_SHIFT)
#define TIME_LEVEL_SHIFT 6
#define TIME_LEVEL (1 << TIME_LEVEL_SHIFT)
#define TIME_NEAR_MASK (TIME_NEAR-1)
#define TIME_LEVEL_MASK (TIME_LEVEL-1)

struct timer_event {
	pOnTimerCallback	m_callbackFunc;
	intptr_t			m_nKey;
	intptr_t			m_pParam1;
};

struct timer_node {
	timer_node*			next;
	uint32_t 			expire;
	uint32_t			m_nRepeatTime;
	uint8_t				m_bIsValid;
};

struct link_list {
	timer_node	head;
	timer_node*	tail;
};
typedef std::unordered_map<intptr_t, timer_node*>	 MapTimerNode;
///////////////////////////////////////////////////////////////////////////////////////////
class OnTimer{
public:
    OnTimer(uint16_t nSleepMilliCount = 10);
	virtual ~OnTimer();

	bool InitTimer();
	void CloseTimer();
	void WaitThreadExit();

	/*	最高精度10ms, 实际误差25ms(sleep)
		100=1s 
		实际100ms精度内的无法完全准确
	*/
	bool AddTimeOut(intptr_t nKey, pOnTimerCallback pFunc, int nTimes, intptr_t pParam1);
	bool AddOnTimer(intptr_t nKey, pOnTimerCallback pFunc, int nTimes, intptr_t pParam1);
	void DelTimer(intptr_t nKey);
protected:
	void TimerRun();

	void TimerUpdate();
protected:
	void timer_add(timer_event& event, int time, int bRepeat);
	void timer_del(intptr_t nKey);

	timer_node* link_clear(link_list *list);
	void timer_execute(std::unique_lock<std::mutex>& lock);
	void timer_shift();
	void move_list(int level, int idx);
	void dispatch_list(struct timer_node *current);
	void add_node(timer_node *node);
	void linklist(link_list *list, timer_node *node);
protected:
	bool							m_bTimerExit;
    std::shared_ptr<std::thread>    m_thread_worker_ptr;

	link_list                		m_near[TIME_NEAR];
	link_list                		m_t[4][TIME_LEVEL];
    std::mutex                      m_lock;
	uint32_t                        m_time;
	uint64_t                        m_current_point;
	MapTimerNode*					m_pMapNode;

    std::chrono::milliseconds       m_dura;
};

__NS_ZILLIZ_IPC_END
