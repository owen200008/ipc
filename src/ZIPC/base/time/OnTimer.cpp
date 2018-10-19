#include "ZIPC/base/time/OnTimer.h"
#include <chrono>
#include <thread>
#include "ZIPC/base/log/IPCLog.h"
#include <cstring>

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
}

OnTimer::~OnTimer(){
	if (!m_bTimerExit){
		CloseTimer();
	}
}

void OnTimer::TimerRun(){
	while (!m_bTimerExit){
		TimerUpdate();
        std::this_thread::sleep_for(m_dura);
	}
}

void OnTimer::TimerUpdate(){
	uint64_t cp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / m_dura.count();
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
	while (m_near[idx].m_head.m_next){
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
		timer_node *temp = current->m_next;
		add_node(current);
		current = temp;
	}
}

void OnTimer::dispatch_list(struct timer_node *current){
	do
	{
		//timer_event * event = (struct timer_event *)(current + 1);
		if (current->m_bIsValid){
            current->m_callback(current->m_nKey);
		}
		timer_node* temp = current;
		current = current->m_next;
		if (temp->m_nRepeatTime > 0 && temp->m_bIsValid){
			//spin unlock, so add is valid
			temp->m_expire += temp->m_nRepeatTime;

            std::unique_lock<std::mutex> lock(m_lock);
			add_node(temp);
		}
		else{
            timer_del(temp->m_nKey);
			delete temp;
		}
	} while (current);
}

intptr_t OnTimer::timer_add(const std::function<void(intptr_t)>& func, int32_t time, int bRepeat){
	timer_node* pNode = new timer_node();
    pNode->m_callback = func;
    pNode->m_nKey = (intptr_t)pNode;
    pNode->m_bIsValid = 1;
    pNode->m_nRepeatTime = bRepeat > 0 ? time : 0;
    std::unique_lock<std::mutex> lock(m_lock);
    pNode->m_expire = time + m_time;
	if (bRepeat > 0){
        m_mapNode[pNode->m_nKey] = pNode;
	}
	add_node(pNode);
    return pNode->m_nKey;
}

void OnTimer::timer_del(intptr_t nKey){
    std::unique_lock<std::mutex> lock(m_lock);
	auto iter = m_mapNode.find(nKey);
	if (iter != m_mapNode.end()){
		timer_node* pNode = iter->second;
		pNode->m_bIsValid = 0;
        m_mapNode.erase(iter);
	}
}

void OnTimer::add_node(timer_node *node){
	uint32_t time = node->m_expire;
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
	list->m_tail->m_next = node;
	list->m_tail = node;
	node->m_next = 0;
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
	m_current_point = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / m_dura.count();

    m_thread_worker_ptr = std::make_shared<std::thread>(&OnTimer::TimerRun, this);
	return true;
}

void OnTimer::CloseTimer(){
	m_bTimerExit = true;
    if (m_thread_worker_ptr != nullptr)
        m_thread_worker_ptr->join();
}

intptr_t OnTimer::AddTimeOut(const std::function<void(intptr_t)>& func, int32_t nTimes){
	if (nTimes <= 0){
        func(0);
        return 0;
	}
    nTimes /= (int32_t)m_dura.count();
    if (nTimes == 0)
        nTimes = 1;
    return timer_add(func, nTimes, 0);
}

intptr_t OnTimer::AddOnTimer(const std::function<void(intptr_t)>& func, int32_t nTimes) {
	if (nTimes > 0) {
        nTimes /= (int32_t)m_dura.count();
        if (nTimes == 0)
            nTimes = 1;
		return timer_add(func, nTimes, 1);
	}
	return 0;
}

bool OnTimer::AddOnTimerUtil(const std::function<bool()>& func, int32_t nTimes) {
    if (nTimes <= 0)
        return false;
    if (func())
        return true;
    intptr_t nKey = AddOnTimer([this, &func](intptr_t nKey) {
        if (func()) {
            DelTimer(nKey);
        }
    }, nTimes);
    return nKey != 0;
}

void OnTimer::DelTimer(intptr_t nKey){
	timer_del(nKey);
}

timer_node* OnTimer::link_clear(link_list *list){
	timer_node * ret = list->m_head.m_next;
	list->m_head.m_next = 0;
	list->m_tail = &(list->m_head);

	return ret;
}

__NS_ZILLIZ_IPC_END