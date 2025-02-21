#include <stdio.h>
#include <stdlib.h>

// BEGIN generic queue
// generic, BUT contains requires data size to be 4 each
struct queue {
int front, rear, capacity;
int **data;
}; // (rear - front + capacity) % capacity

int qsize(struct queue *q) {
return ( (q->rear - q->front + q->capacity) % q->capacity );
}

void enqueue(struct queue *q, int *collision) {
q->data[q->rear] = collision; // [ref, off, qref, qoff]
q->rear = (q->rear + 1)%q->capacity;
}

int *dequeue(struct queue *q) {
int *ret;
  if (q->rear==q->front) {
  printf("You done goofed...\n");
  }
ret = q->data[q->front];
q->front = (q->front + 1) % q->capacity;
return ret;
}

char contains(struct queue *q, int *collision) {
  int i = 0;
  char found = 0;
  char eql;

  i = q->front;
  while (i!=q->rear) {
  eql = 1;
    for (int j = 0; j < 4; j++) {
      eql &= q->data[i][j] == collision[j];
    }
  found |= eql;
  i = (i + 1)%q->capacity;
  }

  return found;
}
// END generic queue

struct priority_queue {
int *prio;
int **data;
int capacity, size;
};

int *top(struct priority_queue *q) { // destroy priority on way out
  if (q->size==0) {
  return 0;
  }

q->size--;

int *ret = q->data[0];
q->prio[0] = q->prio[q->size];
q->data[0] = q->data[q->size];

int *tdata, tprio;

  // bubble down

  int parent_index = 0;
  int child_index;

  while (parent_index * 2 + 2 < q->size) {
    if (q->prio[parent_index] > q->prio[parent_index * 2 + 1]) {
    child_index = parent_index * 2 + 1;
    // bubble left
    } else if (q->prio[parent_index] > q->prio[parent_index * 2 + 2]) {
    child_index = parent_index * 2 + 2;
    // bubble right
    } else {
    break;
    }

  tprio = q->prio[parent_index];
  tdata = q->data[parent_index];
  q->prio[parent_index] = q->prio[child_index];
  q->data[parent_index] = q->data[child_index];
  q->prio[child_index] = tprio;
  q->data[child_index] = tdata;

  parent_index = child_index;

  }
// farthest child * 2 + 2 less than size

return ret;
}

void insert(struct priority_queue *q, int prio, int *insert) {
int child_index = q->size;
q->prio[q->size] = prio;
q->data[q->size] = insert;
q->size++;

int *tdata, tprio;

int parent_index;

  while (child_index != 0) {
  parent_index = (child_index - 1) / 2;
    if (q->prio[parent_index] > q->prio[child_index]) {
    tprio = q->prio[parent_index];
    tdata = q->data[parent_index];
    q->prio[parent_index] = q->prio[child_index];
    q->data[parent_index] = q->data[child_index];
    q->prio[child_index] = tprio;
    q->data[child_index] = tdata;
    }
  child_index = parent_index;
  }

}

/*
int main () {
int arr[4] = {0, 0, 1, 2};
int not[4] = {-1, 0, 1, 2};
int def[4] = {0, 0, 1, 2};

struct queue steve;
steve.capacity = 2;
steve.front = 0;
steve.rear = 0;
steve.data = malloc(2 * sizeof(int *));

enqueue(&steve, def);

printf("%d %d %d\n", contains(&steve, arr), contains(&steve, def), contains(&steve, not));

}
*/
