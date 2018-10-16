/***********************************************************************************************
// filename : 	AtomicBackOff.h
// describ:		auto change length
// version:   	1.0V
************************************************************************************************/
#pragma once

#include <emmintrin.h>
#include <thread>

__NS_ZILLIZ_IPC_START
//////////////////////////////////////////////////////////////////////////
class AtomicBackOff {
        //! Time delay, in units of "pause" instructions.
        /** Should be equal to approximately the number of "pause" instructions
        that take the same time as an context switch. Must be a power of two.*/
        static const int32_t LOOPS_BEFORE_YIELD = 16;
        int32_t count;
    public:
        // In many cases, an object of this type is initialized eagerly on hot path,
        // as in for(atomic_backoff b; ; b.pause()) { /*loop body*/ }
        // For this reason, the construction cost must be very small!
        AtomicBackOff() : count(1) {}
        // This constructor pauses immediately; do not use on hot paths!
        AtomicBackOff(bool) : count(1) { 
        	Pause(); 
        }

        static inline void Pause(int32_t delay) {
            for (; delay>0; --delay)
                _mm_pause();
        }
        //! Pause for a while.
        void Pause() {
            if (count <= LOOPS_BEFORE_YIELD) {
                Pause(count);
                // Pause twice as long the next time.
                count *= 2;
            }
            else {
                // Pause is so long that we might as well yield CPU to scheduler.
                std::this_thread::yield();
            }
        }

        void SwapThread() {
            std::this_thread::yield();
        }

        //! Pause for a few times and return false if saturated.
        bool BoundedPause() {
            Pause(count);
            if (count<LOOPS_BEFORE_YIELD) {
                // Pause twice as long the next time.
                count *= 2;
                return true;
            }
            else {
                return false;
            }
        }

        void Reset() {
            count = 1;
        }
};


__NS_ZILLIZ_IPC_END