#ifdef LOCK_SKIP
#include "3d_rtree_skip.h"
#else
#include "3d_rtree.h"
#endif
#include <unordered_map>
#include "poisson.h"
#include <time.h>
#include <unistd.h>
#include <cfloat>
#include <solar.h>
#ifdef BREAKDOWN
extern double traversal_time;
extern double write_time;
#endif

extern uint64_t flipCount;
extern uint64_t splitCount;
int NUMDATA=100;
int SEARCH=10;
int IpthreadNum=8;

//Possion
class Query {
    public:
        struct Rect qr;
         unsigned long long arrival_time;
};
extern size_t try_move_count;
extern size_t bt2_move_count;
extern size_t lockCount;
extern size_t splitCount;
extern  unsigned long long poisson(double x);
//

void clear_cache() {
    // Remove cache
    int size = 256*1024*1024;
    char *garbage = new char[size];
    for(int i=0;i<size;++i)
        garbage[i] = i;
    for(int i=100;i<size;++i)
        garbage[i] += garbage[i-100];
    delete[] garbage;
}

//generate search data
struct Rect* sr = NULL;
int selection_ratio = 1;
void makeSearch(int num){
  float f_r = selection_ratio;
  sr = new Rect[num];
  srand(0);
  for(int i=0; i<num; i++){
    sr[i].boundary[0] = 0.0; 
    sr[i].boundary[2] = 0.0; 
    sr[i].boundary[3] = FLT_MAX; 
    sr[i].boundary[5] = FLT_MAX;   
    float a = (float)(rand() % (100-(selection_ratio-1)));
    sr[i].boundary[1] = a + 0.49;   
    sr[i].boundary[4] = a + (0.01+f_r);     
  }
}

// thread data
struct thread_data{
	int  tid =0;
	int  hit=0;
  struct kv_t* InsKV=NULL; 
  char* filePath = NULL; 
  int startDataNum=0; // start number of fraction for each thread
  int dataNum=0;      // number of data for each thread
  queue<Query*>* que=NULL;
  vector< unsigned long long>* lv = NULL;  
};

struct Node * total_root = NULL;
//P Thread Insert
bool ended = false;
void* PThreadInsert(void *arg)
{
	// Insert the R-tree boundary region
    sleep(180);
    struct thread_data* td = (struct thread_data*) arg;
    int j = 0;
      for(int i= 0; i< 100; i++){
        RTreeInsertRect((struct Rect*)&(td->InsKV[i].key), (td->filePath), (&total_root), (&total_log[td->tid*2]));
      }
    pthread_exit(NULL);
}

struct Rect *r = NULL;
//struct timespec s;
// P Thread Search
void* PThreadSearch(void* arg)
{
	struct thread_data* td = (struct thread_data*) arg;
    struct timespec s, cur, f;
     unsigned long long cur_time= 0;
    clockid_t clk_id = CLOCK_MONOTONIC;
    clock_gettime(clk_id, &s);
    clock_gettime(clk_id, &cur);
    cur_time = (cur.tv_sec - s.tv_sec)*1000000000LLU + (cur.tv_nsec - s.tv_nsec);
   //unsigned long long old_cur = 0;
	for(int i=0; i < td->dataNum; i++){
        int reSplit = 1;
        Query* q = td->que->front();
        td->que->pop();
        //printf("[%d]cur_time, arr : %llu %llu\n",i, cur_time , q->arrival_time);
        while(q->arrival_time > cur_time){
            clock_gettime(clk_id, &cur);
            cur_time = (cur.tv_sec - s.tv_sec)*1000000000LLU + (cur.tv_nsec - s.tv_nsec);
        }
		  td->hit += hostRTreeSearch(total_root, &(q->qr), reSplit);

        clock_gettime(clk_id, &cur);
            cur_time = (cur.tv_sec - s.tv_sec)*1000000000LLU + (cur.tv_nsec - s.tv_nsec);
        td->lv->push_back(cur_time - q->arrival_time);
		//td->hit += hostRTreeSearch(total_root, &(td->rect[i]), reSplit);
    //old_cur = cur_time;
	}
    clock_gettime(clk_id, &f);
    //printf("SEARCH TIME: %llu\n", (f.tv_sec - s.tv_sec)*1000000000LLU + (f.tv_nsec - s.tv_nsec));
	pthread_exit(NULL);
}

//-----------------------------------------------------------------------
//------------------------------- MAIN ----------------------------------
//-----------------------------------------------------------------------
int main(int argc, char *args[])
{
	// Check the arguments for extra information
	if(argc<5){
        printf("Usage: %s (number_of_INSERT) (number_of_SEARCH) (number_of_Insert_THREADs) (number_of_Search_THREADs) (write_Latency) (fileN)\n", args[0]);
	    exit(1);
	}
	
    NUMDATA = atoi(args[1]);	// Initialize the number of Data
    SEARCH = atoi(args[2]);	// Initialize the number of search Data
    IpthreadNum = atoi(args[3]);	// Initialize the number of insert Thread
    int SpthreadNum = atoi(args[4]);	// Initialize the number of search Thread     
    writeLatency = atoi(args[5]);	// Initialize the number of Thread 
    int fileN = atoi(args[6]);
    selection_ratio = atoi(args[7]);
    printf("INSERT: %d, SEARCH: %d, insert_thread: %d, search_thread: %d, Write_Latency: %d fileN: %d selection_ratio: %d\n", NUMDATA, SEARCH, IpthreadNum, SpthreadNum, writeLatency, fileN, selection_ratio);

    //##################### GET DATA #########################
    struct timeval f1,f2;
    double time_f2;
    int arr_size = 100 * fileN; //warmup;                         
    kv_t* kv_arr = new kv_t[arr_size];  

    gettimeofday(&f1,0); 
    get_kv_narr(fileN, kv_arr); 
    gettimeofday(&f2,0);
    time_f2 = (f2.tv_sec-f1.tv_sec)*1000000 + (f2.tv_usec - f1.tv_usec);  
//    printf("HDF5 file Num        : %d\n", fileN); 
//    printf("throughput           : %.3lf\n", (1000)/(time_f2));  

    //################ GET SEARCH DATA #######################
    makeSearch(SEARCH);
    //########################################################

	//################ CREATE & WARM_UP ######################
	// CREATE RTree total_root node
    int indexSize;

	// Initialize the R-tree
    total_root = RTreeNewIndex();
    log_init(IpthreadNum);

    const int warmData = (16082-1)*100; // 16000*10
    //warm up
    for(int i=0; i< warmData; i++){
        RTreeInsertRect((struct Rect*)&kv_arr[i].key, kv_arr[i].val, &total_root, NULL); 
    }
    printf("Warm up (warm of NUMDATA) end %d/%d\n", warmData, NUMDATA );
    
    const int insertPerThread = NUMDATA / IpthreadNum;  // 200 / 2
    const int searchPerThread = SEARCH / SpthreadNum; // 48*1M / 48 = 1M
    printf("insert: %d, search: %d\n", insertPerThread, searchPerThread);
	//########################################################

	//################# MULTI INSERT #########################

    int rc;
    void *status;
    pthread_t threads_i[IpthreadNum];
    struct thread_data td_i[IpthreadNum];
    
    for(int i=0; i<IpthreadNum; i++){
        int dataS = warmData;    
        int dataE = 100;
        td_i[i].tid = i;
        td_i[i].hit = 0;
        td_i[i].InsKV = &kv_arr[dataS];  
        td_i[i].filePath = kv_arr[dataS].val; 
        td_i[i].startDataNum = dataS;
        td_i[i].dataNum = dataE;
        
//        int dataS = warmData + i*insertPerThread;    
//        int dataE = (i == IpthreadNum-1)? NUMDATA-i*insertPerThread : insertPerThread; 
//        
//        td_i[i].tid = i;
//        td_i[i].hit = 0;
//        td_i[i].InsKV = &kv_arr[dataS];  
//        td_i[i].filePath = kv_arr[dataS].val; 
//        td_i[i].startDataNum = dataS;
//        td_i[i].dataNum = dataE;
    }
//    fprintf(stderr, "Insertion is done.\n");

    clear_cache();
 
	//########################################################
//-------------------------------------------------------------------------
    queue<Query*>* qq = new queue<Query*>[SpthreadNum];
     unsigned long long arrival_time=0;
    rand_val(1);
    srand(1);
    vector< unsigned long long>* latency_vector = new vector< unsigned long long>[SpthreadNum];
    
    //###################  SEARCH DATA #######################
    struct timeval t1,t2;
    double time_t2;
    uint64_t hit = 0;

    printf("[Searching]\n");

    pthread_t threads[SpthreadNum];
    struct thread_data td[SpthreadNum];
   
    int j=0,  h=0; 
    gettimeofday(&t1,0); // start the stopwatch
    //clock_gettime(CLOCK_MONOTONIC, &s);
    //unsigned long long s_time = s.tv_sec * 1000000000LLU + s.tv_nsec;
    for(int i=0; i<SpthreadNum; i++){
        int searchS = i*searchPerThread;
        int searchE = (i==SpthreadNum-1)? SEARCH-searchS : searchPerThread;
         unsigned long long old_arrival = 0;
         arrival_time = 0;
        for(int g=0; g< searchE; g++){
#ifdef SHARED
//          if(selection_ratio == 1)
//          arrival_time += poisson(1.0/2458.660)*10000;
//          if(selection_ratio == 2)
//          arrival_time += poisson(1.0/2512.230)*10000;
//          if(selection_ratio == 4)
//          arrival_time += poisson(1.0/2548.970)*10000;
          //arrival_time += poisson(1.0/2548.970)*10000;
          arrival_time += poisson(1.0/2700.970)*10000;
#else
          if(selection_ratio == 1)
          arrival_time += poisson(1.0/1315.490)*10000;
          if(selection_ratio == 2)
          arrival_time += poisson(1.0/1335.630)*10000;
          if(selection_ratio == 4)
          arrival_time += poisson(1.0/1377.670)*10000;
#endif
          //arrival_time += 1500000;

          //arrival_time *= 1000000;
          Query *q = new Query;
          q->qr = sr[searchS + g];
          //q->arrival_time = 0;
          q->arrival_time = arrival_time ;
          //printf("[%d]q->arrival: %llu\n",g, q->arrival_time);
          qq[i].push(q);
          //old_arrival += arrival_time;
        }
        //printf("[Queuing Poisson samples is DONE]\n");
        td[i].que = &qq[i];
        td[i].lv = &latency_vector[i];

        td[i].tid = i;
        td[i].hit = 0;
        td[i].startDataNum = searchS;
        td[i].dataNum = searchE;
        
        rc = pthread_create(&threads[i], NULL, PThreadSearch, (void *)&td[i]);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    rc = pthread_create(&threads_i[0], NULL, PThreadInsert, (void *)&td_i[0]);
    
        rc = pthread_join(threads_i[0], &status);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
    for(int i=0; i<SpthreadNum; i++){
        rc = pthread_join(threads[i], &status);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
        hit += td[i].hit;
    }
    gettimeofday(&t2,0);
    time_t2 = (t2.tv_sec-t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec);
    printf("host total time (msec): %.3lf\n", time_t2/1000);
    printf("Host Hit counter = %ld, PThread: %d\n", hit, SpthreadNum);
	//########################################################
    //FILE *fp = fopen("pLockFree.txt", "w+"); 
    for(int i=0; i< SpthreadNum; i++){
     for( unsigned long long lv : latency_vector[i])
        printf("%llu\n", lv);
    }

//    RTreePrint(total_root); 
    hostRTreeDestroy(total_root);    // -------------- end of host search ----
	return 0;
}
