/* auto-generated by gen_syscalls.py, don't edit */

#include <syscalls/socket.h>

extern int z_vrfy_zsock_getsockopt(int sock, int level, int optname, void * optval, socklen_t * optlen);
uintptr_t z_mrsh_zsock_getsockopt(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2,
		uintptr_t arg3, uintptr_t arg4, uintptr_t arg5, void *ssf)
{
	_current->syscall_frame = ssf;
	(void) arg5;	/* unused */
	union { uintptr_t x; int val; } parm0;
	parm0.x = arg0;
	union { uintptr_t x; int val; } parm1;
	parm1.x = arg1;
	union { uintptr_t x; int val; } parm2;
	parm2.x = arg2;
	union { uintptr_t x; void * val; } parm3;
	parm3.x = arg3;
	union { uintptr_t x; socklen_t * val; } parm4;
	parm4.x = arg4;
	int ret = z_vrfy_zsock_getsockopt(parm0.val, parm1.val, parm2.val, parm3.val, parm4.val);
	_current->syscall_frame = NULL;
	return (uintptr_t) ret;
}

