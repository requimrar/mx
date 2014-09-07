// misc.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <sys/syscall.h>
#include <sys/types.h>

extern "C" int usleep(useconds_t usec)
{
	Library::SystemCall::Sleep((usec + 999) / 1000);
	return 0;
}
