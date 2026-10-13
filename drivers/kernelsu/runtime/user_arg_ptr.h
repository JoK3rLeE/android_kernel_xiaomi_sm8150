/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __KSU_RUNTIME_USER_ARG_PTR_H
#define __KSU_RUNTIME_USER_ARG_PTR_H

#include <linux/compiler.h>
#include <linux/types.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

/*
 * KernelSU's execve hook needs the same argument-vector representation
 * across runtime and feature code. Keep this definition in one shared
 * header rather than duplicating it in individual translation units.
 */
struct user_arg_ptr {
#ifdef CONFIG_COMPAT
	bool is_compat;
#endif
	union {
		const char __user *const __user *native;
#ifdef CONFIG_COMPAT
		const compat_uptr_t __user *compat;
#endif
	} ptr;
};

#endif /* __KSU_RUNTIME_USER_ARG_PTR_H */
