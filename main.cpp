#include <cstdlib>
#include <ctime>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <vector>
#include <queue>

#define NUM_CHAIRS 5
#define NUM_STUDENTS 15

sem_t ta_sem;

sem_t student_sem;

pthread_mutex_t mutex;

int waiting_students = 0;

std::queue<int> waiting_queue;

void *ta_function(void *arg) {
  while (true) {
    std::cout << "TA is sleeping." << std::endl;
    sem_wait(&ta_sem);
    
    pthread_mutex_lock(&mutex);
    
    if (waiting_students > 0) {
      int student_to_help = waiting_queue.front();
      waiting_queue.pop();
      waiting_students--;
      
      std::cout << "TA is helping student " << student_to_help 
                << ". Remaining waiting students: " << waiting_students <<  std::endl;
      pthread_mutex_unlock(&mutex);
      
      sleep(rand() % 5 + 1);
      
      sem_post(&student_sem);
    } else {
      std::cout << "TA was awakened but no students are waiting! Going back to sleep." << std::endl;
      pthread_mutex_unlock(&mutex);
    }
  }
  return NULL;
}

void *student_function(void *arg) {
  int student_id = *(int *)arg;
  delete (int *)arg;

  sleep(rand() % 10 + 1);

  std::cout << "Student " << student_id << " arrived at TA's office." << std::endl;

  pthread_mutex_lock(&mutex);
  
  if (waiting_students < NUM_CHAIRS) {
    waiting_students++;
    waiting_queue.push(student_id);
    
    std::cout << "Student " << student_id 
              << " takes a seat. Waiting students: " << waiting_students << std::endl;
    
    bool was_ta_sleeping = (waiting_students == 1);
    pthread_mutex_unlock(&mutex);
    
    if (was_ta_sleeping) {
      std::cout << "Student " << student_id << " wakes up the TA." << std::endl;
      sem_post(&ta_sem);
    }
    
    sem_wait(&student_sem);
    
    std::cout << "Student " << student_id << " is getting help from the TA." << std::endl;
    
    sleep(rand() % 3 + 1);
    
    std::cout << "Student " << student_id << " is done getting help and leaves." << std::endl;
    
    pthread_mutex_lock(&mutex);
    if (waiting_students > 0) {
      sem_post(&ta_sem);
    }
    pthread_mutex_unlock(&mutex);
    
  } else {
    std::cout << "No chairs available. Student " << student_id << " will try again later. 🚶" << std::endl;
    pthread_mutex_unlock(&mutex);
    
    sleep(rand() % 5 + 5);
    return student_function(new int(student_id));
  }

  return NULL;
}

int main() {
  srand(time(NULL));

  pthread_mutex_init(&mutex, NULL);
  sem_init(&ta_sem, 0, 0);      
  sem_init(&student_sem, 0, 0); 

  std::cout << "Sleeping TA problem simulation started with " << NUM_STUDENTS
            << " students and " << NUM_CHAIRS << " waiting chairs." << std::endl;

  pthread_t ta_thread;
  pthread_create(&ta_thread, NULL, ta_function, NULL);

  std::vector<pthread_t> student_threads(NUM_STUDENTS);
  for (int i = 0; i < NUM_STUDENTS; ++i) {
    int *student_id = new int(i + 1);
    pthread_create(&student_threads[i], NULL, student_function, (void *)student_id);
  }

  pthread_join(ta_thread, NULL);
  for (int i = 0; i < NUM_STUDENTS; ++i) {
    pthread_join(student_threads[i], NULL);
  }

  sem_destroy(&ta_sem);
  sem_destroy(&student_sem);
  pthread_mutex_destroy(&mutex);

  return 0;
}