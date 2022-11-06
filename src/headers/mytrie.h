#ifndef MY_TRIE_HEADER
#define MY_TRIE_HEADER

#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>
#include <stdlib.h>
#include <headers/log.h>
#include <headers/cpu.h>

#define MAX 26

typedef struct
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

} my_trie_node_t;

static my_trie_node_t *register_tree;
static my_trie_node_t *operator_tree;

typedef struct
{
    char name[64];
    uint64_t value;
} init_t;

static my_trie_node_t *create_node(char letter, bool leaf, bool reg, uint64_t addr, int type)
{
    // create new node
    my_trie_node_t *newNode = (my_trie_node_t *)malloc(sizeof(my_trie_node_t));
    if (newNode == NULL)
    {
        my_log(DEBUG_TRIE, "unable to malloc memory for node");
        exit(0);
    }

    newNode->letter = letter;
    newNode->leaf = leaf;
    if (reg)
    {
        newNode->register_address = addr;
    }
    else
    {
        newNode->operator_type = type;
    }

    for (int i = 0; i < MAX; i++)
    {
        newNode->next[i] = NULL;
    }

    return newNode;
}

void init_register_tree(init_t *elements, size_t length)
{

    if (register_tree == NULL)
    {
        register_tree = create_node(0, false, false, 0, 0);
    }

    my_trie_node_t *node = register_tree;

    while (elements != NULL)
    {
        init_t init_ele = *elements;

        int index = 0;
        int insert = 0;
        char c = 0;
        while (init_ele.name[index] != '\0')
        {
            insert = init_ele.name[index] - 'a';
            if (node->next[insert] == NULL)
            {
                node->next[index] = create_node(init_ele.name[index], false, true, init_ele.value, 0);
            }
            else
            {
                node = node->next[index];
            }
        }

        elements++;
    }

    node->leaf = true;
}

// my_trie_node_t * getRegisterTree()
// {
//     return register_tree;
// }

void init_operator_tree(char **words)
{
}

// my_trie_node_t * getOperatorTree()
// {

// }

int operator_type(char *op)
{
}

uint64_t register_address(char *reg)
{
    my_trie_node_t *node = register_tree;
    int index = 0;
    int insert = 0;
    while (reg[index] != '0')
    {
        insert = reg[index] - 'a';

        if (node->next[insert] == NULL)
        {
            my_log(DEBUG_TRIE, "can not find reg named: %s\n", reg);
            exit(0);
        }
        node = node->next[insert];

        index++;
    }

    if (!node->leaf)
    {
        my_log(DEBUG_TRIE, "reg name is not leaf: %s\n", reg);
        exit(0);
    }

    return node->register_address;
}


void TestRegisterTrie()
{

    init_t init_elements[16] = {
        {"rax", &cput},
    };

}

#endif