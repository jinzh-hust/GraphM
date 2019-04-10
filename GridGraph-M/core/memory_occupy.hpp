#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

typedef struct MEM_PACKED //定义一个mem occupy的结构体
{
    char name[20]; //定义一个char类型的数组名name有20个元素
    unsigned long total;
    char name2[20];
} MEM_OCCUPY;

typedef struct MEM_PACK //定义一个mem occupy的结构体
{
    double total,used_rate;
} MEM_PACK;

double get_memoccupy () // get RAM message
{
    FILE *fd;
    int n;
    double mem_total,mem_used_rate;;
    char buff[256];
    MEM_OCCUPY *m=(MEM_OCCUPY *)malloc(sizeof(MEM_OCCUPY));;
    MEM_PACK *p=(MEM_PACK *)malloc(sizeof(MEM_PACK));
    fd = fopen ("/proc/meminfo", "r");
    fgets (buff, sizeof(buff), fd);
    sscanf (buff, "%s %lu %s\n", m->name, &m->total, m->name2);
    mem_total=m->total;
    fgets (buff, sizeof(buff), fd);
    sscanf (buff, "%s %lu %s\n", m->name, &m->total, m->name2);
    mem_used_rate=(1-m->total/mem_total)*100;
    mem_total=mem_total/(1024*1024);
    p->total=mem_total;
    p->used_rate=mem_used_rate;
    fclose(fd); //关闭文件fd
    return p->total;
}
