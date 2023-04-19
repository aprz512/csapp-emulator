/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz
 * and shall not be used for commercial and profitting purpose
 * without yangminz's permission.
 */

#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sched.h>

#define PAGE_BYTES (4096)

int64_t sum = 0;
int64_t sum1 = 0, sum2 = 0;
int64_t result_page2[PAGE_BYTES / sizeof(int64_t)];
int64_t result_page3[PAGE_BYTES / sizeof(int64_t)];
int64_t sum4 = 0;

typedef struct
{
    int64_t *cache_write_ptr;
    int cpu_id;
    int length;
} param_t;

void *work_thread(void *param)
{
    param_t *p = (param_t *)param;
    int64_t *ptr = p->cache_write_ptr;
    int cpu_id = p->cpu_id;
    int length = p->length;

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);
    pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);

    printf("    * thread[%lu] running on cpu[%d] writes to %p\n",
           pthread_self(), sched_getcpu(), ptr);

    for (int i = 0; i < length; ++i)
    {
        // write - not thread safe
        // just write to make cache line dirty
        *ptr += 1;
    }

    return NULL;
}

int LENGTH = 200000000;

void true_sharing_run()
{
    pthread_t t1, t2;

    param_t p1 = {
        .cache_write_ptr = &sum,
        .cpu_id = 0,
        .length = LENGTH};

    param_t p2 = {
        .cache_write_ptr = &sum,
        .cpu_id = 1,
        .length = LENGTH};

    long t0 = clock();

    pthread_create(&t1, NULL, work_thread, (void *)&p1);
    pthread_create(&t2, NULL, work_thread, (void *)&p2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("[True Sharing]\n\tresult: %ld; elapsed tick tock: %ld\n",
           sum, clock() - t0);
}

void single_run()
{
    long t0 = clock();
    for (int i = 0; i < LENGTH * 2; ++i)
    {
        sum4 += 1;
    }
    printf("[single]\n\tresult: %ld; elapsed tick tock: %ld\n",
           sum4, clock() - t0);
}

void false_sharing_run()
{
    pthread_t t1, t2;

    param_t p1 = {
        .cache_write_ptr = &sum1,
        .cpu_id = 0,
        .length = LENGTH};

    param_t p2 = {
        .cache_write_ptr = &sum2,
        .cpu_id = 1,
        .length = LENGTH};

    long t0 = clock();

    pthread_create(&t1, NULL, work_thread, (void *)&p1);
    pthread_create(&t2, NULL, work_thread, (void *)&p2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("[False Sharing]\n\tresult: %ld; elapsed tick tock: %ld\n",
           sum1 + sum2, clock() - t0);
}

void no_sharing_run()
{
    pthread_t t1, t2;

    param_t p1 = {
        .cache_write_ptr = &result_page2[0],
        .cpu_id = 0,
        .length = LENGTH};

    param_t p2 = {
        .cache_write_ptr = &result_page3[0],
        .cpu_id = 1,
        .length = LENGTH};

    long t0 = clock();

    pthread_create(&t1, NULL, work_thread, (void *)&p1);
    pthread_create(&t2, NULL, work_thread, (void *)&p2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("[No Sharing]\n\tresult: %ld; elapsed tick tock: %ld\n",
           result_page2[0] + result_page3[0], clock() - t0);
}

int main()
{
    single_run();
    true_sharing_run();
    false_sharing_run();
    no_sharing_run();
}