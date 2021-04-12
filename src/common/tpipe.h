#ifndef _TPIPE_H_
#define _TPIPE_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <memory.h>
#include <malloc.h>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include "ipipe.h"

using namespace std;

class TPipe : public IPipe
{
public:
    TPipe() {
        pthread_mutex_init(&pipelock,  NULL);
    }
    ~TPipe() {
    }
public:
    int Size(bool head = true) {
        pthread_mutex_lock(&pipelock);
        int size = (int)(head?headpool:tailpool).size();
        pthread_mutex_unlock(&pipelock);
        return size;
    }
    void Attach(Object * anno, bool head = true){
        pthread_mutex_lock  (&pipelock);
        vector<Object*>& pool = head ? headpool : tailpool;
        vector<Object*>::iterator it = pool.begin();
        while(it != pool.end() && *it != anno) it++;
        if(it == pool.end()) pool.push_back(anno);
        pthread_mutex_unlock(&pipelock);
    }
    void Detach(Object * anno, bool head = true){
        pthread_mutex_lock(&pipelock);
        vector<Object*>& pool = head ? headpool : tailpool;
        for(int i = 0; i < pool.size(); i++){
            if(pool[i] == anno){
                pool.erase(pool.begin()+i--);
            }
        }
        pthread_mutex_unlock(&pipelock);
    }
private:
    vector<Object*>  tailpool;
    vector<Object*>  headpool;
    pthread_mutex_t  pipelock;
};

class TPoolPipe : public TPipe
{
public:
	TPoolPipe(int pipelong) : poolsize(pipelong) {
		pthread_mutex_init(&poollock,  NULL);
		pthread_cond_init (&poolaval,  NULL);
	}
public:
	~TPoolPipe() {
		for(set<void*>::iterator it = memblock.begin(); it != memblock.end(); it++){
			if(*it) free(*it);
		}
	}
public:
	virtual void * Alloc(int size) {
		void * data = NULL;
        pthread_mutex_lock(&poollock);
		if(pooldata.size() == 0 && memblock.size() < poolsize){
			memblock.insert(data=malloc(size));
			pooldata.push_back(data);
		}
		while(pooldata.size() == 0){
			pthread_cond_wait(&poolaval, &poollock);
		}
		data = pooldata.back();
		       pooldata.pop_back();
		if(malloc_usable_size(data) < size) {
			memblock.erase(data);
			memblock.insert(data=realloc(data, size));
		}
		memset(data, 0, size);
        pthread_mutex_unlock(&poollock);
		return data;
	}
	virtual void Free(void * data) {
        pthread_mutex_lock(&poollock);
		if(memblock.find(data) != memblock.end()) {
			pooldata.push_back(data);
		}
     	pthread_cond_signal(&poolaval);
        pthread_mutex_unlock(&poollock);
	}
private:
	int			  	poolsize;
	set<void*> 		memblock;
	vector<void*> 	pooldata;
	pthread_mutex_t	poollock;
	pthread_cond_t 	poolaval;
};

class TQueuePipe : public TPoolPipe
{
public:
	TQueuePipe(int poollong) : TPoolPipe(poollong) {
		pthread_mutex_init(&queuelock, NULL);
		pthread_cond_init (&queueaval, NULL);
	}
	~TQueuePipe() {
	}
public:
	virtual void Push(void * data) {
        pthread_mutex_lock(&queuelock);
		queuedata.push(data);
     	pthread_cond_signal (&queueaval);
        pthread_mutex_unlock(&queuelock);
	}
	virtual void * Pop() {
		void * data = NULL;
        pthread_mutex_lock(&queuelock);
       	while(queuedata.empty()){
			pthread_cond_wait(&queueaval, &queuelock);
		}
		data = queuedata.front();
			   queuedata.pop();
        pthread_mutex_unlock(&queuelock);
		return data;
	}
private:
	queue<void*> 	 queuedata;
	pthread_mutex_t  queuelock;
	pthread_cond_t   queueaval;
};

#endif
