#include <ssw.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "xbm/suits.xbm"
#include "xbm/ranks.xbm"
#include "xbm/directions.xbm"

/*
* Find ELAN from `xinput` and corresponding ID
*
* xinput map-to-output 19 eDP-1
*
*/

enum suit {H, S, C, D};
enum rank {ACE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING};

struct player {
enum suit psuits[4]; // like 0: C, 1: S, 2: H, 3: D
                     // for  CS  player config
                     //      HD
int directions[4];
char lost_cann[4];
int pvect_loca[2];
int board_loca[2];
int round_shot[4][2]; // [0] = magnitude; [1] = boolean
};

union meld {
float j;
int   i;
};

#define DISPLAY_OFF 1920

void draw_suit(int d, int x, int y, enum suit which) {

  switch (which) {

    case C:
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 32; j++) {
      RegionFill(i + x, j + y, 1, 1, !( ( suit0_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;
    case S:
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 32; j++) {
      RegionFill(i + x, j + y, 1, 1, !( ( suit1_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;

    case H:
     for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 32; j++) {
      RegionFill(i + x, j + y, 1, 1, !( ( suit2_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;
    case D:
     for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 32; j++) {
      RegionFill(i + x, j + y, 1, 1, !( ( suit3_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;
  }

}

void draw_rank(int d, int x, int y, enum rank which) { // THIS SHIT IS WHY COMPILER DIRECTIVES EXIST

  switch (which) {

    case ACE:
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 48; j++) {
      RegionFill(i + x, j + y, 1, 1, !( ( rank0_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;
    case TWO:
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 48; j++) {
      RegionFill(i + x, j + y, 1, 1, !( ( rank1_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;
    case THREE:
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 48; j++) {
      RegionFill(i + x, j + y, 1, 1, !( ( rank2_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;
    case FOUR:
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 48; j++) {
      RegionFill(i + x, j + y, 1, 1, !( ( rank3_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;
    case FIVE:
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 48; j++) {
      RegionFill(i + x, j + y, 1, 1, !( ( rank4_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;
    case SIX:
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 48; j++) {
      RegionFill(i + x, j + y, 1, 1, !( ( rank5_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;
    case SEVEN:
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 48; j++) {
      RegionFill(i + x, j + y, 1, 1, !( ( rank6_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;
    case EIGHT:
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 48; j++) {
      RegionFill(i + x, j + y, 1, 1, !( ( rank7_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;
    case NINE:
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 48; j++) {
      RegionFill(i + x, j + y, 1, 1, !( ( rank8_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;
    case TEN:
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 48; j++) {
      RegionFill(i + x, j + y, 1, 1, !( ( rank9_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;
    case JACK:
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 48; j++) {
      RegionFill(i + x, j + y, 1, 1, !( ( rank10_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;
    case QUEEN:
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 48; j++) {
      RegionFill(i + x, j + y, 1, 1, !( ( rank11_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;
    case KING:
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 48; j++) {
      RegionFill(i + x, j + y, 1, 1, !( ( rank12_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;

  }

}

int dir_x(int i, int dir) {
dir = dir>>1;
return dir*31 + (dir ? -1 : 1)*i;
}

void draw_dir(int d, int x, int y, int dir) {

int i_offx, i_offy, offx, offy;
char cond = (dir&1)^(dir>>1);

  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 32; j++) {
    i_offx = dir_x(i, dir);
    i_offy = j;
    offx = cond ? i_offx : i_offy;
    offy = cond ? i_offy : 31 - i_offx;
    RegionFill(x + offx, y + offy, 1, 1, !( ( dir_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
    }
  }

}

void back(void) {
    for (int i = 0; i < 56; i++) {
    RegionScarf(10 + DISPLAY_OFF + 1, 20 + 1, (i + 1) * 34, (i + 1) * 34, RGB(255,255,255), 0);
    RegionScarf(10 + DISPLAY_OFF + 1 + (56 - i) * 34, 20 + 1 + (56 - i) * 34, i * 34, i * 34, RGB(255,255,255), 0);
    }

  RegionFill(10 + DISPLAY_OFF, 1075, 1920, 20, RGB(0,0,0), 0);
  RegionFill(WW(0) - 4, 0, 10, 1080, RGB(0,0,0), 0);
}

void pad(void) {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
    RegionScarf(i * 333 + 1, j * 333 + 1, 333, 333, RGB(255,255,255), 19);
    }
  }
  for (int i = 0; i < 4; i++) {
    RegionScarf(i * 250 + 1, 666, 250, 250, RGB(255, 255, 255), 19);
    draw_suit(19, i * 250 + 116, 773, (enum suit)i);
  }

}

void pad_ranks(struct player *p, int select[4]) {
  for (int i = 0; i < 13; i++) {
    for (int j = 0; j < 4; j++) {
    RegionScarf(i * 68 + 1, j * 150 + 1, 68, 150, RGB(255,255,255), 19);
    }
  }

  RegionScarf(0, 601, 499, 290, RGB(255,255,255), 19); // continue
  RegionScarf(0, 601, 990, 290, RGB(255,255,255), 19); // dir?
  RegionScarf(0, 891, 499, 90, RGB(255,255,255), 19); // 
  RegionScarf(0, 891, 990, 90, RGB(255,255,255), 19); // 


  for (int i = 0; i < 4; i++) {
    if (select[i] < 13) {
      RegionScarf(select[i] * 68 + 10, i * 150 + 10, 48, 130, RGB(255,255,255), 19);
    }
    if (p->lost_cann[i]) {
    RegionFill(13 * 68 + 33, i * 150 + 58, 34, 34, RGB(255,255,255), 19);
    } else {
    draw_dir(19, 13 * 68 + 33, i * 150 + 58, p->directions[i]);
    }
  }

  for (int i = 0; i < 13; i++) {
    for (int j = 0; j < 4; j++) {
      draw_rank(19, i * 68 + 17, j * 150 + 51, (enum rank)i);

      if (p->lost_cann[j]) {
      RegionFill(i * 68, j * 150, 34, 34, RGB(255,255,255), 19);
      RegionFill(i  *68 + 34, j * 150 + 116, 34, 34, RGB(255,255,255), 19);
      } else {
      draw_suit(19, i * 68, j * 150, p->psuits[j]);
      draw_suit(19, i * 68 + 34, j * 150 + 116, p->psuits[j]);
      }

    }
  }

  //for (int i = 0; i < 4; i++) {
  //  RegionScarf(i * 125 + 1, 332, 125, 125, RGB(255, 255, 255), 19);
  //  draw_suit(19, i * 125 + 50, 352, (enum suit)i);
  //}

}


void plist(struct player **multi, int num) {
  for (int i = 0; i < num; i++) {
  RegionScarf(1800, 1 + i * 100, 100, 100, RGB(255,255,255), 0);
    for (int j = 0; j < 4; j++) {
      if (multi[i]->lost_cann[j]) {
      RegionFill(1810 + 34 * (j&1) - 1, 10 + i * 100 + 34 * (j>>1) - 1, 34, 34, RGB(255,255,255), 0);
      RegionFill(1810 + 34 * (j&1) + 5, 10 + i * 100 + 34 * (j>>1) + 5, 23, 23, RGB(0,0,0), 0);
      } else {
      draw_dir(0, 1720 + 34 * (j&1), 10 + i * 100 + 34 * (j>>1), multi[i]->directions[j]);
      draw_suit(0, 1810 + 34 * (j&1), 10 + i * 100 + 34 * (j>>1), (enum suit)multi[i]->psuits[j]);
      }
    }
  }
}

int abs(int x) {
return x < 0 ? -x : x;
}

int sgn(int x) { // not standard
return x < 0;
}

int boil(int x) {
  if (!x) {
  return rand()&1 ? -1 : 1;
  }

  while (abs(x) != 1) {
  x = x>>1;
  }
  return x;
}

void rand_populate(int coords[4][2]) {
coords[0][0] = 19 + abs(rand()%18);
coords[0][1] = 12 + abs(rand()%10);
int x, y;
  for (int i = 1; i < 4; i++) {
    x = 0;
    y = 0;
    while (x == 0 || y == 0) { 
    x = x ? x : rand()%4;
    y = y ? y : rand()%4;
    }
    if (rand()&1) {
    x = -x;
    }
    if (rand()&1) {
    y = -y;
    }
  coords[i][0] = coords[i - 1][0] + x;
  coords[i][1] = coords[i - 1][1] + y; // may still overlap with previous (not immediate previous, maybe i -2)
  }
}

char validate(int coords[4][2]) {
char check = 1;
check &= abs(coords[0][0] - coords[1][0]) < 2;
check &= abs(coords[0][1] - coords[1][1]) < 2;
check &= abs(coords[0][0] - coords[2][0]) < 2;
check &= abs(coords[0][1] - coords[2][1]) < 2;
check &= abs(coords[0][0] - coords[3][0]) < 2;
check &= abs(coords[0][1] - coords[3][1]) < 2;
check &= abs(coords[1][0] - coords[2][0]) < 2;
check &= abs(coords[1][1] - coords[2][1]) < 2;
check &= abs(coords[1][0] - coords[3][0]) < 2;
check &= abs(coords[1][1] - coords[3][1]) < 2;
check &= abs(coords[2][0] - coords[3][0]) < 2;
check &= abs(coords[2][1] - coords[3][1]) < 2;

return check;

}

int intersect(struct player *p, int *strike) {
  if        (p->board_loca[0] == strike[0] && p->board_loca[1] == strike[1]) {
  return 0;
  } else if (p->board_loca[0] + 1 == strike[0] && p->board_loca[1] == strike[1]) {
  return 1;
  } else if (p->board_loca[0] == strike[0] && p->board_loca[1] + 1 == strike[1]) {
  return 2;
  } else if (p->board_loca[0] + 1 == strike[0] && p->board_loca[1] + 1 == strike[1]) {
  return 3;
  }
return 4;
}

int **fire(struct player *p) { // returns int[4][2]
  int *shots[4] = {p->round_shot[0],
                   p->round_shot[1],
                   p->round_shot[2],
                   p->round_shot[3]
                  };

  int *dirs = p->directions;
  int *loca = p->board_loca;
  int *prev = p->pvect_loca;
  char *no_cann = p->lost_cann;

  int **saved_shots = malloc(sizeof(int *) * 4);
  int *landing;

  for (int i = 0; i < 4; i++) {
    landing = malloc(sizeof(int) * 2);
    landing[0] = loca[0] + (i&1);
    landing[1] = loca[1] + (i>>1);
    if (shots[i][1] && !no_cann[i]) {

      switch (dirs[i]) {
        case 0: // decrease y
        landing[1] = landing[1] - shots[i][0] < 0 ? landing[1] - shots[i][0] + 30 : landing[1] - shots[i][0];
        break;
        case 1: // increase x
        landing[0] = (landing[0] + shots[i][0])%56;
        break;
        case 2: // decrease x
        landing[0] = landing[0] - shots[i][0] < 0 ? landing[0] - shots[i][0] + 55 : landing[0] - shots[i][0];
        break;
        case 3: // increase y
        landing[1] = (landing[1] + shots[i][0])%31;
        break;
      }
    } else {
      landing[0] = -1;
      landing[1] = -1;
    }
    saved_shots[i] = landing;
  }

  prev[0] = loca[0]; // for use during animation
  prev[1] = loca[1];

  for (int i = 0; i < 4; i++) {
    if (shots[i][1] && !no_cann[i]) {
      switch (dirs[i]) {
        case 0: // increase y
        loca[1] = (loca[1] + shots[i][0] / 2)%30;
        break;
        case 1: // decrease x
        loca[0] = loca[0] - shots[i][0]/2 < 0 ? loca[0] - shots[i][0]/2 + 55 : loca[0] - shots[i][0]/2;
        break;
        case 2: // increase x
        loca[0] = (loca[0] + shots[i][0] / 2)%55;
        break;
        case 3: // decrease y
        loca[1] = loca[1] - shots[i][0]/2 < 0 ? loca[1] - shots[i][0]/2 + 30 : loca[1] - shots[i][0]/2;
        break;
      }
    }
    shots[i][1] = 0;
  }

  prev[0] = prev[0] - loca[0];
  prev[1] = prev[1] - loca[1];

return saved_shots;

}

void track_collisions(struct player **p, int *ret, int pcount, int *coll_count, int *minor_collisions) {
*coll_count = 0;
*minor_collisions = 0;

int mag_i, mag_j;
int x_diff, y_diff;

  for (int i = 0; i < pcount - 1; i++) {
    for (int j = i + 1; j < pcount; j++) {
      x_diff = abs(p[i]->board_loca[0] - p[j]->board_loca[0]);
      y_diff = abs(p[i]->board_loca[1] - p[j]->board_loca[1]);

      if (x_diff < 2 && y_diff < 2) {
      mag_i = abs(p[i]->pvect_loca[0]) + abs(p[i]->pvect_loca[1]); // no square root needed
      mag_j = abs(p[j]->pvect_loca[0]) + abs(p[j]->pvect_loca[1]);
      ret[(*coll_count) * 2    ] = mag_i < mag_j ? i : j; // reference frame
      ret[(*coll_count) * 2 + 1] = mag_i < mag_j ? j : i; // offending collider that will be corrected
      (*coll_count)++;
      printf("collision found at permutation: (%d, %d)\n", i, j);
        if (x_diff==1 && y_diff==1) {
        (*minor_collisions)++;
        }
      }
    }
  }
  printf("total tracked collisions: %d\nminor collisions: %d\n", *coll_count, *minor_collisions);
}

void fix_collisions(struct player **p, int *collisions, int pcount, int num_collisions) {
  int refer, offend;
  int x_diff, y_diff;
  int off_quad, ref_quad;
  int x_shift, y_shift;

  char *off_cann, *ref_cann;
  int *off_dir, *ref_dir;
  enum suit *off_suit, *ref_suit;

  int tmp_dir, tmp_can;
  enum suit tmp_suit;

  char use_quad;

  printf("collisions to be fixed: %d\n", num_collisions);

  for (int i = 0; i < num_collisions; i++) {
  // investigate the nine cases of p[ref] <> p[offend]
  refer  = collisions[i * 2    ];
  offend = collisions[i * 2 + 1];
  x_diff = p[offend]->board_loca[0] - p[refer]->board_loca[0];
  y_diff = p[offend]->board_loca[1] - p[refer]->board_loca[1];


  off_cann = p[offend]->lost_cann;
  ref_cann = p[refer]->lost_cann;
  off_dir  = p[offend]->directions;
  ref_dir  = p[refer]->directions;
  off_suit = p[offend]->psuits;
  ref_suit = p[refer]->psuits;

  x_shift = boil(p[offend]->pvect_loca[0]) * -1;
  y_shift = boil(p[offend]->pvect_loca[1]) * -1;

  use_quad = 0;

    if (abs(x_diff)==1 && abs(y_diff)==1) {
    off_quad = (sgn(y_diff)<<1) | sgn(x_diff);
    ref_quad = (~off_quad)&3;
    use_quad = 1;
    } else if (!x_diff && !y_diff) { // full body collisions
    p[offend]->board_loca[0] += x_shift;
    p[offend]->board_loca[1] += y_shift;
    off_quad = ( sgn(y_shift)<<1 ) | sgn(x_shift);
    ref_quad = (~off_quad)&3;
    use_quad = 1;
    } else if (x_diff ==  0 && y_diff == -1) { // offender top
    p[offend]->board_loca[0] += x_shift;
      if (x_shift == -1) {
      off_quad = 3;
      ref_quad = 0;
      } else {
      off_quad = 2;
      ref_quad = 1;
      }
    use_quad = 1;
    } else if (x_diff ==  0 && y_diff == 1) { // offender bottom
    p[offend]->board_loca[0] += x_shift;
      if (x_shift == -1) {
      off_quad = 1;
      ref_quad = 2;
      } else {
      off_quad = 0;
      ref_quad = 3;
      }
    use_quad = 1;
    } else if (x_diff == -1 && y_diff == 0) { // offender left
    p[offend]->board_loca[1] += y_shift;
      if (y_shift == -1) {
      off_quad = 3;
      ref_quad = 0;
      } else {
      off_quad = 1;
      ref_quad = 2;
      }
    use_quad = 1;
    } else if (x_diff ==  1 && y_diff == 0) { // offender right
    p[offend]->board_loca[1] += y_shift;
      if (y_shift == -1) {
      off_quad = 2;
      ref_quad = 1;
      } else {
      off_quad = 0;
      ref_quad = 3;
      }
    use_quad = 1;
    } else {
    printf("We fixed it previously!\n");
    }

    if (use_quad) { // we have performed a fix and would like to augment cannons
    printf("ref: %d, off: %d\n", ref_quad, off_quad);
      if (!off_cann[off_quad] && !ref_cann[ref_quad]) { // destroy both cannons
      off_cann[off_quad] = 1;
      ref_cann[ref_quad] = 1;
      } else { // exchange
      tmp_can = off_cann[off_quad];
      off_cann[off_quad] = ref_cann[ref_quad]; // trade cannon state
      ref_cann[ref_quad] = tmp_can;

      tmp_dir = off_dir[off_quad];
      off_dir[off_quad]  = ref_dir[ref_quad]; // trade cannon direction
      ref_dir[ref_quad]  = tmp_dir;

      tmp_suit = off_suit[off_quad];
      off_suit[off_quad] = ref_suit[ref_quad]; // trade cannon suit
      ref_suit[ref_quad] = tmp_suit;
      }
    }
  }
}

void config(struct player *p, int coords[4][2]) {
  enum suit *pos_to_suit = p->psuits;
  int *dir = p->directions;
  int *loca = p->board_loca;
  char *lost = p->lost_cann;
  int *shot[4] =  {p->round_shot[0],
                   p->round_shot[1],
                   p->round_shot[2],
                   p->round_shot[3]
                  };

  char min = 0;
  for (int i = 1; i < 4; i++) {
  min = coords[min][0] <= coords[i][0] && coords[min][1] <= coords[i][1] ? min : i;
  }

  char delta;

  for (int i = 0; i < 4; i++) {
  delta  = (coords[i][1] - coords[min][1])<<1;
  delta |= coords[i][0] - coords[min][0];
  pos_to_suit[delta] = (enum suit)i;

  shot[i][0] = 0; // magnitude
  shot[i][1] = 0; // whether we took it
  lost[i] = 0;
  dir[i] = i;
  }

  loca[0] = 18 + abs(rand()%18);
  loca[1] = 10 + abs(rand()%10);

}

int power_cycle(struct player **p, int pcount, int *pair, int down) {

  char found = 0;
  int where;

  for (int i = 0; i < pcount; i++) {
    where = intersect(p[i], pair);
    if (where != 4) {
      found = 1;
      for (int j = 0; j < 4; j++) {
      p[i]->lost_cann[j] = 0;
      }
    }
  }
  if (found) {
  pair[0] = abs(rand()%56);
  pair[1] = abs(rand()%31);
  }
return found ? 4 + (rand()&7) : down; // stored in main game loop as countdown till next powerup
}

int main(int argc, char **argv) {

srand(23);

union meld nan, inf, reg;

inf.j = 1.0 / 0.0;
nan.j = inf.j / inf.j;

reg.i = inf.i | (~nan.i)<<1 | 1;

printf("%d\n", reg.i);

struct cache c;

XI(0, "", "", 1000, 1000, 0, 0, 19);
XI(1, "", "", 0, 0, 0, 0, 0);



// MAGIC IS 31x56

  int h, w, grid;
  h = 1080 / 31;
  w = 1920 / 56;
  grid = (h + w) / 2;

  while (c.txt != 's') {
  Eve(&c, 19);
    switch (c.txt) {
      case 'c':
        //for (int i = 0; i < 3000; i++) {
        RegionFill(0, 0, 1920, 1080, RGB(0,0,0), 19);
        RegionFill(DISPLAY_OFF, 0, 1920, 1080, RGB(0,0,0), 0);
        Flush(0);
        Flush(19);
        //}
      break;
      default:
      back();
      Flush(0);
    }
  }

  int player_count = 0;

  struct player *players[12];

  int next_pos;

  int temp_player_pos[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};

  int selected = 0;

  char collides;

  char mode = 1;

  Eve(&c, 19);

  while (1) { // player chooses tank config
  pad();

    if (validate(temp_player_pos)) {
      if (player_count) {
      RegionFill(DISPLAY_OFF, 0, 1920, 1080, RGB(0,0,0), 0);

      for (int i = 0; i < 4; i++) { //suit_count
      draw_suit(0, 10 + DISPLAY_OFF + 34 * temp_player_pos[i][0], 20 + 34 * temp_player_pos[i][1], (enum suit)i);
      }

      Flush(0);

        while (c.x > 333 || c.y > 333) {
        Eve(&c, 19);
          if (c.x > 666 && c.y < 333) {
          players[player_count - 1] = malloc(sizeof(struct player));
          config(players[player_count - 1], temp_player_pos);
          player_count++;
          break;
          }
        }
      } else {
        player_count++;
      }
      rand_populate(temp_player_pos);
    }

    plist(players, player_count - 1);

    for (int i = 0; i < 4; i++) {
    draw_suit(0, 10 + DISPLAY_OFF + 34 * temp_player_pos[i][0], 20 + 34 * temp_player_pos[i][1], (enum suit)i);
    }

    back();

    Flush(0);
    Flush(19);

    Eve(&c, 19);
    RegionFill(DISPLAY_OFF, 0, 1920, 1080, RGB(0,0,0), 0);
    if (c.x > 333 && c.y < 333 && c.x < 666) {
      next_pos = temp_player_pos[selected][1] ? temp_player_pos[selected][1] - 1 : temp_player_pos[selected][1];
      collides = 0;
        for (int i = 0; i < 4; i++) {
        collides += next_pos == temp_player_pos[i][1] && temp_player_pos[selected][0] == temp_player_pos[i][0];
        }
        if (!collides) {
        temp_player_pos[selected][1] = next_pos;
        }
    } else if (c.x < 333 && c.y > 333 && c.y < 666) {
      next_pos = temp_player_pos[selected][0] ? temp_player_pos[selected][0] - 1 : temp_player_pos[selected][0];
      collides = 0;
        for (int i = 0; i < 4; i++) {
        collides += next_pos == temp_player_pos[i][0] && temp_player_pos[selected][1] == temp_player_pos[i][1];
        }
        if (!collides) {
        temp_player_pos[selected][0] = next_pos;
        }
    } else if (c.x > 666 && c.y > 333 && c.y < 666) {
      next_pos = ( temp_player_pos[selected][0] + 1 ) % 56;
      collides = 0;
        for (int i = 0; i < 4; i++) {
        collides += next_pos == temp_player_pos[i][0] && temp_player_pos[selected][1] == temp_player_pos[i][1];
        }
        if (!collides) {
        temp_player_pos[selected][0] = next_pos;
        }
    } else if (c.x > 333 && c.x < 666 && c.y > 333 && c.y < 666) {
      next_pos = ( temp_player_pos[selected][1] + 1 ) % 31;
      collides = 0;
        for (int i = 0; i < 4; i++) {
        collides += next_pos == temp_player_pos[i][1] && temp_player_pos[selected][0] == temp_player_pos[i][0];
        }
        if (!collides) {
        temp_player_pos[selected][1] = next_pos;
        }
    } else if (c.y < 333 && c.x > 666 && player_count > 2) {
      player_count--;
      break;
    } else if (c.y > 666) {
      selected = c.x / 250;
    }

  }

  int curr_player = 0;

  while(curr_player != player_count) { // player chooses board loca
    pad();
    plist(players, player_count);
    RegionFill(1700, 0, 10, 1080, RGB(0,0,0), 0);
    RegionFill(1700, curr_player * 100 + 40, 10, 10, RGB(255,255,255), 0);
    for (int i = 0; i < curr_player + 1; i++) {
      for (int j = 0; j < 4; j++) {
        draw_suit(0,
                  10 + DISPLAY_OFF + (players[i]->board_loca[0] + (j&1)) * 34,
                  20 + (players[i]->board_loca[1] + (j>>1)) * 34,
                  players[i]->psuits[j]);
      }
    }
    back();
    Flush(0);
    Flush(19);

    Eve(&c, 19);
    RegionFill(DISPLAY_OFF, 0, 1920, 1080, RGB(0,0,0), 0);
    if (c.x > 333 && c.y < 333 && c.x < 666) {
      next_pos = players[curr_player]->board_loca[1] ? players[curr_player]->board_loca[1] - 1 : 29;
      collides = 0;
        /*for (int i = 0; i < 4; i++) {
        collides += next_pos == temp_player_pos[i][1] && temp_player_pos[selected][0] == temp_player_pos[i][0];
        }
        if (!collides) {*/
        players[curr_player]->board_loca[1] = next_pos;
        //}
    } else if (c.x < 333 && c.y > 333 && c.y < 666) {
      next_pos = players[curr_player]->board_loca[0] ? players[curr_player]->board_loca[0] - 1 : 54;
      collides = 0;
        /*for (int i = 0; i < 4; i++) {
        collides += next_pos == temp_player_pos[i][0] && temp_player_pos[selected][1] == temp_player_pos[i][1];
        }
        if (!collides) {*/
        players[curr_player]->board_loca[0] = next_pos;
        //}
    } else if (c.x > 666 && c.y > 333 && c.y < 666) {
      next_pos = ( players[curr_player]->board_loca[0] + 1 ) % 55;
      collides = 0;
        /*for (int i = 0; i < 4; i++) {
        collides += next_pos == temp_player_pos[i][0] && temp_player_pos[selected][1] == temp_player_pos[i][1];
        }
        if (!collides) {*/
        players[curr_player]->board_loca[0] = next_pos;
        //}
    } else if (c.x > 333 && c.x < 666 && c.y > 333 && c.y < 666) {
      next_pos = ( players[curr_player]->board_loca[1] + 1 ) % 30;
      collides = 0;
        /*for (int i = 0; i < 4; i++) {
        collides += next_pos == temp_player_pos[i][1] && temp_player_pos[selected][0] == temp_player_pos[i][0];
        }
        if (!collides) {*/
        players[curr_player]->board_loca[1] = next_pos;
        //}
    } else if (c.y < 333 && c.x > 666) {
      curr_player++;
    }

  } // choose board loca

  RegionFill(0, 0, 1000, 1000, RGB(0,0,0), 19);
  Flush(19);

  back();
  for (int i = 0; i < player_count; i++) {
    for (int j = 0; j < 4; j++) {
      draw_suit(0,
             10 + DISPLAY_OFF + (players[i]->board_loca[0] + (j&1)) * 34,
             20 + (players[i]->board_loca[1] + (j>>1)) * 34,
             players[i]->psuits[j]);
    }
  }

  Flush(0);

  int pselect[4] = {13, 13, 13, 13};
  int **shot_store[player_count];
  int xloca, yloca;
  int where;

  int collisions[player_count * 2]; // stored as tank[i] tank[j] [2] = {i, j}
  int collision_count, icky_collisions; // Things can get pretty bad, actually

  char view;

  int power_countdown = 8;
  int power_pair[2] = {27, 15};

  while (1) { // main game loop
  curr_player = 0;
  view = 1;

    printf("powerup in: %d\n", power_countdown);

    if (!power_countdown) {
    RegionFill(10 + DISPLAY_OFF + power_pair[0] * 34 + 4, 20 + power_pair[1] * 34 + 4, 26, 26, RGB(0,255,0), 0); // new loca
    }

    while (curr_player < player_count) { // per player shot selection
    pad_ranks(players[curr_player], pselect);
    plist(players, player_count);
    RegionFill(1700, 0, 10, 1280, RGB(0,0,0), 0);
    RegionFill(1700, curr_player * 100 + 40, 10, 10, RGB(255,255,255), 0); // curr player indicate

      for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 4; j++) {
         if (players[curr_player]->lost_cann[j]) {
          RegionFill(10 + DISPLAY_OFF + (players[curr_player]->board_loca[0] + (j&1)) * 34,
                   20 + (players[curr_player]->board_loca[1] + (j>>1)) * 34,
                   34,
                   34,
                   RGB(255,255,255), 0);
          } else {
            if (view) {
            draw_suit(0,
                    10 + DISPLAY_OFF + (players[curr_player]->board_loca[0] + (j&1)) * 34,
                    20 + (players[curr_player]->board_loca[1] + (j>>1)) * 34,
                    players[curr_player]->psuits[j]);
            } else {
            draw_dir(0,
                    10 + DISPLAY_OFF + (players[curr_player]->board_loca[0] + (j&1)) * 34,
                    20 + (players[curr_player]->board_loca[1] + (j>>1)) * 34,
                    players[curr_player]->directions[j]);
            }
          }
        }
      }

    Flush(19);
    Eve(&c, 19);
    RegionFill(0, 0, 1000, 1000, RGB(0,0,0), 19);

      if (c.y < 800 && c.y > 600 && c.x < 500) {
        for (int i = 0; i < 4; i++) {
        pselect[i] = 13;
        }
      curr_player++;
      } else if (c.y < 800 && c.y > 600 && c.x > 500) {
      view = !view;
      } else if (c.y > 800 && c.x > 500) {
      curr_player = (curr_player + 1)%player_count;
        for (int i = 0; i < 4; i++) {
        pselect[i] = players[curr_player]->round_shot[i][1] ? players[curr_player]->round_shot[i][0] - 1 : 13;
        }
      } else if (c.y > 800 && c.x < 500) {
      curr_player = curr_player ? curr_player - 1 : player_count - 1;
        for (int i = 0; i < 4; i++) {
        pselect[i] = players[curr_player]->round_shot[i][1] ? players[curr_player]->round_shot[i][0] - 1 : 13;
        }
      } else if (c.y < 600) {
      xloca = c.x / 68;
      yloca = c.y / 150;
      pselect[yloca] = xloca;
        if (players[curr_player]->round_shot[yloca][1] && players[curr_player]->round_shot[yloca][0] == xloca + 1)  {
        players[curr_player]->round_shot[yloca][1] = 0;
        pselect[yloca] = 13;
        } else {
        players[curr_player]->round_shot[yloca][1] = 1;
        }
        players[curr_player]->round_shot[yloca][0] = xloca + 1;
      } 
    } // shot selection

    // Begin logic for processing kills and whatnot

    for (int i = 0; i < player_count; i++) {
      shot_store[i] = fire(players[i]);
    }

    // collision handling
    track_collisions(players, collisions, player_count, &collision_count, &icky_collisions);
    icky_collisions = collision_count; // count all collisions as major going into loop

    while (icky_collisions) { // while major collisions are nonzero
      fix_collisions(players, collisions, player_count, collision_count);
      track_collisions(players, collisions, player_count, &collision_count, &icky_collisions);
      icky_collisions = collision_count - icky_collisions; // icky_collisions was prev minor_collisions
    }

    if (!power_countdown) {
    RegionFill(10 + DISPLAY_OFF + power_pair[0] * 34 + 4, 20 + power_pair[1] * 34 + 4, 26, 26, RGB(0,0,0), 0); // erase old loca
    Flush(0);
    power_pair[0] = (power_pair[0] + 1)%56;
    } else {
    power_countdown--;
    }

    power_countdown = power_cycle(players, player_count, power_pair, power_countdown);

    back(); // this will not be used aggresively so as to not hurt my FUCKING EYES
    for (float t = 1.0; t >= 0; t -= 0.01) {   // animate deltas
      for (int i = 0; i < player_count; i++) {
        xloca = (t + 0.01) * (34 * players[i]->pvect_loca[0]) + players[i]->board_loca[0] * 34;
        yloca = (t + 0.01) * (34 * players[i]->pvect_loca[1]) + players[i]->board_loca[1] * 34;
        xloca = 34 * (xloca / 34);
        yloca = 34 * (yloca / 34); // calculate animation frame starting coord

        RegionFill(
        xloca + 10 + DISPLAY_OFF,
        yloca + 20, 102, 102, RGB(0,0,0), 0
        );

        for (int j = 0; j < 3; j++) { // compensatory grid redraw
          for (int k = 0; k < 3; k++) { // Warning: this will redraw the torus boundaries
            RegionScarf(
            1 + xloca + j * 34 + 10 + DISPLAY_OFF,
            1 + yloca + k * 34 + 20, 34, 34, RGB(255,255,255), 0
            );
          }
        }

        // finally draw players
        for (int k = 0; k < 4; k++) {
          for (int j = 0; j < 4; j++) {
            if (players[i]->lost_cann[j]) {
              RegionFill(
                   t * (34 * players[i]->pvect_loca[0]) + 10 + DISPLAY_OFF + (players[i]->board_loca[0] + (j&1)) * 34,
                   t * (34 * players[i]->pvect_loca[1]) + 20 + (players[i]->board_loca[1] + (j>>1)) * 34,
                   34,
                   34,
                   RGB(255,255,255), 0);
            } else {
              draw_suit(0,
                   t * (34 * players[i]->pvect_loca[0]) + 10 + DISPLAY_OFF + (players[i]->board_loca[0] + (j&1)) * 34,
                   t * (34 * players[i]->pvect_loca[1]) + 20 + (players[i]->board_loca[1] + (j>>1)) * 34,
                   players[i]->psuits[j]);
            }
          }
        }
      }
    Flush(0);
    } // animate deltas

    // This is stupid and a more efficient way definitely exists 
    for (int i = 0; i < player_count; i++) {
      for (int j = 0; j < player_count; j++) {
      // find fire() intersection with other players
        for (int k = 0; k < 4; k++) {
        where = intersect(players[i], shot_store[j][k]);
          if (where != 4) {
          printf("hit: %d %d\n", shot_store[j][k][0], shot_store[j][k][1]);
          players[i]->lost_cann[where] = 1;
          }
        }
      }
    }

  } // main game loop
} // int main
