#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"

Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
}

Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
  // TODO: call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, m_fd);
}

void Connection::connect(const std::string &hostname, int port) {
  // TODO: call open_clientfd to connect to the server
  // TODO: call rio_readinitb to initialize the rio_t object
  // Convert port to string for open_clientfd
  std::string port_str = std::to_string(port);
  
  // Connect to the server
  m_fd = open_clientfd(hostname.c_str(), port_str.c_str());
  
  if (m_fd < 0) {
    m_last_result = (m_fd == -2) ? INVALID_MSG : EOF_OR_ERROR;
    return;
  }
  
  // Initialize the rio_t object for buffered reading
  rio_readinitb(&m_fdbuf, m_fd);
  m_last_result = SUCCESS;
}

Connection::~Connection() {
  // TODO: close the socket if it is open
  if (is_open()) {
    Close(m_fd);
  }
}

bool Connection::is_open() const {
  // TODO: return true if the connection is open
  return m_fd >= 0;
}

void Connection::close() {
  // TODO: close the connection if it is open
  if (is_open()) {
    Close(m_fd);
    m_fd = -1;
  }
}

bool Connection::send(const Message &msg) {
  // TODO: send a message
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  if (!is_open()) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  
  // Format the message as "tag:data\n"
  std::string formatted = msg.tag + ":" + msg.data + "\n";
  
  // Send the message
  ssize_t bytes_written = rio_writen(m_fd, formatted.c_str(), formatted.length());
  
  if (bytes_written < 0 || (size_t)bytes_written != formatted.length()) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  
  m_last_result = SUCCESS;
  return true;
}

bool Connection::receive(Message &msg) {
  // TODO: receive a message, storing its tag and data in msg
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  if (!is_open()) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  
  char buf[Message::MAX_LEN + 1];
  
  // Read a line from the connection
  ssize_t n = rio_readlineb(&m_fdbuf, buf, sizeof(buf));
  
  if (n < 0) {
    m_last_result = EOF_OR_ERROR;
    return false;
  } else if (n == 0) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  
  // Remove the newline character if present
  if (n > 0 && buf[n-1] == '\n') {
    buf[n-1] = '\0';
  }
  
  // Parse the message (format: "tag:data")
  std::string line(buf);
  size_t colon_pos = line.find(':');
  
  if (colon_pos == std::string::npos) {
    m_last_result = INVALID_MSG;
    return false;
  }
  
  // Extract tag and data
  msg.tag = line.substr(0, colon_pos);
  msg.data = line.substr(colon_pos + 1);
  
  m_last_result = SUCCESS;
  return true;
}
