//
// Created by xue241 on 24-3-4.
//

#ifndef THREAD_POOL_THREADPOOL_H
#define THREAD_POOL_THREADPOOL_H

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <list>
#include <pthread.h>
#include "locker.h"
using namespace std;

class ThreadPool{
private:
    struct NWORKER{
        pthread_t tid;
        bool terminate;
        int isWorking;
        ThreadPool *pool;
    } *m_workers;

    struct NJOB{
        void (*func)(void *arg);
        void *user_data;
    };

public:
    ThreadPool(int numWorkers, int max_jobs);
    ~ThreadPool();
    int pushJob(void (*func)(void *data), void *arg, int len);

private:
    bool _addjob(NJOB *job);
    static void *_run(void *arg);
    void _threadLoop(void *arg);
private:
    list<NJOB *> m_jobs_list;
    int m_max_jobs;
    int m_sum_thread;
    int m_free_thread;

    cond m_cond;
    locker m_mutex;

};

ThreadPool::ThreadPool(int numWorkers, int max_jobs = 10) :m_sum_thread(numWorkers), m_free_thread(numWorkers), m_max_jobs(max_jobs){
    if (numWorkers <= 0 or max_jobs <= 0){
        perror("workers num error. ");
    }

    m_workers = new NWORKER[numWorkers];
    if (!m_workers){
        perror("create workers failed. \n");
    }

    for (int i = 0; i < numWorkers; i++){
        m_workers[i].pool = this;
        int ret = pthread_create(&(m_workers[i].tid), NULL, _run, &m_workers[i]);
        if (ret){
            delete [] m_workers;
            perror("create worker fail\n");
        }

        if (pthread_detach(m_workers[i].tid)){
            delete [] m_workers;
            perror("detach worder fail. \n");
        }
        m_workers[i].terminate = 0;
    }

}

ThreadPool::~ThreadPool() {
    for (int i = 0; i < m_sum_thread; i++){
        m_workers[i].terminate = 1;
    }
    m_mutex.lock();
    m_cond.broadcast();
    m_mutex.unlock();

    delete [] m_workers;
}

bool ThreadPool::_addjob(ThreadPool::NJOB *job) {
    m_mutex.lock();
    if (m_jobs_list.size() >= m_max_jobs){
        m_mutex.unlock();
        return false;
    }

    m_jobs_list.push_back(job);
    m_cond.signal();
    m_mutex.unlock();
}

int ThreadPool::pushJob(void (*func)(void *), void *arg, int len) {
    NJOB *job = (NJOB *)malloc(sizeof(struct NJOB));
    if (job == NULL){
        perror("malloc");
        return -2;
    }

    memset(job, 0, sizeof(NJOB));
    job->user_data = malloc(len);
    memcpy(job->user_data, arg, len);
    job->func = func;
    _addjob(job);

    return 1;
}

void *ThreadPool::_run(void *arg) {
    NWORKER *worker = (NWORKER *) arg;
    worker->pool->_threadLoop(arg);
}

void ThreadPool::_threadLoop(void *arg) {
    NWORKER *worker = (NWORKER *)arg;
    while (1){
        m_mutex.lock();
        while (m_jobs_list.size() == 0){
            if (worker->terminate == 1){
                break;
            }
            m_cond.wait(m_mutex.get());
        }
        if (worker->terminate == 1){
            m_mutex.unlock();
            break;
        }

        NJOB *job = m_jobs_list.front();
        m_jobs_list.pop_front();

        m_mutex.unlock();
        m_free_thread--;
        worker->isWorking = true;
        job->func(job->user_data);
        worker->isWorking = false;

        free(job->user_data);
        free(job);

    }
    free(worker);
    pthread_exit(NULL);
}


#endif //THREAD_POOL_THREADPOOL_H
