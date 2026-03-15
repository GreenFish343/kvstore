// Pavel Bashlov, pb0826
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

struct Node {
    std::string key;
    std::string value;
    Node* next;
    Node(const std::string& k, const std::string& v)
        : key(k), value(v), next(nullptr) {}
};

class Index {
private:
    Node* head; // pointer to the first node

public:
    Index() : head(nullptr) {}
    ~Index() {
        Node* current = head;
        while (current != nullptr) {
            Node* next = current->next;
            delete current;
            current = next;
        }
    }

    void set(const std::string& key, const std::string& value) {
        Node* current = head; // Walk the list looking for an existing key
        while (current != nullptr) {
            if (current->key == key) {
                current->value = value;
                return;
            }
            current = current->next;
        }

        // Key not found — prepend a new node
        Node* newNode = new Node(key, value);
        newNode->next = head;
        head = newNode;
    }

    // Look up a value by key. Returns a pointer to the value string, or nullptr if not found.
    const std::string* get(const std::string& key) const {
        Node* current = head;
        while (current != nullptr) {
            if (current->key == key) {
                return &current->value;
            }
            current = current->next;
        }
        return nullptr;
    }
};

std::string DB_FILE;  // set dynamically in main()
void appendToLog(const std::string& key, const std::string& value) {
    std::ofstream file(DB_FILE, std::ios::app); // append
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open " << DB_FILE << " for writing.\n";
        return;
    }
    file << "SET " << key << " " << value << "\n";
    file.flush(); // flush immediately so data survives a crash
    file.close();
}

void replayLog(Index& index) {
    std::ifstream file(DB_FILE);
    if (!file.is_open()) {
        return; // if nothing to replay
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string command, key, value;
        ss >> command >> key;

        // The value is everything after "SET <key> "
        if (command == "SET" && !key.empty()) {
            std::getline(ss, value);
            // Remove the leading space before the value
            if (!value.empty() && value[0] == ' ') {
                value = value.substr(1);
            }
            index.set(key, value);
        }
    }
    file.close();
}

// Process a single line of input,  Returns false if the EXIT command was received, true otherwise.
bool processCommand(const std::string& line, Index& index) {
    if (line.empty()) return true;
    std::istringstream ss(line);
    std::string command;
    ss >> command;
    for (char& c : command) c = toupper(c);

    if (command == "SET") {
        std::string key, value;
        ss >> key;

        if (key.empty()) {
            std::cout << "ERROR: SET requires a key and value.\n";
            return true;
        }

        std::getline(ss, value); // Capture everything remaining as the value including spaces
        if (!value.empty() && value[0] == ' ') {
            value = value.substr(1);
        }

        if (value.empty()) {
            std::cout << "ERROR: SET requires a value.\n";
            return true;
        }

        appendToLog(key, value); //Write
        index.set(key, value); //Update
        std::cout << "OK" << std::endl;

    } else if (command == "GET") {
        std::string key;
        ss >> key;

        if (key.empty()) {
            std::cout << "ERROR: GET requires a key.\n";
            return true;
        }

        const std::string* value = index.get(key);
        if (value != nullptr) {
            std::cout << *value << std::endl;
        } else {
            std::cout << "" << std::endl;
        }
    } else if (command == "EXIT") {
        return false; //stop main loop
    } else {
        std::cout << "ERROR: Unknown command '" << command << "'. Use SET, GET, or EXIT." << std::endl;
    }
    return true; // keep running
}

int main(int /*argc*/, char* argv[]) {
    std::filesystem::path exePath = std::filesystem::canonical(argv[0]);
    DB_FILE = (exePath.parent_path() / "data.db").string();
    Index index;
    replayLog(index);
    std::string line;

    while (std::getline(std::cin, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        bool keepRunning = processCommand(line, index);
        if (!keepRunning) break;
    }
    return 0;
}
