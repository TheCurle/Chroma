#include <kernel/chroma.h>
#include <lainlib/lainlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void TicketLock(ticketlock_t* Lock) {
    size_t Ticket = atomic_fetch_add_explicit(&Lock->NextTicket, 1, memory_order_relaxed);

    while(atomic_load_explicit(&Lock->NowServing, memory_order_acquire) != Ticket) {
        PAUSE;
    }
}

bool TicketAttemptLock(ticketlock_t* Lock) {
    size_t Ticket = atomic_load_explicit(&Lock->NowServing, memory_order_relaxed);

    return atomic_compare_exchange_strong_explicit(&Lock->NowServing, &Ticket, Ticket + 1, memory_order_acquire, memory_order_relaxed);
}

void TicketUnlock(ticketlock_t* Lock) {
    size_t NextTicket = atomic_load_explicit(&Lock->NowServing, memory_order_relaxed) + 1;
    atomic_store_explicit(&Lock->NowServing, NextTicket, memory_order_release);
}

#ifdef  __cplusplus
}
#endif