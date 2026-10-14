#ifndef __KSU_H_SHELL_SU_ADB_ONLY
#define __KSU_H_SHELL_SU_ADB_ONLY

#include <linux/types.h>

bool ksu_shell_su_allowed_for_current(void);

void ksu_shell_su_adb_only_init(void);

void ksu_shell_su_adb_only_exit(void);

#endif
