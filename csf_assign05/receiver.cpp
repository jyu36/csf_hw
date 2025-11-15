#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 5) {
    std::cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }

  std::string server_hostname = argv[1];
  int server_port = std::stoi(argv[2]);
  std::string username = argv[3];
  std::string room_name = argv[4];

  Connection conn;

  // Connect to server
  try {
    conn.connect(server_hostname, server_port);
  } catch (const std::exception &e) {
    std::cerr << "Failed to connect to server\n";
    return 1;
  }
  
  if (!conn.is_open()) {
    std::cerr << "Failed to connect to server\n";
    return 1;
  }

  // Send rlogin message
  Message rlogin_msg(TAG_RLOGIN, username);
  if (!conn.send(rlogin_msg)) {
    std::cerr << "Failed to send rlogin message\n";
    return 1;
  }

  // Receive response to rlogin
  Message response;
  if (!conn.receive(response)) {
    std::cerr << "Failed to receive rlogin response\n";
    return 1;
  }

  // Check if rlogin was successful
  if (response.tag == TAG_ERR) {
    std::cerr << response.data << "\n";
    return 1;
  }
  
  if (response.tag != TAG_OK) {
    std::cerr << "Unexpected response to rlogin\n";
    return 1;
  }

  // Send join message
  Message join_msg(TAG_JOIN, room_name);
  if (!conn.send(join_msg)) {
    std::cerr << "Failed to send join message\n";
    return 1;
  }

  // Receive response to join
  if (!conn.receive(response)) {
    std::cerr << "Failed to receive join response\n";
    return 1;
  }

  // Check if join was successful
  if (response.tag == TAG_ERR) {
    std::cerr << response.data << "\n";
    return 1;
  }
  
  if (response.tag != TAG_OK) {
    std::cerr << "Unexpected response to join\n";
    return 1;
  }

  // Loop waiting for messages from server
  // The loop continues until connection is closed (Ctrl-C or server disconnect)
  while (true) {
    Message msg;
    if (!conn.receive(msg)) {
      // Connection closed or error occurred
      // This is expected when user hits Ctrl-C
      break;
    }

    if (msg.tag == TAG_DELIVERY) {
      // Parse delivery message: delivery:[room]:[sender]:[message]
      // The data field contains: [room]:[sender]:[message]
      
      // Find the first colon (separates room from rest)
      size_t first_colon = msg.data.find(':');
      if (first_colon == std::string::npos) {
        std::cerr << "Invalid delivery message format\n";
        continue;
      }
      
      // Find the second colon (separates sender from message)
      size_t second_colon = msg.data.find(':', first_colon + 1);
      if (second_colon == std::string::npos) {
        std::cerr << "Invalid delivery message format\n";
        continue;
      }
      
      // Extract sender and message text
      std::string sender = msg.data.substr(first_colon + 1, second_colon - first_colon - 1);
      std::string message_text = msg.data.substr(second_colon + 1);
      
      // Print in required format
      std::cout << sender << ": " << message_text << "\n";
      std::cout.flush(); // Ensure output is displayed immediately
      
    } else if (msg.tag == TAG_ERR) {
      // Handle error messages from server - print exact payload
      std::cerr << msg.data << "\n";
      return 1;
    }
  }
  return 0;
}
