#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "elf.h"

struct trace
{
  char fname[MAXFNAME];
  uint64 addr;
  uint64 fsize;
  struct trace* next;
};

struct {
  struct trace* head;
  uint32 n;
} tracemap;

void
backtrace()
{
  uint64 fp, stop;
  
  fp = r_fp();
  stop = PGROUNDUP(fp);

  while(fp < stop){
    char fname[MAXFNAME];
    uint64 *ra = (uint64*)(fp - 8);
    btracelookup(*ra, fname);
    printf("%s, %p\n", fname, *ra);
    fp = *(uint64*)(fp - 16);
  }
}

static int
elfcopystr(struct inode* ip, char* dst, uint64 firstaddr){
  char targetstr[SEC_NAME_MAX];
  char p;
  int i;

  i = 0;
  do
  {
    readi(ip, 0, (uint64)&p, firstaddr + i, 1);
    targetstr[i++] = p;
  } while (p && i < SEC_NAME_MAX);
  
  targetstr[i-1] = 0;

  memmove(dst, targetstr, i);

  return i;
}

static struct sechdr
elfgetsecbyname(struct inode* ip, struct elfhdr elf, struct sechdr strtab, char* dstsecname)
{
  struct sechdr sechdrdst;
  char secname[SEC_NAME_MAX];

  for(uint32 i = 0; i < elf.shnum; i++){
    uint64 off = elf.shoff + i * elf.shentsize;
    readi(ip, 0, (uint64)&sechdrdst, off, elf.shentsize);
    
    elfcopystr(ip, secname, strtab.sh_offset + sechdrdst.sh_name);

    if (memcmp(dstsecname, secname, strlen(dstsecname)) == 0)
      break;
  }
  return sechdrdst;
}

int 
findloweraddr(struct trace* map, uint64 pcaddr)
{
  uint64 roundedaddr;

  roundedaddr = pcaddr - 1;

  while(pcaddr - roundedaddr < map->fsize){
    if(roundedaddr == map->addr)
      return 1;
    roundedaddr -= 1;
  }
  return 0;
}

// Sequently scans tracemap linked list,
// decrements input addr until differens
// between initial value of addr less than
// map symbol size. If finds then copies
// map symbol name into fname 
int
btracelookup(uint64 addr, char* fname)
{
  struct trace* map;

  map = tracemap.head;

  for(uint32 i = 0; i < tracemap.n; i++){
    if(findloweraddr(map, addr)){
      memmove(fname, map->fname, strlen(map->fname));
      fname[strlen(map->fname)] = 0;
      return 0;
    }
    map = map->next;
  }
  char* notfound = "???\0";
  memmove(fname, notfound, 4);
  return -1;
}

// Adds map at the head of tracemap list.
// Saves new map into the BACKTRACE pages
void
savefuncmap(char* fname, uint64 addr, uint64 fsize)
{
  struct trace* map = (struct trace*)((uint64)BACKTRACE + tracemap.n * sizeof(struct trace));

  memmove(map->fname, fname, strlen(fname));
  map->fname[strlen(fname)] = 0;

  map->addr = addr;
  map->fsize = fsize;

  map->next = tracemap.head;
  tracemap.head = map;
  tracemap.n += 1;
}

void
btracecache()
{
  char* path;
  struct elfhdr elf;
  struct inode *ip;

  struct sechdr shstrtab, symtab, strtab, texthdr;

  path = "tracekernel";

  begin_op();

  if((ip = namei(path)) == 0)
    end_op();

  ilock(ip);

  // Check ELF header
  readi(ip, 0, (uint64)&elf, 0, sizeof(elf));
  
  if(elf.magic != ELF_MAGIC)
    goto cachefailed;

  // sechdr for .shstrtab
  uint64 off;
  off = elf.shoff + elf.shstrndx * elf.shentsize;
  readi(ip, 0, (uint64)&shstrtab, off, elf.shentsize);

  // sechdr for .symtab
  symtab = elfgetsecbyname(ip, elf, shstrtab, ".symtab");
  // sechdr for .strtab
  strtab = elfgetsecbyname(ip, elf, shstrtab, ".strtab");
  // sechdr for .text
  texthdr = elfgetsecbyname(ip, elf, shstrtab, ".text");

  // find index of section .text - nessesary
  // for searhing function names in future
  uint32 textidx;
  textidx = 2*elf.shnum;

  for(uint32 i = 0; i < elf.shnum; i++){
    struct sechdr sechdr;
    uint64 off = elf.shoff + i * elf.shentsize;
    
    readi(ip, 0, (uint64)&sechdr, off, sizeof(sechdr));
    
    if (sechdr.sh_name == texthdr.sh_name){
      textidx = i;
      break;
    }
  }
  // textidx must be different than 2*shnum
  if(textidx >= elf.shnum)
    goto cachefailed;

  // find func names in symtab and strtab sections
  struct sym sym;
  uint64 symtaboff;
  uint64 symtabsz;
  
  // symbol table offset
  symtaboff = symtab.sh_offset;
  // number of entries in symbol table
  symtabsz = symtab.sh_size / symtab.sh_entsize;
  
  for(uint32 i = 0; i < symtabsz; i++){
    uint64 off = symtaboff + i * symtab.sh_entsize;
    
    // read symbol from symbol table
    readi(ip, 0, (uint64)&sym, off, sizeof(sym));
    
    // if symbol refers to the section of
    // code and it's function - its name
    // should be mapped with its address  
    if(sym.st_shndx == textidx && (sym.st_info & STT_FUNC)){
      // get symbol name from strtab section
      char fname[MAXFNAME];
      elfcopystr(ip, fname, strtab.sh_offset + sym.st_name);
      
      // map function name with its addr
      savefuncmap(fname, sym.st_value, sym.st_size);
    }
  }

  iunlockput(ip);
  end_op();
  ip = 0;

cachefailed:
  if(ip){
    iunlockput(ip);
    end_op();
  }
}