// errno.h
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#ifndef _STDC_ERRNO_H_
#define _STDC_ERRNO_H_

#include "sys/types.h"
#include "sys/cdefs.h"


__BEGIN_DECLS

extern __thread int _errno;
#define errno			_errno

#define ENOTBLK		12
#define ENODEV		13
#define EWOULDBLOCK	14
#define EBADF			15
#define EOVERFLOW		16
#define ENOENT		17
#define ENOSPC		18
#define EEXIST 			19
#define EROFS 			20
#define EINVAL 		21
#define ENOTDIR 		22
#define ENOMEM 		23
#define ERANGE 		24
#define EISDIR 			25
#define EPERM 			26
#define EIO 			27
#define ENOEXEC 		28
#define EACCES 		29
#define ESRCH 			30
#define ENOTTY 		31
#define ECHILD 		32
#define ENOSYS		33
#define ENOTSUP 		34
#define EBLOCKING 		35
#define EINTR			36
#define ENOTEMPTY		37
#define EBUSY			38
#define EPIPE			39
#define EILSEQ			40
#define ELAKE			41
#define EMFILE			42
#define EAGAIN		43
#define EEOF			44
#define EBOUND		45
#define EINIT			46
#define ENODRV		47
#define E2BIG			48
#define EFBIG			49
#define EXDEV			50
#define ESPIPE			51
#define ENAMETOOLONG	52
#define ELOOP			53
#define EMLINK		54
#define ENXIO			55
#define EPROTONOSUPPORT	56
#define EAFNOSUPPORT	57
#define ENOTSOCK		58
#define EADDRINUSE		59
#define ETIMEDOUT		60
#define ECONNREFUSED	61
#define EDOM			62
#define EINPROGRESS		63
#define EALREADY		64
#define ESHUTDOWN		65
#define ECONNABORTED	66
#define ECONNRESET		67
#define EADDRNOTAVAIL	68
#define EISCONN		69
#define EFAULT		70
#define EDESTADDRREQ	71
#define EHOSTUNREACH	72
#define EMSGSIZE		73
#define ENETDOWN		74
#define ENETRESET		75
#define ENETUNREACH	76
#define ENOBUFS		77
#define ENOMSG		78
#define ENOPROTOOPT	79
#define ENOTCONN		80
#define EDEADLK		81
#define ENFILE			82
#define EPROTOTYPE		83
#define ENOLCK		84
#define ENOUSER		85
#define ENOGROUP		86
#define ESIGPENDING		87
#define ESTALE			88

__END_DECLS
#define EOPNOTSUPP ENOTSUP
#endif

