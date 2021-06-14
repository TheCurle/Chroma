#pragma once
#include <stdbool.h>
#include <stdatomic.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

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
    atomic_size_t NowServing;
    atomic_size_t NextTicket;
} ticketlock_t;

#define NEW_TICKETLOCK()  (ticketlock_t{0})

void TicketLock(ticketlock_t* Lock);

bool TicketAttemptLock(ticketlock_t* Lock);

void TicketUnlock(ticketlock_t* Lock);

