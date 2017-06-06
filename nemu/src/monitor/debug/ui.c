#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <elf.h>

void cpu_exec(uint32_t);

extern char *strtab;
extern Elf32_Sym *symtab;
extern int nr_symtab_entry;

typedef struct {
    swaddr_t prev_ebp;
    swaddr_t ret_addr;
    uint32_t args[4];
} PartOfStackFrame;

/* We use the ``readline'' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args)
{
    int n = 1;
    if (args) {
        if (sscanf(args, "%d", &n) != 1) {
            puts(" Unrecognized number! Try again!");
            return 1;
        }
    }
    cpu_exec(n);
    return 0;
}

static int cmd_info_r();  //define first, use later
static int cmd_info_w();
static int cmd_info(char *args) {
    if (!args) {
        printf("info r : Show information of [r]egister\ninfo w : Show information of [w]atchpoint");
    } else {
        switch (args[0]) {
            case 'r': return cmd_info_r();
						case 'w': return cmd_info_w();
        }
        printf("Invalid argmuent: %s\n", args);
    }
    return 0;
}

static int cmd_info_r() {
    for (int i = 0; i < 8; i++) {
        printf("%s\t%#10x\t%10d\t", regsl[i], reg_l(i), reg_l(i));
        printf("%s\t%#6x\t%5d", regsw[i], reg_w(i), reg_w(i));
        if (i < 4) {
            printf("\t%s\t%#4x\t%3d\t", regsb[i|4], reg_b(i|4), reg_b(i|4)); // 4 = 0100B
            printf("%s\t%#4x\t%3d\n", regsb[i], reg_b(i), reg_b(i));
        } else printf("\n");
    }

    printf("eflags\t%#10x \n", cpu.eflags);
/*
    printf("eflags\t%#10x [%s%s%s%s%s%s%s]\n", cpu.eflags,
            cpu.cf ? "CF" : "",
            cpu.pf ? "PF" : "",
            cpu.zf ? "ZF" : "",
            cpu.sf ? "SF" : "",
            cpu.ief? "IF" : "",
            cpu.df ? "DF" : "",
            cpu.of ? "OF" : ""
    );
*/
    printf("eip\t%#10x\n", cpu.eip);
		return 0;
}

static int cmd_info_w() {
    info_wp();
    return 0;
}


int breakpoint_counter = 1;
static int cmd_b(char *args) {
	bool suc;
	swaddr_t addr;
	addr = expr (args+1 , &suc); //there is a '+' so args+1
	if (!suc)assert (1);
	sprintf (args,"$eip == 0x%x",addr);
	printf ("Breakpoint %d at 0x%x\n",breakpoint_counter-1,addr);
	WP *f;
	f = new_wp();
	f->val = expr (args,&suc);
	f->b = breakpoint_counter;
	breakpoint_counter++;
	strcpy (f->expr,args);
	return 0;
}

static int cmd_x(char *args) {
    size_t ins = 0;
    bool suc;
    swaddr_t addr = 0;

    //skip the ' '
    while (args && *args != 0 && *args == ' ') args++;

    //read the first number
    for (; args && *args && isdigit(*args); args++) {
        ins = ins * 10 + *args - '0';
    }

    //read the adress
    while (args && *args != 0 && *args == ' ') args++;
    //sscanf(args, "%x", &addr);  scanf the Hnumber
    addr = expr(args,&suc);  //use the expr to calculate

    if (suc)
	while(ins--){
        printf("%#x:\t %08x \n", addr, swaddr_read(addr, 4));
				addr = addr+4;
    }
	else assert (0);
    return 0;
}

static int cmd_p(char *args) {
	uint32_t num ;
	bool suc;

	num = expr(args,&suc);
	if (suc) printf ("0x%x:\t%d\n",num,num);
	else assert (0);
	return 0;
}

static int cmd_w(char *args) {
	WP *f;
	bool suc;
	f = new_wp();
	printf ("Watchpoint %d: %s\n",f->NO,args);
	f->val = expr (args,&suc);
	strcpy (f->expr,args);
	if (!suc) Assert (1,"Wrong\n");
	printf ("Value : %d\n",f->val);
	return 0;
}

static int cmd_d(char *args) {
	int num;
	sscanf (args,"%d",&num);
	delete_wp (num);
	return 0;
}

static void read_ebp (swaddr_t addr , PartOfStackFrame *ebp)
{
	current_sreg = R_SS;
	ebp -> prev_ebp = swaddr_read (addr , 4);
	ebp -> ret_addr = swaddr_read (addr + 4 , 4);
	int i;
	for (i = 0;i < 4;i ++)
	{
		ebp -> args [i] = swaddr_read (addr + 8 + 4 * i , 4);
	}
}
static int cmd_bt(char *args) {
	int i,j = 0;
	PartOfStackFrame now_ebp;
	char tmp [32];
	int tmplen;
	swaddr_t addr = reg_l (R_EBP);
	now_ebp.ret_addr = cpu.eip;
	while (addr > 0)
	{
		printf ("#%d  0x%08x in ",j++,now_ebp.ret_addr);
		for (i=0;i<nr_symtab_entry;i++)
		{
			if (symtab[i].st_value <= now_ebp.ret_addr && symtab[i].st_value +  symtab[i].st_size >= now_ebp.ret_addr && (symtab[i].st_info&0xf) == STT_FUNC)
			{
				tmplen = symtab[i+1].st_name - symtab[i].st_name - 1;
				strncpy (tmp,strtab+symtab[i].st_name,tmplen);
				tmp [tmplen] = '\0';
				break;
			}
		}
		printf("%s\t",tmp);
		read_ebp (addr,&now_ebp);
		if (strcmp (tmp,"main") == 0)printf ("( )\n");
		else printf ("( %d , %d , %d , %d )\n", now_ebp.args[0],now_ebp.args[1],now_ebp.args[2],now_ebp.args[3]);
		addr = now_ebp.prev_ebp;

	}
	return 0;
}

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },

	{ "si", "Run single instruction", cmd_si },
	{ "info", "Show information of [r]egister and [w]atchpoint", cmd_info },
	{ "x", "Calculate the value of the expression and regard the result as the starting memory address.", cmd_x},
	{ "b", "Breakpoint + *ADDR.", cmd_b},
	{ "p", "Expression evaluation", cmd_p},
	{ "w", "Stop the execution of the program if the result of the expression has changed.", cmd_w},
	{ "d", "Delete the Nth watchpoint", cmd_d},
	{ "bt", "Print stack frame chain", cmd_bt}

	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
