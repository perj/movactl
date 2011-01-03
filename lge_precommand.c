
#define SIMPLE_COMMAND(name, cmd, code, arg) COMMAND(name, cmd, 0)
#define UINT_COMMAND(name, cmd, code) COMMAND(name, cmd, 1)
#define UINT2_SUFF_COMMAND(name, cmd, code, suff) COMMAND(name, cmd, 1)

#include "lge_command.h"

#undef SIMPLE_COMMAND
#undef UINT_COMMAND
#undef UINT2_SUFF_COMMAND
