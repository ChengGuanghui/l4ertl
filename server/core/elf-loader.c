#include <l4/util/elf.h>

#include <l4/re/env.h>
#include <l4/re/c/dataspace.h>
#include <l4/re/c/namespace.h>
#include <l4/re/c/rm.h>
#include <l4/re/c/util/cap_alloc.h>

#include <l4/util/elf.h>

#include <stdio.h>
#include <string.h>

extern int elf_loader(char *);

int elf_loader(char *pos)
{
  Elf32_Ehdr *ehdr;
  Elf32_Phdr *phdr;
  const char *cap_name = "rom";
  const char *ds_name = "error_test";
  l4_cap_idx_t tmp, ds_cap;
  l4re_ds_stats_t ds_stat;
  void *buf;
  int i = 0, memsize = 0, err;

  if(l4_is_invalid_cap(tmp = l4re_get_env_cap(cap_name))) {
    printf("Can't get cap.\n");
    return 1;
  }

  if (l4_is_invalid_cap(ds_cap = l4re_util_cap_alloc())) {
    printf("Can't alloc capability.\n");
    return 1;
  }
  if(l4re_ns_query_srv(tmp, ds_name, ds_cap) < 0) {
    printf("Query name failed.\n");
    return 1;
  }

  if(l4re_ds_info(ds_cap, &ds_stat) < 0) {
    printf("Can't get dataspace status.\n");
    return 1;
  }

  if((err = l4re_rm_attach(&buf, ds_stat.size,
                           L4RE_RM_SEARCH_ADDR
                            | L4RE_RM_EAGER_MAP
                            | L4RE_RM_READ_ONLY,
                           ds_cap,
                           0, L4_PAGESHIFT)) < 0)
  {
    printf("Can't attach area.\n");
    return 1;
  }

  ehdr = (Elf32_Ehdr *)(buf);

  if(!(l4util_elf_check_magic(ehdr) & l4util_elf_check_arch(ehdr))) {
    printf("ELF MAGIC and ARCH checking failed.\n");
    return 1;
  }

  //printf("entry 0x%x\n", ehdr->e_entry);
  /* Load each program header */
  for (i = 0; i < ehdr->e_phnum; i++) {
    phdr = (Elf32_Phdr *)((unsigned long)buf + ehdr->e_phoff + i * ehdr->e_phentsize);
    //printf("i = %d, 0x%x, 0x%x\n", i, phdr->p_type, phdr->p_vaddr);
    if((phdr->p_type != PT_LOAD) || (phdr->p_vaddr != ehdr->e_entry))
       continue;	//ignore to load other program sections, specially the overlap of different program sections
    void *src = (void *) buf + phdr->p_offset;
    if (phdr->p_filesz)
      memcpy(pos, src, phdr->p_filesz);
	
      if (phdr->p_filesz < phdr->p_memsz)
         memset(pos + phdr->p_filesz, 0x00, phdr->p_memsz - phdr->p_filesz);
         memsize = phdr->p_memsz;
  }

  printf("0x%x over.\n", ehdr->e_entry);
  l4re_rm_detach(buf);
  return memsize;       
}
