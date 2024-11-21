typedef struct node_t {
  int val;
  struct node_t* next;
} node_t;

typedef struct {
  node_t* head;
  node_t* tail;
} LinkedList;

typedef struct {
  int start;  
  int end;
  LinkedList* list;
} ThreadInfo;

int isPrime(int n);
void addPrime(LinkedList* list, int value);
void freeList(LinkedList* list);
void addPrimesToTxt(LinkedList* list);
void *findPrimes(void *arg);
void getDynamicWeights(int threadCount, double *weights);
void getChunks(int range, int threadCount, double *weights, int *start, int *end);
