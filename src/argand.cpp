// system
#ifdef __linux__ 
    // TODO
#elif _WIN32
    #include "windows.h"
#endif

// local
#include "proj_types.h"

// standard
#include "stdio.h"
#include "string.h"
#include "math.h"



#ifndef TESTING_ENABLE
    #define TESTING 0
#else
    #define TESTING 1
#endif

#ifndef DEBUG_ENABLE
    #define DEBUG 0
#else
    #define DEBUG 1
#endif

#if DEBUG
    #define     debug_raw(x, ...)       printf(             x, __VA_ARGS__)
    #define     debug(x, ...)           printf("\r        " x, __VA_ARGS__)
    #define     debug_info(x, ...)      printf("\r[Info ] " x, __VA_ARGS__)
    #define     debug_warn(x, ...)      printf("\r[Warn ] " x, __VA_ARGS__)
    #define     debug_error(x, ...)     printf("\r[Error] " x, __VA_ARGS__)
    #define     debug_fail(x, ...)      printf("\r[Fail ] " x, __VA_ARGS__)
    #define     debug_exit()            printf("\r[Exit ]\n")

    #define     release_raw(x, ...)  
    #define     release(x, ...)      
    #define     release_info(x, ...) 
    #define     release_warn(x, ...) 
    #define     release_error(x, ...)
    #define     release_fail(x, ...) 
    #define     release_exit()       
#else
    #define     debug_raw(x, ...) 
    #define     debug(x, ...) 
    #define     debug_info(x, ...)  
    #define     debug_warn(x, ...)  
    #define     debug_error(x, ...) 
    #define     debug_fail(x, ...)
    #define     debug_exit()

    #define     release_raw(x, ...)       printf(             x, __VA_ARGS__)
    #define     release(x, ...)           printf("\r        " x, __VA_ARGS__)
    #define     release_info(x, ...)      printf("\r[Info ] " x, __VA_ARGS__)
    #define     release_warn(x, ...)      printf("\r[Warn ] " x, __VA_ARGS__)
    #define     release_error(x, ...)     printf("\r[Error] " x, __VA_ARGS__)
    #define     release_fail(x, ...)      printf("\r[Fail ] " x, __VA_ARGS__)
    #define     release_exit()            printf("\r[Exit ]\n")
#endif

#define     print_pad(x, ...)       if (!TESTING) printf("\r        " x, __VA_ARGS__)
#define     print_hint(x, ...)      if (!TESTING) printf("\r[Hint ] " x, __VA_ARGS__)
#define     print_info(x, ...)      if (!TESTING) printf("\r[Info ] " x, __VA_ARGS__)
#define     print_warn(x, ...)      if (!TESTING) printf("\r[Warn ] " x, __VA_ARGS__)
#define     print_error(x, ...)     if (!TESTING) printf("\r[Error] " x, __VA_ARGS__)
#define     print_fail(x, ...)      if (!TESTING) printf("\r[Fail ] " x, __VA_ARGS__)
#define     print_exit()            if (!TESTING) printf("\r[Exit ]\n")



#define PARSE_VALUE_ERROR    -1
#define PARSE_VALUE_SUCCESS   0

#define TOKEN_ERROR          -1
#define TOKEN_SUCCESS         0

#define SYNTAX_NONE          0x00
#define SYNTAX_ERROR         0x01
#define SYNTAX_ASSIGNMENT    0x02
#define SYNTAX_VALUE         0x04
#define SYNTAX_OPERAND       0x08
#define SYNTAX_INSPECT       0x10
#define SYNTAX_CLEAR         0x20
#define SYNTAX_EVAL          0x40
#define SYNTAX_HELP          0x80

#define POSTFIX_ERROR        -1
#define POSTFIX_SUCCESS       0

#define EVAL_STACK_ERROR     -1
#define EVAL_STACK_SUCCESS    0

#define PARSE_ERROR          -1
#define PARSE_SUCCESS         0

#define EVALUATE_ERROR       -1
#define EVALUATE_SUCCESS      0


#define  INPUT_BUFFER_SIZE      256
#define  TEST_BUFFER_SIZE       64




const u32 pow10[] = {
    1,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000,
    100000000,
    1000000000
};




struct Complex {
    f32 r;
    f32 i;
};

void pow(Complex* a, Complex* c, f32 x) {
    f64 r, r2, i2, t;

    r2 = a->r;
    r2 *= r2;

    i2 = a->i;
    i2 *= i2;

    r = sqrt(r2 + i2);
    t = atan2(a->i, a->r);
    
    r  = pow(r, x);
    t *= x;

    c->r = (r * cos(t));
    c->i = (r * sin(t));
}

void mul(Complex* a, Complex* b, Complex* c) {
    f32 r = (a->r*b->r) - (a->i*b->i);
    f32 i = (a->r*b->i) + (b->r*a->i);
    c->r = r;
    c->i = i;
}

void div(Complex* a, Complex* b, Complex* c) {
    f32 d = (b->r*b->r) + (b->i*b->i);
    f32 r = c->r = ((a->r*b->r) + (a->i*b->i)) / d;
    f32 i = c->i = ((b->r*a->i) - (a->r*b->i)) / d;
    c->r = r;
    c->i = i;
}

void add(Complex* a, Complex* b, Complex* c) {
    c->r = a->r + b->r;
    c->i = a->i + b->i;
}

void sub(Complex* a, Complex* b, Complex* c) {
    c->r = a->r - b->r;
    c->i = a->i - b->i;
}


// NOTE: we assume unary operators act on the operand to the right
enum class ExTokenType {
    NONE,
    BINARY_OPERATOR,
    UNARY_OPERATOR,
    OPERAND,
    VALUE,
};

enum class ExTokenSubType {
    NONE,

    HELP,
    INSPECT,
    CLEAR,
    EVAL,
    EXPR,

    RANGE, 
    OFFSET,

    ANS,
    Z,
    Z0,
    ZP,
    C,

    POW,
    MUL,
    DIV,
    ADD,
    SUB,
    LP,
    RP,
    EQ,

    POS,
    NEG,
};

struct ExToken {
    ExTokenType    type    = ExTokenType::NONE;
    ExTokenSubType subtype = ExTokenSubType::NONE;
    Complex        value   = {0};
};

struct ExTokenItem {
    ExTokenItem* next_token = nullptr;
    ExToken      data;
    ExToken      data_alt;
};

struct ExTokenRule {
    char           string[16];
    ExTokenType    type;
    ExTokenSubType subtype;
};

void free_tokens(ExTokenItem* token_root) {
    ExTokenItem* token = token_root;
    while(token != nullptr && token->data.type != ExTokenType::NONE) {
        ExTokenItem* tmp = token;
        token = token->next_token;
        if (tmp != token_root) {
            free(tmp);
        }
    }
    token_root->data.type = ExTokenType::NONE;
}

void debug_token(ExToken x) {
#if DEBUG
    if      (x.type == ExTokenType::NONE           ) printf("NONE"); 
    else if (x.type == ExTokenType::BINARY_OPERATOR) printf("BOP "); 
    else if (x.type == ExTokenType::UNARY_OPERATOR ) printf("UOP "); 
    else if (x.type == ExTokenType::OPERAND        ) printf("VAR "); 
    else if (x.type == ExTokenType::VALUE          ) printf("VAL "); 

    printf("  ");

    if (x.subtype == ExTokenSubType::NONE ) printf("NONE "); 

    else if (x.subtype == ExTokenSubType::INSPECT) printf("INSPC"); 
    else if (x.subtype == ExTokenSubType::CLEAR  ) printf("CLEAR"); 
    else if (x.subtype == ExTokenSubType::EVAL   ) printf("EVAL "); 
    else if (x.subtype == ExTokenSubType::HELP   ) printf("HELP "); 

    else if (x.subtype == ExTokenSubType::RANGE)  printf("RANGE"); 
    else if (x.subtype == ExTokenSubType::OFFSET) printf("OFF  "); 

    else if (x.subtype == ExTokenSubType::ANS  ) printf("ANS  "); 
    else if (x.subtype == ExTokenSubType::Z    ) printf("Z    "); 
    else if (x.subtype == ExTokenSubType::Z0   ) printf("Z0   "); 
    else if (x.subtype == ExTokenSubType::ZP   ) printf("ZP   "); 
    else if (x.subtype == ExTokenSubType::C    ) printf("C    "); 

    else if (x.subtype == ExTokenSubType::POW  ) printf("POW  "); 
    else if (x.subtype == ExTokenSubType::MUL  ) printf("MUL  "); 
    else if (x.subtype == ExTokenSubType::DIV  ) printf("DIV  "); 
    else if (x.subtype == ExTokenSubType::ADD  ) printf("ADD  "); 
    else if (x.subtype == ExTokenSubType::SUB  ) printf("SUB  "); 
    else if (x.subtype == ExTokenSubType::LP   ) printf("LP   "); 
    else if (x.subtype == ExTokenSubType::RP   ) printf("RP   "); 
    else if (x.subtype == ExTokenSubType::EQ   ) printf("EQ   "); 

    else if (x.subtype == ExTokenSubType::POS  ) printf("POS  "); 
    else if (x.subtype == ExTokenSubType::NEG  ) printf("NEG  "); 

    else printf("?    ");

    printf("  ");

    printf("%+.3f %+.3fi", x.value.r, x.value.i);
#endif
}


struct Parameters {
    Complex last_answer = {0.0, 0.0};

    u32  x_dim  =  16;
    u32  y_dim  =  16;
    f32  min    = -2.0;
    f32  max    =  2.0;
    f32  x_off  =  0.0;
    f32  y_off  =  0.0;

    u32 n_evaluations = 1000;

    u32 plane_size = 0;
    u32 value_size = 0;
    u32 radii_size = 0;

    ExToken* z0_stack =  nullptr;
    ExToken* zp_stack =  nullptr;

    Complex* plane  = nullptr;
    Complex* values = nullptr;
    f32*     radii  = nullptr;
};




i32 parse_value(Complex* value, char** character) {
    char* iter       = *character;
    f32   val        = 0.0;
    u8    numbers    = 0;
    u8    place      = 1;
    u8    fractional = 0;
    u8    complex    = 0;

    while ((*iter >= '0' && *iter <= '9') || *iter == '.' || *iter == 'i') {
        while (*iter == ' ' || *iter == '\t' || *iter == '\r' || *iter == '\n') iter++;
        if (*iter == 0) break;
        if (*iter == '.') { fractional = 1; iter++; continue; }
        if (*iter == 'i') { complex    = 1; iter++; continue; }
        if (*iter >= '0' && *iter <= '9') {
            if (!fractional) {
                val *= 10;
                val += *iter - '0';
            } else if (place < (sizeof(pow10) / sizeof(u32))) {
                val += float(*iter - '0') / (pow10[place]);
                place++;
            }
            numbers++;
            iter++;
            continue;
        }

        // undefined character
        iter = *character;
        break;
    }

    if (numbers == 0 && complex) {
        value->i = 1.0;
        *character = iter - 1;
        return PARSE_VALUE_SUCCESS;
    }
    if (iter != *character) {
        if (complex) value->i = val;
        else         value->r = val;
        *character = iter - 1;
        return PARSE_VALUE_SUCCESS;
    } 

    return PARSE_VALUE_ERROR;
}

i32 tokenize(char* raw_expr, const ExTokenRule* token_rules, ExTokenItem* token_root, u16* n_tokens) {
    ExTokenItem* token = token_root;
    ExTokenItem* prev  = nullptr;
    char* character = raw_expr;

    while (*character != 0) {
        while (*character == ' ' || *character == '\t' || *character == '\r' || *character == '\n') {
            character++;
        }

        if (*character == 0) break;
        if (*character == ',' || *character == ';') { 
            character++; continue; 
        }

        u8    match_found       = 0;
        u8    lookup            = 0;
        const ExTokenRule* rule = token_rules;
        char* iter              = character;

        while (rule->type != ExTokenType::NONE) {
            if (lookup && rule->type != ExTokenType::UNARY_OPERATOR) { rule++; continue; }
            if (!lookup) iter = character;
            for (u8 i = 0; i < sizeof(rule->string); i++) {
                if (rule->string[i] == 0) {
                    // if we found an op, look for potential corresponding ops
                    if (rule->type == ExTokenType::BINARY_OPERATOR) { 
                        lookup = 1;
                        token->data.type = rule->type;
                        token->data.subtype = rule->subtype;
                    }
                    else if (rule->type == ExTokenType::UNARY_OPERATOR)  { 
                        lookup = 0;
                        token->data_alt.type = rule->type;
                        token->data_alt.subtype = rule->subtype;
                        character = iter;
                        match_found = 1;
                    }
                    else {
                        token->data.type = rule->type;
                        token->data.subtype = rule->subtype;
                        character = iter;
                        match_found = 1;
                    }
                }

                if (rule->string[i] != *iter) break;
                if (rule->string[i+1] != 0) iter++;
            }
            if (match_found) break;
            rule++;
        }

        if (lookup) {
            token->data_alt.type = ExTokenType::NONE;
            token->data_alt.subtype = ExTokenSubType::NONE;
            character = iter;
        }

        if (!match_found && !lookup) {
            Complex value = {0};
            i32 result = parse_value(&value, &character);
            if (result == PARSE_VALUE_ERROR) {
                print_error("No matching rule for '%c' in \"%s\"\n", *character, raw_expr);
                return TOKEN_ERROR;
            }

            token->data.type = ExTokenType::VALUE;
            token->data.subtype = ExTokenSubType::NONE;
            token->data.value = value;
        }

        character++;
        token->next_token = (ExTokenItem*) malloc(sizeof(ExTokenItem));
        token = token->next_token;
        token->data.type = ExTokenType::NONE;
        (*n_tokens)++;
    }

    // resolve operators
    token = token_root; 
    prev  = nullptr; 

    while (token != nullptr && token->data.type != ExTokenType::NONE) {
        if (token->data.type == ExTokenType::BINARY_OPERATOR) {
            if (token->data_alt.type != ExTokenType::NONE) {
                // no lval -> unary operator
                if (prev == nullptr) token->data = token->data_alt;
                else if (prev->data.type == ExTokenType::BINARY_OPERATOR && prev->data.subtype != ExTokenSubType::RP) {
                    token->data = token->data_alt;
                }
            }
        }

        prev = token;
        token = token->next_token;
    }

    return TOKEN_SUCCESS;
}

u8 check_syntax(ExTokenItem* token_root, i32 lp_count) {
    debug_raw("\n> Checking Syntax\n\n");

    ExTokenItem* token = token_root; 
    ExTokenItem* prev  = nullptr; 

    u8 recursed      = 0;
    u8 z0_assignment = 0;
    u8 zp_assignment = 0;

    u16 index = 0;
    u8 syntax[INPUT_BUFFER_SIZE] = { SYNTAX_NONE };

    while (token != nullptr && token->data.type != ExTokenType::NONE) {
        debug_token(token->data);
        debug_raw("\n");

        // for catching missing operators like in "3(3 + 2)" or "(3 + 2)3"
        if (prev != nullptr) {
            if (((prev->data.type  == ExTokenType::VALUE || prev->data.type  == ExTokenType::OPERAND) && token->data.subtype == ExTokenSubType::LP) || 
                ((token->data.type == ExTokenType::VALUE || token->data.type == ExTokenType::OPERAND) && prev->data.subtype  == ExTokenSubType::RP)) {
                print_error("Missing operator.\n");
                return SYNTAX_ERROR;
            }
        }

        // push vals and operands onto stack
        if (token->data.type == ExTokenType::OPERAND) {
            if (token->data.subtype == ExTokenSubType::INSPECT) return SYNTAX_INSPECT;
            if (token->data.subtype == ExTokenSubType::CLEAR  ) return SYNTAX_CLEAR;
            if (token->data.subtype == ExTokenSubType::EVAL   ) return SYNTAX_EVAL;
            if (token->data.subtype == ExTokenSubType::HELP   ) return SYNTAX_HELP;

            syntax[index] = SYNTAX_OPERAND;
            index++;
            prev = token;
            token = token->next_token;
            continue;
        }

        if (token->data.type == ExTokenType::VALUE) {
            syntax[index] = SYNTAX_VALUE;
            index++;
            prev = token;
            token = token->next_token;
            continue;
        }

        if (token->data.type == ExTokenType::BINARY_OPERATOR) {
            if (token->data.subtype == ExTokenSubType::LP) {
                lp_count++;
                if (token == token_root) {
                    recursed = 1;

                    prev = token;
                    token = token->next_token;
                    syntax[index] = check_syntax(token, lp_count); 

                    if (syntax[index] == SYNTAX_INSPECT) return SYNTAX_INSPECT;
                    if (syntax[index] == SYNTAX_CLEAR  ) return SYNTAX_CLEAR;
                    if (syntax[index] == SYNTAX_EVAL   ) return SYNTAX_EVAL;
                    if (syntax[index] == SYNTAX_HELP   ) return SYNTAX_HELP;

                    index++;
                    break;
                }
            }

            if (token->data.subtype == ExTokenSubType::RP) {
                if (prev != nullptr && prev->data.type == ExTokenType::BINARY_OPERATOR && prev->data.subtype != ExTokenSubType::LP && prev->data.subtype != ExTokenSubType::RP) {
                    print_error("Missing value to the right of operator.\n");
                    return SYNTAX_ERROR;
                }

                lp_count--;
                if (lp_count < 0) {
                    print_error("Unmatched parenthesis.\n");
                    return SYNTAX_ERROR;
                }

                prev = token;
                token = token->next_token;
                continue;
            }

            if (token->data.subtype == ExTokenSubType::EQ) {
                if (prev != nullptr && prev->data.subtype == ExTokenSubType::Z0) z0_assignment = 1;
                if (prev != nullptr && prev->data.subtype == ExTokenSubType::ZP) zp_assignment = 1;

                if (prev != nullptr && prev->data.subtype == ExTokenSubType::Z) {
                    print_error("Can't assign to iterator z.\n");
                    print_hint("Did you mean z0 or z'?\n");
                    return SYNTAX_ERROR;
                }

                if (prev != nullptr && prev->data.subtype == ExTokenSubType::C) {
                    print_error("Can't assign to constant c.\n");
                    return SYNTAX_ERROR;
                }

                syntax[index] = SYNTAX_ASSIGNMENT;
                index++;
                prev = token;
                token = token->next_token;
                continue;
            }

            if (prev != nullptr) {
                if (prev->data.subtype != ExTokenSubType::RP && token->data.subtype != ExTokenSubType::LP) {
                    if (prev->data.type == ExTokenType::UNARY_OPERATOR || prev->data.type == ExTokenType::BINARY_OPERATOR) {
                        print_error("Double operator used.\n");
                        print_hint("Are you forgetting a value like 3 or i?\n");
                        return SYNTAX_ERROR;
                    }
                }
            }
            else {
                print_error("Missing value to the left of operator.\n");
                return SYNTAX_ERROR;
            }
        }

        prev = token;
        token = token->next_token;
    }

    // catching some errors
    if (!recursed && lp_count) {
        print_error("Unmatched parenthesis.\n");
        return SYNTAX_ERROR;
    }

    if (prev != nullptr && 
        prev->data.type == ExTokenType::BINARY_OPERATOR &&
        prev->data.subtype != ExTokenSubType::RP && (token == nullptr || token->data.type == ExTokenType::NONE)) {
        if (prev->data.subtype == ExTokenSubType::EQ) {
            print_error("Assignment requires value or expression.\n");
        }
        else {
           print_error("Missing value to the right of operator.\n");
        }
        return SYNTAX_ERROR;
    }

    // evaluating the stack to produce an abstract result
    u8 expression = 1;
    u8 result     = SYNTAX_ERROR;

    debug_raw("\n");
    for (u16 i = 0; i < index; i++) {
        debug_raw("[%2u/%u]  ", i, index); 
        if      (syntax[i] == SYNTAX_ERROR     ) debug_raw("ERROR     \n");
        else if (syntax[i] == SYNTAX_NONE      ) debug_raw("NONE      \n");
        else if (syntax[i] == SYNTAX_ASSIGNMENT) debug_raw("ASSIGNMENT\n");
        else if (syntax[i] == SYNTAX_VALUE     ) debug_raw("VALUE     \n");
        else if (syntax[i] == SYNTAX_OPERAND   ) debug_raw("OPERAND   \n");
        else                                     debug_raw("UNKNOWN   %d\n", syntax[i]);

        // errors always propagate
        if (syntax[i] == SYNTAX_ERROR) {
            result = SYNTAX_ERROR;
            break;
        }

        if (syntax[i] == SYNTAX_ASSIGNMENT) {
            // if not set or assignment on expression
            if (result == SYNTAX_VALUE) {
                result = SYNTAX_ERROR;
                print_error("Cannot assign to value nor expression.\n");
                print_hint("You can assign to z0 like in:\n");
                print_pad("z0 = 3.14\n");
                break;
            }
            if (result == SYNTAX_ERROR) {
                result = SYNTAX_ERROR;
                print_error("Need something to assign to.\n");
                break;
            }
            result = SYNTAX_ASSIGNMENT;
            expression = 0;
            continue;
        }

        if (syntax[i] == SYNTAX_OPERAND) {
            result = SYNTAX_OPERAND;
            continue;
        }

        if (syntax[i] == SYNTAX_VALUE && result != SYNTAX_OPERAND) {
            result = SYNTAX_VALUE;
            continue;
        }
    }

    if (!expression && result != SYNTAX_ERROR && result != SYNTAX_ASSIGNMENT) result = SYNTAX_ASSIGNMENT;

    debug_raw("\n< Returning  -  ");
    if (result == SYNTAX_ERROR     ) debug_raw("ERROR     \n");
    if (result == SYNTAX_NONE      ) debug_raw("NONE      \n");
    if (result == SYNTAX_ASSIGNMENT) debug_raw("ASSIGNMENT\n");
    if (result == SYNTAX_VALUE     ) debug_raw("VALUE     \n");
    if (result == SYNTAX_OPERAND   ) debug_raw("OPERAND   \n");

    return result;
}

u8 has_higher_priority(ExTokenSubType a, ExTokenSubType b) {
    u8 a_val = 0;
    if      (a == ExTokenSubType::POS) a_val = 5;
    else if (a == ExTokenSubType::NEG) a_val = 5;
    else if (a == ExTokenSubType::POW) a_val = 4;
    else if (a == ExTokenSubType::MUL) a_val = 3;
    else if (a == ExTokenSubType::DIV) a_val = 3;
    else if (a == ExTokenSubType::ADD) a_val = 2;
    else if (a == ExTokenSubType::SUB) a_val = 2;
    else if (a == ExTokenSubType::LP ) a_val = 1;

    u8 b_val = 0;
    if      (b == ExTokenSubType::POS) b_val = 5;
    else if (b == ExTokenSubType::NEG) b_val = 5;
    else if (b == ExTokenSubType::POW) b_val = 4;
    else if (b == ExTokenSubType::MUL) b_val = 3;
    else if (b == ExTokenSubType::DIV) b_val = 3;
    else if (b == ExTokenSubType::ADD) b_val = 2;
    else if (b == ExTokenSubType::SUB) b_val = 2;
    else if (b == ExTokenSubType::LP ) b_val = 1;

    return a_val >= b_val;
}

i32 create_postfix_stack(Parameters* params, ExTokenItem* token_root, ExToken* output_stack, ExToken* stack) {
    ExTokenItem* token = token_root;
    ExTokenItem* prev  = nullptr;

    i32  output_index = 0;
    i32  stack_index  = 0;

    while (token != nullptr && token->data.type != ExTokenType::NONE) {
        if (token->data.type == ExTokenType::VALUE || token->data.type == ExTokenType::OPERAND) {
            if (token->data.subtype == ExTokenSubType::ANS) {
                token->data.subtype = ExTokenSubType::NONE;
                token->data.value = params->last_answer;
            }
            output_stack[output_index] = token->data;
            output_index++;
            prev  = token;
            token = token->next_token;
            continue;
        }

        if (token->data.subtype == ExTokenSubType::LP) {
            stack[stack_index] = token->data;
            stack_index++;
            prev  = token;
            token = token->next_token;
            continue;
        }

        if (token->data.subtype == ExTokenSubType::RP) {
            while (stack_index > 0) {
                stack_index--;
                if (stack[stack_index].subtype != ExTokenSubType::LP) {
                    output_stack[output_index] = stack[stack_index];
                    output_index++;
                } else {
                    stack[stack_index].type = ExTokenType::NONE;
                    break;
                }
            }

            prev  = token;
            token = token->next_token;
            continue;
        }

        if (token->data.type == ExTokenType::BINARY_OPERATOR || token->data.type == ExTokenType::UNARY_OPERATOR) {
            if (stack_index == 0) {
                stack[stack_index] = token->data;
                stack_index++;
            } 
            else {
                stack_index--;
                while (stack_index >= 0 && !has_higher_priority(token->data.subtype, stack[stack_index].subtype)) {
                    output_stack[output_index] = stack[stack_index];
                    output_index++;
                    stack_index--;
                }
                stack_index++;
                stack[stack_index] = token->data;
                stack_index++;
            }

            prev  = token;
            token = token->next_token;
            continue;
        }

        printf("Unexpected token encountered\n");
        printf(" - type   %u\n", token->data.type);
        printf(" - value  %f\n\n", token->data.value.r);
        return POSTFIX_ERROR;
    }

    // empty stack
    while (stack_index > 0) {
        stack_index--;
        if (stack[stack_index].subtype != ExTokenSubType::LP) {
            output_stack[output_index] = stack[stack_index];
            output_index++;
        }
        stack[stack_index].type = ExTokenType::NONE;
    }

    if (stack_index == 0 && stack[0].type != ExTokenType::NONE && stack[0].subtype != ExTokenSubType::LP) {
        output_stack[output_index] = stack[0];
        output_index++;
        output_stack[output_index].type = ExTokenType::NONE;
    }

#if DEBUG
    printf("\nPostfix Transformation\n");
    printf("--------------------------\n");
    for (u32 i = 0; i < output_index; i++) {
        printf("%p  ", output_stack + i);
        debug_token(output_stack[i]);    
        printf("\n");
    }
#endif

    return POSTFIX_SUCCESS;
}

i32 evaluate_stack(Parameters* params, Complex* result, ExToken* expr, Complex* stack, Complex* z_ptr, Complex* c_ptr) {
    i32 stack_index = 0;

    while (expr != nullptr && expr->type != ExTokenType::NONE) {
        if (expr->type == ExTokenType::VALUE) {
            stack[stack_index] = expr->value;
            stack_index++;
            expr++;
            continue;
        }

        if (expr->type == ExTokenType::OPERAND) {
            if (expr->subtype == ExTokenSubType::Z) {
                if (z_ptr == nullptr) return EVALUATE_ERROR; 
                else stack[stack_index] = *z_ptr;
            }
            else if (expr->subtype == ExTokenSubType::C) {
                if (c_ptr == nullptr) return EVALUATE_ERROR; 
                else stack[stack_index] = *c_ptr;
            }
            else {
                print_error("Whilst evaluating stack found illegal operand: ");
                if      (expr->subtype == ExTokenSubType::HELP   ) printf("HELP\n"); 
                else if (expr->subtype == ExTokenSubType::INSPECT) printf("INSPECT\n");
                else if (expr->subtype == ExTokenSubType::CLEAR  ) printf("CLEAR\n");
                else if (expr->subtype == ExTokenSubType::EVAL   ) printf("EVAL\n");
                else if (expr->subtype == ExTokenSubType::EXPR   ) printf("EXPR\n");
                else if (expr->subtype == ExTokenSubType::RANGE  ) printf("RANGE\n");
                else if (expr->subtype == ExTokenSubType::OFFSET ) printf("OFFSET\n");
                else if (expr->subtype == ExTokenSubType::Z0     ) printf("Z0\n");
                else if (expr->subtype == ExTokenSubType::ZP     ) printf("ZP\n");
                else                                               printf("??\n");
                return EVAL_STACK_ERROR;
            }
            stack_index++;
            expr++;
            continue;
        }

        if (expr->type == ExTokenType::UNARY_OPERATOR) {
            if (stack_index == 0) {
                print_error("Missing value for unary operation\n");
                return EVAL_STACK_ERROR;
            }

            if (expr->subtype == ExTokenSubType::NEG) {
                stack[stack_index-1].r *= -1;
                stack[stack_index-1].i *= -1;
            }

            expr++;
            continue;
        }

        if (expr->type == ExTokenType::BINARY_OPERATOR) {
            Complex right = {0.0, 0.0};
            Complex left  = {0.0, 0.0};

            if (stack_index == 0 || stack_index - 1 == 0) {
                print_error("Missing lvalue and rvalue for binary operation\n");
                return EVAL_STACK_ERROR;
            }

            stack_index -= 2;
            left  = stack[stack_index];
            right = stack[stack_index + 1];

            if (expr->subtype == ExTokenSubType::POW) pow(&left, &left, right.r);
            if (expr->subtype == ExTokenSubType::MUL) mul(&left, &right, &left);
            if (expr->subtype == ExTokenSubType::DIV) div(&left, &right, &left);
            if (expr->subtype == ExTokenSubType::ADD) add(&left, &right, &left);
            if (expr->subtype == ExTokenSubType::SUB) sub(&left, &right, &left);

            stack[stack_index] = left;
            stack_index++;
            expr++;
            continue;
        }
    }

    if (stack_index != 1) {
        print_error("Stack index did not reduce to zero, is %u\n", stack_index);
        return EVAL_STACK_ERROR;
    }

    *result = stack[0];
    if (params != nullptr) params->last_answer = *result;
    return EVAL_STACK_SUCCESS;
}

i32 parse(Parameters* params, ExTokenItem* token_root, u16 n_tokens, u8 syntax_result) {
    i32 result;

    ExToken* expr = (ExToken*) malloc(sizeof(ExToken) * (n_tokens+1));
    memset(expr, 0, sizeof(ExToken) * (n_tokens+1));

    ExToken* expr_stack = (ExToken*) malloc(sizeof(ExToken) * (n_tokens+1));
    memset(expr_stack, 0, sizeof(ExToken) * (n_tokens+1));

    Complex* eval_stack = (Complex*) malloc(sizeof(Complex) * (n_tokens+1)); 
    memset(eval_stack, 0, sizeof(Complex) * (n_tokens+1));

    // parse expression
    if (syntax_result == SYNTAX_VALUE) {
        debug_raw("\n");
        debug_raw("\nEvaluating Expression\n");
        debug_raw("--------------------------\n");

        result = create_postfix_stack(params, token_root, expr, expr_stack);
        if (result == POSTFIX_ERROR) {
            free(expr);
            free(expr_stack);
            free(eval_stack);
            return PARSE_ERROR;
        }

        Complex answer;
        result = evaluate_stack(params, &answer, expr, eval_stack, nullptr, nullptr);

        if (result == EVAL_STACK_ERROR) result = PARSE_ERROR;
        if (result == EVAL_STACK_SUCCESS) {
            result = PARSE_SUCCESS;

            // somewhat aesthetic result output
            u8 real_out = 0;
            if (answer.r > 1e-6 || answer.r < -1e-6) {
                real_out = 1;
                printf("%g ", answer.r);
            }

            u8 comp_out = 0;
            if (answer.i > 1e-6 || answer.i < -1e-6) {
                comp_out = 1;
                if (answer.i < 0) printf("-");
                else if (real_out)             printf("+");
                if (real_out) printf(" ");
                if ((answer.i <  1 + 1e-6 && answer.i >  1 - 1e-6) ||
                    (answer.i < -1 + 1e-6 && answer.i > -1 - 1e-6)) {
                    printf("i");
                }
                else {
                    printf("%gi", fabs(answer.i));
                }
            }

            if (!real_out && !comp_out) {
                printf("0");
            }

            printf("\n\n");
        }
    }

    // parse assignments
    u16 token_offset = 0;
    ExTokenItem* token = token_root;

    u8 assign_range  = 0;
    u8 assign_offset = 0;
    u8 assign_z0     = 0;
    u8 assign_zp     = 0;

    if (syntax_result == SYNTAX_EVAL) {
        ExTokenItem* tmp = token_root; 
        while (tmp != nullptr && tmp->data.type != ExTokenType::NONE) {
            if (tmp->data.type == ExTokenType::VALUE && tmp->data.subtype != ExTokenSubType::ANS) {
                params->n_evaluations = tmp->data.value.r;
                break;
            }
            tmp = tmp->next_token;
        }
        free(expr);
        free(expr_stack);
        free(eval_stack);
        return PARSE_SUCCESS;
    }

    if (syntax_result == SYNTAX_ASSIGNMENT) {
        while (token != nullptr && token->data.type != ExTokenType::NONE) {
            if      (token->data.subtype == ExTokenSubType::RANGE)  assign_range  = 1;
            else if (token->data.subtype == ExTokenSubType::OFFSET) assign_offset = 1;
            else if (token->data.subtype == ExTokenSubType::Z0)     assign_z0     = 1;
            else if (token->data.subtype == ExTokenSubType::ZP)     assign_zp     = 1;
            else break;
            token = token->next_token->next_token;
            token_offset += 2;
        }
    }

    result = create_postfix_stack(params, token, expr, expr_stack);
    if (result == POSTFIX_ERROR) {
        free(expr);
        free(expr_stack);
        free(eval_stack);
        return PARSE_ERROR;
    }

    if (assign_range || assign_offset) {
        u8 args = 0;
        ExToken* tmp = expr; 
        while (tmp != nullptr && tmp->type != ExTokenType::NONE) {
            // assume 2 arguments for all keywords
            if (tmp->type == ExTokenType::VALUE) {
                if      (assign_range  && args == 0) params->min   = tmp->value.r;
                else if (assign_range  && args == 1) params->max   = tmp->value.r;
                else if (assign_offset && args == 0) params->x_off = tmp->value.r;
                else if (assign_offset && args == 1) params->y_off = tmp->value.r;
                if (args++ >= 2) break;
            }
            tmp++;
        }
    }

    u32 n_counted_tokens = n_tokens - token_offset + 1;

    if (assign_z0) {
        params->z0_stack = (ExToken*) malloc(sizeof(ExToken) * n_counted_tokens);
        for (u32 i = 0; i < n_counted_tokens; i++) params->z0_stack[i] = expr[i];
        params->z0_stack[n_counted_tokens-1].type = ExTokenType::NONE;
        params->z0_stack[n_counted_tokens-1].subtype = ExTokenSubType::NONE;
    }

    if (assign_zp) {
        params->zp_stack = (ExToken*) malloc(sizeof(ExToken) * n_counted_tokens);
        for (u32 i = 0; i < n_counted_tokens; i++) params->zp_stack[i] = expr[i];
        params->zp_stack[n_counted_tokens-1].type = ExTokenType::NONE;
        params->zp_stack[n_counted_tokens-1].subtype = ExTokenSubType::NONE;
    }

    free(expr);
    free(expr_stack);
    free(eval_stack);
    return PARSE_SUCCESS;
}

void evaluate_plane(Parameters* params, u16 n_tokens) {
    // grab terminal dims ------------------------------------------
    debug_raw("\n");

#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO console_info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &console_info);
    params->x_dim = (console_info.srWindow.Right  - console_info.srWindow.Left + 1) >> 1;
    params->y_dim = (console_info.srWindow.Bottom - console_info.srWindow.Top  + 1) - 6;
#endif

    debug_raw("Console Dims\n");
    debug_raw("--------------------------\n");
    debug_raw("%u    %u\n\n", params->x_dim, params->y_dim);

    debug_raw("Starting Evaluation\n");
    debug_raw("--------------------------\n");

    // memory allocation --------------------------------------------
    u32 n_elements = params->x_dim * params->y_dim;

    u32 plane_size = sizeof(Complex) * n_elements;
    if (plane_size != params->plane_size) {
        params->plane_size = plane_size;
        if (params->plane != nullptr) free(params->plane);
        params->plane = (Complex*) malloc(plane_size);
        debug_raw("Allocating Plane\n");
    }

    u32 value_size = sizeof(Complex) * n_elements;
    if (value_size != params->value_size) {
        params->value_size = value_size;
        if (params->values != nullptr) free(params->values);
        params->values = (Complex*) malloc(value_size);
        debug_raw("Allocating Values\n");
    }

    u32 radii_size = sizeof(f32) * n_elements;
    if (radii_size != params->radii_size) {
        params->radii_size = radii_size;
        if (params->radii != nullptr) free(params->radii);
        params->radii = (f32*) malloc(radii_size);
        debug_raw("Allocating Radii\n");
    }

    debug_raw("\n");

    // initialization -----------------------------------------------
    f32 ratio, x_min, x_max, y_min, y_max;
    if (params->x_dim > params->y_dim) {
        ratio = f32(params->y_dim) / f32(params->x_dim);
        x_min = params->min;
        x_max = params->max;
        y_min = params->min * ratio;
        y_max = params->max * ratio;
    } else {
        ratio = f32(params->x_dim) / f32(params->y_dim);
        x_min = params->min * ratio;
        x_max = params->max * ratio;
        y_min = params->min;
        y_max = params->max;
    }

    f32 iter = (x_max - x_min) / params->x_dim;
    f32 off  = iter / 2;

    for (u32 y = 0; y < params->y_dim; y++) {
        u32 y_index = (y * params->x_dim);
        f32 y_value = iter*y + off + y_min + params->y_off;
        for (u32 x = 0; x < params->x_dim; x++) {
            f32 x_value = iter*x + off + x_min + params->x_off;
            params->plane[y_index + x].r = x_value;
            params->plane[y_index + x].i = y_value;
        }
    }

    for (u32 i = 0; i < n_elements; i++) {
        params->radii[i]  = 0.0;
    }

    // common stack -------------------------------------------------
    Complex* eval_stack = (Complex*) malloc(sizeof(Complex) * (n_tokens+1));
    memset(eval_stack, 0, sizeof(Complex) * (n_tokens+1));

    // z0 -----------------------------------------------------------
    debug_raw("\nz0: \n");

    u8 z0_failure = 0;
    for (u32 i = 0; i < n_elements; i++) {
        params->values[i] = {0.0, 0.0};
        if (!z0_failure && params->z0_stack != nullptr) {
            Complex answer;
            i32 eval_result = evaluate_stack(params, &answer, &params->z0_stack[0], eval_stack, &params->values[i], &params->plane[i]);
            if (eval_result == EVAL_STACK_ERROR) z0_failure = 1;
            else params->values[i] = answer;
        }
    }

    release_raw("\n");
    if (z0_failure || params->zp_stack == nullptr) {
        free(eval_stack);
        return;
    }

    // zp -----------------------------------------------------------
    debug_raw("\nzp: \n");

    u8 zp_failure = 0;
    for (u32 evaluations = 0; evaluations < params->n_evaluations; evaluations++) {
        release_raw("\rEvaluating: %u / %u", evaluations+1, params->n_evaluations);

        u32 divergences = 0;
        for (u32 i = 0; i < n_elements; i++) {
            Complex answer;
            i32 eval_result = evaluate_stack(params, &answer, &params->zp_stack[0], eval_stack, &params->values[i], &params->plane[i]);
            if (eval_result == EVAL_STACK_ERROR) {
                zp_failure = 1;
                break;
            }

            params->values[i] = answer;

            f32 sqr_rad = (answer.r*answer.r) + (answer.i*answer.i);
            if (sqr_rad > params->radii[i]) params->radii[i] = sqr_rad;
            if (isinf(sqr_rad)) divergences++;
        }
        
        if (divergences == n_elements) {
            printf("\rAll samples diverged!\n\n");
            free(eval_stack);
            return;
        }

        if (zp_failure) {
            free(eval_stack);
            return;
        }
    }

    free(eval_stack);

    // output ------------------------------------------------------
    release_raw("\n\n");

    f32 min_radius = -1;
    f32 max_radius =  0;
    for (u32 i = 0; i < n_elements; i++) {
        if (isinf(params->radii[i])) continue;
        if (min_radius < 0 || min_radius > params->radii[i]) min_radius = params->radii[i];
        if (max_radius < params->radii[i]) max_radius = params->radii[i];
    }

    printf("\n");
    printf("       min     max    offset\n");
    printf("x = (%+1.3f, %+1.3f) + %.3f\n", x_min, x_max, params->x_off);
    printf("y = (%+1.3f, %+1.3f) + %.3f\n", y_min, y_max, params->y_off);
    printf("\n");

    debug_raw("ASCII Representation\n");
    debug_raw("-------------------------------\n");
    char output_shade[] = "-~+oawX#@";
    u32 n_shades = sizeof(output_shade) - 1;
    for (u32 y = 0; y < params->y_dim; y++) {
        for (u32 x = 0; x < params->x_dim; x++) {
            f32 radius = params->radii[(y * params->x_dim) + x];
            if (isinf(radius)) {
                printf("  ");
                continue;
            }

            f32 intensity = 1.0 + 1e-3 - float(radius - min_radius) / float(max_radius - min_radius); 
            u8 shade_index = u8((n_shades-1) * intensity);
            printf("%c ", output_shade[shade_index]);
        }
        printf("\n");
    }
    printf("\n");
}

void help(ExTokenItem* token_root) {
    u8 help_expr = 0;
    u8 help_eval = 0;

    ExTokenItem* tmp = token_root;
    while (tmp != nullptr && tmp->data.type != ExTokenType::NONE) {
        if (tmp->data.subtype == ExTokenSubType::EVAL) help_eval = 1;
        if (tmp->data.subtype == ExTokenSubType::EXPR) help_expr = 1;
        tmp = tmp->next_token;
    }

    if (!help_expr && !help_eval) {
        printf("\n\n");
        printf("   ___                         __\n");
        printf("  / _ | _______ ____ ____  ___/ /\n");
        printf(" / __ |/ __/ _ `/ _ `/ _ \\/ _  / \n");
        printf("/_/ |_/_/  \\_, /\\_,_/_//_/\\_,_/  \n");
        printf("          /___/                  \n");
        printf("\n\n");

        printf("Try:\n");
        printf("  help expressions\n");
        printf("  help evalution\n");
        printf("\n\n");
    }

    if (help_expr) {
        printf("\n[Expressions]\n");
        printf("Argand allows for expressions using complex numbers.\n");
        printf("If you want the real      -1 you type -1.\n");
        printf("If you want the imaginary -1 you type -1i or -i.\n");
        printf("\n");
        printf("Simple operators are available:\n");
        printf("^ or **  for  power\n");
        printf("*        for  multiply\n");
        printf("/        for  divide\n");
        printf("+        for  addition\n");
        printf("-        for  subtraction or negation\n\n");

        printf("(Expression Examples)\n");
        printf(">  (5 + 2i)^2\n");
        printf("   21 + 20i\n");
        printf("\n");
        printf(">  -ans\n");
        printf("  -21 - 20i\n");
        printf("\n\n");
    }

    if (help_eval) {
        printf("\n[Evaluation]\n");
        printf("Argand can evaluate iterated maps over the complex plane and visualize it:\n");
        printf(" - points that diverge are not shown,\n");
        printf(" - points that are locally bound are shown by their maximum square magnitude \n");
        printf("\n");
        printf("Iterated maps are constructed using:\n");
        printf("z0        for  the initial iterative value\n");
        printf("z' or zp  for  the next iterative value\n");
        printf("z         for  the current iterative value\n");
        printf("c         for  the position the sample lies on in the plane\n\n");
        printf("\n");
        printf("There are alse commands for settings parameters for evaluation:\n");
        printf("eval [N]        for  evaluating the map N times\n");
        printf("help or ?       for  displaying this prompt\n");
        printf("clear           for  clearing parameters\n");
        printf("inspect [arg]\n");
        printf("-> inspect      for  inspecting general parameters\n");
        printf("-> inspect c    for  inspecting the constructed complex plane\n");
        printf("-> inspect zp   for  inspecting \n");
        printf("\n");
        printf("Some parameters that can be explicitly set are:\n");
        printf("range   for  adjusting the minimum and maximum bounds of the plane.\n");
        printf("             at evaluation this is proportional to the terminal dims.\n");
        printf("offset  for  setting the offset from the origin.\n");
        printf("\n");

        printf("(Evaluation Example)\n");
        printf(">  # this comment could help you remember what set this is!\n");
        printf(">  z0 = 0\n");
        printf(">  zp = z^2 + c\n");
        printf(">  eval 1000\n");
        printf("\n\n");
    }
}




// order matters! 
// - longer strings first
// - binary ops before unary ops
const ExTokenRule token_rules[] = {
    {"clear",   ExTokenType::OPERAND, ExTokenSubType::CLEAR},
    {"inspect", ExTokenType::OPERAND, ExTokenSubType::INSPECT},

    {"evaluate", ExTokenType::OPERAND, ExTokenSubType::EVAL},
    {"eval",     ExTokenType::OPERAND, ExTokenSubType::EVAL},

    {"expression", ExTokenType::OPERAND, ExTokenSubType::EXPR},
    {"expr",       ExTokenType::OPERAND, ExTokenSubType::EXPR},

    {"help",     ExTokenType::OPERAND, ExTokenSubType::HELP},
    {"?",        ExTokenType::OPERAND, ExTokenSubType::HELP},

    {"range",    ExTokenType::OPERAND, ExTokenSubType::RANGE},
    {"offset",   ExTokenType::OPERAND, ExTokenSubType::OFFSET},
    {"off",      ExTokenType::OPERAND, ExTokenSubType::OFFSET},

    {"ans", ExTokenType::VALUE, ExTokenSubType::ANS},
    {"ANS", ExTokenType::VALUE, ExTokenSubType::ANS},

    {"z0", ExTokenType::OPERAND, ExTokenSubType::Z0},
    {"zp", ExTokenType::OPERAND, ExTokenSubType::ZP},
    {"z'", ExTokenType::OPERAND, ExTokenSubType::ZP},
    {"z",  ExTokenType::OPERAND, ExTokenSubType::Z},
    {"c",  ExTokenType::OPERAND, ExTokenSubType::C},

    {"Z0", ExTokenType::OPERAND, ExTokenSubType::Z0},
    {"ZP", ExTokenType::OPERAND, ExTokenSubType::ZP},
    {"Z'", ExTokenType::OPERAND, ExTokenSubType::ZP},
    {"Z",  ExTokenType::OPERAND, ExTokenSubType::Z},
    {"C",  ExTokenType::OPERAND, ExTokenSubType::C},

    {"**", ExTokenType::BINARY_OPERATOR, ExTokenSubType::POW},
    {"^",  ExTokenType::BINARY_OPERATOR, ExTokenSubType::POW},
    {"*",  ExTokenType::BINARY_OPERATOR, ExTokenSubType::MUL},
    {"/",  ExTokenType::BINARY_OPERATOR, ExTokenSubType::DIV},
    {"+",  ExTokenType::BINARY_OPERATOR, ExTokenSubType::ADD},
    {"-",  ExTokenType::BINARY_OPERATOR, ExTokenSubType::SUB},
    {"(",  ExTokenType::BINARY_OPERATOR, ExTokenSubType::LP},
    {")",  ExTokenType::BINARY_OPERATOR, ExTokenSubType::RP},
    {"=",  ExTokenType::BINARY_OPERATOR, ExTokenSubType::EQ},

    {"+",  ExTokenType::UNARY_OPERATOR, ExTokenSubType::POS},
    {"-",  ExTokenType::UNARY_OPERATOR, ExTokenSubType::NEG},

    {"", ExTokenType::NONE, ExTokenSubType::NONE}
};


#if !TESTING


void main() {
    i32 result;
    char raw_expr[INPUT_BUFFER_SIZE];
    Parameters params;

    while (1) {
        debug_raw("\n------------------------------------\n\n");

        // read string and remove new line
        printf("> ");
        raw_expr[0] = 0;
        fgets(raw_expr, sizeof(raw_expr), stdin);
        if (!raw_expr[0]) break;

        char* iter = raw_expr;
        while(*iter++ != 0);
        u16 raw_length = iter - raw_expr - 2;
        raw_expr[raw_length] = 0;
        if (!raw_expr[0]) continue;

        // quick comment check
        iter = raw_expr;
        while(*iter == ' ' || *iter == '\t') iter++;
        if (*iter == '#') continue;

        // convert to tokens
        u16 n_tokens = 0;
        ExTokenItem token_root;
        result = tokenize(raw_expr, token_rules, &token_root, &n_tokens);
        if (result == TOKEN_ERROR) {
            free_tokens(&token_root);
            printf("\n");
            continue;
        }

#if DEBUG
        else {
            printf("\nTokens Created\n");
            printf("--------------------------\n");
            ExTokenItem* token = &token_root;
            while(token != nullptr && token->data.type != ExTokenType::NONE) {
                printf("%p  ", token);
                debug_token(token->data);    
                printf("\n");
                token = token->next_token;
            }
            printf("\n");
        }
#endif

        // checking syntax
        result = check_syntax(&token_root, 0);
        debug_raw("\n");
        if (result == SYNTAX_NONE) {
            free_tokens(&token_root);
            continue;
        }

        // parse
        if (result & (SYNTAX_ASSIGNMENT|SYNTAX_VALUE|SYNTAX_EVAL)) {
            i32 parse_result = parse(&params, &token_root, n_tokens, result);
            if (parse_result == PARSE_ERROR) {
                free_tokens(&token_root);
                continue;
            }
        }

        if (result == SYNTAX_ERROR)   printf("\n");
        if (result == SYNTAX_OPERAND) print_error("Cannot perform expression with iterators.\n");
        if (result == SYNTAX_EVAL)    evaluate_plane(&params, n_tokens);
        if (result == SYNTAX_HELP)    help(&token_root);
        if (result == SYNTAX_CLEAR) {
            if (params.z0_stack != nullptr) free(params.z0_stack);
            if (params.zp_stack != nullptr) free(params.zp_stack);
            if (params.plane    != nullptr) free(params.plane   );
            if (params.values   != nullptr) free(params.values  );
            if (params.radii    != nullptr) free(params.radii   );

            params.z0_stack = nullptr;
            params.zp_stack = nullptr;
            params.plane    = nullptr;
            params.values   = nullptr;
            params.radii    = nullptr;

            params.last_answer = {0.0, 0.0};

            params.x_dim  =  16;
            params.y_dim  =  16;
            params.min    = -2.0;
            params.max    =  2.0;
            params.x_off  =  0.0;
            params.y_off  =  0.0;

            params.n_evaluations = 1000;

            params.plane_size = 0;
            params.value_size = 0;
            params.radii_size = 0;
        }
        if (result == SYNTAX_INSPECT) {
            u8 inspect_zp = 0;
            u8 inspect_c  = 0;
            ExTokenItem* tmp = &token_root;
            while (tmp != nullptr && tmp->data.type != ExTokenType::NONE) {
                if (tmp->data.subtype == ExTokenSubType::ZP) inspect_zp = 1;
                if (tmp->data.subtype == ExTokenSubType::C ) inspect_c  = 1;
                tmp = tmp->next_token;
            }

            printf("  - range     %+g    %+g\n", params.min, params.max);
            printf("  - offset    %+g    %+g\n", params.x_off, params.y_off);
            if (params.values == nullptr) {
                printf("  - Values are not set\n");
            }
            if (params.plane == nullptr) {
                printf("  - Plane is not set\n");
            }

            if (inspect_zp && params.values != nullptr) {
                printf("Values\n");
                printf("--------------------------\n");
                for (u32 j = 0; j < params.y_dim; j++) {
                    for (u32 i = 0; i < params.x_dim; i++) {
                        printf("(%+.2f,%+.2f)  ", 
                            params.values[(j*params.y_dim) + i].r,
                            params.values[(j*params.y_dim) + i].i);
                    }
                    printf("\n");
                }
                printf("\n");
            }

            if (inspect_c && params.plane != nullptr) {
                printf("Plane\n");
                printf("--------------------------\n");
                for (u32 j = 0; j < params.y_dim; j++) {
                    for (u32 i = 0; i < params.x_dim; i++) {
                        printf("(%+.2f,%+.2f)  ", 
                            params.plane[(j*params.y_dim) + i].r,
                            params.plane[(j*params.y_dim) + i].i);
                    }
                    printf("\n");
                }
                printf("\n");
            }

            printf("\n");
        }

        free_tokens(&token_root);
        continue;
    }
}


#else

struct TestCase {
    char     raw[TEST_BUFFER_SIZE];      // test expression
    u8       syntax;                     // expected syntax result
    Complex  answer;                     // expression result
};

void main() {
    
    TestCase cases[] = {
        // polarity
        {"5",        SYNTAX_VALUE, { 5,0}},
        {"-5",       SYNTAX_VALUE, {-5,0}},
        {"+5",       SYNTAX_VALUE, { 5,0}},
        {"(((+5)))", SYNTAX_VALUE, { 5,0}},

        // binary operators
        {"5 + 5",  SYNTAX_VALUE, {  10,0}},
        {"5 - 5",  SYNTAX_VALUE, {   0,0}},
        {"5 * 5",  SYNTAX_VALUE, {  25,0}},
        {"5 / 5",  SYNTAX_VALUE, {   1,0}},
        {"5 ^ 5",  SYNTAX_VALUE, {3125,0}},
        {"5 ** 5", SYNTAX_VALUE, {3125,0}},
        {"(5+5)",  SYNTAX_VALUE, {  10,0}},
        {"-(5+5)", SYNTAX_VALUE, { -10,0}},
        {"+(5+5)", SYNTAX_VALUE, {  10,0}},

        // unary operator mix
        {"-5^2",  SYNTAX_VALUE, {25,0}},
        {"+5^2",  SYNTAX_VALUE, {25,0}},

        // double operator with polarity
        {"5 + +5",  SYNTAX_VALUE, { 10,0}},
        {"5 + -5",  SYNTAX_VALUE, {  0,0}},
        {"+5 + +5", SYNTAX_VALUE, { 10,0}},
        {"+5 + -5", SYNTAX_VALUE, {  0,0}},
        {"-5 + +5", SYNTAX_VALUE, {  0,0}},
        {"-5 + -5", SYNTAX_VALUE, {-10,0}},

        // polarity in right parenthesis
        {"(5+5) ^ (2)",  SYNTAX_VALUE, {100,0}},
        {"(5+5) ^ (-2)", SYNTAX_VALUE, {0.01,0}},
        {"(5+5) ^ (+2)", SYNTAX_VALUE, {100,0}},

        // polarity in left parenthesis
        {"(5)  ^ (1+1)", SYNTAX_VALUE, {25,0}},
        {"(-5) ^ (1+1)", SYNTAX_VALUE, {25,0}},
        {"(+5) ^ (1+1)", SYNTAX_VALUE, {25,0}},

        // right nesting
        {"(1 * (2 ^ ( 3 - 4)))", SYNTAX_VALUE, {0.5,0}},
        {"(1 * (2 ^ (-3 - 1)))", SYNTAX_VALUE, {0.0625,0}},
        {"(1 * (2 ^ (+3 - 4)))", SYNTAX_VALUE, {0.5,0}},

        // left nesting
        {"((( 3 - 4) ^ 2) + 1)", SYNTAX_VALUE, {2.0, 0}},
        {"(((-2 - 1) ^ 2) + 1)", SYNTAX_VALUE, {10.0, 0}},
        {"(((+3 - 4) ^ 2) + 1)", SYNTAX_VALUE, {2.0, 0}},

        // right nesting with negated left nest
        {"((( -(1 * (2 ^ (+3 - 4))) - 4) ^ 2) + 1)", SYNTAX_VALUE, {21.25, 0}},

        // operand uses
        {"-(+z)",  SYNTAX_OPERAND, NULL},
        {"z + c",  SYNTAX_OPERAND, NULL},
        {"-z + c", SYNTAX_OPERAND, NULL},
        {"5+(z)",  SYNTAX_OPERAND, NULL},
        {"5 ** z", SYNTAX_OPERAND, NULL},

        // assignment
        {"zp = 3", SYNTAX_ASSIGNMENT, NULL},
        {"z' = 3", SYNTAX_ASSIGNMENT, NULL},
        {"z0 = 3", SYNTAX_ASSIGNMENT, NULL},
        {"z0 = c", SYNTAX_ASSIGNMENT, NULL},
        {"z0 = z", SYNTAX_ASSIGNMENT, NULL},

        {"zp = z0 = 3", SYNTAX_ASSIGNMENT, NULL},     // multiple assignment

        {"range = (-3  2.0)", SYNTAX_ASSIGNMENT, NULL},
        {"range = (-3, 2.0)", SYNTAX_ASSIGNMENT, NULL},
        {"range = (-3; 2.0)", SYNTAX_ASSIGNMENT, NULL},

        {"offset = -3  2.0", SYNTAX_ASSIGNMENT, NULL},
        {"offset = -3, 2.0", SYNTAX_ASSIGNMENT, NULL},
        {"offset = -3; 2.0", SYNTAX_ASSIGNMENT, NULL},

        // degenerates
        {"3(+3) ",  SYNTAX_ERROR, NULL},          // missing operator
        {" (+3)3",  SYNTAX_ERROR, NULL},          
        {"z(+3) ",  SYNTAX_ERROR, NULL},          
        {" (+3)z",  SYNTAX_ERROR, NULL},          

        {"5 ++ +5", SYNTAX_ERROR, NULL},          // triple operator
        {"5 ++ -5", SYNTAX_ERROR, NULL},
        {"* 3",     SYNTAX_ERROR, NULL},          // missing lvalue
        {"3 *",     SYNTAX_ERROR, NULL},          // missing right value
        {"3 -",     SYNTAX_ERROR, NULL},
        {"3 +",     SYNTAX_ERROR, NULL},

        {"1 + (2 + (3 *))", SYNTAX_ERROR, NULL},  // right nest - missing rval
        {"1 + (2 + (* 3))", SYNTAX_ERROR, NULL},  // right nest - missing lval
        {"((3 *) + 2) + 1", SYNTAX_ERROR, NULL},  // left nest  - missing rval
        {"((* 3) + 2) + 1", SYNTAX_ERROR, NULL},  // left nest  - missing lval

        {"(((3",    SYNTAX_ERROR, NULL},          // unmatched right parenthesis
        {"(((3)",   SYNTAX_ERROR, NULL},
        {"(((3))",  SYNTAX_ERROR, NULL},

        {"   3)))", SYNTAX_ERROR, NULL},          // unmatched left parenthesis
        {"  (3)))", SYNTAX_ERROR, NULL},
        {" ((3)))", SYNTAX_ERROR, NULL},

        {"z' =", SYNTAX_ERROR, NULL},             // empty assignment - rval
        {"z0 =", SYNTAX_ERROR, NULL},

        {"= 5", SYNTAX_ERROR, NULL},             // empty assignment - lval
        {"= c", SYNTAX_ERROR, NULL},

        // assignment on expression
        {"-5 = +5",             SYNTAX_ERROR, NULL},         
        {"(((-5))) = +5",       SYNTAX_ERROR, NULL},
        {"-5 = (((+5)))",       SYNTAX_ERROR, NULL},
        {"(((-5))) = (((+5)))", SYNTAX_ERROR, NULL},

        {"z  = c", SYNTAX_ERROR, NULL},           // iterator assigned to
        {"c  = z", SYNTAX_ERROR, NULL},           // constant assigned to

        {"range = (-3  )", SYNTAX_ASSIGNMENT, NULL},   // missing rval
        {"range = (-3, )", SYNTAX_ASSIGNMENT, NULL},
        {"range = (-3; )", SYNTAX_ASSIGNMENT, NULL},

        {"range = (  2.0)", SYNTAX_ASSIGNMENT, NULL},  // missing lval
        {"range = (, 2.0)", SYNTAX_ASSIGNMENT, NULL},
        {"range = (; 2.0)", SYNTAX_ASSIGNMENT, NULL},

        {"offset = -3 ", SYNTAX_ASSIGNMENT, NULL},     // missing rval
        {"offset = -3,", SYNTAX_ASSIGNMENT, NULL},
        {"offset = -3;", SYNTAX_ASSIGNMENT, NULL},

        {"offset =   2.0", SYNTAX_ASSIGNMENT, NULL},   // missing lval
        {"offset = , 2.0", SYNTAX_ASSIGNMENT, NULL},
        {"offset = ; 2.0", SYNTAX_ASSIGNMENT, NULL},
    };


    #define N_CASES     sizeof(cases) / sizeof(TestCase)

    u32 longest_expression = 0;
    for (u32 index = 0; index < N_CASES; index++) {
        u32 count = 0;
        char* iter = &cases[index].raw[0];
        while (*iter++) count++;
        if (count > longest_expression) longest_expression = count;
    }

    u32 failures = 0;
    for (u32 index = 0; index < N_CASES; index++) {
        u8  failure      = 0;
        u16 raw_tokens   = 0;
        ExTokenItem raw_root;

        printf("\r(%2u)  ", index);
        char* iter = &cases[index].raw[0];
        u8 count = 0;
        while (*iter) { printf("%c", *iter++); count++; }
        for (u32 i = 0; i < longest_expression - count + 2; i++) printf(" ");

        if (tokenize(&cases[index].raw[0], token_rules, &raw_root, &raw_tokens) == TOKEN_ERROR) {
            printf("[Token]\n");
            free_tokens(&raw_root);
            continue;
        }

        // [test]: syntax 
        u8 syntax_result = check_syntax(&raw_root, 0);
        if (syntax_result != cases[index].syntax) {
            printf("[Syntax] Expected: ");
            if (cases[index].syntax == SYNTAX_NONE      ) printf("NONE      ");
            if (cases[index].syntax == SYNTAX_ERROR     ) printf("ERROR     ");
            if (cases[index].syntax == SYNTAX_ASSIGNMENT) printf("ASSIGNMENT");
            if (cases[index].syntax == SYNTAX_VALUE     ) printf("VALUE     ");
            if (cases[index].syntax == SYNTAX_OPERAND   ) printf("OPERAND   ");
            if (cases[index].syntax == SYNTAX_INSPECT   ) printf("INSPECT   ");
            if (cases[index].syntax == SYNTAX_CLEAR     ) printf("CLEAR     ");
            if (cases[index].syntax == SYNTAX_EVAL      ) printf("EVAL      ");
            if (cases[index].syntax == SYNTAX_HELP      ) printf("HELP      ");

            printf("  Got: ");
            if (syntax_result == SYNTAX_NONE      ) printf("NONE      ");
            if (syntax_result == SYNTAX_ERROR     ) printf("ERROR     ");
            if (syntax_result == SYNTAX_ASSIGNMENT) printf("ASSIGNMENT");
            if (syntax_result == SYNTAX_VALUE     ) printf("VALUE     ");
            if (syntax_result == SYNTAX_OPERAND   ) printf("OPERAND   ");
            if (syntax_result == SYNTAX_INSPECT   ) printf("INSPECT   ");
            if (syntax_result == SYNTAX_CLEAR     ) printf("CLEAR     ");
            if (syntax_result == SYNTAX_EVAL      ) printf("EVAL      ");
            if (syntax_result == SYNTAX_HELP      ) printf("HELP      ");

            printf("\n");
            failures++;
        }

        // [test]: evaluation
        else if (syntax_result == SYNTAX_VALUE) {
            ExToken* expr       = (ExToken*) malloc(sizeof(ExToken) * (raw_tokens+1));
            ExToken* expr_stack = (ExToken*) malloc(sizeof(ExToken) * (raw_tokens+1));
            Complex* eval_stack = (Complex*) malloc(sizeof(Complex) * (raw_tokens+1));

            memset(expr,       0, sizeof(ExToken) * (raw_tokens+1));
            memset(expr_stack, 0, sizeof(ExToken) * (raw_tokens+1));
            memset(eval_stack, 0, sizeof(Complex)    * (raw_tokens+1));

            create_postfix_stack(nullptr, &raw_root, expr, expr_stack);

            Complex expr_result;
            i32 evaluate_result = evaluate_stack(nullptr, &expr_result, expr, eval_stack, nullptr, nullptr);
            if (evaluate_result == EVALUATE_ERROR) {
                printf("[Eval] Error Occurred.\n");
                failures++;
            }
            else if ((expr_result.r > cases[index].answer.r + 1e-6 || expr_result.r < cases[index].answer.r - 1e-6) ||
                     (expr_result.i > cases[index].answer.i + 1e-6 || expr_result.i < cases[index].answer.i - 1e-6)) {
                printf("[Eval] Expected:  %+f", cases[index].answer.r, cases[index].answer.i);
                printf("\tGot:  %4g + %g i\n", expr_result.r, expr_result.i);
                failures++;
            }

            free(expr);
            free(expr_stack);
            free(eval_stack);
        }

        free_tokens(&raw_root);
    }

    printf("\rTesting Complete");
    if (!failures) printf(": Success");
    for (u32 i = 0; i < longest_expression + 1; i++) printf(" ");
    printf("\n");
}

#endif
