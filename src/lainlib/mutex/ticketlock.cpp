#include <lainlib/mutex/ticketlock.h>

#define PAUSE   __asm__ __volatile__("pause")

#ifdef __cplusplus
extern "C" {
#endif

void TicketLock(ticketlock_t* Lock) {
    size_t Ticket = __atomic_fetch_add(&Lock->NextTicket, 1, __ATOMIC_RELAXED);
    __sync_synchronize();
    while(__atomic_load_8(&Lock->NowServing, __ATOMIC_ACQUIRE) != Ticket) {
        PAUSE;
    }
}

bool TicketAttemptLock(ticketlock_t* Lock) {
    size_t Ticket = __atomic_load_8(&Lock->NowServing, __ATOMIC_RELAXED);
    __sync_synchronize();
    return __atomic_compare_exchange_8(&Lock->NowServing, &Ticket, Ticket + 1, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
}

void TicketUnlock(ticketlock_t* Lock) {
    size_t NextTicket = __atomic_load_8(&Lock->NowServing, __ATOMIC_RELAXED) + 1;
    __sync_synchronize();
    __atomic_store_8(&Lock->NowServing, NextTicket, __ATOMIC_RELEASE);
}

#ifdef  __cplusplus
}
#endif