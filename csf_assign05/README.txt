MS1: 
Lily Ru: Implemented connection functions and receiver.cpp
Jiaqi Yu: Implemented functions for the sender client and run testing

MS2: 
Lily Ru: Implemented functions in message_queue and room, as well as server member functions
Jiaqi Yu: Implemented chat_with_sender and chat_with_receiver functions, added synchronization

SYNCHRONIZATION REPORT
Our server implementation uses three main synchronized objects: Server, Room, and MessageQueue. 
Each object has its own mutex to protect its internal state to prevent synchronization bugs.

1. MessageQueue

The MessageQueue class is used by receiver threads:
senders enqueue messages into a receiver’s queue, and the receiver thread dequeues messages. 
Both operations touch the same std::deque<Message*>, so they cannot run concurrently.
To protect this shared queue, I added a pthread mutex (m_lock). Two critical sections:

(a) Adding a message to the queue
Inside enqueue(), I lock the mutex before pushing a message onto m_messages, then unlock immediately afterward. This guarantees that if multiple senders are trying to enqueue messages at the same time, the underlying container is never corrupted.

(b) Removing a message from the queue
In dequeue(), once a thread wakes up due to sem_wait(), it locks the same mutex before popping from m_messages. 
This ensures that dequeuing can never interleave with an enqueue operation.

The semaphore (m_avail) lets receiver threads sleep until there is actually a message available. 
This prevents busy waiting while still letting the queue grow dynamically.


2. Room
A Room stores a set of member User* objects. Multiple senders may join or leave the same room concurrently, or broadcast messages at the same time. 
All of these operations must read or update the same members set.

Room uses a single mutex (lock) wrapped by a small RAII helper (Guard). 

Three critical sections:
(a) Adding a member
When add_member() inserts a User* into the set, the Guard acquires the lock before insertion. Without the lock, two senders joining simultaneously could corrupt the underlying set.

(b) Removing a member
Similarly, remove_member() must erase from the same shared container, so it also uses Guard to protect that operation.

(c) Broadcasting a message
Broadcasting touches the entire members set as it iterates and enqueues messages for each user. 
While the broadcast code itself doesn’t modify the set, iterating over it while another thread inserts or removes members is unsafe—so it must run inside the same critical section.
One mutex protects the entire room’s membership state, every operation that touches the set uses the same lock, the use of Guard eliminates the risk of forgetting to unlock. 
No deadlocks can occur here because the code never attempts to acquire more than one lock at a time.


3. Server
On the server side, the main synchronization issue arises when multiple clients join rooms simultaneously. 
Since the m_rooms map stores all Room* objects, two threads could end up trying to create the same room at the same time.
I protect room creation/lookup with the server’s mutex (m_lock). The entire logic of checking whether a room exists and creating it when needed is enclosed by a Guard in find_or_create_room().
This includes reading from the m_rooms map, creating a new Room if necessary, and inserting it into the map.
I wrapped both the check and the create in one critical sectio. So the server avoids race conditions where two threads simultaneously think the room does not exist and accidentally create duplicates.


4. Avoiding Synchronization Hazards
1. Race conditions
All shared state has exactly one designated mutex: m_messages is protected by MessageQueue::m_lock, members is protected by Room::lock, and m_rooms is protected by Server::m_lock. 
No shared resource is ever accessed without its associated lock.

2. Deadlocks
The system avoids deadlocks by design: No thread ever holds more than one mutex at a time, a lock is never held while performing a blocking call like sem_wait() or network I/O, and no circular locking order is possible.
