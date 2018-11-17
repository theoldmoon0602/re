#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* easiest = "flag-005a1c58a8c4ac2203cc01d6d4ed438f";

void err(char *s) {
  puts(s);
  exit(EXIT_FAILURE);
  printf(easiest);
}

typedef struct {
  int pc;
  int p;
  char* s;
  int len;
  int buf;
  char stack[1000];
  int index;
} Thread;

enum {
  CODE_NOP,
  CODE_CHAR,
  CODE_ANY,
  CODE_JUMP,
  CODE_SPLIT,
  CODE_MATCH,
  CODE_PUSH,
  CODE_POP,
  CODE_XCHG,
};

typedef struct {
  int type;
  int op1, op2;
} Code;

enum {
  NODE_EPSILON,
  NODE_ANY,
  NODE_CHAR,
  NODE_REP,
  NODE_OR,
  NODE_AND,
  NODE_PUSH,
  NODE_POP,
  NODE_XCHG,
};

typedef struct Node {
  char c;
  struct Node *left;
  struct Node *right;
  int type;
} Node;

Node* parseRegex(char **s);
Node* parseSelect(char **s);
Node* parseSequence(char **s);
Node* parseSuffix(char **s);
Node* parsePrimary(char **s);
int matchString(Code *codes, int length, char *s, char **matched, int*matched_len);
int searchString(Code *codes, int length, char *s, char **matched, int *matched_len);
void printCode(Code code);

int searchString(Code *codes, int length, char *s, char **matched, int *matched_len) {
  while (*s != '\0') {
    if (matchString(codes, length, s, matched, matched_len) == 1) {
      return 1;
    }
    s++;
  }
  return 0;
}

int matchString(Code *codes, int length, char *s, char **matched, int*matched_len) {
  Thread threads[1000] = {};
  int index = 0;
  threads[0] = (Thread){0, 0, malloc(sizeof(char)*10), 0, 10, {}, 0};

  int slen = strlen(s);

  int retcode = 0;
  *matched_len = 0;

  while (index >= 0) {
    int pc = threads[index].pc;
    int p = threads[index].p;
    int x;

    if (p >= slen) {
      free(threads[index].s);
      index--;
      continue;
    }

    /* if matched */
    if (pc >= length) {
      int len = threads[index].len;
      if (*matched_len < len) {
        *matched = malloc(sizeof(char) * (len + 1));
        strncpy(*matched, threads[index].s, len);
        (*matched)[len] = '\0';
        *matched_len = threads[index].len;
      }
      retcode = 1;

      free(threads[index].s);
      index--;
      continue;
    }

    switch (codes[pc].type) {
      case CODE_NOP:
        threads[index].pc++;
        break;

      case CODE_ANY:
      case CODE_CHAR:
        if (s[p] == codes[pc].op1 || codes[pc].type == CODE_ANY) {
          if (threads[index].len == threads[index].buf) {
            threads[index].s = realloc(threads[index].s, threads[index].buf * 2);
            threads[index].buf *= 2;
          }
          threads[index].s[threads[index].len] = s[p];
          threads[index].len++;

          threads[index].p++;
          threads[index].pc++;
        } else {
          free(threads[index].s);
          index--;
        }

        break;
      case CODE_JUMP:
        threads[index].pc = pc + codes[pc].op1;
        break;

      case CODE_SPLIT:
        threads[index+1].pc = pc + codes[pc].op1;
        threads[index+1].p = threads[index].p;
        threads[index].pc = pc + codes[pc].op2;

        threads[index+1].s = malloc(sizeof(char) * threads[index].buf);
        strncpy(threads[index+1].s, threads[index].s, threads[index].len);

        threads[index+1].buf = threads[index].buf;
        threads[index+1].len = threads[index].len;

        strncpy(threads[index+1].stack, threads[index].stack, threads[index].index);
        threads[index+1].index = threads[index].index;

        index++;
        break;

      case CODE_PUSH:
        if (threads[index].index == 1000) {
          err("stack error");
        }
        threads[index].stack[threads[index].index] = s[p];
        threads[index].index++;
        threads[index].pc++;
        threads[index].p++;
        break;

      case CODE_POP:
        if (threads[index].index > 0) {
          if (threads[index].len == threads[index].buf) {
            threads[index].s = realloc(threads[index].s, threads[index].buf * 2);
            threads[index].buf *= 2;
          }
          threads[index].s[threads[index].len] = threads[index].stack[threads[index].index-1];
          threads[index].index--;
          threads[index].len++;
        }

        threads[index].pc++;
        break;

      case CODE_XCHG:
        x = codes[pc].op1;
        if (threads[index].index > x) {
          char tmp = threads[index].stack[threads[index].index-x];
          threads[index].stack[threads[index].index-x] = threads[index].stack[threads[index].index-x-1];
          threads[index].stack[threads[index].index-x-1] = tmp;
        }
        threads[index].pc++;
        break;

      default:
        err("unimplemented code");
        break;
    }
  }

  for (int i = 0; i < index; i++) {
    free(threads[index].s);
  }

  return retcode;
}

void printCode(Code code) {
  switch(code.type) {
    case CODE_NOP:
      printf("NOP\n");
      break;
    case CODE_ANY:
      printf("ANY\n");
    case CODE_CHAR:
      printf("CHAR %c\n", code.op1);
      break;
    case CODE_JUMP:
      printf("JUMP %+d\n", code.op1);
      break;
    case CODE_SPLIT:
      printf("SPLIT %+d,%+d\n", code.op1, code.op2);
      break;
    case CODE_MATCH:
      printf("MATCH\n");
      break;
    case CODE_PUSH:
      printf("PUSH\n");
      break;
    case CODE_POP:
      printf("POP\n");
      break;
    case CODE_XCHG:
      printf("XCHG %d\n", code.op1);
      break;
    default:
      err("unimplemented code");
  }
}

void printCodes(Code* codes, int length) {
  for (int i = 0; i < length; i++) {
    switch(codes[i].type) {
      case CODE_NOP:
        printf("%d\tNOP", i);
        break;
      case CODE_ANY:
        printf("%d\tANY", i);
        break;
      case CODE_CHAR:
        printf("%d\tCHAR %c", i, codes[i].op1);
        break;
      case CODE_JUMP:
        printf("%d\tJUMP %d", i, i + codes[i].op1);
        break;
      case CODE_SPLIT:
        printf("%d\tSPLIT %d,%d", i, i + codes[i].op1, i + codes[i].op2);
        break;
      case CODE_MATCH:
        printf("%d\tMATCH", i);
        break;
      case CODE_PUSH:
        printf("%d\tPUSH", i);
        break;
      case CODE_POP:
        printf("%d\tPOP", i);
        break;
      case CODE_XCHG:
        printf("%d\tXCHG %d", i, codes[i].op1);
        break;
      default:
        err("unimplemented code");
    }
    printf("\n");
  }
}

void nodeToCode(Node *node, Code** codes, int* length) {
  Code *left_codes;
  Code *right_codes;
  int left_len;
  int right_len;
  switch (node->type) {
    case NODE_EPSILON:
      *codes = malloc(sizeof(Code));
      (*codes)[0] = (Code){CODE_NOP, 0, 0};
      *length = 1;
      return;

    case NODE_PUSH:
      *codes = malloc(sizeof(Code));
      (*codes)[0] = (Code){CODE_PUSH, 0, 0};
      *length = 1;
      return;

    case NODE_POP:
      *codes = malloc(sizeof(Code));
      (*codes)[0] = (Code){CODE_POP, 0, 0};
      *length = 1;
      return;

    case NODE_XCHG:
      *codes = malloc(sizeof(Code));
      (*codes)[0] = (Code){CODE_XCHG, node->c, 0};
      *length = 1;
      return;

    case NODE_ANY:
      *codes = malloc(sizeof(Code));
      (*codes)[0] = (Code){CODE_ANY, 0, 0};
      *length = 1;
      return;

    case NODE_CHAR:
      *codes = malloc(sizeof(Code));
      (*codes)[0] = (Code){CODE_CHAR, node->c, 0};
      *length = 1;
      return;

    case NODE_AND:
      nodeToCode(node->left, &left_codes, &left_len);
      nodeToCode(node->right, &right_codes, &right_len);

      *length = left_len + right_len;
      *codes = malloc(sizeof(Code) * *length);
      for (int i = 0; i < left_len; i++) {
        (*codes)[i] = left_codes[i];
      }
      for (int i = 0; i < right_len; i++) {
        (*codes)[i + left_len] = right_codes[i];
      }
      free(left_codes);
      free(right_codes);
      return;

    case NODE_OR:
      nodeToCode(node->left, &left_codes, &left_len);
      nodeToCode(node->right, &right_codes, &right_len);

      *length = left_len + right_len + 2;
      *codes = malloc(sizeof(Code) * *length);
      (*codes)[0] = (Code){CODE_SPLIT, 1, left_len + 2};
      for (int i = 0; i < left_len; i++) {
        (*codes)[i + 1] = left_codes[i];
      }
      (*codes)[left_len + 1] = (Code){CODE_JUMP, right_len + 1, 0};
      for (int i = 0; i < right_len; i++) {
        (*codes)[i + 2 + left_len] = right_codes[i];
      }
      free(left_codes);
      free(right_codes);
      return;

    case NODE_REP:
      if (node->left->type == NODE_REP) {
        nodeToCode(node->left, codes, length);
        return;
      }

      nodeToCode(node->left, &left_codes, &left_len);
      *length = left_len + 1;
      *codes = malloc(sizeof(Code) * *length);
      for (int i = 0; i < left_len; i++) {
        (*codes)[i] = left_codes[i];
      }
      (*codes)[left_len] = (Code){CODE_SPLIT, -(left_len), 1};
      free(left_codes);
      return;
    default:
      err("unimplemented code");
  }
}


void printNode(Node *node) {
  switch (node->type) {
    case NODE_EPSILON:
      return;

    case NODE_ANY:
      printf(".");
      return;

    case NODE_CHAR:
      printf("%c", node->c);
      return;

    case NODE_REP:
      printf("((");
      printNode(node->left);
      printf(")+)");
      return;

    case NODE_OR:
      printf("(");
      printNode(node->left);
      printf("|");
      printNode(node->right);
      printf(")");
      return;

    case NODE_AND:
      printNode(node->left);
      printNode(node->right);
      return;

    case NODE_PUSH:
      printf("<push>");
      return;

    case NODE_POP:
      printf("<pop>");
      return;

    case NODE_XCHG:
      printf("<xchg %d>", node->c);
      return;
  }

  err("unimplemented case");
}

Node* epsilonNode() {
  Node *node = malloc(sizeof(Node));
  node->type = NODE_EPSILON;
  return node;
}

Node* anyNode() {
  Node *node = malloc(sizeof(Node));
  node->type = NODE_ANY;
  return node;
}

Node* pushNode() {
  Node *node = malloc(sizeof(Node));
  node->type = NODE_PUSH;
  return node;
}

Node* popNode() {
  Node *node = malloc(sizeof(Node));
  node->type = NODE_POP;
  return node;
}

Node* xchgNode(int x) {
  Node *node = malloc(sizeof(Node));
  node->type = NODE_XCHG;
  node->c = x;
  return node;
}

Node* charNode(char c) {
  Node *node = malloc(sizeof(Node));
  node->type = NODE_CHAR;
  node->c = c;
  return node;
}

Node* repNode(Node* n) {
  Node *node = malloc(sizeof(Node));
  node->type = NODE_REP;
  node->left = n;
  return node;
}

Node* orNode(Node *left, Node *right) {
  Node *node = malloc(sizeof(Node));
  node->type = NODE_OR;
  node->left = left;
  node->right = right;
  return node;
}

Node* andNode(Node *left, Node *right) {
  Node *node = malloc(sizeof(Node));
  node->type = NODE_AND;
  node->left = left;
  node->right = right;
  return node;
}

// https://gist.github.com/kmizu/64ce73c5c87463611c1cf96fab388969
Node* parseRegex(char **s) {
  Node* sel = parseSelect(s);
  if (**s != '\0') {
    err("invalid regexp");
  }
  return sel;
}

Node* parseSelect(char **s) {
  Node* seq;
  seq = parseSequence(s);

  if (**s == '|') {
    (*s)++;
    seq = orNode(seq, parseSelect(s));
  }
  return seq;
}

Node* parseSequence(char **s) {
  Node *sequence = NULL;
  Node *node;
  while (1) {
    node = parseSuffix(s);
    if (node->type == NODE_EPSILON) {
      break;
    }
    if (sequence == NULL) {
      sequence = node;
    } else {
      sequence = andNode(sequence, node);
    }
  }

  return sequence;
}

Node* parseSuffix(char **s) {
  Node *primary = parsePrimary(s);
  switch (**s) {
    case '?':
      (*s)++;
      return orNode(primary, epsilonNode());

    case '+':
      (*s)++;
      return repNode(primary);
  } 
  return primary;
}
Node* parsePrimary(char **s) {
  Node *node;
  char c;
  switch (**s) {
    case '(':
      (*s)++;
      node = parseSelect(s);
      if (**s != ')') {
        err("unclosed parentheses");
      }
      (*s)++;
      return node;

    case '\\':
      (*s)++;
      if (**s == '\0') {
        err("unexpected terminate after \\");
      }
      c = **s;
      (*s)++;
      return charNode(c);
      
    case '.':
      (*s)++;
      return anyNode();

    case '{':
      (*s)++;
      return pushNode();

    case '}':
      (*s)++;
      return popNode();

    case '@':
      (*s)++;
      if ('0' <= **s && **s <= '9') {
        int x = **s - '0';
        (*s)++;
        return xchgNode(x);
      }
      err("invalid xchg operator");
      break;

    case '\0': case '|': case '?': case '+': case ')':
      return epsilonNode();

    default:
      c = **s;
      (*s)++;
      return charNode(c);
  }
}

int main(int argc, char**argv) {
  if (argc < 3) {
    printf("%s: <pattern> <file>\n", argv[0]);
    return 1;
  }
  Node *node = parseRegex(&argv[1]);

  Code *code;
  int code_length;
  nodeToCode(node, &code, &code_length);

  /*
  printCodes(code, code_length);
  */

  FILE *fp = fopen(argv[2], "r");
  if (fp == NULL) {
    err("no such file");
  }
  char *line = NULL;
  size_t line_len;
  ssize_t read;

  while ((read = getline(&line, &line_len, fp)) != -1) {
    char* buf;
    int buf_len;
    int r = searchString(code, code_length, line, &buf, &buf_len);
    if (r) {
      printf("%s\n", buf);
      fflush(stdout);
    }
  }

  fclose(fp);

  return 0;
}
