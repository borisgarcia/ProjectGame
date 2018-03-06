#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define EI_NIDENT 16

typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef int32_t  Elf32_Sword;
typedef uint32_t Elf32_Word;

typedef struct {
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    Elf32_Word p_type;
    Elf32_Off p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
} Elf32_Phdr;

typedef struct {
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
} Elf32_Shdr;

/* Section Types, sh_type */
#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11
#define SHT_LOPROC 0x70000000
#define SHT_HIPROC 0x7fffffff
#define SHT_LOUSER 0x80000000
#define SHT_HIUSER 0xffffffff

/* Section Attribute Flags, sh_flags */
#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MASKPROC 0xf0000000

/* File class types */
#define ELFCLASSNONE    0
#define ELFCLASS32      1
#define ELFCLASS64      2

/* ELF file data encoding */
#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

/* Program segment type */
#define PT_NULL     0
#define PT_LOAD     1
#define PT_DYNAMIC  2
#define PT_INTERP   3
#define PT_NOTE     4
#define PT_SHLIB    5
#define PT_PHDR     6
#define PT_LOPROC   0x70000000
#define PT_HIPROC   0x7fffffff

/* Section Header Types */
#define SHT_NULL        0
#define SHT_PROGBITS    1
#define SHT_SYMTAB      2
#define SHT_STRTAB      3
#define SHT_RELA        4
#define SHT_HASH        5
#define SHT_DYNAMIC     6
#define SHT_NOTE        7
#define SHT_NOBITS      8
#define SHT_REL         9
#define SHT_SHLIB       10
#define SHT_DYNSYM      11
#define SHT_LOPROC      0x70000000
#define SHT_HIPROC      0x7fffffff
#define SHT_LOUSER      0x80000000
#define SHT_HIUSER      0xffffffff

typedef struct {
    uint32_t cs_addr;
    uint32_t ds_addr;
    uint32_t cs_size;
    uint32_t ds_size;
    const char *outfile_name;
    const char *infile_name;
} arguments_t;

int  dumpTextSegmentToHex(uint8_t *buffer, int count, FILE *f);
int  dumpDataSegmentToHex(uint8_t *buffer, int count, FILE *f);
void paddFile(int curr_size, int new_size, FILE *f);
void convertToLittleEndian(Elf32_Ehdr *ehdr);
void sectionHeaderToLittleEndian(Elf32_Shdr *shdr);
void programHeaderToLittleEndian(Elf32_Phdr *phdr);
Elf32_Shdr* loadSectionHeaders(Elf32_Ehdr *elf_hdr, FILE *f);
char*       loadSectionContent(Elf32_Shdr *shdr, FILE *f);
void show_usage();
int parse_args(int argc, char *argv[], arguments_t *args);

int main(int argc, char *argv[])
{
    if (argc < 2) {
        show_usage(argv[0]);
        return 1;
    }

    arguments_t args;

    args.cs_addr = 0x0;
    args.ds_addr = 0x2000;
    args.cs_size = 0;
    args.ds_size = 0;
    args.outfile_name = NULL;
    args.infile_name = NULL;

    if (parse_args(argc, argv, &args) != 0) {
        show_usage(argv[0]);
        return 1;
    }

    FILE *f, *fmem;
    Elf32_Ehdr ehdr;

    f = fopen(args.infile_name, "rb");

    if (f == NULL) {
        printf("Cannot open executable file %s\n", argv[1]);
        return 1;
    }

    fmem = fopen(args.outfile_name, "wb+");

    if (fmem == NULL) {
        printf("Cannot open memory output file %s\n", argv[2]);
        fclose(f);
        return 1;
    }

    fread( (char*)&ehdr, 1, sizeof(Elf32_Ehdr), f );

    if (ehdr.e_ident[4] != ELFCLASS32) {
        printf("The file is not a MIPS32 executable file\n");
        fclose(f);
        fclose(fmem);
        return 1;
    }

    if (ehdr.e_ident[5] == ELFDATA2MSB) {
        convertToLittleEndian(&ehdr);
    }

    /* Program Headers */
    Elf32_Phdr phdr;

    fseek(f, ehdr.e_phoff, SEEK_SET);

    /* Section headers */
    Elf32_Shdr *sectionHeaders = loadSectionHeaders(&ehdr, f);
    char *stringTable = loadSectionContent(&sectionHeaders[ehdr.e_shstrndx], f);
    int i;

    // Dump text section
    printf("Dumping text section ...\n");

    fprintf(fmem, "@%08X\n", args.cs_addr);
    for (i = 0; i < ehdr.e_shnum; i++) {
        char *section_name = &stringTable[sectionHeaders[i].sh_name];

        if (strncmp(section_name, ".text", 4) == 0) {
           char *sectionContent = loadSectionContent(&sectionHeaders[i], f);
           int word_count;

           word_count = dumpTextSegmentToHex(sectionContent, sectionHeaders[i].sh_size, fmem);
           paddFile(word_count, args.cs_size, fmem);
           free(sectionContent);
        }
    }

    // Dump data section
    printf("Dumping data section ...\n");
    int word_count = 0;

    fprintf(fmem, "@%08X\n", args.ds_addr);
    for (i = 0; i < ehdr.e_shnum; i++) {
        char *section_name = &stringTable[sectionHeaders[i].sh_name];

        if (strcmp(section_name, ".data") == 0 ||
            strcmp(section_name, ".got") == 0) {
            char *sectionContent = loadSectionContent(&sectionHeaders[i], f);
            word_count += dumpDataSegmentToHex(sectionContent, sectionHeaders[i].sh_size, fmem);
            free(sectionContent);
        } else if (strcmp(section_name, ".bss") == 0) {
            char *sectionContent = malloc(sectionHeaders[i].sh_size);
            paddFile(word_count, sectionHeaders[i].sh_size / 4, fmem);
            word_count += sectionHeaders[i].sh_size / 4;
        }
    }
    paddFile(word_count, args.ds_size, fmem);

    free(stringTable);
    free(sectionHeaders);

    fclose(f);
    fclose(fmem);
}

void show_usage(const char *program_name) {
    printf("Usage: %s [options] -o <output mem file> <elf executable file>\n", program_name);
    printf("Where options can be:\n");
    printf(" -ca <code address> is the start address (hex) of code section as defined in the BMM file.\n");
    printf(" -da <data address> is the start address (hex) of data section as defined in the BMM file.\n");
    printf(" -cs <code size> the code section size. Fill as needed but no trucation is done.\n");
    printf(" -ds <data size> the data section size. Fill as needed but no trucation is done..\n");
}

int parse_args(int argc, char *argv[], arguments_t *args) {
    int i = 1;

    while (i < argc - 1) {
        if (strcmp(argv[i], "-o") == 0) {
            i++;
            if (i >= argc - 1) {
                return 1;
            }
            args->outfile_name = argv[i];
            i++;
        } else if (strcmp(argv[i], "-ca") == 0) {
            i++;
            if (i >= argc - 1) {
                return 1;
            }
            args->cs_addr = strtoul(argv[i], NULL, 16);
            i++;
        } else if (strcmp(argv[i], "-da") == 0) {
            i++;
            if (i >= argc - 1) {
                return 1;
            }
            args->ds_addr = strtoul(argv[i], NULL, 16);
            i++;
        } else if (strcmp(argv[i], "-cs") == 0) {
            i++;
            if (i >= argc - 1) {
                return 1;
            }
            args->cs_size = strtoul(argv[i], NULL, 10);
            i++;
        } else if (strcmp(argv[i], "-ds") == 0) {
            i++;
            if (i >= argc - 1) {
                return 1;
            }
            args->ds_size = strtoul(argv[i], NULL, 10);
            i++;
        } else {
            return 1;
        }
    }

    args->infile_name = argv[i];

    return 0;
}

int dumpTextSegmentToHex(uint8_t *buffer, int count, FILE *f) {
    int i, word_count = 0;

    while (i < count) {
        uint8_t b0 = buffer[i + 3];
        uint8_t b1 = buffer[i + 2];
        uint8_t b2 = buffer[i + 1];
        uint8_t b3 = buffer[i + 0];

		// Dump the whole word, first 3 lines are 9 bits, the last one is 5 bits
        fprintf(f, "%01X%02X", b1 & 0x1, b0);
        fprintf(f, "%01X%02X", (b2 & 0x3) >> 1, ((b2 & 0x1) << 7) | (b1 >> 1));
        fprintf(f, "%01X%02X", (b3 & 0x7) >> 2, ((b3 & 0x3) << 6) | (b2 >> 2));
        fprintf(f, "%01X%02X",               0,                     (b3 >> 3));

        fprintf(f, "\n");

        i += 4;
        word_count++;
    }

    return word_count;
}

int dumpDataSegmentToHex(uint8_t *buffer, int count, FILE *f) {
    int i, word_count = 0;

    while (i < count) {
        uint8_t b0 = buffer[i + 3];
        uint8_t b1 = buffer[i + 2];
        uint8_t b2 = buffer[i + 1];
        uint8_t b3 = buffer[i + 0];

        fprintf(f, "%02X%02X%02X%02X\n", b3, b2, b1, b0);

        i += 4;
        word_count++;
    }

    return word_count;
}

void paddFile(int curr_size, int new_size, FILE *f) {
    int word_count = new_size - curr_size;

    for (int i = 0; i < word_count; i++) {
        fprintf(f, "00000000\n");
    }
}

Elf32_Half halfWordToLittleEndian(Elf32_Half hword) {
    Elf32_Half result;

    result = ((hword & 0xff00) >> 8);
    result |= ((hword & 0x00ff) << 8);

    return result;
}

Elf32_Word wordToLittleEndian(Elf32_Word word) {
    Elf32_Word result;

    result = ((word & 0xff000000) >> 24);
    result |= ((word & 0x00ff0000) >> 8);
    result |= ((word & 0x0000ff00) << 8);
    result |= ((word & 0x000000ff) << 24);

    return result;
}

void convertToLittleEndian(Elf32_Ehdr *ehdr) {
    ehdr->e_type = halfWordToLittleEndian(ehdr->e_type);
    ehdr->e_machine = halfWordToLittleEndian(ehdr->e_machine);
    ehdr->e_version = wordToLittleEndian(ehdr->e_version);
    ehdr->e_entry = wordToLittleEndian(ehdr->e_entry);
    ehdr->e_phoff = wordToLittleEndian(ehdr->e_phoff);
    ehdr->e_shoff = wordToLittleEndian(ehdr->e_shoff);
    ehdr->e_flags = wordToLittleEndian(ehdr->e_flags);
    ehdr->e_ehsize = halfWordToLittleEndian(ehdr->e_ehsize);
    ehdr->e_phentsize = halfWordToLittleEndian(ehdr->e_phentsize);
    ehdr->e_ehsize = halfWordToLittleEndian(ehdr->e_ehsize);
    ehdr->e_phnum = halfWordToLittleEndian(ehdr->e_phnum);
    ehdr->e_shentsize = halfWordToLittleEndian(ehdr->e_shentsize);
    ehdr->e_shnum = halfWordToLittleEndian(ehdr->e_shnum);
    ehdr->e_shstrndx = halfWordToLittleEndian(ehdr->e_shstrndx);
}

void programHeaderToLittleEndian(Elf32_Phdr *phdr) {
    phdr->p_type = wordToLittleEndian(phdr->p_type);
    phdr->p_offset = wordToLittleEndian(phdr->p_offset);
    phdr->p_vaddr = wordToLittleEndian(phdr->p_vaddr);
    phdr->p_paddr = wordToLittleEndian(phdr->p_paddr);
    phdr->p_filesz = wordToLittleEndian(phdr->p_filesz);
    phdr->p_memsz = wordToLittleEndian(phdr->p_memsz);
    phdr->p_flags = wordToLittleEndian(phdr->p_flags);
    phdr->p_align = wordToLittleEndian(phdr->p_align);
}

void sectionHeaderToLittleEndian(Elf32_Shdr *shdr) {
    shdr->sh_name=wordToLittleEndian(shdr->sh_name);
    shdr->sh_type=wordToLittleEndian(shdr->sh_type);
    shdr->sh_flags=wordToLittleEndian(shdr->sh_flags);
    shdr->sh_addr=wordToLittleEndian(shdr->sh_addr);
    shdr->sh_offset=wordToLittleEndian(shdr->sh_offset);
    shdr->sh_size=wordToLittleEndian(shdr->sh_size);
    shdr->sh_link=wordToLittleEndian(shdr->sh_link);
    shdr->sh_info=wordToLittleEndian(shdr->sh_info);
    shdr->sh_addralign=wordToLittleEndian(shdr->sh_addralign);
    shdr->sh_entsize=wordToLittleEndian(shdr->sh_entsize);
}

Elf32_Shdr *loadSectionHeaders(Elf32_Ehdr *elf_hdr, FILE *f) {
    Elf32_Shdr *sectionHeaders = malloc(elf_hdr->e_shnum * sizeof(Elf32_Shdr));

    fseek(f, elf_hdr->e_shoff, SEEK_SET);

    for (int i = 0; i < elf_hdr->e_shnum; i++) {

        fread((char *)&(sectionHeaders[i]), 1, sizeof(Elf32_Shdr), f);

        if (elf_hdr->e_ident[5] == ELFDATA2MSB) {
            sectionHeaderToLittleEndian(&(sectionHeaders[i]));
        }
    }

    return sectionHeaders;
}

char *loadSectionContent(Elf32_Shdr *shdr, FILE *f) {
    char *buff = malloc(shdr->sh_size);

    if (buff == NULL)
        return NULL;

    long origPos = ftell(f);

    fseek(f, shdr->sh_offset, SEEK_SET);
    fread(buff, 1, shdr->sh_size, f);
    fseek(f, origPos, SEEK_SET);

    return buff;
}
