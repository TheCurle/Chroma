#include <stdbool.h>

typedef struct list_entry {
    struct list_entry* Previous;
    struct list_entry* Next;
} list_entry_t;

#define UNSAFE_CAST(ptr, type, member) \
    ((type*)((char*)(ptr) - (char*)offsetof(type, member)))

#define LISTNEW(var) \
    ((list_entry_t){ 0, 0 })

void ListAdd(list_entry_t* Head, list_entry_t* New);

void ListEmplaceBack(list_entry_t* Head, list_entry_t* Tail);

void ListRemove(list_entry_t* List);

bool ListIsEmpty(list_entry_t* Head);

#define LISTNEXT(current, member) \
    UNSAFE_CAST((current)->member.next, typeof(*(current)), member);

#define LISTPREV(current, member) \
    UNSAFE_CAST((current)->member.prev, typeof(*(curent)), member)

#define LISTFOREACH(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define LISTFOREACHENTRY(pos, head, member) \
    for(pos = UNSAFE_CAST((head)->next, typeof(*(pos)), member); &pos->member != (head); pos = LISTNEXT(pos, member))

#define LASTENTRY 0
