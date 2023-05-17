INSTRUCTION(NONE, {})

INSTRUCTION(NOP, { PUSH(0x90); })

#include "instructions/add.hpp"
#include "instructions/cjmp.hpp"
#include "instructions/div.hpp"
#include "instructions/mov.hpp"
#include "instructions/mul.hpp"
#include "instructions/pop.hpp"
#include "instructions/push.hpp"
#include "instructions/sub.hpp"
