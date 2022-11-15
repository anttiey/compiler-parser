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
    char* operator;

    int value_int;
    double value_double;
    char* value_string;
} Node;

typedef struct ContainerDepth {
    Node* nodes[MAX_CONTAINER];
    int index;
} ContainerDepth;

typedef struct ContainerList {
    ContainerDepth container[MAX_CONTAINER];
    int depth;
} ContainerList;

Node* create_node();

void A();
void E();
void T();
void F();
void S();
void Aprime(); /* This is for A' */
void Eprime(); /* This is for E' */
void Tprime(); /* This is for T' */
Node* Fprime(); /* This is for F' */

char line[MAX_LINE];
char *str;

int tok;

int main(int argc, char *argv[]) {

    while(1) {
        printf(">");
        tok = yylex();
        A();
        print_node;
        printf("\n");
    }
}

Node* create_node() {
    Node* n;
    n = (Node*)malloc(sizeof(Node));

    n->left = 0;
    n->right = 0;

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
            n->value_string = value;
        default:
            n->operator = value;
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
        case LP:
            strcpy(buffer, "(");
            break;
        case RP:
            strcpy(buffer, ")");
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
        printf("%d >> ", i);

        for(int j = 0; j < c->container[i].index; j++) {
            Node* n = c->container[i].nodes[j];
            print_node(n);
            printf(" ");
        }

        printf("\n");
    }
}

void print_tree(Node* n) {
    ContainerList c;
    init_container(&c);
    update_container_from_node(&c, n, 0);
    print_container(&c);
}

void A() {
    if (tok == VARIABLE) {
        Node* n = create_node();
        insert_value(n, tok, yytext);
        Aprime();
    } else {
        Fprime();
        tok = yylex();
        Tprime();
        Eprime();
    }
}

void Aprime() {
    if (tok == ASSIGN) {
        Node* n = create_node();
        insert_value(n, tok, yytext);
        A();
    } else {
        Tprime();
        Eprime();
    }
}

void E() {
    T();
    tok = yylex();
    Eprime();
}

void Eprime() {
    if (tok == PLUS) { /* E' -> +TE' */
        Node* n = create_node();
        insert_value(n, tok, yytext);
        tok = yylex();
        T();
        Eprime();
    } else if (tok == MINUS) { /* E' -> -TE' */
        Node* n = create_node();
        insert_value(n, tok, yytext);
        tok = yylex();
        T();
        Eprime();
    } else { /* E' -> ε */

    }
}

void T() {
    F();
    tok = yylex();
    Tprime();
}

void Tprime() {
    if (tok == MULTI) { /* T' -> *FT' */
        Node* n = create_node();
        insert_value(n, tok, yytext);
        tok = yylex();
        F();
        Tprime();
    } else if (tok == DIVISION) { /* T' -> /FT' */
        Node* n = create_node();
        insert_value(n, tok, yytext);
        tok = yylex();
        F();
        Tprime();
    } else { /* T' -> ε */

    }
}

void F() {
    if (tok == VARIABLE) {
        Node* n = create_node();
        insert_value(n, tok, yytext);
    } else {
        Fprime();
    }
}

Node* Fprime() {
    if (tok == INTEGER) {
        Node* n = create_node();
        insert_value(n, tok, yytext);
    } else if (tok == REAL) {
        Node* n = create_node();
        insert_value(n, tok, yytext);
    } else if (tok == STRING) {
        S();
    } else if (tok == MINUS) {
        Node* n = create_node();
        insert_value(n, tok, yytext);
        tok = yylex();
        F();
    } else if (tok == LP) {
        Node* n = create_node();
        insert_value(n, tok, yytext);
        tok = yylex();
        A();
        if(tok == RP) {
            Node* n = create_node();
            insert_value(n, tok, yytext);
        }
    }
}

void S() {
    Node* n = create_node();
    insert_value(n, tok, yytext);
}