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
    F' -> ( A ) | inum | fnum | S | - F | sub( S, E, E )
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
    Node* nodes[MAX_NUM];
    int index;
} ContainerDepth;

typedef struct ContainerList {
    ContainerDepth container[MAX_NUM];
    int depth;
} ContainerList;

typedef struct Table {
    int index;
    char names[MAX_NUM][MAX_NUM];
    Node nodes[MAX_NUM];
} Table;

Table symbol_table;

Node* tokens[MAX_NUM];
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

void error_handling(int error, char* message) {
    switch(error) {
        case 1:
            printf("RUNTIME ERROR: %s", message);
            break;
        case 2:
            printf("SYNTAX ERROR: %s", message);
            break;
        case 3:
            printf("LEXICAL ERROR: %s", message);
            break;
    }  
}

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
            break;
        case VARIABLE:
            strcpy(n->value_string, value);
            break;
        default:
            strcpy(n->operator, value);
            break;
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
        case VARIABLE:
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

    char buffer[MAX_NUM];
    buffer[0] = 0;

    get_value(n, buffer);

    if(n->type == STRING) {
        printf("\"%s\"", buffer);
    } else {
        printf("%s", buffer);
    }
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

        for(int j = 0; j < c->container[i].index; j++) {
            Node* n = c->container[i].nodes[j];
            print_node(n);
            printf(" ");
        }

        if (i == c->depth - 1) {

        } else {
            printf("\n");
        }
    }
}

void print_ast(Node* n) {
    ContainerList c;
    init_container(&c);
    update_container_from_node(&c, n, 0);
    print_container(&c);
}

void table_init(struct Table *table) {

    for(int i = 0; i < MAX_NUM; i++) {
        table->names[i][0] = 0;
        init_node(&(table->nodes[i]));
    }

     table->index = 0;

}

void table_add(struct Table *table, Node *id, Node *value) {

    char name_string[MAX_NUM];
    name_string[0] = 0;

    get_value(id, name_string);

    for(int i = 0; i < table->index; i++) {
        if (!strcmp(table->names[i], name_string)) {
            table->nodes[i] = *(value);
            return;
        }
    }

    strcpy(table->names[table->index], name_string);
    table->nodes[table->index] = *(value);

    table->index++;

}

Node* table_get(struct Table *table, Node *id) {

    char name_string[MAX_NUM];
    name_string[0] = 0;

    get_value(id, name_string);

    for(int i = 0; i < table->index; i++) {
        if (!strcmp(table->names[i], name_string)) {
            return &table->nodes[i];
        }
    }

    error_handling(1, "UNDEFINED VARIABLE");

    return NULL;

}


Node plus_calculator(Node *left, Node *right) {
    Node result;
    init_node(&result);

    char left_string[MAX_NUM], right_string[MAX_NUM];
    left_string[0] = 0;
    right_string[0] = 0;

    if(left->type > 0 && right->type > 0) {

        if (left->type == VARIABLE) {

            left = table_get(&symbol_table, left);
            if (left == NULL) { return result; }

            if (right->type == VARIABLE) {
                right = table_get(&symbol_table, right);
                if (right == NULL) { return result; }
            } 

            return plus_calculator(left, right);

        } else if (right->type == VARIABLE) {

            right = table_get(&symbol_table, right);
            if (right == NULL) { return result; }
            return plus_calculator(left, right);

        } else if (left->type == STRING || right->type == STRING) {
            result.type = STRING;

            get_value(left, left_string);
            get_value(right, right_string);

            sprintf(result.value_string, "%s%s", left_string, right_string);
        } else if (left->type == REAL) {
            result.type = REAL;

            if (right->type == INTEGER) {
                result.value_double = left->value_double + (double)(right->value_int);
            } else if (right->type == REAL) {
                result.value_double = left->value_double + right->value_double;
            } else {
                result.value_double = left->value_double;
            }
        } else if (right->type == REAL) {
            result.type = REAL;

            if (left->type == INTEGER) {
                result.value_double = (double)(left->value_int) + right->value_double;
            } else {
                result.value_double = right->value_double;
            }
        } else if (left->type == INTEGER) {
            result.type = INTEGER;

            if (right->type == INTEGER) {
                result.value_int = left->value_int + right->value_int;
            } else {
                result.value_int = left->value_int;
            }

        } else if (right->type == INTEGER) {
            result.type = INTEGER;

            result.value_int = right->value_int;
        } 

    } else if (left->type > 0) {
        result = *(left);
    } else if (right->type > 0) {
        result = *(right);
    }

    return result;
}

Node minus_calculator(Node *left, Node *right) {
    Node result;
    init_node(&result);

    char left_string[MAX_NUM], right_string[MAX_NUM];
    left_string[0] = 0;
    right_string[0] = 0;

    if(left->type > 0 && right->type > 0) {
        if (left->type == VARIABLE) {

            left = table_get(&symbol_table, left);
            if (left == NULL) { return result; }

            if (right->type == VARIABLE) {
                right = table_get(&symbol_table, right);
                if (right == NULL) { return result; }
            } 

            return minus_calculator(left, right);

        } else if (right->type == VARIABLE) {

            right = table_get(&symbol_table, right);
            if (right == NULL) { return result; }

            return minus_calculator(left, right);

        } else if (left->type == STRING || right->type == STRING) {

            error_handling(1, "STRING은 MINUS 할 수 없다.");
            return result;

        } else if (left->type == REAL) {
            result.type = REAL;

            if (right->type == INTEGER) {
                result.value_double = left->value_double - (double)(right->value_int);
            } else if (right->type == REAL) {
                result.value_double = left->value_double - right->value_double;
            } else {
                result.value_double = - left->value_double;
            }
        } else if (right->type == REAL) {
            result.type = REAL;

            if (left->type == INTEGER) {
                result.value_double = (double)(left->value_int) - right->value_double;
            } else {
                result.value_double = - right->value_double;
            }
        } else if (left->type == INTEGER) {
            result.type = INTEGER;

            if (right->type == INTEGER) {
                result.value_int = left->value_int - right->value_int;
            } else {
                result.value_int = - left->value_int;
            }

        } else if (right->type == INTEGER) {

            result.type = INTEGER;
            result.value_int = - right->value_int;

        }

    } else if (left->type > 0) {
        result = *(left);

        if (result.type == INTEGER) {
            result.value_int = - result.value_int;
        } else if (result.type == REAL) {
            result.value_double = - result.value_double;
        } else if (result.type == VARIABLE) {
            left = table_get(&symbol_table, left);
            if (left == NULL) { return result; }
            return minus_calculator(left, right);
        }

    } else if (right->type > 0) {
        result = *(right);

        if (result.type == INTEGER) {
            result.value_int = - result.value_int;
        } else if (result.type == REAL) {
            result.value_int = - result.value_int;
        } else if (result.type == VARIABLE) {
            right = table_get(&symbol_table, right);
            if (right == NULL) { return result; }
            return minus_calculator(left, right);
        }
    }
    
    return result;
}

Node multi_calculator(Node *left, Node *right) {
    Node result;
    init_node(&result);

    char left_string[MAX_NUM], right_string[MAX_NUM];
    left_string[0] = 0;
    right_string[0] = 0;

    if(left->type > 0 && right->type > 0) {
        if (left->type == VARIABLE) {

            left = table_get(&symbol_table, left);
            if (left == NULL) { return result; }

            if (right->type == VARIABLE) {
                right = table_get(&symbol_table, right);
                if (right == NULL) { return result; }
            } 

            return multi_calculator(left, right);

        } else if (right->type == VARIABLE) {

            right = table_get(&symbol_table, right);
            if (right == NULL) { return result; }
            return multi_calculator(left, right);

        } else if (left->type == STRING) {

            if (right->type == INTEGER) {
                if (right->value_int < 0) {
                    error_handling(1, "STRING과 음의 INTEGER는 * 불가능함");
                    return result;
                } else if (right->value_int == 0) {
                    strcpy(result.value_string, "");
                } else {
                    result.type = STRING;
                    for(int i = 0; i < right->value_int; i++) {
                        strcat(result.value_string, left->value_string);
                    }
                }
            } else if (right->type == REAL) {
                error_handling(1, "STRING과 REAL은 * 불가능함");
                return result;
            } else if (right->type == STRING) {
                error_handling(1, "STRING과 STRING은 * 불가능함");
                return result;
            }

        } else if (right->type == STRING) {

            if (left->type == INTEGER) {
                error_handling(1, "INTEGER에 STRING을 곱하는 것은 불가능함");
                return result;
            } else if (left->type == REAL) {
                error_handling(1, "REAL에 STRING을 곱하는 것은 불가능함");
                return result;
            } else {
                result.type = STRING;
                strcpy(result.value_string, right->value_string);
            }

        } else if (left->type == REAL) {
            result.type = REAL;

            if (right->type == INTEGER) {
                result.value_double = left->value_double * (double)(right->value_int);
            } else if (right->type == REAL) {
                result.value_double = left->value_double * right->value_double;
            } else {
                result.value_double = left->value_double;
            }
        } else if (right->type == REAL) {
            result.type = REAL;

            if (left->type == INTEGER) {
                result.value_double = (double)(left->value_int) * right->value_double;
            } else if (left->type == REAL) {
                result.value_double = left->value_double * right->value_double;
            } else {
                result.value_double = right->value_double;
            }
        } else if (left->type == INTEGER) {
            result.type = INTEGER;

            if (right->type == INTEGER) {
                result.value_int = left->value_int * right->value_int;
            } else {
                result.value_int = left->value_int;
            }

        } else if (right->type == INTEGER) {
            result.type = INTEGER;
            result.value_int = right->value_int;
        } 
    } else if (left->type > 0) {
        result = *(left);
    } else if (right->type > 0) {
        result = *(right);
    }

    return result;
}

Node division_calculator(Node *left, Node *right) {
    Node result;
    init_node(&result);

    char left_string[MAX_NUM], right_string[MAX_NUM];
    left_string[0] = 0;
    right_string[0] = 0;

    if(left->type > 0 && right->type > 0) {
        if (left->type == VARIABLE) {

            left = table_get(&symbol_table, left);

            if (left == NULL) { return result; }

            if (right->type == VARIABLE) {
                right = table_get(&symbol_table, right);
                if (right == NULL) { return result; }
            } 

            return division_calculator(left, right);

        } else if (right->type == VARIABLE) {

            right = table_get(&symbol_table, right);
            if (right == NULL) { return result; }
            return division_calculator(left, right);

        } else if (left->type == STRING) {

            if (right->type == STRING) {
                if (!strcmp(right->value_string, "")) {
                    error_handling(1, "EMPTY STRING으로 나누는 것은 불가능함");
                } else {
                    result.type = INTEGER;

                    get_value(left, left_string);
                    get_value(right, right_string);

                    char* temp = (char*)malloc(sizeof(char) * (strlen(right_string) + 1));
                    char* ptr;

                    while(1) {
                            
                        strncpy(temp, left_string, strlen(right_string));
                        strcpy(left_string, left_string + strlen(right_string));
                        
                        temp[strlen(right_string) + 1] = '\0';

                        ptr = strstr(temp, right_string);

                        if (ptr == NULL) {
                            break;
                        }

                        result.value_int += 1;
                            
                    }
                }
                
            } else {
                error_handling(1, "STRING은 STRING으로만 나눌 수 있음");
            }

        } else if (left->type == REAL) {

            if (right->type == INTEGER) {
                if (right->value_int <= 0) {
                    error_handling(1, "0보다 작은 수로 DIVISION 할 수 없다.");
                } else {
                    result.type = REAL;
                    result.value_double = left->value_double / (double)(right->value_int);
                }
                
            } else if (right->type == REAL) {
                if (right->value_double <= 0) {
                    error_handling(1, "0보다 작은 수로 DIVISION 할 수 없다.");
                }else {
                    result.type = REAL;
                    result.value_double = left->value_double / right->value_double;
                }
            } else {
                result.value_double = left->value_double;
            }
        } else if (right->type == REAL) {

            if (right->value_double <= 0) {
                error_handling(1, "0보다 작은 수로 DIVISION 할 수 없다.");
            }else if (left->type == INTEGER) {
                result.type = REAL;
                result.value_double = (double)(left->value_int) / right->value_double;
            } else {
                result.type = REAL;
                result.value_double = right->value_double;
            }
        } else if (left->type == INTEGER) {

            if (right->type == INTEGER) {
                if (right->value_int <= 0) {
                    error_handling(1, "0보다 작은 수로 DIVISION 할 수 없다.");
                } else {
                    result.type = INTEGER;
                    result.value_int = left->value_int / right->value_int;
                }
            } else {
                result.value_int = left->value_int;
            }

        } else if (right->type == INTEGER) {
            result.type = INTEGER;

            result.value_int = right->value_int;
        } 
    } else if (left->type > 0) {
        result = *(left);
    } else if (right->type > 0) {
        result = *(right);
    }

    return result;
}

Node print_value(Node* root, int depth) {

    Node result;

    init_node(&result);

    if (root == NULL) {
        return result;
    }

    if (root->type > 10) {
        Node l = print_value(root->left, depth + 1);
        Node r = print_value(root->right, depth + 1);

        switch (root->type)
        {
            case ASSIGN:
            {

                if (l.type > 0 && r.type > 0) {

                    if (l.type == VARIABLE) {

                        if (r.type == INTEGER) {
                            result.type = INTEGER;
                            result.value_int = r.value_int;
                        } else if (r.type == REAL) {
                            result.type = REAL;
                            result.value_double = r.value_double;
                        } else if (r.type == STRING) {
                            result.type = STRING;
                            strcpy(result.value_string, r.value_string);
                        }

                        table_add(&symbol_table, &l, &r);

                    }
                }
                
            }
                break;
            case PLUS:
                result = plus_calculator(&l, &r);
                break;
            case MINUS:
                result = minus_calculator(&l, &r);
                break;
            case MULTI:
                result = multi_calculator(&l, &r);
                break;
            case DIVISION:
                result = division_calculator(&l, &r);
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

        tok = yylex();

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

        node->left = n;

        tok = yylex();

        Node* m = A(node);
        node->right = m;

        return node;
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
        tok = yylex();

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
        Node* node = create_node();
        insert_value(node, tok, yytext);

        return node;
    } else {
        return Fprime(n);
    }
}

Node* Fprime(Node *n) {
    if (tok == INTEGER) {
        Node* node = create_node();
        insert_value(node, tok, yytext);
        return node;
    } else if (tok == REAL) {
        Node* node = create_node();
        insert_value(node , tok, yytext);
        return node;
    } else if (tok == STRING) {
        return S(n);
    } else if (tok == MINUS) {
        Node* node = create_node();
        insert_value(node, tok, yytext);

        tok = yylex();

        Node* m = F(node);
        node->left = m;

        return node;
    } else if (tok == LP) {
        Node* node = create_node();
        insert_value(node, tok, yytext);
        tok = yylex();
        Node* m = A(node);
        if(tok == RP) {
            return m;
        }
    }
}

Node* S(Node *n) {
    char* string = (char*)malloc(sizeof(char) * (strlen(yytext) - 1));
    strncpy(string, yytext + 1, (strlen(yytext) - 2));
    string[strlen(yytext) - 1] = 0;

    Node* node = create_node();
    insert_value(node, tok, string);
    return node;
}

int main(int argc, char *argv[]) {

    table_init(&symbol_table);

    Node* root = NULL;

    while(1) {

        yyin = stdin;

        printf(">");

        tok = yylex();

        if (tok == AST) {
            if (root != NULL) {
              print_ast(root);  
            }

        } else if (tok == SYMBOL) {
            printf("\n===========================\n");
            printf("\tSYMBOL TABLE\n");
            printf("===========================\n");
            printf("NAME\t TYPE\t VALUE\n");

            for(int i = 0; i < symbol_table.index; i++) {

                char buffer[MAX_NUM];
                buffer[0] = 0;

                get_value(&(symbol_table.nodes[i]), buffer);


                switch(symbol_table.nodes[i].type) {
                    case INTEGER:
                        printf("%s\t INT\t %s\n", symbol_table.names[i], buffer);
                        break;
                    case REAL:
                        printf("%s\t REAL\t %s\n", symbol_table.names[i], buffer);
                        break;
                    case STRING:
                        printf("%s\t STRING\t %s\n", symbol_table.names[i], buffer);
                        break;
                    default:
                        break;
                }

            }
            
        } else {
            root = A(NULL);
            print_value(root, 0);

        }

        printf("\n");

        yyrestart(yyin);

    }
}
