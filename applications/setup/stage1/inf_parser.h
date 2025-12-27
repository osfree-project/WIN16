#ifndef INF_PARSER_H
#define INF_PARSER_H

#include "machine_list.h"
#include "setup_core.h"

/* Machine list operations */
void machine_list_init(void);
void machine_list_add(const MachineEntry *entry);
MachineEntry* machine_list_get(int index);
int machine_list_count(void);
void machine_list_clear(void);
void machine_list_free(void);

/* INF parsing functions */
void preparse_inf_file(SetupConfig* config);
void parse_inf_file(const SetupConfig* config);
void parse_machine_section(void);
void parse_mouse_section(void);
void parse_kbdtype_section(void);
void parse_language_section(void);
void parse_network_section(void);

#endif
