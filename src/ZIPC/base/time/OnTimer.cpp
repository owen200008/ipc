#include "ZIPC/base/time/OnTimer.h"
#include <chrono>
#include <thread>
#include "ZIPC/base/log/IPCLog.h"
#include "ZIPC/base/mt/SpinLock.h"

__NS_ZILLIZ_IPC_START
//////////////////////////////////////////////////////////////////////////
typedef void(*timer_execute_func)(void *ud, void *arg);


/////////////////////////////////////////////////////////////////////////////////
OnTimer::OnTimer(uint16_t nSleepMilliCount):m_dura(nSleepMilliCount){
	m_bTimerExit = false;

	memset(m_near, 0, sizeof(link_list) * TIME_NEAR);
	memset(m_t, 0, sizeof(link_list) * TIME_LEVEL * 4);
	m_time = 0;
	m_current_point = 0;

	m_pMapNode = new MapTimerNode();
}

OnTimer::~OnTimer(){
	if (!m_bTimerExit){
		CloseTimer();
	}
	if (m_pMapNode){
		delete m_pMapNode;
		m_pMapNode = nullptr;
	}
}

void OnTimer::TimerRun(){
	while (!m_bTimerExit){
		TimerUpdate();
        std::this_thread::sleep_for(m_dura);
	}
}

void OnTimer::TimerUpdate(){
	uint64_t cp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 10;
	if (cp < m_current_point){
        LogFuncLocation(IPCLog_Error, "time diff error: change from %lld to %lld", cp, m_current_point);
		m_current_point = cp;
	}
	else if (cp != m_current_point){
		uint32_t diff = (uint32_t)(cp - m_current_point);
		m_current_point = cp;
		for (uint32_t i = 0; i < diff; i++){
            std::unique_lock<std::mutex> lock(m_lock);

			// try to dispatch timeout 0 (rare condition)
			timer_execute(lock);

			// shift time first, and then dispatch timer message
			timer_shift();

			timer_execute(lock);
		}
	}
}

void OnTimer::timer_execute(std::unique_lock<std::mutex>& lock){
	int idx = m_time & TIME_NEAR_MASK;
	while (m_near[idx].head.next){
		timer_node *current = link_clear(&m_near[idx]);
        lock.unlock();
		// dispatch_list don't need lock T
		dispatch_list(current);
		lock.lock();
	}
}

void OnTimer::timer_shift(){
	int mask = TIME_NEAR;
	uint32_t ct = ++m_time;
	if (ct == 0){
		move_list(3, 0);
	}
	else{
		uint32_t time = ct >> TIME_NEAR_SHIFT;
		int i = 0;

		while ((ct & (mask - 1)) == 0){
			int idx = time & TIME_LEVEL_MASK;
			if (idx != 0){
				move_list(i, idx);
				break;
			}
			mask <<= TIME_LEVEL_SHIFT;
			time >>= TIME_LEVEL_SHIFT;
			++i;
		}
	}
}

void OnTimer::move_list(int level, int idx){
	timer_node *current = link_clear(&m_t[level][idx]);
	while (current){
		timer_node *temp = current->next;
		add_node(current);
		current = temp;
	}
}

void OnTimer::dispatch_list(struct timer_node *current){
	do
	{
		timer_event * event = (struct timer_event *)(current + 1);
		if (current->m_bIsValid){
			event->m_callbackFunc(event->m_nKey, event->m_pParam1);
		}
		timer_node* temp = current;
		current = current->next;
		if (temp->m_nRepeatTime > 0 && temp->m_bIsValid){
			//spin unlock, so add is valid
			temp->expire += temp->m_nRepeatTime;

            std::unique_lock<std::mutex> lock(m_lock);
			add_node(temp);
		}
		else{
			free(temp);
		}
	} while (current);
}

void OnTimer::timer_add(timer_event& event, int time, int bRepeat){
	int sz = sizeof(event);
	struct timer_node *node = (struct timer_node *)malloc(sizeof(*node) + sz);
	memcpy(node + 1, &event, sz);
	node->m_bIsValid = 1;
	node->m_nRepeatTime = bRepeat > 0 ? time : 0;
    std::unique_lock<std::mutex> lock(m_lock);
	node->expire = time + m_time;
	if (bRepeat > 0){
		(*m_pMapNode)[event.m_nKey] = node;
	}
	add_node(node);
}

void OnTimer::timer_del(intptr_t nKey){
    std::unique_lock<std::mutex> lock(m_lock);
	MapTimerNode::iterator iter = (*m_pMapNode).find(nKey);
	if (iter != (*m_pMapNode).end()){
		timer_node* pNode = iter->second;
		pNode->m_bIsValid = 0;
		(*m_pMapNode).erase(iter);
	}
}

void OnTimer::add_node(timer_node *node){
	uint32_t time = node->expire;
	uint32_t current_time = m_time;

	if ((time | TIME_NEAR_MASK) == (current_time | TIME_NEAR_MASK)){
		linklist(&m_near[time&TIME_NEAR_MASK], node);
	}
	else{
		int i;
		uint32_t mask = TIME_NEAR << TIME_LEVEL_SHIFT;
		for (i = 0; i < 3; i++){
			if ((time | (mask - 1)) == (current_time | (mask - 1))){
				break;
			}
			mask <<= TIME_LEVEL_SHIFT;
		}

		linklist(&m_t[i][((time >> (TIME_NEAR_SHIFT + i*TIME_LEVEL_SHIFT)) & TIME_LEVEL_MASK)], node);
	}
}

void OnTimer::linklist(link_list *list, timer_node *node){
	list->tail->next = node;
	list->tail = node;
	node->next = 0;
}

bool OnTimer::InitTimer(){
	if (m_thread_worker_ptr)
		return false;
	int i;
	for (i = 0; i < TIME_NEAR; i++){
		link_clear(&m_near[i]);
	}
	for (i = 0; i < 4; i++){
		for (int j = 0; j < TIME_LEVEL; j++){
			link_clear(&m_t[i][j]);
		}
	}
	m_current_point = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 10;

    m_thread_worker_ptr = std::make_shared<std::thread>(&OnTimer::TimerRun, this);
	return true;
}

void OnTimer::WaitThreadExit(){
    if (m_thread_worker_ptr != nullptr)
        m_thread_worker_ptr->join();
}

void OnTimer::CloseTimer(){
	m_bTimerExit = true;
}

bool OnTimer::AddTimeOut(intptr_t nKey, pOnTimerCallback pFunc, int nTimes, intptr_t pParam1){
	if (nTimes <= 0){
		pFunc(nKey, pParam1);
	}
	else{
		struct timer_event event;
		event.m_callbackFunc = pFunc;
		event.m_nKey = nKey;
		event.m_pParam1 = pParam1;
		timer_add(event, nTimes, 0);
	}
	return true;
}

bool OnTimer::AddOnTimer(intptr_t nKey, pOnTimerCallback pFunc, int nTimes, intptr_t pParam1) {
	if (nTimes > 0) {
		struct timer_event event;
		event.m_callbackFunc = pFunc;
		event.m_nKey = nKey;
		event.m_pParam1 = pParam1;
		timer_add(event, nTimes, 1);
		return true;
	}
	return false;
}

void OnTimer::DelTimer(intptr_t nKey){
	timer_del(nKey);
}

timer_node* OnTimer::link_clear(link_list *list){
	timer_node * ret = list->head.next;
	list->head.next = 0;
	list->tail = &(list->head);

	return ret;
}

__NS_ZILLIZ_IPC_END