#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <assert.h>

#include "net_request.h"
#include "net_server.h"
#include "core.h"


static Tree* ParseArguments(const char* buffer);

static TreeNode* ParseArgument(const char** buffer);
static TreeNode* ParseFloat(const char** buffer);
static TreeNode* ParseInt(const char** buffer);
static TreeNode* ParseString(const char** buffer);
static TreeNode* ParseArray(const char** buffer);

static void TreeNodePrint(TreeNode* node);

Tree* TreeCreate()
{
    Tree* tree = (Tree*)calloc(1, sizeof(Tree));
    if(!tree) return NULL;

    tree->root = NULL;

    return tree;
}

TreeNode* TreeNodeCreate()
{
    TreeNode* node = (TreeNode*)calloc(1, sizeof(TreeNode));
    if(!node) return NULL;

    node->children = NULL;

    return node;
}

void TreeDestroy(Tree* tree)
{
    if(!tree) return;

    TreeNodeDestroy(tree->root);
}

void TreeNodeDestroy(TreeNode* node)
{
    if(!node) return;

    switch (node->type)
    {
        case NODE_ARRAY:
        {
            for(int i = 0; i < node->value.lenght; i++)
            {
                TreeNodeDestroy(node->children[i]);
            }
            free(node);
            return;
        }
        case NODE_STRING:
            free(node->value.string);
        case NODE_INT:
        case NODE_FLOAT:
        default:
            free(node);
            return;
    }
}


bool ParseRequest(ThreadInfo* info, const char* buffer)
{
    assert(info);
    assert(buffer);

    ClientData* data = info->data;
    int client_socket = data->client_socket;

    Tree* tree = ParseArguments(buffer);
    
    if(!tree->root)
    {
        printf("NO ROOT\n");
    }
    else
    {
        TreeNodePrint(tree->root);
    }

    // TODO

    return true;
}


static Tree* ParseArguments(const char* buffer)
{
    assert(buffer);

    Tree* tree = TreeCreate();
    if(!tree) return tree;


    tree->root = ParseArgument(&buffer);

    return tree;
}

static TreeNode* ParseArgument(const char** buffer)
{
    assert(buffer);
    assert(*buffer);
    
    TreeNode* node;

    node = ParseFloat(buffer);
    if(!node) node = ParseInt(buffer);
    if(!node) node = ParseString(buffer);
    if(!node) node = ParseArray(buffer);

    return node;
}

static TreeNode* ParseFloat(const char** buffer)
{
    assert(buffer);
    assert(*buffer);

    if(**buffer != 'f') return NULL;
    (*buffer)++;

    float value = 0;
    int readen = 0;

    sscanf(*buffer, "%f%n", &value, &readen);
    (*buffer) += readen;

    TreeNode* node = TreeNodeCreate();
    if(!node) return NULL;

    node->type = NODE_FLOAT;
    node->value.Float = value;

    return node;
}

static TreeNode* ParseInt(const char** buffer)
{
    assert(buffer);
    assert(*buffer);

    if(**buffer != 'i') return NULL;
    (*buffer)++;

    int value = 0;
    int readen = 0;

    sscanf(*buffer, "%d%n", &value, &readen);
    (*buffer) += readen;

    TreeNode* node = TreeNodeCreate();
    if(!node) return NULL;

    node->type = NODE_INT;
    node->value.Int = value;

    return node;
}

#define TO_STR3(value) #value
#define TO_STR2(value) TO_STR3(value)
#define TO_STR(value) TO_STR2(value)


static TreeNode* ParseString(const char** buffer)
{
    assert(buffer);
    assert(*buffer);

    if(**buffer != '\"') return NULL;
    (*buffer)++;

    char local_buffer[BUFFER_SIZE + 1] = {0}; 

    if (sscanf(*buffer, "%" TO_STR(BUFFER_SIZE) "[^\"]", local_buffer) == 1) 
    {
        local_buffer[BUFFER_SIZE] = '\0';
        (*buffer) += strlen(local_buffer);
        (*buffer)++;
        
        TreeNode* node = TreeNodeCreate();
        if(!node) return NULL;

        node->type = NODE_STRING;
        node->value.string = strdup(local_buffer);

        return node;
    }
    
    return NULL;
}

static TreeNode* ParseArray(const char** buffer)
{
    assert(buffer);
    assert(*buffer);

    size_t value = 0;
    int readen = 0;

    sscanf(*buffer, "%lu%n", &value, &readen);
    (*buffer) += readen;

    if(readen <= 0) return NULL;

    if(**buffer != '[') return NULL;

    (*buffer)++;

    TreeNode* node = TreeNodeCreate();
    if(!node) return NULL;

    node->type = NODE_ARRAY;
    node->value.lenght = value;
    node->children = (TreeNode**)calloc((size_t)value, sizeof(TreeNode*));

    for(size_t i = 0; i < value; i++)
    {
        TreeNode* child = ParseArgument(buffer);
        if(!child)
        {
            printf("NO CHILD");
            TreeNodeDestroy(node);
            return NULL;
        }

        node->children[i] = child;

        if(i < value - 1)
        {
            if(**buffer != '|')
            {
                TreeNodeDestroy(node);
                return NULL;
            }
            (*buffer)++;
        }
    }

    if(**buffer != ']')
    {   
        free(node);
        return NULL;
    }

    return node;
}

static void TreeNodePrint(TreeNode* node)
{
    assert(node);

    if(node->type == NODE_FLOAT)
    {
        printf("%g\n", node->value.Float);
    }
    else if(node->type == NODE_INT)
    {
        printf("%d\n", node->value.Int);
    }
    else if(node->type == NODE_STRING)
    {
        printf("%s\n", node->value.string);
    }
    else 
    {
        printf("ARRAY\n");
        for(size_t i = 0; i < node->value.lenght; i++)
        {
            TreeNodePrint(node->children[i]);
        }
    }
}