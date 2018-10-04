#ifndef STORAGE_H
#define STORAGE_H
#include "config.h"

const struct CONFIG* read_config(void);
int write_config(struct CONFIG *config);

/* if leg spans a page boundary, then return the pointer to the start of the page *
 * otherwise return null */
void* leg_spans_boundary(struct LEG *leg);

int write_leg(struct LEG *leg);
#endif
