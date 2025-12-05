Lily Ru: Implemented connection functions and receiver.cpp
Jiaqi Yu: Implemented functions for the sender client and run testing

MS2: 
Lily Ru: Implemented functions in message_queue and room, as well as server member functions
Jiaqi Yu: Implemented chat_with_sender and chat_with_receiver functions, added synchronization

SYNCHRONIZATION REPORT

Overview:
Our server implementation uses three main synchronized objects: Server, Room, and MessageQueue.
Each object has its own mutex to protect its internal state, following the principle of
encapsulating synchronization within object boundaries to prevent synchronization bugs.

Critical Sections and Synchronization Primitives:

1. SERVER CLASS (server.cpp, server.h)
   - Protected Resource: m_rooms (std::map<std::string, Room *>)
   - Synchronization Primitive: pthread_mutex_t m_lock
   - Critical Section: find_or_create_room() method (lines 132-145 in server.cpp)
   
   Rationale: Multiple threads may try to join rooms simultaneously. Without synchronization,
   two threads could both check if a room exists, find it doesn't, and both create new Room
   objects for the same room name, causing a resource leak and inconsistent state. The Guard
   lock ensures that only one thread at a time can read or modify the m_rooms map.
   
   Implementation: We use a Guard object (RAII pattern) that acquires m_lock when entering
   the critical section and automatically releases it when leaving scope. This prevents
   forgetting to release the lock and avoids deadlocks from early returns.

2. ROOM CLASS (room.cpp, room.h)
   - Protected Resources: members (std::set<User *>)
   - Synchronization Primitive: pthread_mutex_t lock
   - Critical Sections:
     * add_member() (lines 18-22 in room.cpp)
     * remove_member() (lines 24-28 in room.cpp)
     * broadcast_message() (lines 30-37 in room.cpp)
   
   Rationale: Multiple scenarios require synchronization:
   - Two receivers joining the same room simultaneously
   - A receiver leaving while a sender broadcasts a message
   - Multiple senders broadcasting to the same room simultaneously
   
   Without synchronization, the members set could be corrupted during concurrent modifications,
   or broadcast_message could iterate over members while another thread is adding/removing,
   causing undefined behavior or messages being delivered to the wrong set of users.
   
   Implementation: Each method uses a Guard lock on the Room's private mutex. The
   broadcast_message() method holds the lock for the entire duration of enqueueing messages
   to all receivers, ensuring a consistent snapshot of room membership. This prevents a
   receiver from being removed mid-broadcast, which could cause use-after-free errors.

3. MESSAGEQUEUE CLASS (message_queue.cpp, message_queue.h)
   - Protected Resources: m_messages (std::deque<Message *>)
   - Synchronization Primitives: 
     * pthread_mutex_t m_lock (protects queue modifications)
     * sem_t m_avail (counts available messages and blocks receivers when empty)
   
   Critical Sections:
     * enqueue() (lines 17-28 in message_queue.cpp)
     * dequeue() (lines 31-48 in message_queue.cpp)
     * ~MessageQueue() (lines 12-24 in message_queue.cpp)
   
   Rationale: The MessageQueue implements a producer-consumer pattern where sender threads
   enqueue messages and receiver threads dequeue them. This requires careful synchronization:
   - Multiple senders might enqueue to the same receiver simultaneously
   - The receiver thread must block when no messages are available
   - Queue must be protected during concurrent access
   
   Implementation: We use a semaphore as a "smart counter" of available messages. When a
   sender enqueues a message, it locks the mutex, adds to the queue, unlocks the mutex, then
   posts the semaphore to signal message availability. When a receiver dequeues, it first
   waits on the semaphore (blocking if count is 0), then locks the mutex, removes a message,
   and unlocks. This separation ensures that the Room lock is released before the receiver
   waits on its queue, preventing deadlock.
   
   The destructor also locks the mutex and frees all remaining messages to prevent memory
   leaks when a receiver disconnects before consuming all messages.

Deadlock Prevention:
Our design prevents deadlocks through several mechanisms:
1. Lock Hierarchy: We never hold a Room lock while acquiring a MessageQueue lock
2. Single Lock per Operation: Each critical section acquires only one lock
3. RAII Guard Pattern: Ensures locks are always released, even with early returns
4. Semaphore/Lock Separation: Receivers wait on semaphore without holding any mutex

Race Condition Prevention:
1. All access to shared data structures (m_rooms, members, m_messages) is protected by locks
2. The broadcast_message() holds the Room lock while iterating over all members, ensuring
   no member is added/removed during the iteration
3. Heap-allocated Message objects prevent thread-local data from being shared between threads
4. Receiver threads clean up their User objects before thread termination

Testing Approach:
The synchronization was tested using:
1. Multiple concurrent senders in the same room
2. Receivers joining/leaving during active message broadcasts
3. The provided test_concurrent.sh script with high iteration counts
4. Manual stress testing with rapid connect/disconnect cycles
