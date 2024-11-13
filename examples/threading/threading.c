#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    struct thread_data* param = (struct thread_data *) thread_param;
	int rc;
	
	usleep(param->wait_to_obtain_ms * 1000);
	rc = pthread_mutex_lock(param->mutex);
	if (rc !=0){
		printf("pthread_mutex_lock failed with %d\n", rc);
		param->thread_complete_success = false;
		return thread_param;
	}

	usleep(param->wait_to_release_ms * 1000);
	
	rc = pthread_mutex_unlock(param->mutex);
	if (rc !=0){
		printf("pthread_mutex_unlock failed with %d\n", rc);
		param->thread_complete_success = false;
		return thread_param;
	}

	param->thread_complete_success = true;
	return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
	struct thread_data *thread_param = (struct thread_data*)malloc(sizeof(struct thread_data));
	thread_param->mutex = mutex;
	thread_param->wait_to_obtain_ms = wait_to_obtain_ms;
	thread_param->wait_to_release_ms = wait_to_release_ms;
	
	int rc;
/*	rc = pthread_mutex_init(thread_param->mutex, NULL);
	
	if (rc != 0){
		printf("pthread_mutex_init failed with %d\n", rc);
		return false;
	}*/

	rc = pthread_create(thread,
				  	   NULL,
				   	   threadfunc,
				   	   (void*)thread_param );
	if (rc != 0){
		printf("pthread_create failed wtih %d\n", rc);
		return false;
	}

	return true; 
}

