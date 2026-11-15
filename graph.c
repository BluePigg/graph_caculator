#include <raylib.h>
#include <stdio.h>

#define CMD_LEN 100
#define STK_LEN 64
#define ZOOM_UNIT 1.1
#define MOVE_UNIT 10

enum {
  OP_NUM = 0,
  OP_X,
  OP_Y,
  OP_PLUS,
  OP_MINUS,
  OP_NEG,
  OP_MUL,
  OP_DIV,
  OP_POW,
  OP_OPEN,
  OP_CLOSE,
  OP_PI,
  OP_E,
  OP_SIN,
  OP_COS,
  OP_TAN,
  OP_LOG
};

int W = 800;
int H = 800;

float zoom = 1.0f;
int scr_x = 0;
int scr_y = 0;

// 0:draw function. 1:convergence check. (ex:fractal)
int mode = 0;

char cmd[CMD_LEN];

int op_stk[STK_LEN];
int res_stk[STK_LEN];
float res_val_stk[STK_LEN];

char *input(const char *msg) {
  printf("%s", msg);
  fflush(stdout);
  fgets(cmd, CMD_LEN, stdin);
  return cmd;
}

int getRank(int x) {
  if (x == OP_POW)
    return 4;
  if (x == OP_NEG)
    return 3;
  if (x == OP_MUL || x == OP_DIV)
    return 2;
  if (x == OP_PLUS || x == OP_MINUS)
    return 1;
  return 0;
}

int get_oenum(char op, int number_flag) {
  int oenum = -1;
  if (op == '-' && number_flag == 0)
    oenum = OP_NEG;
  else if (op == '-')
    oenum = OP_MINUS;
  else if (op == '+')
    oenum = OP_PLUS;
  else if (op == '*')
    oenum = OP_MUL;
  else if (op == '/')
    oenum = OP_DIV;
  else if (op == '^')
    oenum = OP_POW;
  else if (op == '(')
    oenum = OP_OPEN;
  else if (op == ')')
    oenum = OP_CLOSE;
  return oenum;
}

int is_insideFenum(int en) {
  return (en == OP_LOG || en == OP_SIN || en == OP_COS || en == OP_TAN);
}

// is a lower than b or equal?
int isLower(int a, int b) { return (getRank(a) - getRank(b) <= 0); }

// when \0 found, comparison will instantly end.
int cmpstr(const char *a, const char *b) {
  while (*a && *b && (*a == *b)) {
    a++;
    b++;
  }
  return *a == *b;
}

int operator_operation(char op, int *stk_n, int *op_n, int *number_flag) {
  int oenum = get_oenum(op, *number_flag);
  if (oenum == -1)
    return 0;

  if (oenum == OP_CLOSE) {
    while (0 < *op_n && op_stk[*op_n - 1] != OP_OPEN) {
      res_stk[*stk_n] = op_stk[--(*op_n)];
      res_val_stk[(*stk_n)++] = 0;
    }
    if (*op_n > 0 && op_stk[*op_n - 1] == OP_OPEN) {
      (*op_n)--;
    }
    while (0 < *op_n && is_insideFenum(op_stk[*op_n - 1])) {
      res_stk[*stk_n] = op_stk[--(*op_n)];
      res_val_stk[(*stk_n)++] = 0;
    }
    *number_flag = 1;
    return 1;
  }

  if (oenum != OP_NEG && oenum != OP_OPEN) {
    while (*op_n > 0 && isLower(oenum, op_stk[*op_n - 1])) {
      res_stk[*stk_n] = op_stk[--(*op_n)];
      res_val_stk[(*stk_n)++] = 0;
    }
  }
  op_stk[(*op_n)++] = oenum;
  *number_flag = 0;
  return 1;
}

int special_operation(const char *sp, int *stk_n, int *op_n, int *number_flag) {
  int isSpecial = 1;
  if (cmpstr(sp, "x")) {
    res_stk[*stk_n] = OP_X;
    res_val_stk[(*stk_n)++] = 0;
    *number_flag = 1;
  } else if (cmpstr(sp, "y")) {
    res_stk[*stk_n] = OP_Y;
    res_val_stk[(*stk_n)++] = 0;
    *number_flag = 1;
  } else if (cmpstr(sp, "pi")) {
    res_stk[*stk_n] = OP_PI;
    res_val_stk[(*stk_n)++] = 0;
    *number_flag = 1;
  } else if (cmpstr(sp, "e")) {
    res_stk[*stk_n] = OP_E;
    res_val_stk[(*stk_n)++] = 0;
    *number_flag = 1;
  } else if (cmpstr(sp, "log")) {
    op_stk[(*op_n)++] = OP_LOG;
    *number_flag = 0;
  } else if (cmpstr(sp, "sin")) {
    op_stk[(*op_n)++] = OP_SIN;
    *number_flag = 0;
  } else if (cmpstr(sp, "cos")) {
    op_stk[(*op_n)++] = OP_COS;
    *number_flag = 0;
  } else if (cmpstr(sp, "tan")) {
    op_stk[(*op_n)++] = OP_TAN;
    *number_flag = 0;
  } else {
    isSpecial = 0;
  }
  return isSpecial;
}

int fill_stk_w_cmd() {

  int number_temp = 0;
  int number_temp_pow = 0;
  int number_added = 0;

  char temp_str[4];
  int temp_str_idx = 0;

  int number_flag = 0;

  int i = 0;
  // it means "(res,res_val) stk next" and "op stk next"
  int stk_n = 0;
  int op_n = 0;
  char c;
  while (i < CMD_LEN && (c = cmd[i]) != '\n' && c != '\0') {
    if (c == '=') {
      if (number_added) {
        res_stk[stk_n] = OP_NUM;
        res_val_stk[stk_n++] = number_temp;
      }
      if (temp_str_idx >= 1) {
        temp_str[temp_str_idx] = '\0';
        int isSpecial =
            special_operation(temp_str, &stk_n, &op_n, &number_flag);

        if (isSpecial) {
          temp_str_idx = 0;
        } else if (temp_str_idx >= 3) {
          return 1;
        }
      }

      while (op_n > 0) {
        res_stk[stk_n] = op_stk[--op_n];
        res_val_stk[stk_n++] = 0;
      }
      number_temp = 0;
      number_temp_pow = 0;
      number_added = 0;
      temp_str_idx = 0;
      number_flag = 0;
      i++;
      continue;
    }

    int isNumber = 0;
    if ('0' <= c && c <= '9') {
      number_temp = number_temp * 10 + (c - '0');
      number_added = 1;
      isNumber++;
    } else if (number_added) {
      res_stk[stk_n] = OP_NUM;
      res_val_stk[stk_n++] = number_temp;

      number_temp = 0;
      number_temp_pow = 0;
      number_added = 0;

      number_flag = 1;
    }
    int isOp = operator_operation(c, &stk_n, &op_n, &number_flag);
    if (!(isNumber || isOp) && c != ' ') {
      temp_str[temp_str_idx++] = c;
      if (temp_str_idx >= 1) {
        temp_str[temp_str_idx] = '\0';
        int isSpecial =
            special_operation(temp_str, &stk_n, &op_n, &number_flag);

        if (isSpecial) {
          temp_str_idx = 0;
        } else if (temp_str_idx >= 3) {
          return 1;
        }
      }
    }

    i++;
  }

  if (number_added) {
    res_stk[stk_n] = OP_NUM;
    res_val_stk[stk_n++] = number_temp;
  }
  if (temp_str_idx >= 1) {
    temp_str[temp_str_idx] = '\0';
    int isSpecial = special_operation(temp_str, &stk_n, &op_n, &number_flag);

    if (isSpecial) {
      temp_str_idx = 0;
    } else if (temp_str_idx >= 3) {
      return 1;
    }
  }

  while (op_n > 0) {
    res_stk[stk_n] = op_stk[--op_n];
    res_val_stk[stk_n++] = 0;
  }

  res_stk[stk_n] = OP_MINUS;
  res_val_stk[stk_n++] = 0;

  return 0;
}

int main(int argc, char *argv[]) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(W, H, "Graphing.");
  SetTargetFPS(60);

  Shader shader = LoadShader(0, "gpu.frag");

  while (!WindowShouldClose()) {
    float for_zoom =
        IsKeyDown(KEY_EQUAL) * ZOOM_UNIT + IsKeyDown(KEY_MINUS) / ZOOM_UNIT;
    zoom *= (for_zoom != 0) ? for_zoom : 1.0f;
    scr_x +=
        IsKeyDown(KEY_RIGHT) * MOVE_UNIT + IsKeyDown(KEY_LEFT) * MOVE_UNIT * -1;
    scr_y +=
        IsKeyDown(KEY_UP) * MOVE_UNIT + IsKeyDown(KEY_DOWN) * MOVE_UNIT * -1;

    if (IsKeyDown(KEY_SPACE)) {
      input("Please enter new function to draw!\n>");
      int fill_fail = fill_stk_w_cmd();
      if (fill_fail) {
        return 1;
      }
    }

    BeginDrawing();

    ClearBackground(WHITE);

    BeginShaderMode(shader);

    DrawRectangle(0, 0, W, H, WHITE);

    EndShaderMode();

    EndDrawing();
  }

  UnloadShader(shader);

  CloseWindow();

  return 0;
}
