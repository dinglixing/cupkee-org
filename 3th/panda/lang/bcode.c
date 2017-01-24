/*
MIT License

Copyright (c) 2016 Lixing Ding <ding.lixing@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "bcode.h"

int bcode_parse(const uint8_t *code, int *offset, const char **name, int *param1, int *param2)
{
    int shift, index;

    if (!name || !param1 || !param2) {
        return -1;
    }

    shift = offset ? *offset : 0;
    switch(code[shift++]) {
    case BC_STOP:       *name = "STOP"; if(offset) *offset = shift; return 0;
    case BC_PASS:       *name = "PASS"; if(offset) *offset = shift; return 0;

    /* Return instruction */
    case BC_RET0:       *name = "RET0"; if(offset) *offset = shift; return 0;
    case BC_RET:        *name = "RET";  if(offset) *offset = shift; return 0;

    /* Jump instruction */
    case BC_SJMP:       *param1 = (int8_t) (code[shift++]);
                        *name = "SJMP"; if(offset) *offset = shift; return 1;

    case BC_JMP:        index = (int8_t) (code[shift++]);
                        *param1 = (index << 8) | (code[shift++]);
                        *name = "JMP"; if(offset) *offset = shift; return 1;

    case BC_SJMP_T:     *param1 = (int8_t) (code[shift++]);
                        *name  = "SJMP_T"; if(offset) *offset = shift; return 1;

    case BC_SJMP_F:     *param1 = (int8_t) (code[shift++]);
                        *name = "SJMP_F"; if(offset) *offset = shift; return 1;

    case BC_JMP_T:      index = (int8_t) (code[shift++]);
                        *param1 = (index << 8) | (code[shift++]);
                        *name  = "JMP_T"; if(offset) *offset = shift; return 1;

    case BC_JMP_F:      index = (int8_t) (code[shift++]);
                        *param1 = (index << 8) | (code[shift++]);
                        *name  = "JMP_F"; if(offset) *offset = shift; return 1;

    case BC_POP_SJMP_T: *param1 = (int8_t) (code[shift++]);
                        *name  = "POP_SJMP_T"; if(offset) *offset = shift; return 1;

    case BC_POP_SJMP_F: *param1 = (int8_t) (code[shift++]);
                        *name  = "POP_SJMP_F"; if(offset) *offset = shift; return 1;

    case BC_POP_JMP_T:  index = (int8_t) (code[shift++]);
                        *param1 = (index << 8) | (code[shift++]);
                        *name  = "POP_JMP_T"; if(offset) *offset = shift; return 1;

    case BC_POP_JMP_F:  index = (int8_t) (code[shift++]);
                        *param1 = (index << 8) | (code[shift++]);
                        *name  = "POP_JMP_F"; if(offset) *offset = shift; return 1;

    case BC_PUSH_UND:   *name = "PUSH_UND"; if(offset) *offset = shift; return 0;
    case BC_PUSH_NAN:   *name = "PUSH_NAN"; if(offset) *offset = shift; return 0;
    case BC_PUSH_TRUE:  *name = "PUSH_TRUE"; if(offset) *offset = shift; return 0;
    case BC_PUSH_FALSE: *name = "PUSH_FALSE"; if(offset) *offset = shift; return 0;
    case BC_PUSH_ZERO:  *name = "PUSH_ZERO"; if(offset) *offset = shift; return 0;

    case BC_PUSH_NUM:   index = (code[shift++]);
                        *param1 = (index << 8) | (code[shift++]);
                        *name  = "PUSH_NUM"; if(offset) *offset = shift; return 1;

    case BC_PUSH_STR:   index = (code[shift++]);
                        *param1 = (index << 8) | (code[shift++]);
                        *name  = "PUSH_STR"; if(offset) *offset = shift; return 1;

    case BC_PUSH_VAR:   *param1 = (code[shift++]);
                        *param2 = (code[shift++]);
                        *name  = "PUSH_VAR"; if(offset) *offset = shift; return 2;

    case BC_PUSH_REF:*param1 = (code[shift++]);
                        *param2 = (code[shift++]);
                        *name  = "PUSH_REF"; if(offset) *offset = shift; return 2;

    case BC_PUSH_SCRIPT:index = (code[shift++]);
                        *param1 = (index << 8) | (code[shift++]);
                        *name  = "PUSH_SCRIPT"; if(offset) *offset = shift; return 1;

    case BC_PUSH_NATIVE:index = (code[shift++]);
                        *param1 = (index << 8) | (code[shift++]);
                        *name  = "PUSH_NATIVE"; if(offset) *offset = shift; return 1;

    case BC_POP:        *name  = "POP"; if(offset) *offset = shift; return 0;

    case BC_NEG:        *name  = "NEG"; if(offset) *offset = shift; return 0;
    case BC_NOT:        *name  = "NOT"; if(offset) *offset = shift; return 0;
    case BC_LOGIC_NOT:  *name  = "LOGIC_NOT"; if(offset) *offset = shift; return 0;

    case BC_INC:        *name  = "INC"; if(offset) *offset = shift; return 0;
    case BC_INCP:       *name  = "INCP"; if(offset) *offset = shift; return 0;
    case BC_DEC:        *name  = "DEC"; if(offset) *offset = shift; return 0;
    case BC_DECP:       *name  = "DECP"; if(offset) *offset = shift; return 0;

    case BC_MUL:        *name  = "MUL"; if(offset) *offset = shift; return 0;
    case BC_DIV:        *name  = "DIV"; if(offset) *offset = shift; return 0;
    case BC_MOD:        *name  = "MOD"; if(offset) *offset = shift; return 0;
    case BC_ADD:        *name  = "ADD"; if(offset) *offset = shift; return 0;
    case BC_SUB:        *name  = "SUB"; if(offset) *offset = shift; return 0;

    case BC_AAND:       *name = "LOGIC_AND"; if(offset) *offset = shift; return 0;
    case BC_AOR:        *name = "LOGIC_OR"; if(offset) *offset = shift; return 0;
    case BC_AXOR:       *name = "LOGIC_XOR"; if(offset) *offset = shift; return 0;

    case BC_LSHIFT:     *name = "LSHIFT"; if(offset) *offset = shift; return 0;
    case BC_RSHIFT:     *name = "RSHIFT"; if(offset) *offset = shift; return 0;

    case BC_TEQ:        *name = "TEQ"; if(offset) *offset = shift; return 0;
    case BC_TNE:        *name = "TNE"; if(offset) *offset = shift; return 0;
    case BC_TGT:        *name = "TGT"; if(offset) *offset = shift; return 0;
    case BC_TGE:        *name = "TGE"; if(offset) *offset = shift; return 0;
    case BC_TLT:        *name = "TLT"; if(offset) *offset = shift; return 0;
    case BC_TLE:        *name = "TLE"; if(offset) *offset = shift; return 0;

    case BC_TIN:        *name = "TIN"; if(offset) *offset = shift; return 0;

    case BC_FUNC_CALL:  *param1 = code[shift++];
                        *name = "CALL"; if(offset) *offset = shift; return 1;

    case BC_ARRAY:      index = (code[shift++]);
                        *param1 = (index << 8) | (code[shift++]);
                        *name  = "ARRAY"; if(offset) *offset = shift; return 1;

    case BC_DICT:       index = (code[shift++]);
                        *param1 = (index << 8) | (code[shift++]);
                        *name  = "DICT"; if(offset) *offset = shift; return 1;

    case BC_PROP:       *name = "PROP"; if(offset) *offset = shift; return 0;
    case BC_PROP_METH:  *name = "PROP_METH"; if(offset) *offset = shift; return 0;

    case BC_ELEM:       *name = "ELEM"; if(offset) *offset = shift; return 0;
    case BC_ELEM_METH:  *name = "ELEM_METH"; if(offset) *offset = shift; return 0;

    case BC_ASSIGN:     *name = "ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_ADD_ASSIGN: *name = "ADD_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_SUB_ASSIGN: *name = "SUB_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_MUL_ASSIGN: *name = "MUL_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_DIV_ASSIGN: *name = "DIV_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_MOD_ASSIGN: *name = "MOD_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_AND_ASSIGN: *name = "AND_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_OR_ASSIGN:  *name = "OR_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_XOR_ASSIGN: *name = "XOR_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_LSHIFT_ASSIGN:     *name = "LS_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_RSHIFT_ASSIGN:     *name = "RS_ASSIGN"; if(offset) *offset = shift; return 0;

    case BC_PROP_ASSIGN:     *name = "PROP_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_PROP_ADD_ASSIGN: *name = "PROP_ADD_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_PROP_SUB_ASSIGN: *name = "PROP_SUB_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_PROP_MUL_ASSIGN: *name = "PROP_MUL_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_PROP_DIV_ASSIGN: *name = "PROP_DIV_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_PROP_MOD_ASSIGN: *name = "PROP_MOD_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_PROP_AND_ASSIGN: *name = "PROP_AND_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_PROP_OR_ASSIGN:  *name = "PROP_OR_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_PROP_XOR_ASSIGN: *name = "PROP_XOR_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_PROP_LSHIFT_ASSIGN:     *name = "PROP_LS_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_PROP_RSHIFT_ASSIGN:     *name = "PROP_RS_ASSIGN"; if(offset) *offset = shift; return 0;

    case BC_ELEM_ASSIGN:*name = "ELEM_ASSING"; if(offset) *offset = shift; return 0;
    case BC_ELEM_ADD_ASSIGN: *name = "ELEM_ADD_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_ELEM_SUB_ASSIGN: *name = "ELEM_SUB_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_ELEM_MUL_ASSIGN: *name = "ELEM_MUL_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_ELEM_DIV_ASSIGN: *name = "ELEM_DIV_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_ELEM_MOD_ASSIGN: *name = "ELEM_MOD_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_ELEM_AND_ASSIGN: *name = "ELEM_AND_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_ELEM_OR_ASSIGN:  *name = "ELEM_OR_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_ELEM_XOR_ASSIGN: *name = "ELEM_XOR_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_ELEM_LSHIFT_ASSIGN:     *name = "ELEM_LS_ASSIGN"; if(offset) *offset = shift; return 0;
    case BC_ELEM_RSHIFT_ASSIGN:     *name = "ELEM_RS_ASSIGN"; if(offset) *offset = shift; return 0;

    default:            *name = "UNKNOWN"; if(offset) *offset = shift; return 0;
    }
}

