#include "common.h"
#include <elf.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

void panic(const char *sen)
{
    printf("%s",sen);
    exit(1);
}
typedef struct mytab{
    char *strtab_data;
    Elf32_Sym*symbols;
    Elf32_Shdr*symtab;
    Elf32_Shdr*strtab;
}Tab;
Tab *tab_data = NULL;
Tab parse_args_ftrace(const char *path)
{
    //打开 ELF 文件记录偏移量
    FILE* elf_file = fopen(path,"rb");
    if(!elf_file)
    {
        fprintf(stderr, "Failed to open ELF file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    //读取ELF头部
    Elf32_Ehdr ehdr;
    if(fread(&ehdr,sizeof(ehdr),1,elf_file)!=1)
    {
        fclose(elf_file);
        panic("cannot read the header location");
    }
    //定位到节区头
    if(fseek(elf_file,ehdr.e_shoff,SEEK_SET)!=0)
    {
        fclose(elf_file);
        panic("seek to header failed");   
    }
    //读取节区头
    Elf32_Shdr *shdr = (Elf32_Shdr*)malloc(sizeof(Elf32_Shdr)*ehdr.e_shnum);
    if(!shdr)
    {
        fclose(elf_file);
        panic("header alloction failed");
    }
    if(fread(shdr,sizeof(Elf32_Shdr),ehdr.e_shnum,elf_file)!=ehdr.e_shnum)
    {
        free(shdr);
        fclose(elf_file);
        panic("cannot read header");
    }
    //读取节区名称字符串表
    char *shstrtab = (char*)malloc(shdr[ehdr.e_shstrndx].sh_size);
    if (!shstrtab) {
        free(shdr);
        fclose(elf_file);
        panic("shstrtab allocation failed");
    }
    if(fseek(elf_file,shdr[ehdr.e_shstrndx].sh_offset,SEEK_SET)!=0||
        fread(shstrtab,shdr[ehdr.e_shstrndx].sh_size,1,elf_file)!=1)
    {
        free(shstrtab);
        free(shdr);
        fclose(elf_file);
        panic("section name string table read failed");
    }
    Elf32_Shdr *strtab = NULL,*symtab = NULL;
    //遍历所有节区头找到两个表的起始位置
    for(int i = 0;i<ehdr.e_shnum;i++)
    {
        char*name = shstrtab+shdr[i].sh_name;
        if(strcmp(name,".strtab") == 0)   strtab = &shdr[i];
        if(strcmp(name,".symtab") == 0)   symtab = &shdr[i];
    }
    if(!strtab||!symtab)
    {
        free(shstrtab);
        free(shdr);
        fclose(elf_file);
        panic("sections not found");
    }
    //加载字符串表数据
    char *strtab_data = (char*)malloc(strtab->sh_size);
    if (!strtab_data) {
        free(shstrtab);
        free(shdr);
        fclose(elf_file);
        panic("strtab's data allocation failed");
    }
    
    if (fseek(elf_file, strtab->sh_offset, SEEK_SET) != 0 ||
        fread(strtab_data, strtab->sh_size, 1, elf_file) != 1) {
        free(strtab_data);
        free(shstrtab);
        free(shdr);
        fclose(elf_file);
        panic("string table read failed");
    }
    //加载符号表数据
    Elf32_Sym* symbols = (Elf32_Sym*)malloc(symtab->sh_size);
    if(!symbols)
    {
        free(shstrtab);
        free(shdr);
        fclose(elf_file);
        panic("symtab's data reading failed");
    }
    if(fseek(elf_file,symtab->sh_offset,SEEK_SET)!=0||
        fread(symbols,symtab->sh_size,1,elf_file)!=1)
    {
        free(symbols);
        free(strtab_data);
        free(shstrtab);
        free(shdr);
        panic("symbol table read failed");
    }
    Tab mytab = {
        strtab_data,
        symbols,
        symtab,
        strtab
    };
    return mytab;
}
void init_ftrace(const char *path)
{
    if(!path)   panic("path no");
    if(!tab_data) {
        tab_data = (Tab*)malloc(sizeof(Tab));
        if(!tab_data) panic("tab_data allocation failed");
    }
    if (tab_data->strtab_data) free(tab_data->strtab_data);
    if (tab_data->symbols) free(tab_data->symbols);
    *tab_data = parse_args_ftrace(path);
}
int tab_depth = 1;
void call_function(word_t pc,word_t dnpc)
{
    if(!tab_data || !tab_data->symbols)
    {
        printf("没解析出东西");
        return;
    }
    size_t sym_count = tab_data->symtab->sh_size / sizeof(Elf32_Sym);
    char*name = NULL;
    for(size_t i = 0;i<sym_count;i++)
    {
        Elf32_Sym *sym = &tab_data->symbols[i];
        
        if(ELF32_ST_TYPE(sym->st_info) == STT_FUNC&&dnpc>=sym->st_value&&dnpc<sym->st_value+sym->st_size)
        {
            if(sym->st_name < tab_data->strtab->sh_size) {
                name = tab_data->strtab_data + sym->st_name;
                break;
            }
        }
    }
    if(!name)
    {
        name = "???";
    }
    printf("0x%08x:",pc);
    tab_depth++;
    for(int i = 0;i<tab_depth;i++)
    {
        printf("  ");
    }
    printf("call [%s@0x%08x]\n",name,dnpc);
    
}
void ret_function(word_t pc,word_t dnpc)
{
    if(!tab_data || !tab_data->symbols)
    {
        printf("没解析出东西");
        return;
    }
    size_t sym_count = tab_data->symtab->sh_size / sizeof(Elf32_Sym);
    char*name = NULL;
    for(size_t i = 0;i<sym_count;i++)
    {
        Elf32_Sym *sym = &tab_data->symbols[i];
        
        if(ELF32_ST_TYPE(sym->st_info) == STT_FUNC&&dnpc>=sym->st_value&&dnpc<sym->st_value+sym->st_size)
        {
            if(sym->st_name < tab_data->strtab->sh_size) {
                name = tab_data->strtab_data + sym->st_name;
                break;
            }
        }
    }
    if(!name)
    {
        name = "???";
    }
    printf("0x%08x:",pc);
    tab_depth--;
    for(int i = 0;i<tab_depth;i++)
    {
        printf("  ");
    }
    printf("ret [%s]\n",name);
}