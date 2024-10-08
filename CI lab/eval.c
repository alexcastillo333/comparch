/**************************************************************************
 * C S 429 EEL interpreter
 * 
 * eval.c - This file contains the skeleton of functions to be implemented by
 * you. When completed, it will contain the code used to evaluate an expression
 * based on its AST.
 * 
 * Copyright (c) 2021. S. Chatterjee, X. Shen, T. Byrd. All rights reserved.
 * May not be used, modified, or copied without permission.
 **************************************************************************/ 

#include "ci.h"

/* Function interfaces */

/* Returns true if the given token is a binary operator and false otherwise */
extern bool is_binop(token_t);
/* Returns true if the given token is a unary operator and false otherwise */
extern bool is_unop(token_t);
/* It might be helpful to note that TOK_QUESTION is the only ternary operator. */

char *strrev(char *str);


/* infer_type() - set the type of a non-root node based on the types of children
 * Parameter: A node pointer, possibly NULL.
 * Return value: None.
 * Side effect: The type field of the node is updated.
 * (STUDENT TODO)
 */

static void infer_type(node_t *nptr) {
    // check running status - you can ignore this
    if (terminate || ignore_input) return;

    // Week 1 TODO: Implement a recursive post-order traversal of the AST. Remember to include a base case.

    // Base case, we are in the null child of a node
    if (nptr == NULL) {
        return;
    }
    for (int i = 0; i < 3; i++) {
        infer_type(nptr->children[i]);
    }



    switch (nptr->node_type) {
        // For each week, you will also need to include error checking for each type.
        // Week 1 TODO: Implement type inference for all operators on int and bool types.
        // Week 2 TODO: Extend type inference to handle operators on string types.
        // Week 3 TODO: Implement tpye evaluation for variables.
        case NT_INTERNAL:
            switch (nptr->tok) {
                // For reference, the identity (do nothing) operator is implemented for you.
                case TOK_IDENTITY:
                    nptr->type = nptr->children[0]->type; 
                    break;  
                case TOK_NOT:
                    if (nptr->children[0]->type == BOOL_TYPE) {
                        nptr->type = BOOL_TYPE;
                    } else {
                        handle_error(ERR_TYPE);
                    }
                    break;
                case TOK_UMINUS:
                    if (nptr->children[0]->type == INT_TYPE) {
                        nptr->type = INT_TYPE;
                    } else if (nptr->children[0]->type == STRING_TYPE) {
                        nptr->type = STRING_TYPE;
                    } else {
                        handle_error(ERR_TYPE);
                    }
                    break;
                case TOK_EQ:
                case TOK_GT:
                case TOK_LT:
                    if (nptr->children[0]->type == INT_TYPE && nptr->children[1]->type == INT_TYPE) {
                        nptr->type = BOOL_TYPE;
                    } else if (nptr->children[0]->type == STRING_TYPE && nptr->children[1]->type == STRING_TYPE) {
                        nptr->type = BOOL_TYPE;
                    } else {
                        handle_error(ERR_TYPE);
                    }
                    break;
                case TOK_MOD:
                case TOK_DIV:
                case TOK_BMINUS:
                    if (nptr->children[0]->type == INT_TYPE && nptr->children[1]->type == INT_TYPE) {
                        nptr->type = INT_TYPE;
                    } else {
                        handle_error(ERR_TYPE);
                    }
                    break;
                case TOK_TIMES:
                    if (nptr->children[0]->type == INT_TYPE && nptr->children[1]->type == INT_TYPE) {
                        nptr->type = INT_TYPE;
                    } else if (nptr->children[0]->type == STRING_TYPE && nptr->children[1]->type == INT_TYPE) {
                        nptr->type = STRING_TYPE;
                    } else {
                        handle_error(ERR_TYPE);
                    }
                    break;
                case TOK_PLUS:
                    if (nptr->children[0]->type == INT_TYPE && nptr->children[1]->type == INT_TYPE) {
                        nptr->type = INT_TYPE;
                    } else if (nptr->children[0]->type == STRING_TYPE && nptr->children[1]->type == STRING_TYPE) {
                        nptr->type = STRING_TYPE;
                    } else {
                        handle_error(ERR_TYPE);
                    }
                    break;
                case TOK_QUESTION: 
                case TOK_COLON:
                    if (nptr->children[0]->type == BOOL_TYPE && 
                            nptr->children[1]->type == nptr->children[2]->type) {
                        nptr->type = nptr->children[1]->type;        
                    } else {
                        handle_error(ERR_TYPE);
                    }
                    break;
                case TOK_AND:
                case TOK_OR:
                    if (nptr->children[0]->type == BOOL_TYPE &&
                            nptr->children[1]->type == BOOL_TYPE) {
                        nptr->type = BOOL_TYPE;
                    } else {
                        handle_error(ERR_TYPE);
                    }
                    break;
                default:
                    break;
            }
        case NT_LEAF:
            if (nptr->type == ID_TYPE) {
                entry_t * var = get(nptr->val.sval);
                if (nptr->tok == TOK_ID && var == NULL) {
                    handle_error(ERR_UNDEFINED);
                    return;
                }
                free(nptr->val.sval);
                nptr->type = var->type;
                if (nptr->type == INT_TYPE) {
                    nptr->val.ival = var->val.ival;
                } else if (nptr->type == STRING_TYPE) {
                    nptr->val.sval = (char*) malloc(1+strlen(var->val.sval));
                    strcpy(nptr->val.sval, var->val.sval);
                }
            }
            break;
        default:
            break;
    }
    return;
}

/* infer_root() - set the type of the root node based on the types of children
 * Parameter: A pointer to a root node, possibly NULL.
 * Return value: None.
 * Side effect: The type field of the node is updated. 
 */

static void infer_root(node_t *nptr) {
    if (nptr == NULL) return;
    // check running status
    if (terminate || ignore_input) return;
    // check for assignment
    if (nptr->type == ID_TYPE) {
        infer_type(nptr->children[1]);
    } else {
        for (int i = 0; i < 3; ++i) {
            infer_type(nptr->children[i]);
        }
        if (nptr->children[0] == NULL) {
            logging(LOG_ERROR, "failed to find child node");
            return;
        }
        nptr->type = nptr->children[0]->type;
    }
    return;
}

/* eval_node() - set the value of a non-root node based on the values of children
 * Parameter: A node pointer, possibly NULL.
 * Return value: None.
 * Side effect: The val field of the node is updated.
 * (STUDENT TODO) 
 */

static void eval_node(node_t *nptr) {
    // check running status - you can ignore this.
    if (terminate || ignore_input) return;

    // make 1 TODO: Implement a recursive post-order traversal of the AST. Remember to include a base case.

    // Base case, we are in the null child of a node
    if (nptr == NULL) {
        return;
    }
    if (nptr->tok == TOK_QUESTION) {
        eval_node(nptr->children[0]);
    } else {
        for (int i = 0; i < 2; i++) {
            eval_node(nptr->children[i]);
        }
    }
    switch (nptr->node_type) {
        case NT_INTERNAL:
            // Week 1 TODO: Implement evaluation for all operators on int and bool types.
            // Week 2 TODO: Extend evaluation to handle operators on string types.
            if (is_unop(nptr->tok)) {
                switch (nptr->tok) {
                    case TOK_UMINUS:
                        switch (nptr->type) {
                            case INT_TYPE:
                                nptr->val.ival = -1 * nptr->children[0]->val.ival;
                                break;
                            case STRING_TYPE:
                                nptr->val.sval = strrev(nptr->children[0]->val.sval);
                                break;
                            default:
                                break;
                        }  
                        break; 
                    case TOK_NOT:
                        nptr->val.bval = ! nptr->children[0]->val.bval;
                        break;
                    default:
                        break;
                }
            }
            if (is_binop(nptr->tok)) {
                switch (nptr->tok) {
                    case TOK_EQ:
                        switch (nptr->children[0]->type) {
                            case INT_TYPE:
                                if (nptr->children[0]->val.ival == nptr->children[1]->val.ival) {
                                    nptr->val.bval = true;
                                } else {
                                    nptr->val.bval = false;
                                }
                                break;
                            case STRING_TYPE:
                                if (strcmp(nptr->children[0]->val.sval, nptr->children[1]->val.sval) == 0) {
                                    nptr->val.bval = true;
                                } else {
                                    nptr->val.bval = false;
                                }
                                break;
                            default:
                                break;
                        }
                        break;
                    case TOK_GT:
                        switch (nptr->children[0]->type) {
                            case INT_TYPE:
                                if (nptr->children[0]->val.ival > nptr->children[1]->val.ival) {
                                    nptr->val.bval = true;
                                } else {
                                    nptr->val.bval = false;
                                }
                                break;
                            case STRING_TYPE:
                                if (strcmp(nptr->children[0]->val.sval, nptr->children[1]->val.sval) > 0) {
                                    nptr->val.bval = true;
                                } else {
                                    nptr->val.bval = false;
                                }
                                break;
                            default:
                                break;
                        }
                        break;
                    case TOK_LT:
                        switch (nptr->children[0]->type) {
                            case INT_TYPE:
                                if (nptr->children[0]->val.ival < nptr->children[1]->val.ival) {
                                    nptr->val.bval = true;
                                } else {
                                    nptr->val.bval = false;
                                }
                                break;
                            case STRING_TYPE:
                                if (strcmp(nptr->children[0]->val.sval, nptr->children[1]->val.sval) < 0) {
                                    nptr->val.bval = true;
                                } else {
                                    nptr->val.bval = false;
                                }
                                break;
                            default:
                                break;
                        }
                        break;
                    case TOK_OR:
                        if (nptr->children[0]->val.bval || nptr->children[1]->val.bval) {
                            nptr->val.bval = true;
                        } else {
                            nptr->val.bval = false;
                        }
                        break;
                    case TOK_AND:
                        if (nptr->children[0]->val.bval && nptr->children[1]->val.bval) {
                            nptr->val.bval = true;
                        } else {
                            nptr->val.bval = false;
                        }
                        break;
                    case TOK_MOD:
                        if (nptr->children[1]->val.ival == 0) {
                            handle_error(ERR_EVAL);
                            break;
                        } else {
                        nptr->val.ival = nptr->children[0]->val.ival % nptr->children[1]->val.ival;    
                        break;
                        }
                    case TOK_DIV:
                        if (nptr->children[1]->val.ival == 0) {
                            handle_error(ERR_EVAL);
                            break;
                        } else {
                        nptr->val.ival = nptr->children[0]->val.ival / nptr->children[1]->val.ival;    
                        break;
                        }
                    case TOK_TIMES:
                        switch (nptr->type) {
                            case INT_TYPE:
                                nptr->val.ival = nptr->children[0]->val.ival * nptr->children[1]->val.ival;
                                break;
                            case STRING_TYPE: ;
                                if (nptr->children[1]->val.ival < 0) {
                                    handle_error(ERR_EVAL);
                                    break;
                                }
                                nptr->val.sval = (char *) malloc((strlen(nptr->children[0]->val.sval) *
                                        (nptr->children[1]->val.ival)) + 1);
                                nptr->val.sval[0] = '\0';
                                for (int i = 0; i < nptr->children[1]->val.ival * strlen(nptr->children[0]->val.sval); i++) {
                                    nptr->val.sval[i] = nptr->children[0]->val.sval[i % strlen(nptr->children[0]->val.sval)];
                                }
                                nptr->val.sval[(strlen(nptr->children[0]->val.sval) *
                                        (nptr->children[1]->val.ival))] = '\0';
                                break;
                            default:
                                break;    
                        }
                        break;
                    case TOK_BMINUS:
                        nptr->val.ival = nptr->children[0]->val.ival - nptr->children[1]->val.ival;    
                        break;
                    case TOK_PLUS:
                        switch (nptr->type) {
                            case INT_TYPE:
                                nptr->val.ival = nptr->children[0]->val.ival + nptr->children[1]->val.ival;    
                                break;
                            case STRING_TYPE: ;
                                nptr->val.sval = (char *) malloc(sizeof(char) * (strlen(nptr->children[0]->val.sval) 
                                        + strlen(nptr->children[1]->val.sval) + 1));
                                for (int i = 0; i < strlen(nptr->children[0]->val.sval) + 1; i++) {
                                    nptr->val.sval[i] = nptr->children[0]->val.sval[i];
                                }
                                strcat(nptr->val.sval, nptr->children[1]->val.sval);
                                break;
                            default:
                                break;    
                        }
                        break;                
                    default:
                        break;
                }
            }
            if (nptr->tok == TOK_QUESTION) {
                if (nptr->type == INT_TYPE) {
                    if (nptr->children[0]->val.bval) {
                        eval_node(nptr->children[1]);
                        nptr->val.ival = nptr->children[1]->val.ival;
                    } else {
                        eval_node(nptr->children[2]);
                        nptr->val.ival = nptr->children[2]->val.ival;
                    }
                } else if (nptr->type == BOOL_TYPE) {
                    if (nptr->children[0]->val.bval) {
                        eval_node(nptr->children[1]);
                        nptr->val.bval = nptr->children[1]->val.bval;
                    } else {
                        eval_node(nptr->children[2]);
                        nptr->val.bval = nptr->children[2]->val.bval;
                    }
                } else if (nptr->type == STRING_TYPE) {
                    if (nptr->children[0]->val.bval) {
                        eval_node(nptr->children[1]);
                        nptr->val.sval = malloc(1+strlen(nptr->children[1]->val.sval));
                        strcpy(nptr->val.sval, nptr->children[1]->val.sval);
                    } else {
                        eval_node(nptr->children[2]);
                        malloc(1+strlen(nptr->children[2]->val.sval));
                        strcpy(nptr->val.sval, nptr->children[2]->val.sval);
                    }

                }
            }
            // For reference, the identity (do-nothing) operator has been implemented for you.
            if (nptr->tok == TOK_IDENTITY) {
                if (nptr->type == STRING_TYPE) {
                    // Week 2 TODO: You'll need to make a copy of the string.
                    nptr->val.sval = (char *) malloc(strlen(nptr->children[0]->val.sval) + 1);
                    strcpy(nptr->val.sval, nptr->children[0]->val.sval);
                } else {
                    nptr->val.ival = nptr->children[0]->val.ival;
                }
            }
            break;
        case NT_LEAF:
            break;
        default:
            break;
    }
    return;
}

/* eval_root() - set the value of the root node based on the values of children 
 * Parameter: A pointer to a root node, possibly NULL.
 * Return value: None.
 * Side effect: The val dield of the node is updated. 
 */

void eval_root(node_t *nptr) {
    if (nptr == NULL) return;
    // check running status
    if (terminate || ignore_input) return;

    // check for assignment
    if (nptr->type == ID_TYPE) {
        eval_node(nptr->children[1]);
        if (terminate || ignore_input) return;
        
        if (nptr->children[0] == NULL) {
            logging(LOG_ERROR, "failed to find child node");
            return;
        }
        put(nptr->children[0]->val.sval, nptr->children[1]);
        return;
    }

    for (int i = 0; i < 2; ++i) {
        eval_node(nptr->children[i]);
    }
    if (terminate || ignore_input) return;
    
    if (nptr->type == STRING_TYPE) {
        nptr->val.sval = (char *) malloc(strlen(nptr->children[0]->val.sval) + 1);
        if (! nptr->val.sval) {
            logging(LOG_FATAL, "failed to allocate string");
            return;
        }
        strcpy(nptr->val.sval, nptr->children[0]->val.sval);
    } else {
        nptr->val.ival = nptr->children[0]->val.ival;
    }
    return;
}

/* infer_and_eval() - wrapper for calling infer() and eval() 
 * Parameter: A pointer to a root node.
 * Return value: none.
 * Side effect: The type and val fields of the node are updated. 
 */

void infer_and_eval(node_t *nptr) {
    infer_root(nptr);
    eval_root(nptr);
    return;
}

/* strrev() - helper function to reverse a given string 
 * Parameter: The string to reverse.
 * Return value: The reversed string. The input string is not modified.
 * (STUDENT TODO)
 */

char *strrev(char *str) {
    // Week 2 TODO: Implement copying and reversing the string.
    char* rev = (char *) malloc((strlen(str) + 1));
    for (int i = 1; i < strlen(str) + 1; i++) {
        rev[i - 1] = str[strlen(str) - i];
    }
    rev[strlen(str)] = '\0';
    return rev;
}

/* cleanup() - frees the space allocated to the AST
 * Parameter: The node to free.
 */
void cleanup(node_t *nptr) {
    // Week 2 TODO: Recursively free each node in the AST
    if (nptr == NULL) {
        return;
    }
    for (int i = 0; i < 3; i++) {
        cleanup(nptr->children[i]);
    }
    if (nptr->type == ID_TYPE || nptr->type == STRING_TYPE) {
        free(nptr->val.sval);
    }
    free(nptr);
    return;
}

