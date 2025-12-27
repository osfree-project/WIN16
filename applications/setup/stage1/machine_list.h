#ifndef MACHINE_LIST_H
#define MACHINE_LIST_H


typedef struct {
    char far *description;
    char far *machine_id;
    char far *system_drv;
    char far *kbd_drv;
    char far *kbd_type;
    char far *mouse_drv;
    char far *disp_drv;
    char far *sound_drv;
    char far *comm_drv;
    char far *himem_switch;
    char far *ebios;
    char far *cookies[10];
    int cookie_count;
} MachineEntry;

/* Dynamic machine list structure */
typedef struct MachineListNode {
    MachineEntry entry;
    struct MachineListNode *next;
} MachineListNode;

/* Machine list operations */
void machine_list_init(void);
void machine_list_add(const MachineEntry *entry);
MachineEntry* machine_list_get(int index);
int machine_list_count(void);
void machine_list_clear(void);
void machine_list_free(void);

#endif
