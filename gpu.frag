#version 330

#define  OP_NUM 0
#define  OP_X 1
#define  OP_Y 2
#define  OP_PLUS 3
#define  OP_MINUS 4
#define  OP_NEG 5
#define  OP_MUL 6 
#define  OP_DIV 7 
#define  OP_POW 8 
#define  OP_OPEN 9 
#define  OP_CLOSE 10 
#define  OP_PI 11 
#define  OP_E 12 
#define  OP_SIN 13 
#define  OP_COS 14
#define  OP_TAN 15 
#define  OP_LOG 16


out vec4 finalColor;

uniform float zoom;
uniform vec2 scr_pos;
uniform vec2 scr_size;

uniform int stack_length;
uniform int[64] main_stack;
uniform float[64] value_stack;

float zoom_unit = 100.0;

bool isSingled(int op) {
  return (op==OP_SIN || op==OP_COS || op==OP_TAN || op==OP_NEG || op==OP_LOG);
}

float error_margin(vec2 cur_pos,int ms[64], float[64] vs, int sl) {
  if (sl<=0) {return 1e9;}
  float[64] memory;
  int memory_idx = 0;
  int stack_idx = 0;

  for (int i=0; i<sl; i++) {
    int cur_op = ms[i];
    float cur_va = vs[i];

    if (cur_op == OP_NUM) {
      memory[memory_idx++] = cur_va;
    } else if (cur_op == OP_X) {
      memory[memory_idx++] = cur_pos.x;
    } else if (cur_op == OP_Y) {
      memory[memory_idx++] = cur_pos.y;
    } else if (cur_op == OP_PI) {
      memory[memory_idx++] = 3.14159265359;
    } else if (cur_op == OP_E) {
      memory[memory_idx++] = exp(1.0);
    } else if (0<memory_idx && isSingled(cur_op)) {
      float target = memory[memory_idx-1];
      if (cur_op == OP_SIN) {
        memory[memory_idx-1] = sin(target);
      } else if (cur_op == OP_COS) {
        memory[memory_idx-1] = cos(target);
      } else if (cur_op == OP_TAN) {
        memory[memory_idx-1] = tan(target);
      } else if (cur_op == OP_LOG) {
        memory[memory_idx-1] = log(target);
      } else if (cur_op == OP_NEG) {
        memory[memory_idx-1] = -target;
      }
    } else if (1<memory_idx) {
      float tleft = memory[memory_idx-2];
      float tright = memory[memory_idx-1];
      int nidx = (--memory_idx)-1;
      if (cur_op == OP_PLUS) {
        memory[nidx] = tleft + tright;
      } else if (cur_op == OP_MINUS) {
        memory[nidx] = tleft-tright;
      } else if (cur_op == OP_MUL) {
        memory[nidx] = tleft*tright;
      } else if (cur_op == OP_DIV) {
        memory[nidx] = tleft/tright;
      } else if (cur_op == OP_POW) {
        if (tleft<0.0 && floor(tright)==tright) {
          float psign = ((mod(abs(tright), 2.0))==1.0) ? -1.0 : 1.0;
          memory[nidx] = pow(-tleft,tright)*psign;
        } else {
          memory[nidx] = pow(tleft,tright);
        }
      }
    }
  }

  return memory[0];
}

bool does_accept(float margin) {
  return abs(margin) <= fwidth(margin)*1.5;
}

void main() {
  finalColor = vec4(1.0);

  vec2 p = (gl_FragCoord.xy - 0.5 * scr_size) / (zoom * zoom_unit) + scr_pos;

  float emargin = error_margin(p,main_stack,value_stack,stack_length);
  float xmargin = p.x;
  float ymargin = p.y;
  if (does_accept(emargin)) {
    finalColor = vec4(1.0,0,0,1.0);
  } else if (does_accept(xmargin) || does_accept(ymargin)) {
    finalColor = vec4(0,0,0,1.0);
  }
}
