#ifndef SECTION_LIST_H
#define SECTION_LIST_H

/* Section list operations */
int inf_section_find(const char far *section_name, char far * far * *lines, int *line_count);
void inf_sections_free_all(void);

/* INF file loading */
void load_inf_file(const char far *filename);

/* Section removal function */
int inf_section_remove(const char far *section_name);

#endif
