## GROUP MEMBERS

1. Harsh Agrawal - 220425
2. Naman Kumar Jaiswal- 220687
3. Priyanshu Singh 220830

## We have used port 12345 for listening

# Chat Server with Groups & Private Messages

## Description

This project is a multi-threaded chat server designed to facilitate real-time communication through private messages, public broadcasts and group chats. It also includes user authentication to ensure secure access.

---

## Hardware & Software Requirements

The given client code requires the libraries `<unistd.h>` for utility fuctions & `<arpa/inet.h>` for socket programming, both of which are Linux/MacOS reserved. The chat server requires a POSIX compliant system.

## Chat Server Features

Our team has implemented all the features as were required in the Assignment. An Overview of the implemented features are as follows:

### 1. Basic Server Functionality

The chat server is built using **TCP sockets** and supports **multiple concurrent client connections** using **multithreading**. It maintains a list of connected clients along with their usernames.

#### 1.1 TCP-Based Server

- The server listens on port 8080 for incoming client connections.
- It accepts client connections using the `accept()` system call.
- A dedicated thread is spawned for each client to handle their messages independently.

#### 1.2 Handling Multiple Clients

- The server supports multiple concurrent client connections by using multithreading (`std::thread`).
- Each client connection runs in its own thread, allowing simultaneous messaging.
- Client data is protected using mutex locks (`std::mutex`) to prevent race conditions.

#### 1.3 Maintaining Active Clients

- The server maintains a map of active clients, mapping client socket descriptors to their corresponding usernames.
- This allows the server to efficiently send messages to specific users or broadcast messages to all connected users.

---

### 2. User Authentication

To ensure that only authorized users can connect, the server is loaded with an username-password authentication system.

#### 2.1 Credentials Storage

- The server has the access to the users.txt file which contains the username-password pairs for each valid user in the `"username":"password"` format, each pair separated by an "\n" (newline character).
- This text file is immediately read upon Server Initialisation into an Unordered Map. This allows quick lookup and validation during user authentication.

#### 2.2 Authentication Process

- When a client connects, the server prompts them for their username and password, upon receiving the credentials from the client side, if they match an entry in the Unordered Map, authentication succeeds, and the client is allowed to join.

#### 2.3 Failed Authentication

- If authentication fails (invalid username or incorrect password), the server immediately disconnects the client. Whether the Authentication attempt succeeds or not is immediately conveyed to the client.

---

### 3. Messaging Features

Clients can send and receive messages in three different ways. All the three types of message receive is implemented on another thread which makes it run in the background and not block any other feature. The three ways are:

#### 3.1 Broadcast Messages

These are sent to all the people currently connected to the server. The user can send them using
`/broadcast <message>`.

#### 3.2 Private Messages

- Clients can send a private message to a specific user using their username using
  `/msg <username> <message>`. The server looks up the recipient in the active clients map and forwards the message to the appropriate socket number.
- If the user is not found (offline), nothing happens. Both parties need to be online (connected to the server) for efficient communication.

#### 3.3 Group Messages

- Clients can send messages to a specific group using `/group msg <group_name> <message> ` command. This requires the group to exist and the user to be a part of the group. If the conditions satisfy, message is sent, else apt reply is given from the server. This is implemented using a group mapping.

#### Receiving messages

- The user will actively be looking for the receiving messages and will be displayed on the terminal. This is done using a parallel thread.

---

### 4. Group Management

The server has all the following Group Commands Supported.

#### 4.1 Creating a Group

- Any client can create a new group using: `/create_group <group_name>`
- The server checks if the group already exists:
  - If yes, it notifies the client that the group already exists.
  - If no, the server creates the group and adds the creator as the first member.
- The group name must not contain a space.

#### 4.2 Joining a Group

- Clients can join an existing group using: `/join_group <group_name>`
- The server verifies if the group exists:
  - If no, an error message is sent.
  - If yes, the user is added to the group and a notification is sent to all active group members.
- Any client is able to create a group.

#### 4.3 Leaving a Group

- Clients can leave a group using: `/leave_group <group_name>`
- The server removes the user from the group's membership list.
- If the user was not a member, an error message is sent.

#### 4.4 Maintaining Group Memberships

- The server keeps track of **groups and their members** using a **mapping of group names to sets of client sockets**.
- This allows efficient message forwarding within groups with very efficient lookup time.

---

### 5. Command Processing & Execution

The server efficiently processes and executes commands sent by clients using an efficient command parser which segregates them into appropriate responses.

#### 5.1 Commands Implemented

- All the necessary commands like `/msg`, `/broadcast`, `/join_group`, `/group_msg` and `/leave_group` have been implemented.

#### 5.2 Handling Invalid Commands

- If a user sends an unrecognized command, the server responds with:
  ```
  Invalid command
  ```
- This prevents unintended behavior and provides better user feedback.

---

## Design Decisions

### 1. Multi-Threaded Connection Handling

- Decision: We opted for a thread-per-connection model, where each client connection is handled by a separate thread.
- **Reason**:
  - **Simplicity and Responsiveness:** Threads are lightweight compared to processes, allowing for concurrent handling of multiple clients without the overhead of creating separate processes.
  - **Ease of Synchronization:** Sharing data between threads (e.g., active clients and group mappings) is simpler using shared memory. We use `std::mutex` to protect these shared resources, ensuring that operations like adding/removing clients or groups remain consistent.
  - **Scalability Considerations:** A thread-per-connection model strikes a good balance between performance and implementation simplicity for this assignment.

### 2. Command Parsing and Design Choice for Messaging Syntax

- Decision: We implemented a Command Parser which identifies the command and segregates the command using string parsing techniques into the apt response and correct Arguments to the server endpoint. Furthermore, the following set of commands are implemented to handle different messaging features:

  - **Broadcast Messaging:** `/broadcast <message>`
  - **Private Messaging:** `/msg <username> <message>`
  - **Group Creation, Joining, and Messaging:**
    - Create a group: `/create_group <group_name>`
    - Join a group: `/join_group <group_name>`
    - Send a group message: `/group_msg <group_name> <message>`
    - Leave a group: `/leave_group <group_name>`

- **Reason:**
  - **Ease of Use:** Instead of making separate server API endpoints for each command, we have implemented everything into a single handleClient() endpoint. For this we required an efficient string parser and categorizing into apt response which the server should give. This makes the server efficient and light weight.
  - **Clarity and Consistency:** Using clear and distinct command keywords (like `/msg`, `/broadcast`, etc.) simplifies the parsing process on the server side.

### 3. Synchronization Mechanisms

- Decision: Shared data structures, such as the active clients map and group memberships, are protected using `std::mutex` (with lock guards for RAII).
- **Reason:**
  - **Preventing Data Races:** Multiple client threads can concurrently access and modify these shared resources, so synchronization is critical to maintain data consistency.
  - **Simplicity in Management:** By using `std::mutex` and `std::lock_guard`, we ensure that locks are properly released even if an exception occurs, reducing the risk of deadlocks and resource leaks.
  - **Safe Data Sharing** Shared data structures (active clients, groups) are modified in a **thread-safe manner**. While these structures are being modified another thread can not access them.

### 4. Storing Session Data

- Decision: Storing the Session data like, usernames and password, active clients, groups using Unordered maps.
- **Reason:**
  - **Efficient Data Retrieval:** Using unordered maps allows for constant average-time complexity (O(1)) for lookups, insertions, and deletions. This efficiency is crucial when managing dynamic data such as active clients and group memberships in real-time.

---

## Implementation

### High-Level Overview of Important Functions

- **`Read_UsernameAndPasswords()`**

  - **Purpose:** Reads user credentials from a file (`users.txt`) and populates an in-memory mapping of usernames to passwords.
  - **Key Steps:**
    - Opens the file and reads each line.
    - Parses each line using a delimiter (colon `:`) to separate the username and password.
    - Stores the cleaned data in the `user_password_map` for quick authentication.

- **`start_listening()`**

  - **Purpose:** Sets up the server socket for listening to incoming client connections.
  - **Key Steps:**
    - Creates a socket with IPv4 and TCP settings.
    - Binds the socket to the specified port (8080) on all available network interfaces.
    - Puts the socket into listening mode with a defined backlog, allowing the server to accept multiple connection requests.

- **`Accept_And_Authenticate_Client()`**

  - **Purpose:** Accepts new client connections and authenticates them before allowing further communication.
  - **Key Steps:**
    - Uses the `accept()` call to wait for an incoming client.
    - Prompts the client for a username and password.
    - Checks the credentials against the `user_password_map`.
    - On successful authentication:
      - Notifies the client and all other active users.
      - Spawns a new thread to handle the client's session using the `handleClient()` function.
    - On failure:
      - Sends an appropriate error message and closes the connection.

- **`handleClient()`**
  - **Purpose:** Processes all messages from an authenticated client.
  - **Key Steps:**
    - Sends a welcome message upon connection.
    - Enters a loop to continuously read commands from the client.
    - Uses command parsing (based on prefixes such as `/broadcast`, `/msg`, `/create_group`, etc.) to determine which action to perform.
    - Ensures thread-safe operations when accessing or modifying shared data structures (active clients, groups) by locking the appropriate mutexes.
    - Cleans up by removing the client from the active clients map upon disconnection.

---

## Code Flow

![Server Architecture Diagram](https://drive.google.com/uc?export=view&id=1c9_6X2Dkd4UWAD1xAsA2ate9W3GuLNmt)

### 1. Initialization (main):

- **Read Credentials:**  
  The `Read_UsernameAndPasswords()` function loads all valid username/password pairs from `users.txt` into the `user_password_map`.

- **Start Listening:**  
  The `start_listening()` function creates a TCP socket, binds it to port 8080, and begins listening for incoming connections.

### 2. Accepting and Authenticating Clients:

- **Connection Acceptance:**  
  In an infinite loop, `Accept_And_Authenticate_Client()` waits for a client to connect using the `accept()` system call.

- **Authentication:**  
  Once a connection is accepted, the server prompts the client for credentials. The provided username and password are validated against the pre-loaded `user_password_map`.

- **Spawning Threads:**  
  If authentication is successful, the client is added to the active clients map and a new thread is spawned to handle the client using `handleClient()`. Otherwise, the connection is closed with an appropriate error message.

### 3. Handling Client Communication:

- **Welcome Message:**  
  The `handleClient()` function sends an initial welcome message to the connected client.

- **Command Processing:**  
  The function then enters a loop where it continuously waits for messages from the client. Based on the command prefix (e.g., `/broadcast`, `/msg`, `/create_group`, etc.), the server performs the corresponding action.

- **Thread Safety:**  
  Access to shared resources (like active clients and group mappings) is synchronized using mutexes (`std::mutex`) to prevent race conditions.

- **Client Disconnection:**  
  When a client disconnects or an error occurs during message reception, the client is removed from the active clients map, and the thread handling that client terminates.

---

## Testing

### Correctness Testing

To ensure the chat server functions as intended, we devised and executed a comprehensive series of tests covering various scenarios. Our correctness testing focused on verifying that each feature operates correctly under both typical and edge-case conditions.

#### Authentication Testing

- **Valid Credentials:**  
  Tested with valid username and password pairs from `users.txt` to confirm successful authentication.
- **Invalid Credentials:**  
  Verified that incorrect passwords or non-existent usernames result in proper error messages and immediate disconnection.

#### Message Command Testing

- **Broadcast Messages:**
  - **Test Case:** A client sends a message using `/broadcast <message>`.
  - **Expected Result:** All connected clients (except the sender) receive the broadcasted message.
- **Private Messages:**
  - **Test Case:** A client sends a message using `/msg <username> <message>`.
  - **Expected Result:** Only the specified recipient receives the message, and the sender is notified if the user is not connected.
- **Group Messaging:**
  - **Test Case:** A client sends a group message using `/group_msg <group_name> <message>`.
  - **Expected Result:** All members of the specified group receive the message, provided that the sender is a member of that group.
- **Invalid Commands:**
  - **Test Case:** The client sends an unrecognized or improperly formatted command.
  - **Expected Result:** The server responds with an "Invalid command" message, ensuring no unintended behavior occurs.

#### Group Management Testing

- **Creating Groups:**
  - **Test Case:** Use `/create_group <group_name>` to create a new group.
  - **Expected Result:** The group is created and the creator is added as the first member; duplicate group names or names with spaces are rejected.
- **Joining Groups:**
  - **Test Case:** Use `/join_group <group_name>` for joining an existing group.
  - **Expected Result:** The client is added to the group and a notification is sent to existing group members.
- **Leaving Groups:**
  - **Test Case:** Use `/leave_group <group_name>` to leave a group.
  - **Expected Result:** The client is removed from the group’s membership list, and a confirmation message is sent.

#### Concurrency Testing

- **Simultaneous Client Connections:**
  - **Test Case:** Multiple clients connect concurrently and perform different operations (authentication, messaging, group operations) simultaneously.
  - **Expected Result:** The server manages these concurrent operations without data races or deadlocks, maintaining data consistency through proper synchronization.
- **Stress Testing:**
  - Simulated heavy load scenarios to verify that the server can handle a high number of concurrent connections and message exchanges.

Each of these tests was executed to validate the correctness of the server's functionality, ensuring that all components work together seamlessly under various conditions.

## Restrictions in the Server

- Maximum Clients: No explicit limit, but constrained by system resources and listen(server_fd, 10), which queues up to 10 pending connections.
- Maximum Groups: No explicit limit, but constrained by unordered_map<string, set<int>> groups;. Limited by available memory.
- Maximum Members per Group: No explicit limit, but effectively constrained by std::set<int>, which stores client sockets per group.
- Maximum Message Size: 1024 bytes (defined by #define BUFFER_SIZE 1024 and used in char buffer[BUFFER_SIZE];).

---

## Challenges

### Challenge 1: Handling Multiple Concurrent Client Connections

**Problem:**  
 The first problem faced was how to connect multiple clients to the server and keep listening for new connections simultaneously.

**Solution:**  
 This was solved by listening to the incoming connections on the server in a loop and then simultaneously creating a new thread for the server to handle the client by calling the `handleClient` function on the new thread and passing the client socket to it as an argument. After that, the server can continue to listen for incoming connections on the main thread.

### Challenge 2: Managing Asynchronous Message Communication

**Problem:**  
 Based on the class lecture examples, the send and receive messages in the server and clients had to be written in pair (i.e., if there is a send command in the server there should be a receive command in the client and vice versa). However, in the chat application, there is no definite order of messages because at any time the server might be sending someone else's message to the client or the client might be sending a message to the server.

**Solution:**  
 We got a hint from the provided client code itself, where once the client code gets authenticated, it starts a new thread that listens for incoming messages from the server and prints them, while the main thread of the client is used to send messages to the server whenever a message is typed by the user. We used the same approach in the server code as well, where we created a new thread for each client to handle the incoming messages from that client. This dedicated thread processes the received commands and performs operations accordingly, sending the message to other clients, to a specific client, or to a group of clients by obtaining the respective client socket from the `activeClients` map.

---

### Further Improvements

While the current implementation meets the assignment requirements, there are several enhancements that could be made to evolve this project into a practical, real-world chat server:

1. Encryption and Decryption

   - Current Limitation:
     - All messages and authentication data (usernames and passwords) are transmitted in plain text, which poses a security risk.
   - Potential Improvement:
     - Implement end-to-end encryption using protocols such as TLS/SSL to secure all communication between clients and the server.
     - Utilize modern cryptographic libraries (e.g., OpenSSL) to handle encryption/decryption, ensuring that sensitive data remains secure during transit.

2. Persistent Message Storage
   - Current Limitation:
     - The server does not store messages, meaning that chat history is lost upon server restart or user disconnection.
   - Potential Improvement:
     - Integrate a database (such as MySQL, PostgreSQL, or even a NoSQL option like MongoDB) to persist chat messages.
     - Implement message retrieval features so that users can view past conversations even after long periods, which is essential for a practical, real-world chat application.

---

## Team Contribution

Team Members: Harsh Agrawal (220425), Naman Kumar Jaiswal (220687) and Priyanshu Singh (220830)

Contributions: The project was a collaborative effort, with each team member contributing equally three folds across all aspects.

---

## Sources Referred

- [Neural Nine](https://www.youtube.com/watch?v=3UOyky9sEQY&t=247s)
- [CS425 Github Repository](https://github.com/privacy-iitk/cs425-2025)

---

## Declaration

We hereby declare that this assignment was completed independently and did not involve plagiarism of any form.

---

## Feedback

It was a great learning experience!
