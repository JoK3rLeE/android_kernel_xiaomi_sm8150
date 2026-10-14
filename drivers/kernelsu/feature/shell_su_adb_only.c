#include <linux/cred.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/static_key.h>
#include <linux/types.h>

#include "feature/shell_su_adb_only.h"
#include "policy/feature.h"
#include "selinux/selinux.h"

#include "klog.h" // IWYU pragma: keep

/*
 * A granted shell UID may only use su when its process tree originates from
 * adbd or from a process already running in the KernelSU SELinux domain.
 * This is an additional check; the normal allowlist grant is still required.
 */
static DEFINE_STATIC_KEY_FALSE(ksu_shell_su_adb_only);

/* Bound ancestor traversal; if the chain is unexpectedly deep, deny access. */
#define MAX_ANCESTOR_DEPTH 128

static bool has_trusted_ancestor(void)
{
	struct task_struct *task;
	int depth = 0;
	bool found = false;

	rcu_read_lock();
	for (task = current;
	     task->pid > 1 && depth < MAX_ANCESTOR_DEPTH;
	     task = rcu_dereference(task->real_parent), depth++) {
		const struct cred *cred = __task_cred(task);

		if (is_adbd(cred) || is_task_ksu_domain(cred)) {
			found = true;
			break;
		}
	}
	rcu_read_unlock();

	return found;
}

bool ksu_shell_su_allowed_for_current(void)
{
	if (!static_branch_unlikely(&ksu_shell_su_adb_only))
		return true;

	return has_trusted_ancestor();
}

static int shell_su_adb_only_feature_get(u64 *value)
{
	*value = static_key_enabled(&ksu_shell_su_adb_only) ? 1 : 0;
	return 0;
}

static int shell_su_adb_only_feature_set(u64 value)
{
	bool enable = value != 0;

	if (enable)
		static_key_enable(&ksu_shell_su_adb_only.key);
	else
		static_key_disable(&ksu_shell_su_adb_only.key);

	pr_info("shell_su_adb_only: set to %d\n", enable);
	return 0;
}

static const struct ksu_feature_handler shell_su_adb_only_handler = {
	.feature_id = KSU_FEATURE_SHELL_SU_ADB_ONLY,
	.name = "shell_su_adb_only",
	.get_handler = shell_su_adb_only_feature_get,
	.set_handler = shell_su_adb_only_feature_set,
};

void __init ksu_shell_su_adb_only_init(void)
{
	if (ksu_register_feature_handler(&shell_su_adb_only_handler))
		pr_err("Failed to register shell_su_adb_only feature handler\n");
}

void __exit ksu_shell_su_adb_only_exit(void)
{
	ksu_unregister_feature_handler(KSU_FEATURE_SHELL_SU_ADB_ONLY);
}
