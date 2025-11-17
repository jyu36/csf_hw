#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  std::string server_hostname;
  int server_port;
  std::string username;

  server_hostname = argv[1];
  server_port = std::stoi(argv[2]);
  username = argv[3];

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

  // Send slogin message
  Message slogin_msg(TAG_SLOGIN, username);
  if (!conn.send(slogin_msg)) {
    std::cerr << "Failed to send slogin message\n";
    return 1;
  }

  // Receive response to slogin
  Message response;
  if (!conn.receive(response)) {
    std::cerr << "Failed to receive slogin response\n";
    return 1;
  }

  // Check if slogin was successful
  if (response.tag == TAG_ERR) {
    std::cerr << response.data << "\n";
    return 1;
  }
  
  if (response.tag != TAG_OK) {
    std::cerr << "Unexpected response to slogin\n";
    return 1;
  }

  // Loop reading commands from user
  std::string line;
  while (std::getline(std::cin, line)) {
    // Trim the line
    line = trim(line);
    // Skip empty lines
    if (line.empty()) {
      continue;
    }
    // Check if it's a command (starts with '/')
    if (line[0] == '/') {
      // Parse the command
      std::istringstream iss(line);
      std::string command;
      iss >> command;

      if (command == "/join") {
        // Extract room name
        std::string room_name;
        iss >> room_name;
        
        if (room_name.empty()) {
          std::cerr << "Error: /join requires a room name\n";
          continue;
        }

        // Send join message
        Message join_msg(TAG_JOIN, room_name);
        if (!conn.send(join_msg)) {
          std::cerr << "Failed to send join message\n";
          return 1;
        }

        // Wait for response
        if (!conn.receive(response)) {
          std::cerr << "Failed to receive join response\n";
          return 1;
        }

        // Check response
        if (response.tag == TAG_ERR) {
          std::cerr << response.data << "\n";
        } else if (response.tag != TAG_OK) {
          std::cerr << "Unexpected response to join\n";
        }

      } else if (command == "/leave") {
        // Send leave message (payload is ignored but must be present)
        Message leave_msg(TAG_LEAVE, "");
        if (!conn.send(leave_msg)) {
          std::cerr << "Failed to send leave message\n";
          return 1;
        }

        // Wait for response
        if (!conn.receive(response)) {
          std::cerr << "Failed to receive leave response\n";
          return 1;
        }

        // Check response
        if (response.tag == TAG_ERR) {
          std::cerr << response.data << "\n";
        } else if (response.tag != TAG_OK) {
          std::cerr << "Unexpected response to leave\n";
        }

      } else if (command == "/quit") {
        // Send quit message (payload is ignored but must be present)
        Message quit_msg(TAG_QUIT, "");
        if (!conn.send(quit_msg)) {
          std::cerr << "Failed to send quit message\n";
          return 1;
        }

        // Wait for response
        if (!conn.receive(response)) {
          std::cerr << "Failed to receive quit response\n";
          return 1;
        }

        // Check response
        if (response.tag == TAG_ERR) {
          std::cerr << response.data << "\n";
        }
        
        // Exit with code 0 after quit
        return 0;

      } else {
        // Unrecognized command
        std::cerr << "Error: unrecognized command\n";
      }

    } else {
      // Regular message - send as sendall
      Message sendall_msg(TAG_SENDALL, line);
      if (!conn.send(sendall_msg)) {
        std::cerr << "Failed to send message\n";
        return 1;
      }

      // Wait for response
      if (!conn.receive(response)) {
        std::cerr << "Failed to receive sendall response\n";
        return 1;
      }

      // Check response
      if (response.tag == TAG_ERR) {
        std::cerr << response.data << "\n";
      } else if (response.tag != TAG_OK) {
        std::cerr << "Unexpected response to sendall\n";
      }
    }
  }

  return 0;
}
