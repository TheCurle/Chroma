#pragma once
#include <stdbool.h>
#include <stddef.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

#ifdef __cplusplus
extern "C" {
#endif


/* This file provides a simple implementation of a ticket-based locking system.
 * You should probably prefer Spinlock over Ticketlock.
 *
 * Create a new lock with NEW_TICKETLOCK(),
 *  lock a resource with TicketLock().
 *
 * Use TicketUnlock() to free the resource after you are done.
 *
 */

typedef struct {
    size_t NowServing;
    size_t NextTicket;
} ticketlock_t;

#define NEW_TICKETLOCK()  (ticketlock_t{0})

void TicketLock(ticketlock_t* Lock);

bool TicketAttemptLock(ticketlock_t* Lock);

void TicketUnlock(ticketlock_t* Lock);

#ifdef  __cplusplus
}
#endif