#include <stdio.h>
#include <pthread.h>
#include <stdlib.h> 
#include <math.h>
#include "functions.h"


int counter = 0; //to report one how many primes counted
pthread_mutex_t mutex; //use this for incrementing counter
int report;

int main(int argc, char* argv[]) {

  if(argc != 4) {
    printf("Please give: <range> <threads> <report>\n");
    return 1;
  } 

  pthread_mutex_init(&mutex, NULL);
  
  int range = atoi(argv[1]);
  int threadCount = atoi(argv[2]);
  report = atoi(argv[3]);
  
  pthread_t p_threads[threadCount];
  ThreadInfo thread_info[threadCount];
  
  //use simple indexing to get start and end values
  int start[threadCount];
  int end[threadCount];
  double weights[threadCount];

  getDynamicWeights(threadCount, weights);
  getChunks(range, threadCount, weights, start, end);

//   printf("Here weres the chunks that were made:\n");
// 
//   for(int i = 0; i < threadCount; i++) {
//     printf("Chunk %d: %d-%d\n",i, start[i], end[i]);
//   }
 
  //now loop through threads 
  for(int i = 0; i < threadCount; i++) {
    thread_info[i].start = start[i];
    thread_info[i].end = end[i];
    thread_info[i].list = (LinkedList *)malloc(sizeof(LinkedList));
    thread_info[i].list->head = NULL;
    thread_info[i].list->tail = NULL;

    pthread_create(&p_threads[i], NULL, findPrimes, &thread_info[i]);
  }
  
  //now join all of the threads
  for(int i = 0; i < threadCount; i++) {
    pthread_join(p_threads[i], NULL);
  }

  pthread_mutex_destroy(&mutex);

  FILE *primes = fopen("primes.txt", "w");

  //Now loop through threads list, add them to txt file 
  for(int i = 0; i < threadCount; i++) {
    node_t *curr = thread_info[i].list->head;

    while(curr) {
      fprintf(primes, "%d\n", curr->val);
      curr = curr->next;
    }

    //Now free list 
    freeList(thread_info[i].list);

    //and actually the list itself, oops
    free(thread_info[i].list);
  }

  fclose(primes); //close the file 
  printf("All done calculating. Enjoy.\n");
  printf("Total numbers calculated: %d\n", counter);
  return 0;
}


int isPrime(int n) {
  if(n <= 1) return 0;
  if(n == 2) return 1;
  if(n % 2 == 0) return 0;
  for(int i = 3; i <= sqrt(n); i++) {
    if(n % i == 0) return 0;
  }

  return 1;
}

void* findPrimes(void *arg) {
  ThreadInfo* info = (ThreadInfo *)arg;

  for(int i = info->start; i <= info->end; i++) {
 
    if(isPrime(i)) {
      addPrime(info->list, i);
    }

    pthread_mutex_lock(&mutex);
    counter++;
    if(counter % report == 0) printf("Have procced %d thus far.\n", counter);
    pthread_mutex_unlock(&mutex);
  }

  pthread_exit(NULL);
}

//maintain constant insertion time with 0(1), can now easily add to txt file
void addPrime(LinkedList *list, int value) {
  node_t *newNode = (node_t *)malloc(sizeof(node_t));
  newNode->val = value;

  if(list->head == NULL) {
    list->head = newNode;
    list->tail = newNode;
  } else {
    list->tail->next = newNode;
    list->tail = newNode;
  }
}

void freeList(LinkedList *list) {
  node_t *curr = list->head;

  while(curr) {
    node_t *temp = curr;
    curr = curr->next;
    free(temp);
  }

  list->head = NULL;
  list->tail = NULL;
}

void getDynamicWeights(int threadCount, double *weights) {
  double total;

  for(int i = 0; i < threadCount; i++) {
    weights[i] = 1.0 / (i+1); //so earlier threads doing less computation get more numbers
    total += weights[i]; 
  }

  //normalize into a percentage basically
  for(int i = 0; i < threadCount; i++) {
    weights[i] /= total;
  }
}

//now take the weights and convert them into ranges that the threads will eventually use
void getChunks(int range, int threadCount, double *weights, int *start, int *end) {
  int cumulative = 0; //track the sum of the ranges

  for(int i = 0; i < threadCount; i++) {
    start[i] = cumulative + 1;
    int chunkSize = (int)(range * weights[i]);  
    cumulative += chunkSize;
    end[i] = (i == threadCount - 1) ? range : cumulative; //if numbers left over, give them to the last thread
  }
}
