#include <ssw.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "xbm/suits.xbm"
#include "xbm/ranks.xbm"
#include "xbm/directions.xbm"
#include "queue.h"

/*
* Find ELAN from `xinput` and corresponding ID
*
* xinput map-to-output 19 eDP-1
*
*/

/*
 * Update: Feb 17th 2025
 * Record: first game played amongst six people
 * Bugs discovered: 
 *
 *  1). strictly vertical collisions at 0,0 such that two tanks overlap in two quadrants
 *  results (possibly and probably) in the offending tank being propelled off the map
 *
 *  2). Visual bugs 
 *  Rectify by drawing tanks not by ordering of [0-player_count] but by tanks with missing
 *  cannons first and tanks with all cannons second
 *
 * Thoughts on main gameplay loop:
 *  Ultimate concerns:
 *    1). damage being too severe to keep playing (one cannon gone = feels like half movement is gone)
 *    2). pacing in terms of board size and number of possible moves a player can make
 *
 *  Possible alleviations:
 *    1). recoil still viable but missing cannons do not fire projectiles(*1)
 *    2). more stuff could happen per board effectuation (board update/round)
 * 
 * ~~?Spreadshot?~~
 *
 *   Definite alleviations:
 *    1). Just throw more cards at it or sum shi-
 *    2). closing map: implemented by start_x, start_y, end_x, end_y
 *        start_x++, start_y++, end_x--, end_y-- every 2 rounds
 *        initial values:
 *          (start_x, start_y) = (0, 0) // these will be underhead ( loca_z - start_z <= 0 ? end_z - 1 : loca_z - 1 ) 
 *          (end_x, end_y) = (31, 55)   // these will be mod overhead ( (loca_z = loca_z + 1)%(end_z - 1) )
 *
 * (*1): projectiles should have ANIMATION
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

long long color;
};

union meld {
float j;
int   i;
};

void prioritize(struct player **players, int pcount, int *prio_arr) {
  int magnitudes[pcount];
  int prioplayer_map[pcount];
  int insert, j;

  for (int i = 0; i < pcount; i++) {
  insert = 0;
    for (j = 0; j < 4; j++) {
    insert += players[i]->round_shot[j][1] ? players[i]->round_shot[j][0] : 0;
    }

    if (!i) {
    j = 0;
    } else {
      for (j = i; magnitudes[j - 1] < insert && j > 0; j--) { // insertion sort: will sort in descending order
      magnitudes[j] = magnitudes[j - 1];
      prioplayer_map[j] = prioplayer_map[j - 1];
      }
    }

    magnitudes[j] = insert;
    prioplayer_map[j] = i;
  }

  for (int i = 0; i < pcount; i++) {
  prio_arr[prioplayer_map[i]] = i;
  }

}

#define DISPLAY_OFF 1920

void draw_suit(int d, int x, int y, enum suit which, int scale) { // take color param

  switch (which) {

    case C:
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 32; j++) {
      RegionFill(i * scale + x, j * scale + y, scale, scale, !( ( suit0_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;
    case S:
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 32; j++) {
      RegionFill(i * scale + x, j * scale + y, scale, scale, !( ( suit1_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;

    case H:
     for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 32; j++) {
      RegionFill(i * scale + x, j * scale + y, scale, scale, !( ( suit2_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
      }
    }
    break;
    case D:
     for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 32; j++) {
      RegionFill(i * scale + x, j * scale + y, scale, scale, !( ( suit3_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
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

void draw_dir(int d, int x, int y, int dir, int scale) {

int i_offx, i_offy, offx, offy;
char cond = (dir&1)^(dir>>1);

  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 32; j++) {
    i_offx = dir_x(i, dir);
    i_offy = j;
    offx = cond ? i_offx : i_offy;
    offy = cond ? i_offy : 31 - i_offx;
    RegionFill(x + scale * offx, y + scale * offy, scale, scale, !( ( dir_bits[j * 4 + i / 8]>>(i&7) )&1 ) ? RGB(255,255,255) : RGB(0,0,0), d);
    }
  }

}

void back(int d, int *dim) { // I **NEVER** want to touch this **EVER** again
  int width, height;
  width =  dim[2] - dim[0];
  height = dim[3] - dim[1];

  char which = width < height;

  int min, max, depth;
  min = which ? width : height;
  max = which ? height : width;
  depth = max%min;

  int x_depth, y_depth, x_turn, y_turn;
  x_depth = which ? 0 : depth;
  y_depth = which ? depth : 0;
  x_turn =  which ? min : max;
  y_turn =  which ? max : min;

    for (int i = 0; i < min; i++) {
      RegionScarf(1 + dim[0] * 34 + 10 + DISPLAY_OFF, 1 + dim[1] * 34 + 20, (i + 1) * 34, (i + 1) * 34, RGB(255,255,255), d);
      RegionScarf(10 + DISPLAY_OFF + 1 + (min - i + dim[0]) * 34, 20 + 1 + (min - i + dim[1]) * 34, i * 34, i * 34, RGB(255,255,255), d);
    }

  for (int z = 0; z < max/min; z++) {
    for (int i = 0; i < min; i++) {
      RegionScarf(dim[0] * 34 + (!which) * (z * min) * 34 + x_depth * 34 + 1 + 10 + DISPLAY_OFF, dim[1] * 34 + which * (z * min) * 34 + y_depth * 34 + 1 + 20, (i + 1) * 34, (i + 1) * 34, RGB(255,255,255), d);
      RegionScarf(dim[0] * 34 + !which * (z * min) * 34 + 10 + DISPLAY_OFF + 1 + (min - i + x_depth) * 34, dim[1] * 34 + which * (z * min) * 34 + 20 + 1 + (min - i + y_depth) * 34, i * 34, i * 34, RGB(255,255,255), d);
    }
  }

}

void cover(int *where) {
RegionFill(10 + DISPLAY_OFF + 34 * where[0], 20 + 34 * where[1], 34, 34, RGB(0,0,0), 0);
RegionScarf(10 + DISPLAY_OFF + 34 * where[0] + 1, 20 + 34 * where[1] + 1, 34, 34, RGB(255,255,255), 0);
}

int dir_map(int x) {
x+=2;
return ((x>>2) | x&3);
}

void pad(void) {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
    RegionScarf(i * 333 + 1, j * 333 + 1, 333, 333, RGB(255,255,255), 19);
      if (j == 1 || ((i == 1) && (j==0))) {
      draw_dir(19, i * 333 + 6, 333 * j + 6, j ? dir_map(i) : j, 10);
      }
    }
  }
  for (int i = 0; i < 4; i++) {
    RegionScarf(i * 250 + 1, 666, 250, 250, RGB(255, 255, 255), 19);
    draw_suit(19, i * 250 + 26, 666 + 26, (enum suit)i, 6);
  }

}

void pad_color(long long color, int range) {
  for (int i = 0; i < 64; i++) {
    RegionFill(788, 350 + i * 8, 150, 8, RGB(0, 0, i * 4), 19);
    for (int j = 0; j < 64; j++) {
    RegionFill(i * 12, j * 12, 12, 12, RGB(i * 4, j * 4, range), 19);
    }
  }
  RegionFill(788, 20, 150, 300, color, 19);
}

void pad_ranks(struct player *p, int select[4]) {
  for (int i = 0; i < 13; i++) {
    for (int j = 0; j < 4; j++) {
    RegionScarf(i * 68 + 1, j * 150 + 1, 68, 150, RGB(255,255,255), 19);
    }
  }

  RegionScarf(0, 601, 499, 190, RGB(255,255,255), 19); // continue
  RegionScarf(0, 601, 990, 190, RGB(255,255,255), 19); // dir?
  RegionScarf(0, 800, 499, 190, RGB(255,255,255), 19); // up 
  RegionScarf(0, 800, 990, 190, RGB(255,255,255), 19); // down


  for (int i = 0; i < 4; i++) {
    if (select[i] < 13) {
      RegionScarf(select[i] * 68 + 10, i * 150 + 10, 48, 130, RGB(255,255,255), 19);
    }

    draw_dir(19, 13 * 68 + 33, i * 150 + 58, p->directions[i], 1);
  }

  for (int i = 0; i < 13; i++) {
    for (int j = 0; j < 4; j++) {
      draw_rank(19, i * 68 + 17, j * 150 + 51, (enum rank)i);
      draw_suit(19, i * 68, j * 150, p->psuits[j], 1);

      if (p->lost_cann[j]) {
      RegionFill(i * 68 + 34, j * 150 + 116, 34, 34, RGB(255,255,255), 19);
      } else {
      draw_suit(19, i * 68 + 34, j * 150 + 116, p->psuits[j], 1);
      }
    }
  }

}


void plist(struct player **multi, int num) {
  for (int i = 0; i < num; i++) {
  RegionScarf(1800, 1 + i * 100, 100, 100, RGB(255,255,255), 0);
    for (int j = 0; j < 4; j++) {
      if (multi[i]->lost_cann[j]) {
      RegionFill(1810 + 34 * (j&1) - 1, 10 + i * 100 + 34 * (j>>1) - 1, 34, 34, RGB(255,255,255), 0);
      RegionFill(1810 + 34 * (j&1) + 5, 10 + i * 100 + 34 * (j>>1) + 5, 23, 23, RGB(0,0,0), 0);
      } else {
      draw_dir(0, 1720 + 34 * (j&1), 10 + i * 100 + 34 * (j>>1), multi[i]->directions[j], 1);
      draw_suit(0, 1810 + 34 * (j&1), 10 + i * 100 + 34 * (j>>1), (enum suit)multi[i]->psuits[j], 1);
      }
    }
  }
}

int abs(int x) {
return x < 0 ? -x : x;
}

int sgn(int x) {
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

  int **saved_shots = malloc(sizeof(int *) * 4); // free this at some point
  int *landing;

  float dx, dy; // for recoil
  float reduct;

  for (int i = 0; i < 4; i++) { // projectile landings
    landing = malloc(sizeof(int) * 6);
    landing[0] = loca[0] + (i&1);
    landing[1] = loca[1] + (i>>1);
    landing[2] = 0;
    landing[3] = 0;
    landing[4] = landing[0];
    landing[5] = landing[1]; // tacked on for animation
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
    landing[2] = (landing[0] + landing[4])/2;
    landing[3] = (landing[1] + landing[5])/2 - abs(landing[5] - landing[1]);
    saved_shots[i] = landing;
  } // projectile landings

  // BEGIN RECOIL
  prev[0] = loca[0]; // for use during animation
  prev[1] = loca[1];

  dx = 0;
  dy = 0;

  for (int i = 0; i < 4; i++) {
    if (shots[i][1]) {
      reduct = no_cann[i] ? 1.0 : 2.0; // if no cannon, double recoil
      switch (dirs[i]) {
        case 0: // increase y
        dy += (float)shots[i][0] / reduct;
        break;
        case 1: // decrease x
        dx += -(float)shots[i][0] / reduct;
        break;
        case 2: // increase x
        dx += (float)shots[i][0] / reduct;
        break;
        case 3: // decrease y
        dy += -(float)shots[i][0] / reduct;
        break;
      }
    }
    shots[i][1] = 0;
  }

  loca[0] = (int)((loca[0] + dx < 0 ? 55 : 0) + loca[0] + dx) % 55; // 55 = 56 - 1
  loca[1] = (int)((loca[1] + dy < 0 ? 30 : 0) + loca[1] + dy) % 30; // because of players being big

  prev[0] = prev[0] - loca[0];
  prev[1] = prev[1] - loca[1];
  // END RECOIL

return saved_shots;

}

void track_fix_collisions(struct player **p, int pcount, struct queue *q, int *priority) {
// TODO: set prev loca to curr loca
char use_quad;
int *collision;
int mag_i, mag_j;
int refer, offend;
int x_diff, y_diff;
int x_shift, y_shift;
int coll_count, minor_collisions, major_collisions;
int ref_quad, off_quad;

struct priority_queue collisions;
collisions.size = 0;
collisions.data = malloc(sizeof(int *) * pcount);
collisions.prio = malloc(sizeof(int) * pcount);

  for (int i = 0; i < pcount - 1; i++) {
    for (int j = i + 1; j < pcount; j++) {
      x_diff = abs(p[i]->board_loca[0] - p[j]->board_loca[0]);
      y_diff = abs(p[i]->board_loca[1] - p[j]->board_loca[1]);
      mag_i = abs(p[i]->pvect_loca[0]) + abs(p[i]->pvect_loca[1]); // no square root needed
      mag_j = abs(p[j]->pvect_loca[0]) + abs(p[j]->pvect_loca[1]);

      refer  = mag_i < mag_j ? i : j;
      offend = mag_i < mag_j ? j : i;

      if (x_diff < 2 && y_diff < 2) {
      collision = malloc(sizeof(int) * 4); // [2]&[3] unused for later
      collision[0] = refer;
      collision[1] = offend;
      insert(&collisions, priority[offend], collision);
      }
    }
  }

  while (collisions.size) { // investigate the nine cases of p[ref] <> p[offend]
  collision = top(&collisions);
  refer  = collision[0];
  offend = collision[1];
  x_diff = p[offend]->board_loca[0] - p[refer]->board_loca[0];
  y_diff = p[offend]->board_loca[1] - p[refer]->board_loca[1];

  x_shift = boil(p[offend]->pvect_loca[0]) * -1;
  y_shift = boil(p[offend]->pvect_loca[1]) * -1;

  p[offend]->pvect_loca[0] = p[offend]->board_loca[0];
  p[offend]->pvect_loca[1] = p[offend]->board_loca[1];

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

    p[offend]->pvect_loca[0] = p[offend]->pvect_loca[0] - p[offend]->board_loca[0];
    p[offend]->pvect_loca[1] = p[offend]->pvect_loca[1] - p[offend]->board_loca[1];

    collision[2] = ref_quad;
    collision[3] = off_quad;

    if (use_quad && !contains(q, collision)) {
    enqueue(q, collision);
    } else {
    free(collision);
    }
  } // end collision inspection

}

void trade(struct player **p, struct queue *q) {
int *trade_cond;
int ref_quad, off_quad, refer, offend;

char *ref_cann, *off_cann;
int *ref_dir, *off_dir;
enum suit *ref_suit, *off_suit;

int tmp_dir, tmp_can;
enum suit tmp_suit;

  while (qsize(q)) {

  trade_cond = dequeue(q);

    printf("collision looks like:\n");

    printf("%d %d %d %d\n", trade_cond[0], trade_cond[1], trade_cond[2], trade_cond[3]);

  refer = trade_cond[0];
  offend = trade_cond[1];
  ref_quad = trade_cond[2];
  off_quad = trade_cond[3];

  ref_cann = p[refer]->lost_cann;
  off_cann = p[offend]->lost_cann;
  ref_dir  = p[refer]->directions;
  off_dir  = p[offend]->directions;
  ref_suit = p[refer]->psuits;
  off_suit = p[offend]->psuits;

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

void reset_pvect(struct player **p, int pcount) {
  for (int i = 0; i < pcount; i++) {
  p[i]->pvect_loca[0] = 0;
  p[i]->pvect_loca[1] = 0;
  }
}

void config(struct player *p, int coords[4][2], long long scolor) {
  p->color = scolor;
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
    if (where != 4 && !down) {
      found = 1;
      for (int j = 0; j < 4; j++) {
      p[i]->lost_cann[j] = 0;
      }
    }
  }
  if (found) { // check for intersect
  pair[0] = abs(rand()%56);
  pair[1] = abs(rand()%31);
  }
return found ? 4 + (rand()&7) : down; // stored in main game loop as countdown till next powerup
}

void animate(struct player **players, int player_count) {
int xloca, yloca;

  for (float t = 1.0; t >= 0; t -= 0.01) {   // animate deltas
    for (int i = 0; i < player_count; i++) {
      xloca = (t + 0.01) * (34 * players[i]->pvect_loca[0]) + players[i]->board_loca[0] * 34;
      yloca = (t + 0.01) * (34 * players[i]->pvect_loca[1]) + players[i]->board_loca[1] * 34;
      xloca = 34 * (xloca / 34);
      yloca = 34 * (yloca / 34); // calculate animation frame starting coord

      RegionFill(
      xloca + 10 + DISPLAY_OFF - 34,
      yloca + 20 - 34, 136, 136, RGB(0,0,0), 0
      );

      for (int j = 0; j < 4; j++) { // TODO: replace with back(custom) once its ready
        for (int k = 0; k < 4; k++) {
          RegionScarf(
          1 + xloca + j * 34 + 10 + DISPLAY_OFF - 34,
          1 + yloca + k * 34 + 20 - 34, 34, 34, RGB(255,255,255), 0
          );
        }
      }

      RegionScarf(8 + 10 + DISPLAY_OFF + 34 * (players[i]->pvect_loca[0] + players[i]->board_loca[0]),
                  8 + 20 + 34 * (players[i]->pvect_loca[1] + players[i]->board_loca[1]), 53, 53, players[i]->color, 0); // previous loca

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
                   players[i]->psuits[j], 1);
          }
        }
      }
    }
  Flush(0);
  } // animate deltas
}

int DIM[4] = {0, 0, 56, 31};


void *ping(void *input) {
  void **retrieve = (void**)input;
  int **cond = (int **)retrieve;
  struct player **players = (struct player **)cond[1];
  int *pcount = (int *)cond[2];
  int *dim = (int *)(cond[3]);
  XI(1, "", "", 0, 0, 0, 0, 2);
  back(2, dim);

  // redraw players
    for (int i = 0; i < *pcount; i++) {
      for (int z = 0; z < 10; z++) { 
        for (int j = 0; j < 4; j++) {
          if (players[i]->lost_cann[j]) {
          RegionFill(10 + DISPLAY_OFF + (players[i]->board_loca[0] + (j&1)) * 34,
                   20 + (players[i]->board_loca[1] + (j>>1)) * 34,
                   34,
                   34,
                   RGB(255,255,255), 2);
          Flush(0);
          } else {
          draw_suit(2,
                    10 + DISPLAY_OFF + (players[i]->board_loca[0] + (j&1)) * 34,
                    20 + (players[i]->board_loca[1] + (j>>1)) * 34,
                    players[i]->psuits[j], 1);
          }
        }
      }
    }

  while(!cond[0][0]) {
      for (int j = 0; j < 10; j++) {
        for (int i = 0; i < *pcount; i++) {
        RegionScarf(DISPLAY_OFF + 10 + 34 * players[i]->board_loca[0] - j, 20 + 34 * players[i]->board_loca[1] - j, 68 + j * 2, 68 + j * 2, players[i]->color, 2);
        usleep(20000);
        Flush(2);
        }
      }
      for (int j = 0; j < 10; j++) {
        for (int i = 0; i < *pcount; i++) {
        RegionScarf(DISPLAY_OFF + 10 + 34 * players[i]->board_loca[0] - j, 20 + 34 * players[i]->board_loca[1] - j, 68 + j * 2, 68 + j * 2, RGB(0,0,0), 2);
        usleep(20000);
        Flush(2);
        }
      }
  }
}

pthread_t animthread;
void *bundle[4]; // void **
int killthread;

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

  int player_count = 0;

  struct player *players[12];

  int next_pos;

  int temp_player_pos[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};

  int selected = 0;

  char collides;

  char mode = 1;

  Eve(&c, 19);

  RegionFill(DISPLAY_OFF, 0, 1920, 1080, RGB(0,0,0), 0);
  RegionFill(0, 0, 1000, 1000, RGB(0,0,0), 19);
  back(0, DIM);
  pad();
  Flush(0);
  Flush(19);

  long long scolor = 255;
  int scolor_range = 0;

  while (1) { // player chooses tank config
    RegionFill(0, 0, 1000, 1000, RGB(0,0,0), 19);
    pad();

    if (validate(temp_player_pos)) {
      if (player_count) {
      RegionFill(DISPLAY_OFF, 0, 1920, 1080, RGB(0,0,0), 0);
      back(0, DIM);

        for (int i = 0; i < 4; i++) { //suit_count
        draw_suit(0, 10 + DISPLAY_OFF + 34 * temp_player_pos[i][0], 20 + 34 * temp_player_pos[i][1], (enum suit)i, 1);
        }

      Flush(0);

        RegionFill(0, 0, 1000, 1000, RGB(0,0,0), 19);
        RegionScarf(768, 0, 200, 333, RGB(255,255,255), 19);
        RegionScarf(768, 333, 200, 600, RGB(255,255,255), 19);

        while (c.x > 333 || c.y < 768) {
        pad_color(scolor, scolor_range);
        Flush(19);
        Eve(&c, 19);
          if (c.x > 768 && c.y < 333) {
          players[player_count - 1] = malloc(sizeof(struct player));
          config(players[player_count - 1], temp_player_pos, scolor);
          player_count++;
          break;
          } else if (c.x < 768 && c.y < 768) {
          scolor = RGB(c.x/3, c.y/3, scolor_range);
          } else if (c.x > 768 && c.y > 333 && c.y < 768) {
          scolor_range = ( c.y - 333 )/ 2;
          }
        }

      RegionFill(DISPLAY_OFF, 0, 1920, 1080, RGB(0,0,0), 0);
      back(0, DIM);
      // TO FIX PERSISTENT TANK GRAPHIC BUG

      } else {
        player_count++;
      }
      rand_populate(temp_player_pos);
    }

    RegionFill(0, 0, 1000, 1000, RGB(0,0,0), 19);
    pad();

    plist(players, player_count - 1);

    for (int i = 0; i < 4; i++) {
    draw_suit(0, 10 + DISPLAY_OFF + 34 * temp_player_pos[i][0], 20 + 34 * temp_player_pos[i][1], (enum suit)i, 1);
    }

// back();

    Flush(0);
    Flush(19);

    Eve(&c, 19);

// fill whole

    if (c.x > 333 && c.y < 333 && c.x < 666) {
      next_pos = temp_player_pos[selected][1] ? temp_player_pos[selected][1] - 1 : temp_player_pos[selected][1];
      collides = 0;
        for (int i = 0; i < 4; i++) {
        collides += next_pos == temp_player_pos[i][1] && temp_player_pos[selected][0] == temp_player_pos[i][0];
        }
        if (!collides) {
        cover(temp_player_pos[selected]);
        temp_player_pos[selected][1] = next_pos;
        }
    } else if (c.x < 333 && c.y > 333 && c.y < 666) {
      next_pos = temp_player_pos[selected][0] ? temp_player_pos[selected][0] - 1 : temp_player_pos[selected][0];
      collides = 0;
        for (int i = 0; i < 4; i++) {
        collides += next_pos == temp_player_pos[i][0] && temp_player_pos[selected][1] == temp_player_pos[i][1];
        }
        if (!collides) {
        cover(temp_player_pos[selected]);
        temp_player_pos[selected][0] = next_pos;
        }
    } else if (c.x > 666 && c.y > 333 && c.y < 666) {
      next_pos = ( temp_player_pos[selected][0] + 1 ) % 56;
      collides = 0;
        for (int i = 0; i < 4; i++) {
        collides += next_pos == temp_player_pos[i][0] && temp_player_pos[selected][1] == temp_player_pos[i][1];
        }
        if (!collides) {
        cover(temp_player_pos[selected]);
        temp_player_pos[selected][0] = next_pos;
        }
    } else if (c.x > 333 && c.x < 666 && c.y > 333 && c.y < 666) {
      next_pos = ( temp_player_pos[selected][1] + 1 ) % 31;
      collides = 0;
        for (int i = 0; i < 4; i++) {
        collides += next_pos == temp_player_pos[i][1] && temp_player_pos[selected][0] == temp_player_pos[i][0];
        }
        if (!collides) {
        cover(temp_player_pos[selected]);
        temp_player_pos[selected][1] = next_pos;
        }
    } else if (c.y < 333 && c.x > 666 && player_count > 2) {
      player_count--;
      break;
    } else if (c.y > 666) {
      selected = c.x / 250;
    }

  } // end config



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
                  players[i]->psuits[j], 1);
      }
    }
    back(0, DIM);
    Flush(0);
    Flush(19);

    Eve(&c, 19);
    RegionFill(DISPLAY_OFF, 0, 1920, 1080, RGB(0,0,0), 0);
    if (c.x > 333 && c.y < 333 && c.x < 666) {
      next_pos = players[curr_player]->board_loca[1] ? players[curr_player]->board_loca[1] - 1 : 29;
      // TODO: ADD track_collisions(); here
      collides = 0;
        //if (!collides) {
        players[curr_player]->board_loca[1] = next_pos;
        //}
    } else if (c.x < 333 && c.y > 333 && c.y < 666) {
      next_pos = players[curr_player]->board_loca[0] ? players[curr_player]->board_loca[0] - 1 : 54;
      collides = 0;
        //if (!collides) {
        players[curr_player]->board_loca[0] = next_pos;
        //}
    } else if (c.x > 666 && c.y > 333 && c.y < 666) {
      next_pos = ( players[curr_player]->board_loca[0] + 1 ) % 55;
      collides = 0;
        //if (!collides) {
        players[curr_player]->board_loca[0] = next_pos;
        //}
    } else if (c.x > 333 && c.x < 666 && c.y > 333 && c.y < 666) {
      next_pos = ( players[curr_player]->board_loca[1] + 1 ) % 30;
      collides = 0;
        //if (!collides) {
        players[curr_player]->board_loca[1] = next_pos;
        //}
    } else if (c.y < 333 && c.x > 666) {
      curr_player++;
    }

  } // choose board loca

  RegionFill(0, 0, 1000, 1000, RGB(0,0,0), 19);
  Flush(19);

  back(0, DIM);
  for (int i = 0; i < player_count; i++) {
    for (int j = 0; j < 4; j++) {
      draw_suit(0,
             10 + DISPLAY_OFF + (players[i]->board_loca[0] + (j&1)) * 34,
             20 + (players[i]->board_loca[1] + (j>>1)) * 34,
             players[i]->psuits[j], 1);
    }
  }

  Flush(0);

  int player_index[player_count];

  int pselect[4] = {13, 13, 13, 13};
  int **shot_store[player_count];
  int xloca, yloca;
  int where;

  int *who[player_count * player_count];
  struct queue collisions;
  collisions.capacity = player_count * player_count; // TODO: fix queue and add contains routine
  collisions.rear = 0;
  collisions.front = 0;
  collisions.data = who;

  int prev_coll, coll;

  char view;

  int power_countdown = 8;
  int power_pair[2] = {27, 15};

  int board_dim[3][4] = {{0, 0, 56, 31}, {0, 0, 0, 0}, {0, 0, 0, 0}};

  bundle[0] = (void *)&killthread;
  bundle[1] = (void *)players;
  bundle[2] = (void *)&player_count;
  bundle[3] = (void *)board_dim;

  while (1) { // main game loop ----------------------------------------------------------------------------
  curr_player = 0;
  view = 1;

    if (1) {
    ;
    } // board too small? kill everyone!

    printf("powerup in: %d\n", power_countdown);

    RegionFill(0, 0, WW(0), WH(0), RGB(0,0,0), 0);
    back(0, board_dim[0]);

    if (!power_countdown) {
    RegionFill(10 + DISPLAY_OFF + power_pair[0] * 34 + 4, 20 + power_pair[1] * 34 + 4, 26, 26, RGB(0,255,0), 0); // new loca
    }


    for (int i = 0; i < player_count; i++) {
      for (int z = 0; z < 10; z++) { 
        for (int j = 0; j < 4; j++) {
          if (players[i]->lost_cann[j]) {
          RegionFill(10 + DISPLAY_OFF + (players[i]->board_loca[0] + (j&1)) * 34,
                   20 + (players[i]->board_loca[1] + (j>>1)) * 34,
                   34,
                   34,
                   RGB(255,255,255), 0);
          Flush(0);
          } else {
          draw_suit(0,
                    10 + DISPLAY_OFF + (players[i]->board_loca[0] + (j&1)) * 34,
                    20 + (players[i]->board_loca[1] + (j>>1)) * 34,
                    players[i]->psuits[j], 1);
          }
        }
      }
    }

    killthread = 0;
    pthread_create(&animthread, 0, ping, (void *)bundle);
    while (curr_player != player_count) { // player shot selection
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
                    players[curr_player]->psuits[j],  1);
            } else {
            draw_dir(0,
                    10 + DISPLAY_OFF + (players[curr_player]->board_loca[0] + (j&1)) * 34,
                    20 + (players[curr_player]->board_loca[1] + (j>>1)) * 34,
                    players[curr_player]->directions[j], 1);
            }
          }
        }
      }

    Flush(19);
    Eve(&c, 19);
    RegionFill(0, 0, 1000, 1000, RGB(0,0,0), 19);

      if (c.y < 800 && c.y > 600 && c.x < 500) {
      curr_player++;
        if (curr_player != player_count) {
          for (int i = 0; i < 4; i++) {
          pselect[i] = players[curr_player]->round_shot[i][1] ? players[curr_player]->round_shot[i][0] - 1 : 13;
          }
        } else {
          for (int i = 0; i < 4; i++) {
          pselect[i] = 13;
          }
        }
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
    killthread =  1;
    pthread_join(animthread, 0);


    prioritize(players, player_count, player_index);

    // Begin logic for processing kills and whatnot

    for (int i = 0; i < player_count; i++) {
      shot_store[i] = fire(players[i]);
    }

    animate(players, player_count);

    // collision handling
    prev_coll = 0;
    track_fix_collisions(players, player_count, &collisions, player_index);
    coll = qsize(&collisions);
    reset_pvect(players, player_count);
    animate(players, player_count);

    while (coll != prev_coll) { // no new collisions
      prev_coll = coll;
      track_fix_collisions(players, player_count, &collisions, player_index);
      reset_pvect(players, player_count);
      animate(players, player_count);
      coll = qsize(&collisions);
    } // TODO: intermediate animations


    trade(players, &collisions); // no player index: order is based on queue

    printf("after trade:\n");
    for (int z = 0; z < player_count; z++) {
    printf("%d %d %d %d\n", players[z]->lost_cann[0], players[z]->lost_cann[1], players[z]->lost_cann[2], players[z]->lost_cann[3]);
    }

    if (!power_countdown) {
    RegionFill(10 + DISPLAY_OFF + power_pair[0] * 34 + 4, 20 + power_pair[1] * 34 + 4, 26, 26, RGB(0,0,0), 0); // erase old loca
    Flush(0);
    power_pair[0] = (power_pair[0] + 1)%(board_dim[0][2] + 1);
    } else {
    power_countdown--;
    }

    board_dim[0][0]++;
    board_dim[0][1]++;
    board_dim[0][2]--;
    board_dim[0][3]--;

    power_countdown = power_cycle(players, player_count, power_pair, power_countdown);

    // for all players store their loca into prev
    // adjust based on borders
    // do fix if collision number has increased

    // borders should shrink before shots land or are even animated

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

// animate projectiles

  } // main game loop
} // int main
