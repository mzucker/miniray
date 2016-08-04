import ply.lex as lex
import sys
import re

######################################################################

def get_tokens(istr):

    tokens = ['CPPLINE', 'INT', 'DOUBLE', 'STRING', 'COMMENT',
              'IDENTIFIER', 'OP', 'WHITESPACE']

    t_CPPLINE = r'\#.*?\n'
    t_COMMENT = r'(?s)(/\*.*?\*/|//[^\n]*\n)'
    t_INT = r'[0-9]+[uUlL]*'
    t_DOUBLE = r'([0-9]*\.[0-9]+|[0-9]+)([eE]-?[0-9]+)?[fFlL]?'
    t_STRING = r'"([^"\\]|\\.)*"'
    t_IDENTIFIER = r'[_A-Za-z][_A-Za-z0-9]*'
    t_OP = r'(\+\+|--|!=|>=|<=|>>|<<|&&|\|\||\+=|-=|\*=|/=|%=|&=|\|=|\^=|<<=|>>=|->|==)'
    t_WHITESPACE = r'[ \n\t]+'

    literals = '()=+-*/%!~^&|{}[];,?:<>.'

    def t_error(t):
        print >> sys.stderr, 'error tokenizing, character is', t.value[0]
        sys.exit(1)

    l = lex.lex()
    l.input(istr)

    tokens = []

    buf = ''
    iscpp = True
    defines = {}
    
    while True:
        tok = l.token()
        if not tok:
            break
        if tok.type == 'CPPLINE':
            if not iscpp:
                sys.stderr.write('error: C preprocessor stuff not at top!\n');
                sys.exit(1)
            else:
                m = re.match(r'\#define\s+(\S+)\s(\S+)\n', tok.value)
                if m:
                    defines[m.group(2)] = m.group(1)
                buf += tok.value
        elif tok.type != 'WHITESPACE' and tok.type != 'COMMENT':
            iscpp = False
            if defines.has_key(tok.value):
                tokens.append(defines[tok.value])
            else:
                tokens.append(tok.value)

    return buf, tokens

######################################################################

def can_join(buf, tok):
    ops = ['++','--','!=','>=','<=','>>','<<','&&','||','+=',
           '-=','*=','/=','%=','&=','|=','\=','^=','->','=='];
    if len(buf) == 0 or buf[-1].isspace():
        return True
    elif buf[-1].isalnum() and tok[0].isalnum():
        return False
    else:
        return not ops.count(buf[-1] + tok[0])

######################################################################

def wrap_simple(filename, buf, tokens, width):

    line = ''
    last_space = -1

    for token in tokens:

        need_whitespace = not can_join(line, token)
        line_remaining = width - len(line)
        token_needs = len(token)

        if need_whitespace:
            token_needs += 1

        if token_needs > line_remaining:
            if last_space >= 0:
                buf += line[:last_space] + '\n'
                line = line[last_space+1:]
                last_space = -1
                line_remaining = width - len(line)
            if token_needs > line_remaining:
                buf += line + '\n'
                line = ''
                line_remaining = 0

        if need_whitespace:
            last_space = len(line)
            line += ' '

        line += token

    buf += line

    cnt = len(buf)

    ws = len(re.findall(r'(?s)\s', buf))
    br = len(re.findall(r'(?s)[{};]\s', buf))

    print >> sys.stderr, '/* {:>30}: total size={:4}, contest length={:4} */'.format(
        filename, cnt, cnt-ws-br)

    print buf


    
######################################################################

filename = sys.argv[1]
f = open(filename)
istr = f.read()

(buf, tokens) = get_tokens(istr)

wrap_simple(filename, buf, tokens, 80)
