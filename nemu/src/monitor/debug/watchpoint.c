#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"
#include "monitor/monitor.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp() {
	WP *f,*p;
	f = free_;
	p = head;
	free_ = free_->next;
	f->next = NULL;

	if (p == NULL) head = f;
	else {
		while (p->next!=NULL)p=p->next;
		p->next = f;
		}
	return f;
}

void free_wp (WP *wp) {
	//delete wp from head
	//add wp to free
	WP *f,*p;

	f = head;
	if (head == NULL) assert(0);
	if (head->NO == wp->NO) {
		head = head->next;  //wp is the head watchpoint
	}
	else {
	while (f->next != NULL && f->next->NO != wp->NO) f = f->next;
	//find the f, f->next = wp
	//use NO to judge
	if (f->next == NULL && f->NO == wp->NO) printf ("Such watchpoint doesn't exist");
	else if (f->next->NO == wp->NO) f->next = f->next->next; //delete wp
	else assert (0);
	}

	p = free_;
	if (p == NULL) free_ = wp;
	else {
		while (p->next!=NULL)p=p->next;
		p->next = wp;
		//or wp->next = free_; free_ = wp;
	}

	wp->next = NULL;
	wp->val = 0;
	wp->b = 0;
	wp->expr[0] = '\0';
}

bool check_wp() { //check if wp remains unchanged
	WP *f;
	f = head;
	bool key = true;
	bool suc;
	while (f != NULL)
	{
		uint32_t tmp_expr = expr (f->expr,&suc);
		if (!suc) assert (1);
		if (tmp_expr != f->val) //the value has changed
		{
			key = false;
			if (f->b)  //it is a breakpoint
			{
				printf ("Hit breakpoint %d at 0x%08x\n",f->b,cpu.eip);
				f = f->next;
				continue;
			}
			printf ("Watchpoint %d: %s\n",f->NO,f->expr);
			printf ("Old value = %d\n",f->val);
			printf ("New value = %d\n",tmp_expr);
			f->val = tmp_expr;
			//return ui_mainloop();
		}
		f = f->next;
	}
	return key;
}

void delete_wp(int num) {
	WP *f;
	f = &wp_pool[num];
	free_wp (f);
}

void info_wp() {
	WP *f;
	f=head;
	while (f!=NULL)
	{
		printf ("Watchpoint %d: %s = %d\n",f->NO,f->expr,f->val);
		f = f->next;
	}
}
