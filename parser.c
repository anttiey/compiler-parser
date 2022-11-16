#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "header.h"

extern int yylex();
extern FILE* yyin;
extern char* yytext;
extern void yyrestart(FILE* input);

/*
    A  -> id A' | F' T' E'
    A' -> = A | T' E'
    E  -> T E'
    E' -> + T E' | - T E' | ε
    T  -> F T'
    T' -> * F T' | / F T' | ε
    F  -> id | F'
    F' -> ( A ) | inum | fnum | S | - F
    S  -> str
*/

typedef struct Node {
    struct Node *left;
    struct Node *right;

    int type;
    char operator[MAX_NUM];

    int value_int;
    double value_double;
    char value_string[MAX_NUM];
} Node;

typedef struct ContainerDepth {
    Node* nodes[MAX_CONTAINER];
    int index;
} ContainerDepth;

typedef struct ContainerList {
    ContainerDepth container[MAX_CONTAINER];
    int depth;
} ContainerList;

Node* tokens[MAX_LINE];
int tok;

Node* A(Node* n);
Node* E(Node* n);
Node* T(Node* n);
Node* F(Node* n);
Node* S(Node* n);
Node* Aprime(Node* n); /* This is for A' */
Node* Eprime(Node* n); /* This is for E' */
Node* Tprime(Node* n); /* This is for T' */
Node* Fprime(Node* n); /* This is for F' */

void init_node(Node *node) {
    node->left = 0;
    node->right = 0;

    node->type = 0;
    node->value_int = 0;
    node->value_double = 0l;
    node->value_string[0] = 0;
    node->operator[0] = 0;
}

Node* create_node() {
    Node* n = (Node*)malloc(sizeof(Node));

    init_node(n);

    return n;
}

Node* insert_value(Node* n, int type, char* value) {
    n->type = type;

    switch(type) {
        case INTEGER: 
            n->value_int = atoi(value);
            break;
        case REAL:
            n->value_double = atof(value);
            break;
        case STRING:
            strcpy(n->value_string, value);
        default:
            strcpy(n->operator, value);
    }

    return n;
}

void get_constant(Node *n, char* buffer) {
    switch (n->type)
    {
        case INTEGER:
            sprintf(buffer, "%d", n->value_int);
            break;
        case REAL:
            sprintf(buffer, "%f", n->value_double);
            break;
        case STRING:
            sprintf(buffer, "%s", n->value_string);
            break;
        default:
            break;
    }
}

void get_operator(Node* n, char* buffer) {
    if (n == NULL) { return; }

    switch (n->type)
    {
        case ASSIGN:
            strcpy(buffer, "=");
            break;
        case PLUS:
            strcpy(buffer, "+");
            break;
        case MINUS:
            strcpy(buffer, "-");
            break;
        case MULTI:
            strcpy(buffer, "*");
            break;
        case DIVISION:
            strcpy(buffer, "/");
            break;
        default:
            break;
    }
}

int get_size(Node* n) {
    int result = 0;

    if (n == NULL) { return result; }
    if (n->left != NULL) { result += 1; }
    if (n->right != NULL) { result += 1; }

    return result;
}

void get_value(Node* n, char* value) {
    if (n == NULL) { return; }

    char buffer[MAX_NUM];
    buffer[0] = 0;

    if (n->type > 10) {
        get_operator(n, buffer);
        sprintf(value, "%s%d", buffer, get_size(n));
    } else {
        get_constant(n, buffer);
        sprintf(value, "%s", buffer);
    }
}

void print_node(Node* n) {
    if (n == NULL) { return; }

    char buffer[100];
    buffer[0] = 0;

    get_value(n, buffer);
    printf("%s", buffer);
}

void clear_node(Node* n) {
    if (n == NULL) { return; }

    clear_node(n->left);
    clear_node(n->right);

    free(n);
}

void init_container(ContainerList* c) {
    c->depth = 0;

    for(int i = 0; i < MAX_CONTAINER; i++) {
        c->container[i].index = 0;

        for(int j = 0; j < MAX_CONTAINER; j++) {
            c->container[i].nodes[j] = 0;
        }
    }
}

void add_container_node(ContainerList* c, Node* n, int depth) {
    if (n == NULL) { return; }

    if (depth >= c->depth) {
        c->depth = depth + 1;
    }

    int index = c->container[depth].index;
    c->container[depth].nodes[index] = n;
    c->container[depth].index = index + 1;
}

void update_container_from_node(ContainerList* c, Node* n, int depth) {
    if (n == NULL) { return; }

    add_container_node(c, n, depth);

    update_container_from_node(c, n->left, depth + 1);
    update_container_from_node(c, n->right, depth + 1);
}

void print_container(ContainerList* c) {
    for(int i = 0; i < c->depth; i++) {
        printf("> ");

        for(int j = 0; j < c->container[i].index; j++) {
            Node* n = c->container[i].nodes[j];
            print_node(n);
            printf(" ");
        }

        printf("\n");
    }
}

void print_act(Node* n) {
    ContainerList c;
    init_container(&c);
    update_container_from_node(&c, n, 0);
    print_container(&c);
}

Node print_value(Node* root, int depth) {

    Node result;

    init_node(&result);
    result.type = INTEGER;

    if (root == NULL) {
        return result;
    }

    if (root->type > 10) {
        Node l = print_value(root->left, depth + 1);
        Node r = print_value(root->right, depth + 1);

        switch (root->type)
        {
            case ASSIGN:
                break;
            case PLUS:
                {
                    if (root->left != NULL && root->right != NULL) {
                        result.value_int = l.value_int + r.value_int;
                    } else if (root->left) {
                        result.value_int = l.value_int;
                    } else if (root->right) {
                        result.value_int = r.value_int;
                    } else {
                        //
                    }
                }
                break;
            case MINUS:
                {
                    if (root->left != NULL && root->right != NULL) {
                        result.value_int = l.value_int - r.value_int;
                    } else if (root->left) {
                        result.value_int = - l.value_int;
                    } else if (root->right) {
                        result.value_int = - r.value_int;
                    } else {
                        //
                    }
                }
                break;
            case MULTI:
                {
                    if (root->left != NULL && root->right != NULL) {
                        result.value_int = l.value_int * r.value_int;
                    } else if (root->left) {
                        result.value_int = l.value_int;
                    } else if (root->right) {
                        result.value_int = r.value_int;
                    } else {
                        //
                    }
                }
                break;
            case DIVISION:
                {
                    if (root->left != NULL && root->right != NULL) {
                        result.value_int = l.value_int / r.value_int;
                    } else if (root->left) {
                        result.value_int = l.value_int;
                    } else if (root->right) {
                        result.value_int = r.value_int;
                    } else {
                        //
                    }
                }
                break;
            default:
                break;
        }
    }
    else {
        result = *root;
    }

    if (depth > 0) {
        return result;
    } else {
        print_node(&result);
    }

}


Node* A(Node *n) {
    if (tok == VARIABLE) {
        Node* node = create_node();
        insert_value(node, tok, yytext);
        return Aprime(node);
    } else {
        Node* m = Fprime(n);
        tok = yylex();
        Node* l = Tprime(m);
        return Eprime(l);
    }
}

Node* Aprime(Node* n) {
    if (tok == ASSIGN) {
        Node* node = create_node();
        insert_value(node, tok, yytext);

        tok = yylex();

        Node* m = A(node);
        node->right = m;

        return m;
    } else {
        Node* node = Tprime(n);
        return Eprime(node);
    }
}

Node* E(Node *n) {
    Node* m = T(n);
    tok = yylex();
    return Eprime(m);
}

Node* Eprime(Node *n) {
    if (tok == PLUS) { /* E' -> +TE' */
        Node* node = create_node();
        insert_value(node, tok, yytext);

        node->left = n;

        tok = yylex();
        Node* m = T(node);

        node->right = m;

        return Eprime(node);
    } else if (tok == MINUS) { /* E' -> -TE' */
        Node* node = create_node();
        insert_value(node, tok, yytext);

        node->left = n;

        tok = yylex();
        Node* m = T(node);

        node->right = m;

        return Eprime(node);
    } else { /* E' -> ε */
        return n;
    }
}

Node* T(Node *n) {
    Node* m = F(n);
    tok = yylex();
    return Tprime(m);
}

Node* Tprime(Node* n) {
    if (tok == MULTI) { /* T' -> *FT' */
        Node* node = create_node();
        insert_value(node, tok, yytext);

        node->left = n;

        tok = yylex();
        Node* m = F(node);

        node->right = m;

        return Tprime(node);
    } else if (tok == DIVISION) { /* T' -> /FT' */
        Node* node = create_node();
        insert_value(node, tok, yytext);

        node->left = n;

        tok = yylex();
        Node* m = F(node);

        node->right = m;

        return Tprime(node);
    } else { /* T' -> ε */
        return n;
    }
}

Node* F(Node *n) {
    if (tok == VARIABLE) {
        Node* n = create_node();
        insert_value(n, tok, yytext);
    } else {
        return Fprime(n);
    }
}

Node* Fprime(Node *n) {
    if (tok == INTEGER) {
        Node* node = create_node();
        insert_value(node, tok, yytext);
        printf("print: %d ", node->value_int);
        return node;
    } else if (tok == REAL) {
        Node* node = create_node();
        insert_value(node , tok, yytext);
        printf("print: %f ", atof(yytext));
        return node;
    } else if (tok == STRING) {
        return S(n);
    } else if (tok == MINUS) {
        Node* node = create_node();
        insert_value(node, tok, yytext);

        tok = yylex();

        Node* m = F(node);
        node->left = m;

        return m;
    } else if (tok == LP) {
        Node* node = create_node();
        insert_value(node, tok, yytext);
        tok = yylex();
        A(node);
        if(tok == RP) {
            Node* node = create_node();
            insert_value(node, tok, yytext);
        }
    }
}

Node* S(Node *n) {
    Node* node = create_node();
    insert_value(node, tok, yytext);
    printf("print: %s ", yytext);
    return node;
}

int main(int argc, char *argv[]) {

    while(1) {
        printf(">");
        tok = yylex();
        Node* root = A(NULL);
        printf("\n");
        print_value(root, 0);
        printf("\n");
        print_act(root);
    }
}
