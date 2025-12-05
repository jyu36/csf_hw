#include <cassert>
#include <ctime>
#include "message.h"
#include "message_queue.h"

MessageQueue::MessageQueue() {
  // TODO: initialize the mutex and the semaphore
  pthread_mutex_init(&m_lock, nullptr);
  sem_init(&m_avail, 0, 0);
}

MessageQueue::~MessageQueue() {
  // TODO: destroy the mutex and the semaphore
  // Free all remaining messages to prevent memory leaks
  pthread_mutex_lock(&m_lock);
  while (!m_messages.empty()) {
    Message *msg = m_messages.front();
    m_messages.pop_front();
    delete msg;
  }
  pthread_mutex_unlock(&m_lock);
  
  pthread_mutex_destroy(&m_lock);
  sem_destroy(&m_avail);
}

void MessageQueue::enqueue(Message *msg) {
  // TODO: put the specified message on the queue
  // lock while modifying queue
  pthread_mutex_lock(&m_lock);

  m_messages.push_back(msg);

  pthread_mutex_unlock(&m_lock);

  // be sure to notify any thread waiting for a message to be
  // available by calling sem_post
  sem_post(&m_avail);
}

Message *MessageQueue::dequeue() {
  // Note: using sem_wait instead of sem_timedwait for Mac compatibility
  // This will block indefinitely until a message is available
  if (sem_wait(&m_avail) == -1) {
    return nullptr;
  }

  // Remove the next message from the queue
  pthread_mutex_lock(&m_lock);
  
  Message *msg = nullptr;
  if (!m_messages.empty()) {
    msg = m_messages.front();
    m_messages.pop_front();
  }
  
  pthread_mutex_unlock(&m_lock);
  return msg;
}
