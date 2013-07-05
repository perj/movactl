
#define THROTTLED_COMMAND(name, code, arg, s, ms) COMMAND(name, code arg, 0)
#define SIMPLE_COMMAND(name, code, arg) COMMAND(name, code arg, 0)
#define SIGNINT_COMMAND(name, code, prefix) COMMAND(name, code prefix, 1)
#define UINT_COMMAND(name, code, prefix, width) COMMAND(name, code prefix, 1)

#include "marantz_command.h"

#undef THROTTLED_COMMAND
#undef SIMPLE_COMMAND
#undef SIGNINT_COMMAND
#undef UINT_COMMAND
