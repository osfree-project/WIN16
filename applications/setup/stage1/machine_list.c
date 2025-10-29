#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "machine_list.h"
#include "log.h"

static MachineListNode *machine_list_head = NULL;
static MachineListNode *machine_list_tail = NULL;
static int machine_list_size = 0;

void machine_list_init(void) {
    machine_list_head = NULL;
    machine_list_tail = NULL;
    machine_list_size = 0;
    log_message("machine_list: Initialized empty machine list");
}

void machine_list_add(const MachineEntry *entry) {
    MachineListNode *new_node;
    
    if (!entry) {
        log_message("machine_list: ERROR - null entry");
        return;
    }
    
    new_node = (MachineListNode*)malloc(sizeof(MachineListNode));
    if (!new_node) {
        log_message("machine_list: ERROR - memory allocation failed");
        return;
    }
    
    /* Copy the entry */
    memcpy(&new_node->entry, entry, sizeof(MachineEntry));
    new_node->next = NULL;
    
    /* Add to list */
    if (!machine_list_head) {
        machine_list_head = new_node;
        machine_list_tail = new_node;
    } else {
        machine_list_tail->next = new_node;
        machine_list_tail = new_node;
    }
    
    machine_list_size++;
    log_message("machine_list: Added machine entry '%Fs', total count: %d", 
               entry->description ? entry->description : "NULL", machine_list_size);
}

MachineEntry* machine_list_get(int index) {
    MachineListNode *current;
    int i;
    
    if (index < 0 || index >= machine_list_size) {
        return NULL;
    }
    
    current = machine_list_head;
    for (i = 0; i < index && current != NULL; i++) {
        current = current->next;
    }
    
    return current ? &current->entry : NULL;
}

int machine_list_count(void) {
    return machine_list_size;
}

/* NEW: Function to free a MachineEntry */
static void free_machine_entry(MachineEntry* entry) {
    int i;

    if (entry->description) _ffree(entry->description);
    if (entry->machine_id) _ffree(entry->machine_id);
    if (entry->system_drv) _ffree(entry->system_drv);
    if (entry->kbd_drv) _ffree(entry->kbd_drv);
    if (entry->kbd_type) _ffree(entry->kbd_type);
    if (entry->mouse_drv) _ffree(entry->mouse_drv);
    if (entry->disp_drv) _ffree(entry->disp_drv);
    if (entry->sound_drv) _ffree(entry->sound_drv);
    if (entry->comm_drv) _ffree(entry->comm_drv);
    if (entry->himem_switch) _ffree(entry->himem_switch);
    if (entry->ebios) _ffree(entry->ebios);
    for (i = 0; i < entry->cookie_count; i++) {
        if (entry->cookies[i]) _ffree(entry->cookies[i]);
    }
    entry->cookie_count = 0;
}

void machine_list_clear(void) {
    MachineListNode *current, *next;
    
    current = machine_list_head;
    while (current != NULL) {
        next = current->next;
        free_machine_entry(&current->entry);
        free(current);
        current = next;
    }
    
    machine_list_head = NULL;
    machine_list_tail = NULL;
    machine_list_size = 0;
    log_message("machine_list: Cleared all machine entries");
}

void machine_list_free(void) {
    machine_list_clear();
}
