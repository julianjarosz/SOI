/* This file contains some dumping routines for debugging. */

#include "kernel.h"
#include <minix/com.h>
#include "proc.h"

FORWARD _PROTOTYPE(char *proc_name, (int proc_nr));

#define click_to_round_k(n) \
	((unsigned) ((((unsigned long) (n) << CLICK_SHIFT) + 512) / 1024))

/*===========================================================================*
 *				p_dmp    				     *
 *===========================================================================*/
#if (CHIP == INTEL)
PUBLIC void p_dmp()
{
/* Proc table dump */

  register struct proc *rp;
  static struct proc *oldrp = BEG_PROC_ADDR;
  int n = 0;
  phys_clicks text, data, size;

  printf("\nc --pid --pc- ---sp- flag -user --sys-- -text- -data- -size- -recv- command\n");
    
  for (rp = oldrp; rp < END_PROC_ADDR; rp++) {
	if (isemptyp(rp)) continue;
	if (++n > 20) break;
	text = rp->p_map[T].mem_phys;
	data = rp->p_map[D].mem_phys;
	size = rp->p_map[T].mem_len
		+ ((rp->p_map[S].mem_phys + rp->p_map[S].mem_len) - data);
	printf("%1d ", rp->category); // dodanie wyniku do kolumny c (category)
	if (rp->p_pid == 0) {
		printf("(%3d)", proc_number(rp));
	} else {
		printf("%5d", rp->p_pid);
	}
	printf(" %5lx %6lx %2x %7lu %7lu %5uK %5uK %5uK ",
	       (unsigned long) rp->p_reg.pc,
	       (unsigned long) rp->p_reg.sp,
	       rp->p_flags,
	       rp->user_time, rp->sys_time,
	       click_to_round_k(text), click_to_round_k(data),
	       click_to_round_k(size));
	if (rp->p_flags & RECEIVING) {
		printf("%-7.7s", proc_name(rp->p_getfrom));
	} else
	if (rp->p_flags & SENDING) {
		printf("S:%-5.5s", proc_name(rp->p_sendto));
	} else
	if (rp->p_flags == 0) {
		printf("       ");
	}
	printf("%s\n", rp->p_name);
  }
  if (rp == END_PROC_ADDR) rp = BEG_PROC_ADDR; else printf("--more--\r");
  oldrp = rp;
}
#endif				/* (CHIP == INTEL) */