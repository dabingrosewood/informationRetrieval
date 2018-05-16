#include <stdio.h>  
#include <stdlib.h>  
  
typedef int type;  
typedef struct node  
{  
    type value;  
    struct node *left;  
    struct node *right;  
}Search_tree;  
  
Search_tree *search_tree_create(type value)//创建  
{  
    Search_tree *p = (Search_tree*)malloc(sizeof(Search_tree));  
    if(NULL == p)  
        return NULL;  
    p->value = value;  
    p->left = p->right = NULL;  
    return p;  
}  
  
void search_tree_destroy(Search_tree *t)//销毁  
{  
    if(t)  
    {  
        if(t->left)  
            search_tree_destroy(t->left);  
        if(t->right)  
            search_tree_destroy(t->right);  
        free(t);  
        t = NULL;  
    }  
}  
  
 Search_tree *search_tree_find(Search_tree *t, type value)//根据指定值查找  
{  
    if(t == NULL)  
        return NULL;  
    if(t->value == value)  
        return t;  
    else if(t->value > value)  
        return search_tree_find(t->left, value);  
    else  
        return search_tree_find(t->right, value);  
}  

  
Search_tree *search_tree_insert(Search_tree *t, type value)//插入  
{  
    // printf("node of %d is building\n",value);
    if(t == NULL)// no root  
        t = search_tree_create(value);  
    else if(t->value > value)  
        t->left = search_tree_insert(t->left, value);  
    else if(t->value < value)  
        t->right = search_tree_insert(t->right, value);  
    return t;  
}  
  
  
void search_tree_order(Search_tree *t)//中序输出  
{  
    if(t == NULL)  
        return ;  
    search_tree_order(t->left);  
    printf("%d ", t->value);  
    search_tree_order(t->right);  
}  