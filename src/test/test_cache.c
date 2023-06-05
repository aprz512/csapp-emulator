#include <stdio.h>
#include <string.h>
#include "headers/address.h"
#include "headers/memory.h"
#include "headers/cachetest.h"
#include "headers/common.h"
#include <assert.h>
#include "headers/mmu.h"


void increase_hit_count();
void increase_miss_count();
void increase_evicted_count();
void fill_trace_buf(char *result);
void reset_buf();
void append_buf(char *ch);
void trim(char *ch);
uint64_t read_addr(char *line);
void check_counter(char *line);


int main(int argc, char *argv[])
{

    char traces[5][30] = {
        "../traces/dave.trace",
        "../traces/long.trace",
        "../traces/trans.trace",
        "../traces/yi.trace",
        "../traces/yi2.trace",
    };

    char results[5][30] = {
        "../traces/result/dave.result",
        "../traces/result/long.result",
        "../traces/result/trans.result",
        "../traces/result/yi.result",
        "../traces/result/yi2.result",
    };

    for (size_t i = 0; i < 5; i++)
    {
        FILE *trace_fd = fopen(traces[i], "r");
        if (trace_fd == NULL)
        {
            printf("can not open %s\n", traces[i]);
            exit(-1);
        }

        FILE *result_fd = fopen(results[i], "r");
        if (result_fd == NULL)
        {
            printf("can not open %s\n", results[i]);
            exit(-1);
        }

        cache_test_tool.open = 1;
        cache_test_tool.cache_hit_count = 0;
        cache_test_tool.cache_miss_count = 0;
        cache_test_tool.cache_evict_count = 0;
        cache_test_tool.dirty_bytes_in_cache_count = 0;
        cache_test_tool.dirty_bytes_evicted_count = 0;
        cache_test_tool.trace_ptr = (char *)&cache_test_tool.trace_buf;
        sram_reset();

        int max_line_length = 128;
        char trace_line[max_line_length];
        char result_line[max_line_length];

        while (fgets(trace_line, max_line_length, trace_fd) != NULL)
        {
            int len = strlen(trace_line);
            trim(trace_line);

            if (len <= 0)
            {
                break;
            }

            if (trace_line[0] == 'I')
            {
                continue;
            }

            reset_buf();
            // append_buf(trace_line);
            // my_log(DEBUG_CACHE, "line[0] = |%c|\n", trace_line[0]);
            if (trace_line[0] == 'L')
            {
                // char *addr = strtok(trace_line[2], " ");
                sram_cache_read(read_addr(trace_line));
            }
            else if (trace_line[0] == 'S')
            {
                // char *addr = strtok(trace_line[2], " ");
                sram_cache_write(read_addr(trace_line), 0);
            }
            else if (trace_line[0] == 'M')
            {
                // char *addr = strtok(trace_line[2], " ");
                sram_cache_read(read_addr(trace_line));
                sram_cache_write(read_addr(trace_line), 0);
            }
            else
            {
                my_log(DEBUG_CACHE, "failed, unknow line: [%s]\n", trace_line);
                exit(0);
            }

            fgets(result_line, max_line_length, result_fd);
            trim(result_line);
            // trim()

            int buf_len = strlen(cache_test_tool.trace_buf);
            int result_len = strlen(result_line);
            int search_index = buf_len;
            while (search_index < buf_len)
            {
                if (cache_test_tool.trace_buf[buf_len - 1 - search_index] != result_line[result_len - 1 - search_index])
                {
                    my_log(DEBUG_CACHE, "failed, cache line: [%s], result line: [%s]\n", cache_test_tool.trace_buf, result_line);
                    exit(0);
                }

                search_index++;
            }

            my_log(DEBUG_CACHE, "pass, cache line: [%s], result line: [%s]\n", cache_test_tool.trace_buf, result_line);
        }

        fgets(result_line, max_line_length, result_fd);
        trim(result_line);
        check_counter((char *)&result_line);
    }
    return 0;
}