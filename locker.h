//
// Created by xue241 on 24-3-4.
//
#include <semaphore.h>
#include <pthread.h>
#include <iostream>
#include <exception>

class sem{
private:
    sem_t m_sem;
public:
    sem(){
        if (sem_init(&m_sem, 0, 0) != 0){
            throw std::exception();
        }
    }

    sem(int num){
        if (sem_init(&m_sem, 0, num) != 0){
            throw std::exception();
        }
    }

    ~sem(){
        sem_destroy(&m_sem);
    }

    void wait(){
        sem_wait(&m_sem);
    }

    void post(){
        sem_wait(&m_sem);
    }
};

class locker{
private:
    pthread_mutex_t m_mutex;
public:
    locker(){
        if (pthread_mutex_init(&m_mutex, NULL) != 0){
            throw std::exception();
        }
    }

    ~locker(){
        pthread_mutex_destroy(&m_mutex);
    }

    void lock(){
        pthread_mutex_lock(&m_mutex);
    }

    void unlock(){
        pthread_mutex_unlock(&m_mutex);
    }

    pthread_mutex_t *get(){
        return &m_mutex;
    }

};

class cond{
private:
    pthread_cond_t m_cond;

public:
    cond(){
        if (pthread_cond_init(&m_cond, NULL) != 0){
            throw std::exception();
        }
    }

    ~cond(){
        pthread_cond_destroy(&m_cond);
    }

    bool wait(pthread_mutex_t *m_mutex){
        int ret = 0;
        ret = pthread_cond_wait(&m_cond, m_mutex);
        return ret;
    }

    bool timewait(pthread_mutex_t *m_mutex, struct timespec t){
        int ret = 0;
        ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
        return ret;
    }

    bool signal(){
        return pthread_cond_signal(&m_cond) == 0;
    }

    bool broadcast(){
        return pthread_cond_broadcast(&m_cond) == 0;
    }

};
