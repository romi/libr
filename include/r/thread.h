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
#ifndef _R_THREAD_H_
#define _R_THREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _thread_t thread_t;

typedef void (*thread_run_t)(void* data);

thread_t* new_thread(thread_run_t run, void *data, int realtime, int autodelete);
void delete_thread(thread_t* thread);
int thread_join(thread_t* thread);


typedef struct _mutex_t mutex_t;

mutex_t *new_mutex();
void delete_mutex(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);


typedef struct _condition_t condition_t;

condition_t *new_condition();
void delete_condition(condition_t *condition);
void condition_wait(condition_t *condition, mutex_t *mutex);
void condition_signal(condition_t *condition);

#ifdef __cplusplus
}
#endif

#endif // _R_THREAD_H_
