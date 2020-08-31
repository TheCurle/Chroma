#include <lainlib/list/list.h>

void ListAdd(list_entry_t* Head, list_entry_t* New) {
    New->Next = Head->Next;
    New->Previous = Head;
    New->Next->Previous = New;
    Head->Next = New;
}

void ListEmplaceBack(list_entry_t* Head, list_entry_t* New) {
    New->Next = Head;
    New->Previous = Head->Previous;
    New->Previous->Next = New;
    Head->Previous = New;
}

void ListRemove(list_entry_t* Entry) {
    Entry->Next->Previous = Entry->Previous;
    Entry->Previous->Next = Entry->Next;

    Entry->Previous = (void*)0xDEADull;
    Entry->Next = (void*)0xBEEFull;
}

bool ListIsEmpty(list_entry_t* Head) {
    return Head->Next == Head;
}