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

void *worker(void *arg) {
  pthread_detach(pthread_self());

  // TODO: use a static cast to convert arg from a void* to
  //       whatever pointer type describes the object(s) needed
  //       to communicate with a client (sender or receiver)
  ClientInfo *info = static_cast<ClientInfo *>(arg);
  Connection *conn = info->conn;
  Server *server = info->server;

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
    // Handle sender login
    conn->send(Message(TAG_OK, "logged in as sender"));
    chat_with_sender(info); //helper function to handle sender
  } else if (login_msg.tag == TAG_RLOGIN) {
    // Handle receiver login
    // (Implementation of receiver handling would go here)
    conn->send(Message(TAG_OK, "logged in as receiver"));
    chat_with_receiver(info); //helper function to handle receiver
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
