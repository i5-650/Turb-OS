#pragma once

#include <stivale2.h>

namespace turbo {

	extern struct stivale2_struct_tag_smp *smp_tag;
	extern struct stivale2_struct_tag_memmap *mmap_tag;
	extern struct stivale2_struct_tag_rsdp *rsdp_tag;
	extern struct stivale2_struct_tag_framebuffer *frm_tag;
	extern struct stivale2_struct_tag_terminal *term_tag;
	extern struct stivale2_struct_tag_modules *mod_tag;
	extern struct stivale2_struct_tag_cmdline *cmd_tag;
	extern struct stivale2_struct_tag_kernel_file_v2 *kfilev2_tag;
	extern struct stivale2_struct_tag_hhdm *hhdm_tag;
	extern struct stivale2_struct_tag_pmrs *pmrs_tag;
	extern struct stivale2_struct_tag_kernel_base_address *kbaddr_tag;

	extern char *cmdline;

	int find_module(char *name);

	void main(struct stivale2_struct *stivale2_struct);
}