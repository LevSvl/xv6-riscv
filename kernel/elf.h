// Format of an ELF executable file

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

// File header
struct elfhdr {
  uint magic;  // must equal ELF_MAGIC
  uchar elf[12];
  ushort type;
  ushort machine;
  uint version;
  uint64 entry;
  uint64 phoff;
  uint64 shoff;
  uint flags;
  ushort ehsize;
  ushort phentsize;
  ushort phnum;
  ushort shentsize;
  ushort shnum;
  ushort shstrndx;
};

// Program section header
struct proghdr {
  uint32 type;
  uint32 flags;
  uint64 off;
  uint64 vaddr;
  uint64 paddr;
  uint64 filesz;
  uint64 memsz;
  uint64 align;
};

// Section header
struct sechdr {
  uint32	sh_name;		  /* 0-1    Section name (string tbl index) */
  uint32	sh_type;		  /* 2-3    Section type */
  uint64	sh_flags;		  /* 4-11   Section flags */
  uint64	sh_addr;		  /* 12-19  Section virtual addr at execution */
  uint64	sh_offset;	  /* 20-27  Section file offset */
  uint64	sh_size;		  /* 28-35  Section size in bytes */
  uint32	sh_link;		  /* 36-37  Link to another section */
  uint32	sh_info;		  /* 38-39  Additional section information */
  uint64	sh_addralign;	/* 40-47  Section alignment */
  uint64	sh_entsize;		/* 48-55  Entry size if section holds table */
};

// Symbol table entry
struct sym{
    uint32        st_name;
    uchar         st_info;
    uchar         st_other;
    ushort        st_shndx;
    uint64        st_value;
    uint64        st_size;
};

// Values for Proghdr type
#define ELF_PROG_LOAD           1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4

// Values for Symbol info using by backtrace
#define STT_FUNC	              2		/* Symbol is a code object */

#define SEC_NAME_MAX            32
