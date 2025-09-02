#include "common.h"
#include "svdpi.h"

uint8_t*ram;
uint8_t*rom;

void load(const char* filename,uint8_t*rom,size_t max)
{
  FILE *file = fopen(filename,"rb");
  if(!file)
  {
    printf("open failed!\n");
    fprintf(stderr, "Failed to open file: %s\n", filename);
    exit(1);
  }
  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  fseek(file, 0, SEEK_SET);
  if (file_size > max) 
  {
    printf("file is flowing\n");
    fclose(file);
    exit(1);
  }
  size_t bytes_read = fread(rom+0x80000000, 1, file_size, file);
  if (bytes_read!=file_size)
  {
    printf("reading failed!\n");
    fclose(file);
    exit(1);
  }
}

extern "C" uint8_t pmem_read_ram(int raddr){
  switch(raddr-(raddr&~0x3u))
  {
    case 0:return ram[raddr&~0x3u];
    case 1:return ram[raddr&~0x3u+1];
    case 2:return ram[raddr&~0x3u+2];
    case 3:return ram[raddr&~0x3u+3];
    default:return 0;
  }
  
}
extern "C" int pmem_read_rom(int raddr){
  if(raddr>=0xa0000048&&raddr<=0xa0000050) {
    time_t now = time(NULL);
    return (int)now;
  }
  return rom[raddr & ~0x3u]|rom[(raddr & ~0x3u)+1]<<8|rom[(raddr & ~0x3u)+2]<<16|rom[(raddr & ~0x3u)+3]<<24;
}
extern "C" void pmem_write(int waddr, int wdata, int wmask){
  if(waddr>=0xa00003f8&&waddr<=0xa0000400) {putchar(wdata);}
  int waddr_a = waddr & ~0x3u;
  int32_t original_data = 
        (ram[waddr_a])           |
        (ram[waddr_a + 1] << 8)  |
        (ram[waddr_a + 2] << 16) |
        (ram[waddr_a + 3] << 24);
  int32_t new_data = original_data;
    if (wmask & 0x1) new_data = (new_data & ~0x000000FFu) | (wdata & 0x000000FFu);
    if (wmask & 0x2) new_data = (new_data & ~0x0000FF00u) | (wdata & 0x0000FF00u); 
    if (wmask & 0x4) new_data = (new_data & ~0x00FF0000u) | (wdata & 0x00FF0000u); 
    if (wmask & 0x8) new_data = (new_data & ~0xFF000000u) | (wdata & 0xFF000000u);
  ram[waddr_a + 0] = (new_data >> 0)  & 0xFF;
  ram[waddr_a + 1] = (new_data >> 8)  & 0xFF;
  ram[waddr_a + 2] = (new_data >> 16) & 0xFF;
  ram[waddr_a + 3] = (new_data >> 24) & 0xFF;
}