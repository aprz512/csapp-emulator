#ifndef MY_TRIE_HEADER
#define MY_TRIE_HEADER

#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>
#include <stdlib.h>
#include <headers/log.h>
#include <headers/cpu.h>
#include <assert.h>

#define MAX 27

typedef struct MY_TRIE_NODE my_trie_node_t;

struct MY_TRIE_NODE
{
    // flag in the tree
    bool leaf;

    // letter in operator and register name
    char letter;

    // do not want tow node strcut
    // so can use union
    union
    {
        uint64_t register_address;
        int operator_type;
    };

    my_trie_node_t *next[MAX];
};

typedef struct
{
    char name[64];
    uint64_t value;
} init_t;

static my_trie_node_t *create_node(char letter, bool leaf, bool reg, uint64_t addr, int type);

void init_register_tree(my_trie_node_t **register_tree, init_t *elements, size_t node_len);
uint64_t register_address(my_trie_node_t *register_tree, const char *reg);
void TestRegisterTrie();

void init_operator_tree(my_trie_node_t **operator_tree, init_t *elements, size_t node_len);
int operator_type(my_trie_node_t *operator_tree, const char *op);
void TestOperatorTree();
void TestOperatorTree2();

#endif