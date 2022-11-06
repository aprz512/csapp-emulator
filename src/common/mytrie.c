#include "headers/mytrie.h"

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

void init_register_tree(my_trie_node_t **register_tree, init_t *elements, size_t node_len)
{

    if (*register_tree == NULL)
    {
        *register_tree = create_node(0, false, false, 0, 0);
    }

    size_t node_index = 0;

    while (node_index < node_len)
    {
        my_trie_node_t *temp_node = *register_tree;

        init_t init_ele = *elements;

        int char_index = 0;
        int insert = 0;
        char c = 0;
        my_log(DEBUG_TRIE, "start insert element : %s\n", init_ele.name);
        while (init_ele.name[char_index] != '\0')
        {
            insert = init_ele.name[char_index] == '%' ? MAX - 1 : init_ele.name[char_index] - 'a';
            my_log(DEBUG_TRIE, "insert element char : %c in position %d\n", init_ele.name[char_index], insert);
            if (temp_node->next[insert] == NULL)
            {
                my_log(DEBUG_TRIE, "find new letter : %c \n", init_ele.name[char_index]);
                temp_node->next[insert] = create_node(init_ele.name[char_index], false, true, init_ele.value, 0);
            }

            temp_node = temp_node->next[insert];
            char_index++;
        }

        temp_node->leaf = true;

        node_index++;
        elements++;
    }
}

uint64_t register_address(my_trie_node_t *register_tree, char *reg)
{
    my_trie_node_t *target = register_tree;
    int index = 0;
    int insert = 0;
    while (reg[index] != '\0')
    {
        insert = reg[index] == '%' ? MAX - 1 : reg[index] - 'a';

        my_log(DEBUG_TRIE, "find insert letter: %c in position %d\n", reg[index], insert);
        if (target->next[insert] == NULL)
        {
            my_log(DEBUG_TRIE, "can not find reg named: %s\n", reg);
            exit(0);
        }
        target = target->next[insert];

        index++;
    }

    if (!target->leaf)
    {
        my_log(DEBUG_TRIE, "reg name is not leaf: %s\n", reg);
        exit(0);
    }

    return target->register_address;
}

void TestRegisterTrie()
{
    init_t init_elements[72] = {
        {"%rax", (uint64_t)&cpu_reg.rax},
        {"%rbx", (uint64_t)&cpu_reg.rbx},
        {"%rcx", (uint64_t)&cpu_reg.rcx},
        {"%rdx", (uint64_t)&cpu_reg.rdx},
        {"%rsi", (uint64_t)&cpu_reg.rsi},
        {"%rdi", (uint64_t)&cpu_reg.rdi},
        {"%rbp", (uint64_t)&cpu_reg.rbp},
        {"%rsp", (uint64_t)&cpu_reg.rsp},
        {"%r8", (uint64_t)&cpu_reg.r8},
        {"%r9", (uint64_t)&cpu_reg.r9},
        {"%r10", (uint64_t)&cpu_reg.r10},
        {"%r11", (uint64_t)&cpu_reg.r11},
        {"%r12", (uint64_t)&cpu_reg.r12},
        {"%r13", (uint64_t)&cpu_reg.r13},
        {"%r14", (uint64_t)&cpu_reg.r14},
        {"%r15", (uint64_t)&cpu_reg.r15},
        {"%eax", (uint64_t) & (cpu_reg.eax)},
        {"%ebx", (uint64_t) & (cpu_reg.ebx)},
        {"%ecx", (uint64_t) & (cpu_reg.ecx)},
        {"%edx", (uint64_t) & (cpu_reg.edx)},
        {"%esi", (uint64_t) & (cpu_reg.esi)},
        {"%edi", (uint64_t) & (cpu_reg.edi)},
        {"%ebp", (uint64_t) & (cpu_reg.ebp)},
        {"%esp", (uint64_t) & (cpu_reg.esp)},
        {"%r8d", (uint64_t) & (cpu_reg.r8d)},
        {"%r9d", (uint64_t) & (cpu_reg.r9d)},
        {"%r10d", (uint64_t) & (cpu_reg.r10d)},
        {"%r11d", (uint64_t) & (cpu_reg.r11d)},
        {"%r12d", (uint64_t) & (cpu_reg.r12d)},
        {"%r13d", (uint64_t) & (cpu_reg.r13d)},
        {"%r14d", (uint64_t) & (cpu_reg.r14d)},
        {"%r15d", (uint64_t) & (cpu_reg.r15d)},
        {"%ax", (uint64_t) & (cpu_reg.ax)},
        {"%bx", (uint64_t) & (cpu_reg.bx)},
        {"%cx", (uint64_t) & (cpu_reg.cx)},
        {"%dx", (uint64_t) & (cpu_reg.dx)},
        {"%si", (uint64_t) & (cpu_reg.si)},
        {"%di", (uint64_t) & (cpu_reg.di)},
        {"%bp", (uint64_t) & (cpu_reg.bp)},
        {"%sp", (uint64_t) & (cpu_reg.sp)},
        {"%r8w", (uint64_t) & (cpu_reg.r8w)},
        {"%r9w", (uint64_t) & (cpu_reg.r9w)},
        {"%r10w", (uint64_t) & (cpu_reg.r10w)},
        {"%r11w", (uint64_t) & (cpu_reg.r11w)},
        {"%r12w", (uint64_t) & (cpu_reg.r12w)},
        {"%r13w", (uint64_t) & (cpu_reg.r13w)},
        {"%r14w", (uint64_t) & (cpu_reg.r14w)},
        {"%r15w", (uint64_t) & (cpu_reg.r15w)},
        {"%ah", (uint64_t) & (cpu_reg.ah)},
        {"%bh", (uint64_t) & (cpu_reg.bh)},
        {"%ch", (uint64_t) & (cpu_reg.ch)},
        {"%dh", (uint64_t) & (cpu_reg.dh)},
        {"%sih", (uint64_t) & (cpu_reg.sih)},
        {"%dih", (uint64_t) & (cpu_reg.dih)},
        {"%bph", (uint64_t) & (cpu_reg.bph)},
        {"%sph", (uint64_t) & (cpu_reg.sph)},
        {"%r8w", (uint64_t) & (cpu_reg.r8b)},
        {"%r9w", (uint64_t) & (cpu_reg.r9b)},
        {"%r10w", (uint64_t) & (cpu_reg.r10b)},
        {"%r11w", (uint64_t) & (cpu_reg.r11b)},
        {"%r12w", (uint64_t) & (cpu_reg.r12b)},
        {"%r13w", (uint64_t) & (cpu_reg.r13b)},
        {"%r14w", (uint64_t) & (cpu_reg.r14b)},
        {"%r15w", (uint64_t) & (cpu_reg.r15b)},
        {"%al", (uint64_t) & (cpu_reg.al)},
        {"%bl", (uint64_t) & (cpu_reg.bl)},
        {"%cl", (uint64_t) & (cpu_reg.cl)},
        {"%dl", (uint64_t) & (cpu_reg.dl)},
        {"%sil", (uint64_t) & (cpu_reg.sil)},
        {"%dil", (uint64_t) & (cpu_reg.dil)},
        {"%bpl", (uint64_t) & (cpu_reg.bpl)},
        {"%spl", (uint64_t) & (cpu_reg.spl)},
    };

    my_trie_node_t *tree = NULL;

    init_register_tree(&tree, init_elements, sizeof(init_elements) / sizeof(init_t));

    my_log(DEBUG_TRIE, "rax address = %x\n", register_address(tree, "%rax"));
    my_log(DEBUG_TRIE, "rax address = %x\n", &cpu_reg.rax);
    my_log(DEBUG_TRIE, "spl address = %x\n", register_address(tree, "%spl"));
    my_log(DEBUG_TRIE, "spl address = %x\n", &cpu_reg.spl);
}

void init_operator_tree(my_trie_node_t **operator_tree, init_t *elements, size_t node_len)
{

    if (*operator_tree == NULL)
    {
        *operator_tree = create_node(0, false, false, 0, 0);
    }

    size_t node_index = 0;

    while (node_index < node_len)
    {
        my_trie_node_t *temp_node = *operator_tree;
        init_t init_ele = *elements;

        int char_index = 0;
        int insert = 0;
        char c = 0;
        my_log(DEBUG_TRIE, "start insert element : %s at %x\n", init_ele.name, temp_node);
        while (init_ele.name[char_index] != '\0')
        {
            insert = init_ele.name[char_index] == '%' ? MAX - 1 : init_ele.name[char_index] - 'a';
            my_log(DEBUG_TRIE, "insert element char : %c in position %d\n", init_ele.name[char_index], insert);
            if (temp_node->next[insert] == NULL)
            {
                my_log(DEBUG_TRIE, "find new letter : %c \n", init_ele.name[char_index]);
                temp_node->next[insert] = create_node(init_ele.name[char_index], false, true, init_ele.value, 0);
            }

            temp_node = temp_node->next[insert];
            char_index++;
        }

        temp_node->leaf = true;

        node_index++;
        elements++;
    }
}

int operator_type(const my_trie_node_t const *operator_tree, char *op)
{
    my_trie_node_t *target = operator_tree;
    int index = 0;
    int insert = 0;
    while (op[index] != '\0')
    {
        insert = op[index] == '%' ? MAX - 1 : op[index] - 'a';

        my_log(DEBUG_TRIE, "find insert letter: %c in position %d\n", op[index], insert);
        if (target->next[insert] == NULL)
        {
            my_log(DEBUG_TRIE, "can not find operator type: %s\n", op);
            exit(0);
        }
        target = target->next[insert];

        index++;
    }

    if (!target->leaf)
    {
        my_log(DEBUG_TRIE, "operator type is not leaf: %s\n", op);
        exit(0);
    }

    return target->operator_type;
}

void TestOperatorTree()
{

    /*
        INST_MOV,   // 0
        INST_PUSH,  // 1
        INST_POP,   // 2
        INST_LEAVE, // 3
        INST_CALL,  // 4
        INST_RET,   // 5
        INST_ADD,   // 6
        INST_SUB,   // 7
        INST_CMP,   // 8
        INST_JNE,   // 9
        INST_JMP,   // 10

    */
    init_t init_elements[11] = {
        {"mov", 0},
        {"push", 1},
        {"pop", 2},
        {"leave", 3},
        {"callq", 4},
        {"retq", 5},
        {"add", 6},
        {"sub", 7},
        {"cmp", 8},
        {"jne", 9},
        {"jmp", 10},
    };

    my_trie_node_t *tree = NULL;

    printf("%d\n", sizeof(init_elements) / sizeof(init_t));
    init_operator_tree(&tree, init_elements, sizeof(init_elements) / sizeof(init_t));

    printf("00\n");

    my_log(DEBUG_TRIE, "mov = %x\n", operator_type(tree, "mov"));
    my_log(DEBUG_TRIE, "mov = %x\n", 0);
    my_log(DEBUG_TRIE, "add = %x\n", operator_type(tree, "add"));
    my_log(DEBUG_TRIE, "add = %x\n", 6);
    my_log(DEBUG_TRIE, "jmp = %x\n", operator_type(tree, "jmp"));
    my_log(DEBUG_TRIE, "jmp = %x\n", 10);
}

static my_trie_node_t *operator_tree = NULL;

void TestOperatorTree2()
{

    /*
        INST_MOV,   // 0
        INST_PUSH,  // 1
        INST_POP,   // 2
        INST_LEAVE, // 3
        INST_CALL,  // 4
        INST_RET,   // 5
        INST_ADD,   // 6
        INST_SUB,   // 7
        INST_CMP,   // 8
        INST_JNE,   // 9
        INST_JMP,   // 10

    */
    if (operator_tree == NULL)
    {
        init_t operator_init_elements[11] = {
            {"mov", 0},
            {"push", 1},
            {"pop", 2},
            {"leave", 3},
            {"callq", 4},
            {"retq", 5},
            {"add", 6},
            {"sub", 7},
            {"cmp", 8},
            {"jne", 9},
            {"jmp", 10},
        };

        init_operator_tree(&operator_tree, operator_init_elements, sizeof(operator_init_elements) / sizeof(init_t));
    }

    my_log(DEBUG_TRIE, "mov = %x\n", operator_type(operator_tree, "mov"));
    my_log(DEBUG_TRIE, "mov = %x\n", 0);
    my_log(DEBUG_TRIE, "add = %x\n", operator_type(operator_tree, "add"));
    my_log(DEBUG_TRIE, "add = %x\n", 6);
    my_log(DEBUG_TRIE, "jmp = %x\n", operator_type(operator_tree, "jmp"));
    my_log(DEBUG_TRIE, "jmp = %x\n", 10);
}
