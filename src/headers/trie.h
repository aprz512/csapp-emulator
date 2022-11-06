#ifndef TRIE_HEADER
#define TRIE_HEADER

#include <stdint.h>

#define MAX 26

typedef struct TRIE_NODE_STRUCT
{
    int type;
    union
    {
        uint64_t reg_address;
        int operator_type;
    };
    struct TRIE_NODE_STRUCT *node[MAX];

} InstPrefixTreeNode;

static InstPrefixTreeNode *operator_trie;
static InstPrefixTreeNode *register_trie;

#endif