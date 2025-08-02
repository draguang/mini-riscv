// #include <common.h>
// #include <elf.h>
// #include <unistd.h>
// #include <fcntl.h>

// void parse_args_ftrace(char *path)
// {
//     //load the file and record the offset
//     int elf_file = open(path,O_RDONLY);
//     if(elf_file<0)  panic("elf opening failed!");
//     //ready for the head
//     Elf32_Ehdr ehdr;
//     read(elf_file,&ehdr,sizeof(ehdr));
//     lseek(elf_file,e_shoff,SEEK_SET);//e_shoff what meaning????SEEK_SET!!!!!!!!
//     Elf32_Shdr shdr[ehdr.e_shnum];//record the sh section
//     read(elf_file,shdr,sizeof(Elf32_Shdr)*ehdr.e_shnum);
//     //analyse the strtab
//     char *shstrtab = malloc(shdr[ehdr.e_shstrndx].sh_size);
//     lseek(elf_file,shdr[ehdr.e_shstrndx].sh_offset,SEEK_SET);
//     read(elf_file,shstrtab,shdr[ehdr.e_shstrndx].sh_size);
//     Elf32_Shdr *strtab = NULL,*symtab = NULL;
//     //遍历所有节区头找到两个表的起始位置
//     for(int i = 0;i<ehdr.e_shnum;i++)
//     {
//         char*name = shstrtab+shdr[i].sh_name;
//         if(strcmp(name,"strtab") == 1)   strtab = &shdr[i];
//         if(strcmp(name,"symtab") == 1)   symtab = &shdr[i];
//     }
//     if(!strtab||!symtab)
//     {
//         printf("didn't find the start of tab");
//         panic();
//     }
//     //load the name of function(strtab)
//     char *strtab_data = malloc(strtab->sh_size);
//     lseek(elf_file,strtab->sh_offset,SEEK_SET);
//     read(elf_file,strtab_data,strtab->sh_size);
//     //load symbol tab
//     Elf32_Sym* symbols = malloc(symtab->sh_size);
//     lseek(elf_file,symtab->sh_offset,SEEK_SET);
//     read(elf_file,symbols,symtab->sh_size);
// }
// void init_ftrace()
// {

// }