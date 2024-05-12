#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdlib> 

using namespace std;

class HashTable
{
private:

    static const int TABLE_SIZE = 100;

    struct Node
    {
        string key;
        string value;
        Node* next;
        Node(const string& key, const string& value) : key(key), value(value), next(nullptr) {}
    };
    Node* table[TABLE_SIZE];

    int hash(const string& key)
    {
        int hashValue = 0;
        for (char c : key)
        {
            hashValue += c;
        }
        return hashValue % TABLE_SIZE;
    }

public:
    HashTable()
    {
        for (int i = 0; i < TABLE_SIZE; ++i)
        {
            table[i] = nullptr;
        }
    }

    void insert(const string& key, const string& value)
    {
        int index = hash(key);
        Node* newNode = new Node(key, value);
        newNode->next = table[index];
        table[index] = newNode;
    }

    string* find(const string& key)
    {
        int index = hash(key);
        Node* curr = table[index];
        while (curr != nullptr)
        {
            if (curr->key == key)
            {
                return &(curr->value);
            }
            curr = curr->next;
        }
        return nullptr;
    }

    void saveToFile(const string& filename)
    {
        ofstream file(filename);
        if (file.is_open()) {
            for (int i = 0; i < TABLE_SIZE; ++i)
            {
                Node* curr = table[i];
                while (curr != nullptr)
                {
                    file << curr->key << "," << curr->value << "\n";
                    curr = curr->next;
                }
            }
            file.close();
        }
        else {
            cout << "Unable to open file: " << filename << endl;
        }
    }

    void loadFromFile(const string& filename)
    {
        ifstream file(filename);
        if (file.is_open())
        {
            string line;
            while (getline(file, line))
            {
                stringstream ss(line);
                string key, value;
                getline(ss, key, ',');
                getline(ss, value);
                insert(key, value);
            }
            file.close();
        }
        else {
            cout << "Unable to open file: " << filename << endl;
        }
    }
};

class UserDatabase
{
private:
    HashTable users;
    const string userFilename = "users.txt"; // File to store user information

public:
    UserDatabase() {
        loadUsersFromFile(); // Load user information when the program starts
    }

    bool registerUser(const string& username, const string& password) {
        if (users.find(username) == nullptr) {
            users.insert(username, password);
            saveUsersToFile(); // Save user information after registration
            return true; // Registration successful
        }
        return false; // Username already exists
    }

    bool loginUser(const string& username, const string& password)
    {
        string* storedPassword = users.find(username);
        if (storedPassword != nullptr && *storedPassword == password)
        {
            cout << "Login successful. Welcome, " << username << "!" << endl;
            return true;
        }
        cout << "Invalid username or password." << endl;
        return false;
    }

    void saveUsersToFile()
    {
        users.saveToFile(userFilename);
    }

    void loadUsersFromFile()
    {
        users.loadFromFile(userFilename);
    }
};

class RepositoryManager
{
private:
    struct RepositoryNode
    {
        string repositoryName;
        bool isPublic;
        vector<string> files;
        vector<pair<string, vector<string>>> commitHistory; // (Commit Message, Changed Files)
        RepositoryNode* left;
        RepositoryNode* right;

        RepositoryNode(string name, bool visibility) : repositoryName(name), isPublic(visibility), left(nullptr), right(nullptr) {}
    };
    RepositoryNode* root;

    void deleteTree(RepositoryNode* node)
    {
        if (node == nullptr)
            return;
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }

public:
    RepositoryManager() : root(nullptr) {}

    void createRepository(const string& name, bool isPublic)
    {
        root = insert(root, name, isPublic);
    }

    RepositoryNode* insert(RepositoryNode* node, const string& name, bool isPublic)
    {
        if (node == nullptr)
        {
            return new RepositoryNode(name, isPublic);
        }
        if (name < node->repositoryName)
        {
            node->left = insert(node->left, name, isPublic);
        }
        else if (name > node->repositoryName)
        {
            node->right = insert(node->right, name, isPublic);
        }
        return node;
    }

    void displayRepositories() {
        displayInOrder(root);
    }

    void displayInOrder(RepositoryNode* node) {
        if (node == nullptr)
            return;
        displayInOrder(node->left);
        cout << "Repository Name: " << node->repositoryName << ", Visibility: " << (node->isPublic ? "Public" : "Private") << endl;
        displayInOrder(node->right);
    }

    void deleteRepository(const string& name) {
        root = deleteNode(root, name);
    }

    RepositoryNode* deleteNode(RepositoryNode* node, const string& name) {
        if (node == nullptr)
            return nullptr;
        if (name < node->repositoryName)
            node->left = deleteNode(node->left, name);
        else if (name > node->repositoryName)
            node->right = deleteNode(node->right, name);
        else {
            if (node->left == nullptr) {
                RepositoryNode* temp = node->right;
                delete node;
                return temp;
            }
            else if (node->right == nullptr) {
                RepositoryNode* temp = node->left;
                delete node;
                return temp;
            }
            RepositoryNode* temp = minValueNode(node->right);
            node->repositoryName = temp->repositoryName;
            node->right = deleteNode(node->right, temp->repositoryName);
        }
        return node;
    }

    RepositoryNode* minValueNode(RepositoryNode* node) {
        RepositoryNode* current = node;
        while (current && current->left != nullptr)
            current = current->left;
        return current;
    }

    void forkRepository(const string& sourceRepo, const string& destinationRepo) {
        RepositoryNode* source = search(root, sourceRepo);
        if (source != nullptr) {
            root = insert(root, destinationRepo, source->isPublic);
            root->files = source->files;
            root->commitHistory = source->commitHistory;
        }
        else {
            cout << "Source repository not found." << endl;
        }
    }

    void addCommit(const string& repoName, const string& commitMessage, const vector<string>& changedFiles) {
        RepositoryNode* repo = search(root, repoName);
        if (repo != nullptr) {
            repo->commitHistory.push_back(make_pair(commitMessage, changedFiles));
        }
        else {
            cout << "Repository not found." << endl;
        }
    }

    void viewRepositoryStats(const string& repoName) {
        RepositoryNode* repo = search(root, repoName);
        if (repo != nullptr) {
            cout << "Repository Name: " << repo->repositoryName << endl;
            cout << "Visibility: " << (repo->isPublic ? "Public" : "Private") << endl;
            cout << "Files: ";
            for (const auto& file : repo->files) {
                cout << file << ", ";
            }
            cout << endl;
            cout << "Commit History: " << endl;
            for (const auto& commit : repo->commitHistory) {
                cout << "Commit Message: " << commit.first << ", Files Changed: ";
                for (const auto& file : commit.second) {
                    cout << file << ", ";
                }
                cout << endl;
            }
        }
        else {
            cout << "Repository not found." << endl;
        }
    }

    RepositoryNode* search(RepositoryNode* node, const string& name) {
        if (node == nullptr || node->repositoryName == name)
            return node;
        if (name < node->repositoryName)
            return search(node->left, name);
        return search(node->right, name);
    }

    void addFile(const string& repoName, const string& fileName) {
        RepositoryNode* repo = search(root, repoName);
        if (repo != nullptr) {
            repo->files.push_back(fileName);
        }
        else {
            cout << "Repository not found." << endl;
        }
    }

    void deleteFile(const string& repoName, const string& fileName) {
        RepositoryNode* repo = search(root, repoName);
        if (repo != nullptr) {
            bool found = false;
            for (auto it = repo->files.begin(); it != repo->files.end(); ++it) {
                if (*it == fileName) {
                    repo->files.erase(it);
                    found = true;
                    break;
                }
            }
            if (!found) {
                cout << "File not found in repository." << endl;
            }
        }
        else {
            cout << "Repository not found." << endl;
        }
    }

};

class User {
private:
    string username;
    string password;

public:
    User(const string& username, const string& password)
        : username(username), password(password) {}

    const string& getUsername() const {
        return username;
    }

    bool validatePassword(const string& inputPassword) const {
        return password == inputPassword;
    }
};

class Graph {
private:
    static const int MAX_USERS = 1000;
    bool adjMatrix[MAX_USERS][MAX_USERS];

public:
    Graph() {
        for (int i = 0; i < MAX_USERS; ++i) {
            for (int j = 0; j < MAX_USERS; ++j) {
                adjMatrix[i][j] = false;
            }
        }
    }

    void addEdge(int user1, int user2) {
        adjMatrix[user1][user2] = true;
        adjMatrix[user2][user1] = true;
    }

    void removeEdge(int user1, int user2) {
        adjMatrix[user1][user2] = false;
        adjMatrix[user2][user1] = false;
    }

    void displayFollowedUsers(int user) {
        cout << "Users followed by User " << user << ": ";
        for (int i = 0; i < MAX_USERS; ++i) {
            if (adjMatrix[user][i]) {
                cout << i << " ";
            }
        }
        cout << endl;
    }

    void followUser(int follower, int followee) {
        adjMatrix[follower][followee] = true;
    }

    void unfollowUser(int follower, int followee) {
        adjMatrix[follower][followee] = false;
    }
};



int main()
{
    UserDatabase userDB;
    RepositoryManager repoManager;
    Graph socialGraph;
    vector<User> loggedInUsers;

    int choice;
    do {
        system("cls");
        cout << "\n1. Register\n";
        cout << "2. Login\n";
        cout << "3. Create Repository\n";
        cout << "4. Display Repositories\n";
        cout << "5. Delete Repository\n";
        cout << "6. Fork Repository\n";
        cout << "7. Add Commit\n";
        cout << "8. View Repository Stats\n";
        cout << "9. Follow User\n";
        cout << "10. Unfollow User\n";
        cout << "11. Display Followed Users\n";
        cout << "12. Add File to Repository\n";
        cout << "13. Delete File from Repository\n";
        cout << "14. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
        {
            cin.ignore();
            cout << "Enter username for registration: ";
            string regUsername;
            getline(cin, regUsername);
            cout << "Enter password for registration: ";
            string regPassword;
            getline(cin, regPassword);
            if (userDB.registerUser(regUsername, regPassword))
            {
                cout << "Registration successful." << endl;
            }
            else
            {
                cout << "Username already exists." << endl;
            }
            break;
        }
        case 2:
        {
            cin.ignore();
            cout << "Enter username for login: ";
            string loginUsername;
            getline(cin, loginUsername);
            cout << "Enter password for login: ";
            string loginPassword;
            getline(cin, loginPassword);
            if (userDB.loginUser(loginUsername, loginPassword))
            {
                loggedInUsers.push_back(User(loginUsername, loginPassword));
            }
            break;
        }
        case 3:
        {
            if (loggedInUsers.empty())
            {
                cout << "Please login first to create a repository." << endl;
            }
            else
            {
                cin.ignore();
                cout << "Enter repository name: ";
                string repoName;
                getline(cin, repoName);
                bool isPublic;
                cout << "Set visibility (1 for Public, 0 for Private): ";
                cin >> isPublic;
                repoManager.createRepository(repoName, isPublic);
                cout << "Repository created successfully!" << endl;
            }
            break;
        }
        case 4: {
            cout << "All Repositories:" << endl;
            repoManager.displayRepositories();
            break;
        }
        case 5: {
            if (loggedInUsers.empty()) {
                cout << "Please login first to delete a repository." << endl;
            }
            else {
                cin.ignore();
                cout << "Enter the name of the repository you want to delete: ";
                string repoToDelete;
                getline(cin, repoToDelete);
                repoManager.deleteRepository(repoToDelete);
                cout << "Repository deleted successfully!" << endl;
            }
            break;
        }
        case 6: {
            if (loggedInUsers.empty()) {
                cout << "Please login first to fork a repository." << endl;
            }
            else {
                cin.ignore();
                cout << "Enter the name of the repository you want to fork: ";
                string sourceRepo;
                getline(cin, sourceRepo);
                cout << "Enter the name of your forked repository: ";
                string destinationRepo;
                getline(cin, destinationRepo);
                repoManager.forkRepository(sourceRepo, destinationRepo);
                cout << "Repository forked successfully!" << endl;
            }
            break;
        }
        case 7: {
            if (loggedInUsers.empty()) {
                cout << "Please login first to add a commit." << endl;
            }
            else {
                cin.ignore();
                cout << "Enter the name of the repository: ";
                string repoName;
                getline(cin, repoName);
                cout << "Enter commit message: ";
                string commitMessage;
                getline(cin, commitMessage);
                cout << "Enter files changed (separated by space): ";
                string filesChanged;
                getline(cin, filesChanged);
                vector<string> changedFiles;
                size_t pos = 0;
                while ((pos = filesChanged.find(" ")) != string::npos) {
                    changedFiles.push_back(filesChanged.substr(0, pos));
                    filesChanged.erase(0, pos + 1);
                }
                changedFiles.push_back(filesChanged); // Add the last file
                repoManager.addCommit(repoName, commitMessage, changedFiles);
                cout << "Commit added successfully!" << endl;
            }
            break;
        }
        case 8: {
            cin.ignore();
            cout << "Enter the name of the repository: ";
            string repoName;
            getline(cin, repoName);
            repoManager.viewRepositoryStats(repoName);
            break;
        }
        case 9: {
            if (loggedInUsers.empty()) {
                cout << "Please login first to follow users." << endl;
            }
            else {
                cin.ignore();
                cout << "Enter the username of the user you want to follow: ";
                string followUsername;
                getline(cin, followUsername);
                int follower = loggedInUsers.size() - 1;
                int followee = 0; // Assuming the user to follow is at index 0 for demonstration
                socialGraph.followUser(follower, followee);
                cout << "You are now following user " << followUsername << "." << endl;
            }
            break;
        }
        case 10: {
            if (loggedInUsers.empty()) {
                cout << "Please login first to unfollow users." << endl;
            }
            else {
                cout << "Enter the username of the user you want to unfollow: ";
                string unfollowUsername;
                getline(cin, unfollowUsername);
                int follower = loggedInUsers.size() - 1;
                int followee = 0; // Assuming the user to unfollow is at index 0 for demonstration
                socialGraph.unfollowUser(follower, followee);
                cout << "You have unfollowed user " << unfollowUsername << "." << endl;
            }
            break;
        }
        case 11: {
            if (loggedInUsers.empty()) {
                cout << "Please login first to display followed users." << endl;
            }
            else {
                cout << "Your followed users:" << endl;
                int userIndex = loggedInUsers.size() - 1;
                socialGraph.displayFollowedUsers(userIndex);
            }
            break;
        }
        case 12: {
            if (loggedInUsers.empty()) {
                cout << "Please login first to add a file to a repository." << endl;
            }
            else {
                cin.ignore();
                cout << "Enter the name of the repository: ";
                string repoName;
                getline(cin, repoName);
                cout << "Enter the name of the file to add: ";
                string fileName;
                getline(cin, fileName);
                repoManager.addFile(repoName, fileName);
                cout << "File added to the repository." << endl;
            }
            break;
        }
        case 13: {
            if (loggedInUsers.empty()) {
                cout << "Please login first to delete a file from a repository." << endl;
            }
            else {
                cin.ignore();
                cout << "Enter the name of the repository: ";
                string repoName;
                getline(cin, repoName);
                cout << "Enter the name of the file to delete: ";
                string fileName;
                getline(cin, fileName);
                repoManager.deleteFile(repoName, fileName);
                cout << "File deleted from the repository." << endl;
            }
            break;
        }
        case 14: {
            cout << "Exiting...\n";
            break;
        }
        default:
            cout << "Invalid choice! Please enter a valid option.\n";
        }
        
    } while (choice != 14);

    return 0;
}
