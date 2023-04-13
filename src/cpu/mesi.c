#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef enum
{
    M,
    E,
    S,
    I
} cache_line_state_t;

#define CORE_NUM (4)

typedef struct
{
    int cache;
    cache_line_state_t cacheline_state;
} core_t;

typedef struct
{
    core_t core[CORE_NUM];
} cpu_t;

static int mem_value = 10;
static cpu_t cpu;

void write_core_state(int core_num, int state)
{
    cpu.core[core_num].cacheline_state = state;
}

void write_core_val(int core_num, int val)
{
    cpu.core[core_num].cache = val;
}

int read_core_state(int core_num)
{
    return cpu.core[core_num].cacheline_state;
}

int read_core_val(int core_num)
{
    return cpu.core[core_num].cache;
}
int check_state()
{
    int m_count = 0;
    int e_count = 0;
    int s_count = 0;
    int i_count = 0;

    for (int i = 0; i < CORE_NUM; ++i)
    {
        if (read_core_state(i) == M)
        {
            m_count += 1;
        }
        else if (read_core_state(i) == E)
        {
            e_count += 1;
        }
        else if (read_core_state(i) == S)
        {
            s_count += 1;
        }
        else if (read_core_state(i) == I)
        {
            i_count += 1;
        }
    }

    /*
        M   E   S   I
    M   X   X   X   O
    E   X   X   X   O
    S   X   X   O   O
    I   O   O   O   O
    */

    printf("M %d\t E %d\t S %d\t I %d\n", m_count, e_count, s_count, i_count);

    if ((m_count == 1 && i_count == (CORE_NUM - 1)) ||
        (e_count == 1 && i_count == (CORE_NUM - 1)) ||
        (s_count >= 2 && i_count == (CORE_NUM - s_count)) ||
        (i_count == CORE_NUM))
    {
        return 1;
    }

    return 0;
}

int read(int core_num)
{
    if (read_core_state(core_num) == M)
    {
        printf("[%d] read hit; dirty value %d\n", core_num, read_core_val(core_num));
        // 缓存命中
        // M 也是独占的，所以可以直接返回缓存行的数据
        return cpu.core[core_num].cache;
    }
    else if (read_core_state(core_num) == E)
    {
        printf("[%d] read hit; exclusive clean value %d\n", core_num, read_core_val(core_num));
        // 缓存命中
        // E 也是独占的，所以可以直接返回缓存行的数据
        return cpu.core[core_num].cache;
    }
    else if (read_core_state(core_num) == S)
    {
        printf("[%d] read hit; shared clean value %d\n", core_num, read_core_val(core_num));
        // 缓存命中
        // S 是共享的，但是没有修改，所以可以直接返回缓存行的数据
        return cpu.core[core_num].cache;
    }
    else if (read_core_state(core_num) == I)
    {
        // 缓存未命中
        for (int i = 0; i < CORE_NUM; i++)
        {
            if (i == core_num)
            {
                continue;
            }

            if (read_core_state(i) == M)
            {
                printf("[%d] read miss; [%d] supplies dirty value %d; write back; s_count == 2\n", core_num, i, read_core_val(core_num));
                // 需要先将其他 core 的数据写回
                mem_value = cpu.core[i].cache;
                // 将状态都改为 S
                write_core_state(i, S);
                write_core_state(core_num, S);
                // 将 core i 的数据同步到 core num 上，这里不从内存中读取，太慢了，直接从 core i 的缓存里面取
                write_core_val(core_num, cpu.core[i].cache);
                return read_core_val(core_num);
            }
            else if (read_core_state(i) == E)
            {
                printf("[%d] read miss; [%d] supplies clean value %d; s_count == 2\n", core_num, i, read_core_val(core_num));
                // 将状态都改为 S
                write_core_state(i, S);
                write_core_state(core_num, S);
                // 将 core i 的数据同步到 core num 上，这里不从内存中读取，太慢了，直接从 core i 的缓存里面取
                write_core_val(core_num, cpu.core[i].cache);
                return read_core_val(core_num);
            }
            else if (read_core_state(i) == S)
            {
                printf("[%d] read miss; [%d] supplies clean value %d; s_count >= 3\n", core_num, i, read_core_val(core_num));
                // 将自身的状态改为 S
                write_core_state(core_num, S);
                // 将 core i 的数据同步到 core num 上，这里不从内存中读取，太慢了，直接从 core i 的缓存里面取
                write_core_val(core_num, cpu.core[i].cache);
                return read_core_val(core_num);
            }
        }

        // 将自身的状态改为 E
        write_core_state(core_num, E);
        write_core_val(core_num, mem_value);
        printf("[%d] read miss; mem supplies clean value %d; e_count == 1\n", core_num, read_core_val(core_num));
        return read_core_val(core_num);
    }
}

void write(int core_num, int value)
{
    if (read_core_state(core_num) == M)
    {
        // 缓存命中
        // M 是独占的，直接修改缓存即可
        write_core_val(core_num, value);
        printf("[%d] write hit; update to value %d\n", core_num, read_core_val(core_num));
        return;
    }
    else if (read_core_state(core_num) == E)
    {
        // 缓存命中
        // E 是独占的，所以直接改为 M 即可
        write_core_state(core_num, M);
        write_core_val(core_num, value);
        printf("[%d] write hit; update to value %d\n", core_num, read_core_val(core_num));
        return;
    }
    else if (read_core_state(core_num) == S)
    {
        // 广播，让其他的变为 I
        for (int j = 0; j < CORE_NUM; j++)
        {
            if (j == core_num)
            {
                continue;
            }
            write_core_state(j, I);
        }

        // 将自己的状态改成 M
        write_core_state(core_num, M);
        write_core_val(core_num, value);

        printf("[%d] write hit; broadcast invalid; update to value %d\n", core_num, read_core_val(core_num));
        return;
    }
    else if (read_core_state(core_num) == I)
    {
        for (int j = 0; j < CORE_NUM; ++j)
        {
            if (core_num == j)
            {
                continue;
            }

            if (read_core_state(j) == M)
            {
                // 需要将其他 core 的数据写回到内存
                mem_value = read_core_val(j);

                // 将其他 core 的状态置为 I
                write_core_state(j, I);

                // 从内存读入数据到缓存
                write_core_val(core_num, mem_value);
                write_core_state(core_num, M);
                // 写入数据
                write_core_val(core_num, value);

                printf("[%d] write miss; broadcast invalid to M; update to value %d\n", core_num, read_core_val(core_num));

                return;
            }
            else if (read_core_state(j) == E)
            {
                write_core_state(j, I);

                // 从内存读入数据到缓存
                write_core_val(core_num, mem_value);
                write_core_state(core_num, M);
                // 写入数据
                write_core_val(core_num, value);

                printf("[%d] write miss; broadcast invalid to E; update to value %d\n", core_num, read_core_val(core_num));

                return;
            }
            else if (read_core_state(j) == S)
            {

                for (int k = 0; k < CORE_NUM; ++k)
                {
                    if (core_num != k)
                    {
                        // 可能有多个 S，将所有置为 I
                        write_core_state(k, I);
                    }
                }

                // 从内存读入数据到缓存
                write_core_val(core_num, mem_value);
                write_core_state(core_num, M);
                // 写入数据
                write_core_val(core_num, value);

                printf("[%d] write miss; broadcast invalid to S; update to value %d\n", core_num, read_core_val(core_num));

                return;
            }
        }

        // 从内存读入数据
        write_core_val(core_num, mem_value);
        write_core_state(core_num, M);
        // 写入数据
        write_core_val(core_num, value);
        printf("[%d] write miss; all invalid; update to value %d\n", core_num, read_core_val(core_num));
    }
}

void evict(int core_num)
{
    if (read_core_state(core_num) == M)
    {
        // 将数据写回到内存
        write_core_val(core_num, mem_value);
        write_core_state(core_num, I);

        printf("[%d] evict; wrizte back value %d\n", core_num, read_core_val(core_num));
    }
    else if (read_core_state(core_num) == E)
    {
        // 改变状态即可
        write_core_state(core_num, I);

        printf("[%d] evict\n", core_num);
    }
    else if (read_core_state(core_num) == S)
    {
        // 改变状态即可
        write_core_state(core_num, I);

        // 检查，如果 S 状态只有一个了，那么就将该状态从 S 改成 E
        int s_count = 0;
        int last_s = -1;

        for (int j = 0; j < CORE_NUM; ++j)
        {
            if (read_core_state(j) == S)
            {
                last_s = j;
                s_count++;
            }
        }

        if (s_count == 1)
        {
            write_core_state(last_s, E);
        }

        printf("[%d] evict\n", core_num);
    }
}

void print_cacheline()
{
    for (int i = 0; i < CORE_NUM; ++i)
    {
        char c;

        switch (read_core_state(i))
        {
        case M:
            c = 'M';
            break;
        case E:
            c = 'E';
            break;
        case S:
            c = 'S';
            break;
        case I:
            c = 'I';
            break;
        default:
            c = '?';
        }

        printf("[%d] state %c value %d\n", i, c, read_core_val(i));
    }
    printf("mem value %d\n", mem_value);
}

int main()
{
    srand(123456);

    int read_value;

    for (int i = 0; i < CORE_NUM; ++i)
    {
        write_core_state(i, I);
        write_core_val(i, 0);
    }

    print_cacheline();

    for (int i = 0; i < 10; ++i)
    {
        int core_index = rand() % CORE_NUM;
        int op = rand() % 3;

        printf("core_index = %d, op = %d\n", core_index, op);

        if (op == 0)
        {
            read(core_index);
        }
        else if (op == 1)
        {
            write(core_index, rand() % 1000);
        }
        else if (op == 2)
        {
            evict(core_index);
        }

        print_cacheline();

        if (check_state() == 0)
        {
            printf("failed\n");

            return 0;
        }
    }

    printf("pass\n");

    return 0;
}