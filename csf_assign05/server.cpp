#include <pthread.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <set>
#include <vector>
#include <cctype>
#include <cassert>
#include "message.h"
#include "connection.h"
#include "user.h"
#include "room.h"
#include "guard.h"
#include "server.h"

////////////////////////////////////////////////////////////////////////
// Server implementation data types
////////////////////////////////////////////////////////////////////////

// TODO: add any additional data types that might be helpful
//       for implementing the Server member functions
struct ClientInfo {
  //Used to pass multiple arguments to client handler thread
  Connection *conn;
  Server *server;
  
  ClientInfo(Connection *c, Server *s) : conn(c), server(s) {}
};

////////////////////////////////////////////////////////////////////////
// Client thread functions
////////////////////////////////////////////////////////////////////////

namespace {

// Helper function to validate username/room name
bool is_valid_name(const std::string &name) {
  if (name.empty()) {
    return false;
  }
  for (char c : name) {
    if (!isalnum(c)) {
      return false;
    }
  }
  return true;
}

// Helper function to handle sender client
void chat_with_sender(ClientInfo *info, const std::string &username) {
  Connection *conn = info->conn;
  Server *server = info->server;
  Room *current_room = nullptr;
  
  // Process sender commands
  Message msg;
  while (true) {
    if (!conn->receive(msg)) {
      break; // Connection closed
    }
    
    if (msg.tag == TAG_JOIN) {
      // Validate room name
      if (!is_valid_name(msg.data)) {
        conn->send(Message(TAG_ERR, "invalid room name"));
        continue;
      }
      
      // Leave current room if in one
      if (current_room != nullptr) {
        current_room = nullptr;
      }
      
      // Join new room
      current_room = server->find_or_create_room(msg.data);
      conn->send(Message(TAG_OK, "joined room"));
      
    } else if (msg.tag == TAG_SENDALL) {
      // Check if in a room
      if (current_room == nullptr) {
        conn->send(Message(TAG_ERR, "not in a room"));
        continue;
      }
      
      // Broadcast message to all receivers in room
      current_room->broadcast_message(username, msg.data);
      conn->send(Message(TAG_OK, "message sent"));
      
    } else if (msg.tag == TAG_LEAVE) {
      // Check if in a room
      if (current_room == nullptr) {
        conn->send(Message(TAG_ERR, "not in a room"));
        continue;
      }
      
      // Leave room
      current_room = nullptr;
      conn->send(Message(TAG_OK, "left room"));
      
    } else if (msg.tag == TAG_QUIT) {
      // Send OK and exit
      conn->send(Message(TAG_OK, "bye"));
      break;
      
    } else {
      // Invalid message tag
      conn->send(Message(TAG_ERR, "invalid command"));
    }
  }
}

// Helper function to handle receiver client
void chat_with_receiver(ClientInfo *info, const std::string &username) {
  Connection *conn = info->conn;
  Server *server = info->server;
  User *user = nullptr;
  Room *current_room = nullptr;
  
  // Create User object
  user = new User(username);
  
  // Expect JOIN message
  Message msg;
  if (!conn->receive(msg)) {
    delete user;
    return;
  }
  
  if (msg.tag != TAG_JOIN) {
    conn->send(Message(TAG_ERR, "expected join"));
    delete user;
    return;
  }
  
  // Validate room name
  if (!is_valid_name(msg.data)) {
    conn->send(Message(TAG_ERR, "invalid room name"));
    delete user;
    return;
  }
  
  // Join room
  current_room = server->find_or_create_room(msg.data);
  current_room->add_member(user);
  
  // Send OK response
  if (!conn->send(Message(TAG_OK, "joined room"))) {
    current_room->remove_member(user);
    delete user;
    return;
  }
  
  // Loop delivering messages to receiver
  while (true) {
    Message *delivery_msg = user->mqueue.dequeue();
    
    if (delivery_msg == nullptr) {
      // Timeout, check if connection still alive by trying to send
      continue;
    }
    
    // Try to send message to receiver
    if (!conn->send(*delivery_msg)) {
      // Connection closed, cleanup
      delete delivery_msg;
      break;
    }
    
    delete delivery_msg;
  }
  
  // Cleanup
  if (current_room != nullptr) {
    current_room->remove_member(user);
  }
  delete user;
}

void *worker(void *arg) {
  pthread_detach(pthread_self());

  // TODO: use a static cast to convert arg from a void* to
  //       whatever pointer type describes the object(s) needed
  //       to communicate with a client (sender or receiver)
  ClientInfo *info = static_cast<ClientInfo *>(arg);
  Connection *conn = info->conn;

  // TODO: read login message (should be tagged either with
  //       TAG_SLOGIN or TAG_RLOGIN), send response
  Message login_msg;
  if (!conn->receive(login_msg)) {
    delete info;
    delete conn;
    return nullptr;
  }

  // TODO: depending on whether the client logged in as a sender or
  //       receiver, communicate with the client (implementing
  //       separate helper functions for each of these possibilities
  //       is a good idea)
  if (login_msg.tag == TAG_SLOGIN) {
    // Validate username
    if (!is_valid_name(login_msg.data)) {
      conn->send(Message(TAG_ERR, "invalid username"));
    } else {
      // Handle sender login
      conn->send(Message(TAG_OK, "logged in as sender"));
      chat_with_sender(info, login_msg.data);
    }
  } else if (login_msg.tag == TAG_RLOGIN) {
    // Validate username
    if (!is_valid_name(login_msg.data)) {
      conn->send(Message(TAG_ERR, "invalid username"));
    } else {
      // Handle receiver login
      conn->send(Message(TAG_OK, "logged in as receiver"));
      chat_with_receiver(info, login_msg.data);
    }
  } else {
    // Invalid login tag
    conn->send(Message(TAG_ERR, "invalid login"));
  }

  // Cleanup
  delete conn;
  delete info;

  return nullptr;
}

}

////////////////////////////////////////////////////////////////////////
// Server member function implementation
////////////////////////////////////////////////////////////////////////

Server::Server(int port)
  : m_port(port)
  , m_ssock(-1) {
  // TODO: initialize mutex
  pthread_mutex_init(&m_lock, nullptr);
}

Server::~Server() {
  // TODO: destroy mutex
  pthread_mutex_destroy(&m_lock);
}

bool Server::listen() {
  // TODO: use open_listenfd to create the server socket, return true
  //       if successful, false if not
  m_ssock = open_listenfd(std::to_string(m_port).c_str());
  return m_ssock != -1;
}

void Server::handle_client_requests() {
  // TODO: infinite loop calling accept or Accept, starting a new
  //       pthread for each connected client
  while (true) {
    // Accept a new client connection
    int client_fd = accept(m_ssock, nullptr, nullptr);
    // Handle error if accept fails
    if (client_fd == -1) {
      std::cerr << "Error accepting connection\n";
      continue;
    }
    // Create Connection and ClientInfo objects
    Connection *conn = new Connection(client_fd);
    ClientInfo *info = new ClientInfo(conn, this);

    // Create a new thread to handle the client
    pthread_t thread_id;
    // handle the error if pthread_create fails
    if (pthread_create(&thread_id, nullptr, worker, info) != 0) {
      std::cerr << "Error creating thread\n";
      delete conn;
      delete info;
    }
  }
}

//Critical Section of Synchronized Room Creation
Room *Server::find_or_create_room(const std::string &room_name) {
  // TODO: return a pointer to the unique Room object representing
  //       the named chat room, creating a new one if necessary
  Guard guard(m_lock);
  // check if room exists
  auto it = m_rooms.find(room_name);
  if (it != m_rooms.end()) {
    return it->second; //if so, return existing room
  } else { //if not, create a new room
    Room *new_room = new Room(room_name);
    m_rooms[room_name] = new_room;
    return new_room;
  }
}
