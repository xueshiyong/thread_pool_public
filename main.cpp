//
// Created by xue241 on 24-3-4.
//


#include "threadpool.h"

void testFun(void* arg){
    printf("i = %d\n", *(int *)arg);
}


int main(){
    ThreadPool *pool = new ThreadPool(1000, 2000);

    printf("线程池初始化成功\n");
    int i = 0;
    for (i = 0; i < 1000; ++i) {
        pool->pushJob(testFun, &i, sizeof(int));
    }
}