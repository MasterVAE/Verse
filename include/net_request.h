#ifndef NET_REQUEST_H
#define NET_REQUEST_H

#include "net_server.h"

enum NodeType
{
    NODE_ARRAY,
    NODE_INT,
    NODE_FLOAT,
    NODE_STRING
};

union NodeValue
{
    float Float;
    int Int;
    char* string; 
    size_t lenght;
};

struct TreeNode
{
    NodeType type;
    NodeValue value;
    TreeNode** children;
};

struct Tree
{
    TreeNode* root;
};

void ParseRequest(ThreadInfo* info, const char* buffer);
Tree* ParseArgument(const char* buffer);

Tree* TreeCreate();
void TreeDestroy(Tree* tree);

TreeNode* TreeNodeCreate();
void TreeNodeDestroy(TreeNode* node);

#endif // NET_REQUEST_H