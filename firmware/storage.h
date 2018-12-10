#ifndef STORAGE_H
#define STORAGE_H
#include "config.h"

const struct CONFIG* read_config(void);
int write_config(struct CONFIG *config);

int write_data(void *destination,  const void *source, int length);

#endif
