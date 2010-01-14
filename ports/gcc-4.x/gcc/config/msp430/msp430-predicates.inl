#pragma once

static inline int msp430_attribute_exists(tree func, const char *attr_name)
{
	gcc_assert(TREE_CODE (func) == FUNCTION_DECL);
	return lookup_attribute (attr_name, DECL_ATTRIBUTES (func)) != NULL_TREE;
}

static inline int msp430_naked_function_p(tree func)
{
	return msp430_attribute_exists(func, "naked");
}

static inline int msp430_task_function_p (tree func)
{
	return msp430_attribute_exists(func, "task");
}

static inline int msp430_save_prologue_function_p (tree func)
{
	return msp430_attribute_exists(func, "saveprologue");
}

static inline int interrupt_function_p (tree func)
{
	return msp430_attribute_exists(func, "interrupt");
}

static inline int msp430_critical_function_p (tree func)
{
	return msp430_attribute_exists(func, "critical");
}

static inline int msp430_reentrant_function_p (tree func)
{
	return msp430_attribute_exists(func, "reentrant");
}

static inline int noint_hwmul_function_p (tree func)
{
	return msp430_attribute_exists(func, "noint_hwmul");
}

static inline int signal_function_p (tree func)
{
	return msp430_attribute_exists(func, "signal");
}

static inline int wakeup_function_p (tree func)
{
	return msp430_attribute_exists(func, "wakeup");
}
