#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;

	/* TODO: Add more members if necessary */

	uint32_t val; //to store the old value
	char expr [32]; //to store the expression
	int b; //it is a breakpoint

} WP;

WP* new_wp ();
void free_wp(WP *);
bool check_wp();
void delete_wp(int );
void info_wp();
#endif
