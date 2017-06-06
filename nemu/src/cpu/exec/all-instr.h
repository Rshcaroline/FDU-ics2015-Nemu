#include "prefix/prefix.h"

#include "data-mov/mov.h"
#include "data-mov/xchg.h"
#include "data-mov/push.h"
#include "data-mov/leave.h"
#include "data-mov/movsx.h"
#include "data-mov/movzx.h"
#include "data-mov/pop.h"
#include "data-mov/cltd.h"
#include "data-mov/cwtl.h"

#include "arith/dec.h"
#include "arith/inc.h"
#include "arith/neg.h"
#include "arith/imul.h"
#include "arith/mul.h"
#include "arith/idiv.h"
#include "arith/div.h"
#include "arith/sub.h"
#include "arith/cmp.h"
#include "arith/add.h"
#include "arith/adc.h"
#include "arith/sbb.h"

#include "CMOVcc/cmovcc.h"

#include "con-trans/call.h"
#include "con-trans/je.h"
#include "con-trans/jbe.h"
#include "con-trans/jmp.h"
#include "con-trans/ret.h"
#include "con-trans/ja.h"
#include "con-trans/jae.h"
#include "con-trans/jb.h"
#include "con-trans/jg.h"
#include "con-trans/jge.h"
#include "con-trans/jl.h"
#include "con-trans/jle.h"
#include "con-trans/jne.h"
#include "con-trans/jno.h"
#include "con-trans/jnp.h"
#include "con-trans/jns.h"
#include "con-trans/jo.h"
#include "con-trans/jp.h"
#include "con-trans/js.h"

#include "logic/and.h"
#include "logic/or.h"
#include "logic/not.h"
#include "logic/xor.h"
#include "logic/sar.h"
#include "logic/shl.h"
#include "logic/shr.h"
#include "logic/shrd.h"
#include "logic/test.h"
#include "logic/setb.h"
#include "logic/sete.h"
#include "logic/setne.h"
#include "logic/seto.h"
#include "logic/setp.h"
#include "logic/sets.h"
#include "logic/std.h"

#include "string/rep.h"
#include "string/cld.h"
#include "string/stos.h"
#include "string/cmps.h"
#include "string/movs.h"

#include "misc/misc.h"

#include "special/special.h"