/*
 * Copyright (c) 2000-2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * @OSF_COPYRIGHT@
 * 
 */

#ifndef	I386_CPU_DATA
#define I386_CPU_DATA

#include <mach_assert.h>

#if	defined(__GNUC__)

#include <kern/assert.h>
#include <kern/kern_types.h>
#include <kern/processor.h>
#include <kern/pms.h>
#include <pexpert/pexpert.h>
#include <mach/i386/thread_status.h>
#include <i386/rtclock.h>
#include <i386/pmCPU.h>

/*
 * Data structures referenced (anonymously) from per-cpu data:
 */
struct cpu_core;
struct cpu_cons_buffer;
struct cpu_desc_table;


/*
 * Data structures embedded in per-cpu data:
 */
typedef struct rtclock_timer {
	uint64_t	deadline;
	boolean_t	is_set;
	boolean_t	has_expired;
} rtclock_timer_t;

typedef struct {
	uint64_t	tsc_base;		/* timestamp */
	uint64_t	ns_base;		/* nanoseconds */
	uint32_t	scale;			/* tsc -> nanosec multiplier */
	uint32_t	shift;			/* tsc -> nanosec shift/div */
} rtc_nanotime_t;

typedef struct {
	struct i386_tss         *cdi_ktss;
#if    MACH_KDB
	struct i386_tss         *cdi_dbtss;
#endif /* MACH_KDB */
	struct fake_descriptor  *cdi_gdt;
	struct fake_descriptor  *cdi_idt;
	struct fake_descriptor  *cdi_ldt;
	vm_offset_t		cdi_sstk;
} cpu_desc_index_t;

typedef enum {
	TASK_MAP_32BIT,			/* 32-bit, compatibility mode */ 
	TASK_MAP_64BIT,			/* 64-bit, separate address space */ 
	TASK_MAP_64BIT_SHARED		/* 64-bit, kernel-shared addr space */
} task_map_t;

/*
 * This structure is used on entry into the (uber-)kernel on syscall from
 * a 64-bit user. It contains the address of the machine state save area
 * for the current thread and a temporary place to save the user's rsp
 * before loading this address into rsp.
 */
typedef struct {
	addr64_t	cu_isf;		/* thread->pcb->iss.isf */
	uint64_t	cu_tmp;		/* temporary scratch */	
        addr64_t	cu_user_gs_base;
} cpu_uber_t;

/*
 * Per-cpu data.
 *
 * Each processor has a per-cpu data area which is dereferenced through the
 * current_cpu_datap() macro. For speed, the %gs segment is based here, and
 * using this, inlines provides single-instruction access to frequently used
 * members - such as get_cpu_number()/cpu_number(), and get_active_thread()/
 * current_thread(). 
 * 
 * Cpu data owned by another processor can be accessed using the
 * cpu_datap(cpu_number) macro which uses the cpu_data_ptr[] array of per-cpu
 * pointers.
 */
typedef struct cpu_data
{
	struct cpu_data		*cpu_this;		/* pointer to myself */
	thread_t		cpu_active_thread;
	void			*cpu_int_state;		/* interrupt state */
	vm_offset_t		cpu_active_stack;	/* kernel stack base */
	vm_offset_t		cpu_kernel_stack;	/* kernel stack top */
	vm_offset_t		cpu_int_stack_top;
	int			cpu_preemption_level;
	int			cpu_simple_lock_count;
	int			cpu_interrupt_level;
	int			cpu_number;		/* Logical CPU */
	int			cpu_phys_number;	/* Physical CPU */
	cpu_id_t		cpu_id;			/* Platform Expert */
	int			cpu_signals;		/* IPI events */
	int			cpu_mcount_off;		/* mcount recursion */
	ast_t			cpu_pending_ast;
	int			cpu_type;
	int			cpu_subtype;
	int			cpu_threadtype;
	int			cpu_running;
	uint64_t		rtclock_intr_deadline;
	rtclock_timer_t		rtclock_timer;
        boolean_t		cpu_is64bit;
        task_map_t		cpu_task_map;
        addr64_t		cpu_task_cr3;
        addr64_t		cpu_active_cr3;
        addr64_t		cpu_kernel_cr3;
	cpu_uber_t		cpu_uber;
	void			*cpu_chud;
	void			*cpu_console_buf;
	struct cpu_core		*cpu_core;		/* cpu's parent core */
	struct processor	*cpu_processor;
	struct cpu_pmap		*cpu_pmap;
	struct cpu_desc_table	*cpu_desc_tablep;
	struct fake_descriptor	*cpu_ldtp;
	cpu_desc_index_t	cpu_desc_index;
	int			cpu_ldt;
#ifdef MACH_KDB
	/* XXX Untested: */
	int			cpu_db_pass_thru;
	vm_offset_t	cpu_db_stacks;
	void		*cpu_kdb_saved_state;
	spl_t		cpu_kdb_saved_ipl;
	int			cpu_kdb_is_slave;
	int			cpu_kdb_active;
#endif /* MACH_KDB */
	boolean_t		cpu_iflag;
	boolean_t		cpu_boot_complete;
	int			cpu_hibernate;
	pmsd			pms;					/* Power Management Stepper control */
	uint64_t		rtcPop;			/* when the etimer wants a timer pop */
	
	vm_offset_t	cpu_copywindow_base;
	uint64_t	*cpu_copywindow_pdp;
	
	vm_offset_t	cpu_physwindow_base;
	uint64_t	*cpu_physwindow_ptep;
	void 		*cpu_hi_iss;
	boolean_t	cpu_tlb_invalid;
	
	uint64_t	*cpu_pmHpet;		/* Address of the HPET for this processor */
	uint32_t	cpu_pmHpetVec;		/* Interrupt vector for HPET for this processor */
/*	Statistics */
	pmStats_t	cpu_pmStats;		/* Power management data */
	uint32_t	cpu_hwIntCnt[256];		/* Interrupt counts */
	
	uint64_t		cpu_dr7; /* debug control register */
} cpu_data_t;

extern cpu_data_t	*cpu_data_ptr[];  
extern cpu_data_t	cpu_data_master;  

/* Macro to generate inline bodies to retrieve per-cpu data fields. */
#define offsetof(TYPE,MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define CPU_DATA_GET(member,type)					\
	type ret;							\
	__asm__ volatile ("movl %%gs:%P1,%0"				\
		: "=r" (ret)						\
		: "i" (offsetof(cpu_data_t,member)));			\
	return ret;

/*
 * Everyone within the osfmk part of the kernel can use the fast
 * inline versions of these routines.  Everyone outside, must call
 * the real thing,
 */
static inline thread_t
get_active_thread(void)
{
	CPU_DATA_GET(cpu_active_thread,thread_t)
}
#define current_thread_fast()		get_active_thread()
#define current_thread()		current_thread_fast()

static inline boolean_t
get_is64bit(void)
{
	CPU_DATA_GET(cpu_is64bit, boolean_t)
}
#define cpu_mode_is64bit()		get_is64bit()

static inline int
get_preemption_level(void)
{
	CPU_DATA_GET(cpu_preemption_level,int)
}
static inline int
get_simple_lock_count(void)
{
	CPU_DATA_GET(cpu_simple_lock_count,int)
}
static inline int
get_interrupt_level(void)
{
	CPU_DATA_GET(cpu_interrupt_level,int)
}
static inline int
get_cpu_number(void)
{
	CPU_DATA_GET(cpu_number,int)
}
static inline int
get_cpu_phys_number(void)
{
	CPU_DATA_GET(cpu_phys_number,int)
}
static inline struct
cpu_core * get_cpu_core(void)
{
	CPU_DATA_GET(cpu_core,struct cpu_core *)
}

static inline void
disable_preemption(void)
{
	__asm__ volatile ("incl %%gs:%P0"
			:
			: "i" (offsetof(cpu_data_t, cpu_preemption_level)));
}

static inline void
enable_preemption(void)
{
	assert(get_preemption_level() > 0);

	__asm__ volatile ("decl %%gs:%P0		\n\t"
			  "jne 1f			\n\t"
			  "call _kernel_preempt_check	\n\t"
			  "1:"
			: /* no outputs */
			: "i" (offsetof(cpu_data_t, cpu_preemption_level))
			: "eax", "ecx", "edx", "cc", "memory");
}

static inline void
enable_preemption_no_check(void)
{
	assert(get_preemption_level() > 0);

	__asm__ volatile ("decl %%gs:%P0"
			: /* no outputs */
			: "i" (offsetof(cpu_data_t, cpu_preemption_level))
			: "cc", "memory");
}

static inline void
mp_disable_preemption(void)
{
	disable_preemption();
}

static inline void
mp_enable_preemption(void)
{
	enable_preemption();
}

static inline void
mp_enable_preemption_no_check(void)
{
	enable_preemption_no_check();
}

static inline cpu_data_t *
current_cpu_datap(void)
{
	CPU_DATA_GET(cpu_this, cpu_data_t *);
}

static inline cpu_data_t *
cpu_datap(int cpu)
{
	assert(cpu_data_ptr[cpu]);
	return cpu_data_ptr[cpu];
}

extern cpu_data_t *cpu_data_alloc(boolean_t is_boot_cpu);

#else	/* !defined(__GNUC__) */

#endif	/* defined(__GNUC__) */

#endif	/* I386_CPU_DATA */
