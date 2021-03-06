// write.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../../include/unistd.h"
#include <sys/syscall.h>


extern "C" ssize_t write(int fildes, const void* buf, size_t nbyte)
{
	return Library::SystemCall::Write(fildes, (uint8_t*) buf, nbyte);
}
