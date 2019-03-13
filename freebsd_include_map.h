static const IncludeMapEntry freebsd_include_map[] = 

{
  // x86 -> machine or std
  { "<x86/acpica_machdep.h>", kPrivate, "<machine/acpica_machdep.h>", kPublic },
  { "<x86/_align.h>", kPrivate, "<sys/param.h>", kPublic },
  { "<x86/apicvar.h>", kPrivate, "<machine/smp.h>", kPublic },
  { "<x86/apm_bios.h>", kPrivate, "<machine/apm_bios.h>", kPublic },
  { "<x86/bus.h>", kPrivate, "<machine/bus.h>", kPublic },
  { "<x86/cputypes.h>", kPrivate, "<machine/cputypes.h>", kPublic },
  { "<x86/dump.h>", kPrivate, "<machine/dump.h>", kPublic },
  { "<x86/elf.h>", kPrivate, "<elf.h>", kPublic },
  { "<x86/endian.h>", kPrivate, "<endian.h>", kPublic },
  { "<x86/fdt.h>", kPrivate, "<machine/fdt.h>", kPublic },
  { "<x86/float.h>", kPrivate, "<float.h>", kPublic },
  { "<x86/fpu.h>", kPrivate, "<machine/fpu.h>", kPublic },
  { "<x86/fpu.h>", kPrivate, "<machine/npx.h>", kPublic },
  { "<x86/frame.h>", kPrivate, "<sys/pmc.h>", kPublic },
  { "<x86/_inttypes.h>", kPrivate, "<inttypes.h>", kPublic },
  { "<x86/_limits.h>", kPrivate, "<sys/limits.h>", kPublic },
  { "<x86/metadata.h>", kPrivate, "<machine/metadata.h>", kPublic },
  { "<x86/ofw_machdep.h>", kPrivate, "<machine/ofw_machdep.h>", kPublic },
  { "<x86/pci_cfgreg.h>", kPrivate, "<machine/pci_cfgreg.h>", kPublic },
  { "<x86/psl.h>", kPrivate, "<machine/psl.h>", kPublic },
  { "<x86/ptrace.h>", kPrivate, "<ptrace.h>", kPublic },
  { "<x86/pvclock.h>", kPrivate, "<machine/pvclock.h>", kPublic },
  { "<x86/reg.h>", kPrivate, "<ptrace.h>", kPublic },
  { "<x86/segments.h>", kPrivate, "<machine/segments.h>", kPublic },
  { "<x86/setjmp.h>", kPrivate, "<setjmp.h>", kPublic },
  { "<x86/sigframe.h>", kPrivate, "<machine/sigframe.h>", kPublic },
  { "<x86/signal.h>", kPrivate, "<sys/signal.h>", kPublic },
  { "<x86/specialreg.h>", kPrivate, "<machine/specialreg.h>", kPublic },
  { "<x86/stack.h>", kPrivate, "<machine/stack.h>", kPublic },
  { "<x86/stdarg.h>", kPrivate, "<stdarg.h>", kPublic },
  { "<x86/_stdint.h>", kPrivate, "<stdint.h>", kPublic },
  { "<x86/sysarch.h>", kPrivate, "<machine/sysarch.h>", kPublic },
  { "<x86/trap.h>", kPrivate, "<machine/trap.h>", kPublic },
  { "<x86/_types.h>", kPrivate, "<stddef.h>", kPublic },
  { "<x86/ucontext.h>", kPrivate, "<ucontext.h>", kPublic },
  { "<x86/vdso.h>", kPrivate, "<sys/vdso.h>", kPublic },
  { "<x86/x86_smp.h>", kPrivate, "<machine/smp.h>", kPublic },
  { "<x86/x86_var.h>", kPrivate, "<sys/sf_buf.h>", kPublic },
  // machine -> sys or std
  { "<machine/acle-compat.h>", kPrivate, "<sys/cdefs.h>", kPublic },
  { "<machine/_align.h>", kPrivate, "<sys/param.h>", kPublic },
  { "<machine/atomic.h>", kPrivate, "<sys/mutex.h>", kPublic },
  { "<machine/_bus.h>", kPrivate, "<machine/bus.h>", kPublic },
  { "<machine/_bus.h>", kPrivate, "<sys/bus.h>", kPublic },
  { "<machine/counter.h>", kPrivate, "<sys/counter.h>", kPublic },
  { "<machine/cpufunc.h>", kPrivate, "<sys/mutex.h>", kPublic },
  { "<machine/efi.h>", kPrivate, "<sys/efi.h>", kPublic },
  { "<machine/elf.h>", kPrivate, "<elf.h>", kPublic },
  { "<machine/endian.h>", kPrivate, "<sys/endian.h>", kPublic },
  { "<machine/exec.h>", kPrivate, "<sys/exec.h>", kPublic },
  { "<machine/frame.h>", kPrivate, "<sys/pmc.h>", kPublic },
  { "<machine/ieeefp.h>", kPrivate, "<ieeefp.h>", kPublic },
  { "<machine/in_cksum.h>", kPrivate, "<netinet/ip_compat.h>", kPublic },
  { "<machine/_inttypes.h>", kPrivate, "<inttypes.h>", kPublic },
  { "<machine/_limits.h>", kPrivate, "<sys/limits.h>", kPublic },
  { "<machine/md_var.h>", kPrivate, "<sys/sf_buf.h>", kPublic },
  { "<machine/param.h>", kPrivate, "<sys/param.h>", kPublic },
  { "<machine/pcb.h>", kPrivate, "<sys/user.h>", kPublic },
  { "<machine/pcpu.h>", kPrivate, "<sys/pcpu.h>", kPublic },
  { "<machine/pmc_mdep.h>", kPrivate, "<sys/pmc.h>", kPublic },
  { "<machine/proc.h>", kPrivate, "<sys/proc.h>", kPublic },
  { "<machine/profile.h>", kPrivate, "<sys/gmon.h>", kPublic },
  { "<machine/ptrace.h>", kPrivate, "<ptrace.h>", kPublic },
  { "<machine/reg.h>", kPrivate, "<ptrace.h>", kPublic },
  { "<machine/reloc.h>", kPrivate, "<a.out.h>", kPublic },
  { "<machine/resource.h>", kPrivate, "<sys/rman.h>", kPublic },
  { "<machine/runq.h>", kPrivate, "<sys/runq.h>", kPublic },
  { "<machine/setjmp.h>", kPrivate, "<setjmp.h>", kPublic },
  { "<machine/sf_buf.h>", kPrivate, "<sys/sf_buf.h>", kPublic },
  { "<machine/signal.h>", kPrivate, "<sys/signal.h>", kPublic },
  { "<machine/_stdint.h>", kPrivate, "<stdint.h>", kPublic },
  { "<machine/_types.h>", kPrivate, "<stddef.h>", kPublic },
  { "<machine/ucontext.h>", kPrivate, "<ucontext.h>", kPublic },
  { "<machine/varargs.h>", kPrivate, "<varargs.h>", kPublic },
  { "<machine/vdso.h>", kPrivate, "<sys/vdso.h>", kPublic },
  { "<machine/vm.h>", kPrivate, "<vm/vm.h>", kPublic },
  { "<machine/vmparam.h>", kPrivate, "<vm/vm_param.h>", kPublic },
  // others
  // _foo -> foo
  { "<sys/_bitset.h>", kPrivate, "<sys/cpuset.h>", kPublic },
  { "<sys/_bus_dma.h>", kPrivate, "<sys/bus_dma.h>", kPublic },
  { "<sys/_callout.h>", kPrivate, "<sys/callout.h>", kPublic },
  { "<sys/_cpuset.h>", kPrivate, "<sys/cpuset.h>", kPublic },
  { "<sys/_ffcounter.h>", kPrivate, "<sys/sysproto.h>", kPublic },
  { "<sys/_iovec.h>", kPrivate, "<sys/uio.h>", kPublic },
  { "<sys/_lock.h>", kPrivate, "<sys/lock.h>", kPublic },
  { "<sys/_lockmgr.h>", kPrivate, "<sys/lockmgr.h>", kPublic },
  { "<sys/_mutex.h>", kPrivate, "<sys/mutex.h>", kPublic },
  { "<sys/_null.h>", kPrivate, "<stddef.h>", kPublic },
  { "<sys/_pctrie.h>", kPrivate, "<sys/pctrie.h>", kPublic },
  { "<sys/_pthreadtypes.h>", kPrivate, "<pthread.h>", kPublic },
  { "<sys/_rmlock.h>", kPrivate, "<sys/rmlock.h>", kPublic },
  { "<sys/_semaphore.h>", kPrivate, "<sys/sysproto.h>", kPublic },
  { "<sys/_sigset.h>", kPrivate, "<sys/select.h>", kPublic },
  { "<sys/_sockaddr_storage.h>", kPrivate, "<sys/socket.h>", kPublic },
  { "<sys/_stack.h>", kPrivate, "<sys/stack.h>", kPublic },
  { "<sys/_stdint.h>", kPrivate, "<stdint.h>", kPublic },
  { "<sys/_sx.h>", kPrivate, "<sys/sx.h>", kPublic },
  { "<sys/_task.h>", kPrivate, "<sys/taskqueue.h>", kPublic },
  { "<sys/_termios.h>", kPrivate, "<termios.h>", kPublic },
  { "<sys/_timespec.h>", kPrivate, "<sys/timespec.h>", kPublic },
  { "<sys/_timeval.h>", kPrivate, "<sys/time.h>", kPublic },
  { "<sys/_types.h>", kPrivate, "<sys/types.h>", kPublic },
  { "<sys/_ucontext.h>", kPrivate, "<ucontext.h>", kPublic },
  { "<sys/_umtx.h>", kPrivate, "<sys/umtx.h>", kPublic },
  { "<sys/_vm_domain.h>", kPrivate, "<vm/vm_domain.h>", kPublic },
  { "<xlocale/_ctype.h>", kPrivate, "<xlocale.h>", kPublic },
  { "<xlocale/_inttypes.h>", kPrivate, "<xlocale.h>", kPublic },
  { "<xlocale/_langinfo.h>", kPrivate, "<xlocale.h>", kPublic },
  { "<xlocale/_locale.h>", kPrivate, "<xlocale.h>", kPublic },
  { "<xlocale/_monetary.h>", kPrivate, "<xlocale.h>", kPublic },
  { "<xlocale/_stdio.h>", kPrivate, "<xlocale.h>", kPublic },
  { "<xlocale/_stdlib.h>", kPrivate, "<xlocale.h>", kPublic },
  { "<xlocale/_string.h>", kPrivate, "<xlocale.h>", kPublic },
  { "<xlocale/_time.h>", kPrivate, "<xlocale.h>", kPublic },
  { "<xlocale/_wchar.h>", kPrivate, "<xlocale.h>", kPublic },
};
