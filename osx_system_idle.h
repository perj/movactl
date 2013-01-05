#ifndef OSX_SYSTEM_IDLE
#define OSX_SYSTEM_IDLE

#ifdef __cplusplus
extern "C" {
#endif

struct timespec;

/*
 * Returns 0 if found, -1 on error.
 * dst is untouched in the latter case.
 */
int osx_system_idle(struct timespec *dst);

#ifdef __cplusplus
}
#endif

#endif
