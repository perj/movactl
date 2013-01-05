
#include <time.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>

int
osx_system_idle(struct timespec *dst)
{
	kern_return_t err;
	io_iterator_t it;
	io_registry_entry_t service;
	bool set = false;

	err = IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("IOHIDSystem"), &it);
	if (err)
		return -1;

	while ((service = IOIteratorNext(it)))
	{
		CFNumberRef value = IORegistryEntryCreateCFProperty(service, CFSTR("HIDIdleTime"), kCFAllocatorDefault, 0);
		long long n;
		struct timespec ts;

		if (!value)
			goto next;
		if (!CFNumberGetValue(value, kCFNumberLongLongType, &n))
			goto next;

		ts.tv_sec = n / 1000000000;
		ts.tv_nsec = n % 1000000000;

		if (!set || ts.tv_sec < dst->tv_sec || (ts.tv_sec == dst->tv_sec && ts.tv_nsec < dst->tv_nsec))
			*dst = ts;
		set = true;

next:
		if (value)
			CFRelease(value);
		IOObjectRelease(service);
	}
	IOObjectRelease(it);

	return set ? 0 : -1;
}
