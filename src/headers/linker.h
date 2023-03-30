#ifndef LINKER_GUARD
#define LINKER_GUARD

#include <stdint.h>
#include <stdlib.h>

#define MAX_CHAR_SECTION_NAME (32)
#define MAX_CHAR_SYMBOl_NAME (32)

typedef struct
{
    char sh_name[MAX_CHAR_SECTION_NAME]; // .text 之类的
    uint64_t sh_addr;                    // runtime 地址
    uint64_t sh_offset;                  // 该 section 从 txt 文件的第几行开始
    uint64_t sh_size;                    // 该 section 占据 txt 文件的几行
} sh_entry_t;

// 正所谓二八定律，该 linker 只处理几种形式的 type 与 bind，其他的暂不考虑
// 全部 type 与 bind 可以参考 elf.h 中的 symbol binding 与 symbal type 部分
typedef enum
{
    STB_LOCAL,
    STB_GLOBAL,
    STB_WEAK
} st_bind_t;

typedef enum
{
    STT_NOTYPE,
    STT_OBJECT,
    STT_FUNC
} st_type_t;

typedef struct
{
    char st_name[MAX_CHAR_SYMBOl_NAME];
    st_bind_t bind;
    st_type_t type;
    char st_shndx[MAX_CHAR_SECTION_NAME];
    uint64_t st_value;      // 符号表在 section 内的偏移
    uint64_t st_size;       // 改符号占据几行，函数占据很多行，变量一般占据1行
} st_entry_t;


// 因为是处理 txt 文件，所以需要将全部字符串内容读到内存里面
// 为了简单，限制一下 txt 文件里面的信息大小
#define MAX_ELF_FILE_LENGTH (64) // txt 有效信息最大长度（不包括括号之类的）
#define MAX_ELF_FILE_WIDTH (128) // txt 一行的最大宽度

typedef struct
{
    char buffer[MAX_ELF_FILE_LENGTH][MAX_ELF_FILE_WIDTH];
    uint64_t line_count;    // 有效行
    uint64_t sht_count;     // section header 数量
    sh_entry_t *sht;        // section header 内容
    uint64_t symt_count;    // 符号表数量
    st_entry_t *symt;       // 符号表内容
} elf_t;

#define MAX_SYMBOL_MAP_LENGTH (64)
#define MAX_SECTION_BUFFER_LENGTH (64)

// 用来储存两个源文件的链接映射关系
typedef struct
{
    elf_t *elf;      // src elf file
    st_entry_t *src; // src symbol
    st_entry_t *dst; // 用作重定位
} smap_t;

void parse_elf(char *filename, elf_t *elf);
void free_elf(elf_t *elf);
void link_elf(elf_t **srcs, int num_srcs, elf_t *dst);

#endif