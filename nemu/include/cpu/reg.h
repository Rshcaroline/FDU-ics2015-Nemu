#ifndef __REG_H__
#define __REG_H__

#include "common.h"

enum { R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI };
enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };
enum { R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH };
enum { R_ES, R_CS, R_SS, R_DS, R_FS, R_GS };

/*
struct SR {
    uint16_t sel;
    uint32_t base;
    uint32_t limit;
//    uint16_t access;
};
*/

struct SREG{
		uint16_t selector;
		union {
			struct {
				uint32_t seg_base1 :16;
				uint32_t seg_base2 :8;
				uint32_t seg_base3 :8;
			};
			uint32_t seg_base;
		};
		union {
			struct {
				uint32_t seg_limit1 :16;
				uint32_t seg_limit2 :4;
				uint32_t seg_limit3 :12;
			};
			uint32_t seg_limit;
		};
};

typedef struct {
    // General-purpose registers
    union {
        union {
            uint32_t _32;
            uint16_t _16;
            uint8_t _8[2];
        } gpr[8];

        struct {
            /* Do NOT change the order of the GPRs' definitions. */
            uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
        };
    };


    // Segment registers
    union {
        struct SREG sr[6];
        struct {
            struct SREG es, cs, ss, ds, fs, gs;
        };
    };


    // EFLAGS
    struct {
        union {
            uint32_t eflags;
            struct {
                uint8_t CF : 1;
                uint8_t __reserved1 : 1;
                uint8_t PF : 1;
                uint8_t __reserved2 : 1;
                uint8_t AF : 1;
                uint8_t __reserved3 : 1;
                uint8_t ZF : 1;
                uint8_t SF : 1;
                uint8_t TF : 1;
                uint8_t IF : 1; /* if */
                uint8_t DF : 1;
                uint8_t OF : 1;
                uint16_t IOPL : 1;
                uint8_t NT : 1;
                uint8_t __reserved4 : 1;
                uint8_t RF : 1;
                uint8_t VM : 1;
                /*
                uint8_t ac : 1;
                uint8_t vif : 1;
                uint8_t vip : 1;
                uint8_t id : 1;
                */
            };
        };
    };

    struct GDTR{
    		uint32_t base_addr;
    		uint16_t seg_limit;
    }gdtr;

    struct IDTR{
    		uint32_t base_addr;
    		uint16_t seg_limit;
    }idtr;

    // EIP
    swaddr_t eip;

} CPU_state;

typedef struct {
	union {
		struct {
			uint32_t seg_limit1	:16;
			uint32_t seg_base1	:16;
		};
		uint32_t first_part;
	};
	union {
		struct {
			uint32_t seg_base2 	:8;
			uint32_t type		:5;
			uint32_t dpl		:2;
			uint32_t p		:1;
			uint32_t seg_limit2	:4;
			uint32_t avl		:1;
			uint32_t 		:1;
			uint32_t b		:1;
			uint32_t g		:1;
			uint32_t seg_base3	:8;
		};
		uint32_t second_part;
	};
}SEG_descriptor;

extern CPU_state cpu;
uint8_t current_sreg;
SEG_descriptor *seg_des;

static inline int check_reg_index(int index) {
    assert(index >= 0 && index < 8);
    return index;
}

#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])

extern const char* regsl[];
extern const char* regsw[];
extern const char* regsb[];
//extern const char* regss[];

#endif
