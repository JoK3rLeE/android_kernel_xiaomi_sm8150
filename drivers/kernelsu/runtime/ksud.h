#ifndef __KSU_H_KSUD
#define __KSU_H_KSUD

#include <linux/compiler_types.h>
#include <linux/types.h>

#define KSUD_PATH "/data/adb/ksud"

struct filename;

#include "user_arg_ptr.h"

int ksu_handle_execveat_ksud(int *fd, struct filename **filename_ptr,
			     struct user_arg_ptr *argv,
			     struct user_arg_ptr *envp, int *flags);

void ksu_ksud_init();
void ksu_ksud_exit();

void ksu_stop_input_hook_runtime(void);

extern bool ksu_execveat_hook __read_mostly;

#endif