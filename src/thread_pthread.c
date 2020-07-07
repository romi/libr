/*
  ROMI libr

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  The libr library provides some hardware abstractions and low-level
  utility functions.

  libr is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#include <pthread.h>
#include "r.h"

struct _thread_t {
        pthread_t thread;
        thread_run_t run;
        void *data;
};

static void* _run(void* data);

thread_t* new_thread(thread_run_t run, void *data)
{
        thread_t* thread = r_new(thread_t);
        thread->run = run;
        thread->data = data;

        int ret = pthread_create(&thread->thread, NULL, _run, (void*) thread);
        if (ret != 0) {
                r_err("new_thread: failed to create the thread");
                delete_thread(thread);
                return NULL;
        }
        
        return thread;
}

static void* _run(void* data)
{
        thread_t* thread = (thread_t*) data;
        thread->run(thread->data);
        return NULL;
}

void delete_thread(thread_t* thread)
{
        r_delete(thread);
}

int thread_join(thread_t* thread)
{
       return pthread_join(thread->thread, NULL);
}

/************************************************************/

struct _mutex_t
{
        pthread_mutex_t mutex;
};

mutex_t *new_mutex()
{
        mutex_t *mutex = r_new(mutex_t);
        pthread_mutex_init(&mutex->mutex, NULL);
        return mutex;
}

void delete_mutex(mutex_t *mutex)
{
        if (mutex) {
                pthread_mutex_destroy(&mutex->mutex);
                r_delete(mutex);
        }
}

void mutex_lock(mutex_t *mutex)
{
        pthread_mutex_lock(&mutex->mutex);
}

void mutex_unlock(mutex_t *mutex)
{
        pthread_mutex_unlock(&mutex->mutex);
}

/************************************************************/

struct _condition_t
{
        pthread_cond_t cond;
};

condition_t *new_condition()
{
        condition_t *condition = r_new(condition_t);
        pthread_cond_init(&condition->cond, NULL);
        return condition;
}

void delete_condition(condition_t *condition)
{
        if (condition) {
                pthread_cond_destroy(&condition->cond);
                r_delete(condition);
        }
}

void condition_wait(condition_t *condition, mutex_t *mutex)
{
        pthread_cond_wait(&condition->cond, &mutex->mutex);
}

void condition_signal(condition_t *condition)
{
        pthread_cond_signal(&condition->cond);
}
