#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Course {
    string courseNumber;
    string courseTitle;
    vector<string> prerequisites;
};

struct Node {
    Course course;
    Node* left;
    Node* right;

    Node(Course aCourse) {
        course = aCourse;
        left = nullptr;
        right = nullptr;
    }
};

class BinarySearchTree {
private:
    Node* root;

    void addNode(Node* node, Course course) {
        if (course.courseNumber < node->course.courseNumber) {
            if (node->left == nullptr) {
                node->left = new Node(course);
            }
            else {
                addNode(node->left, course);
            }
        }
        else {
            if (node->right == nullptr) {
                node->right = new Node(course);
            }
            else {
                addNode(node->right, course);
            }
        }
    }

    void inOrder(Node* node) const {
        if (node == nullptr) {
            return;
        }

        inOrder(node->left);
        cout << node->course.courseNumber << ", " << node->course.courseTitle << endl;
        inOrder(node->right);
    }

    void deleteTree(Node* node) {
        if (node == nullptr) {
            return;
        }

        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }

public:
    BinarySearchTree() {
        root = nullptr;
    }

    ~BinarySearchTree() {
        deleteTree(root);
    }

    void clear() {
        deleteTree(root);
        root = nullptr;
    }

    void insert(Course course) {
        if (root == nullptr) {
            root = new Node(course);
        }
        else {
            addNode(root, course);
        }
    }

    Course search(string courseNumber) const {
        Node* current = root;

        while (current != nullptr) {
            if (current->course.courseNumber == courseNumber) {
                return current->course;
            }
            else if (courseNumber < current->course.courseNumber) {
                current = current->left;
            }
            else {
                current = current->right;
            }
        }

        return Course();
    }

    void printCourseList() const {
        inOrder(root);
    }

    bool isEmpty() const {
        return root == nullptr;
    }
};

string trim(string value) {
    size_t first = value.find_first_not_of(" \t\r\n");
    size_t last = value.find_last_not_of(" \t\r\n");

    if (first == string::npos || last == string::npos) {
        return "";
    }

    return value.substr(first, last - first + 1);
}

string toUpper(string value) {
    transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(toupper(c));
    });
    return value;
}

vector<string> splitCSVLine(string line) {
    vector<string> tokens;
    string token;
    stringstream lineStream(line);

    while (getline(lineStream, token, ',')) {
        tokens.push_back(trim(token));
    }

    return tokens;
}

bool courseExists(const vector<string>& courseNumbers, const string& courseNumber) {
    for (const string& number : courseNumbers) {
        if (number == courseNumber) {
            return true;
        }
    }
    return false;
}

// Loads the course file, validates each line, and stores courses in the tree.
bool loadCourses(const string& fileName, BinarySearchTree& courseTree) {
    ifstream courseFile(fileName);

    if (!courseFile.is_open()) {
        cout << "Error: Could not open file " << fileName << endl;
        return false;
    }

    vector<vector<string>> validLines;
    vector<string> courseNumbers;
    string line;

    while (getline(courseFile, line)) {
        if (trim(line).empty()) {
            continue;
        }

        vector<string> tokens = splitCSVLine(line);

        if (tokens.size() < 2 || tokens.at(0).empty() || tokens.at(1).empty()) {
            cout << "Error: Invalid course line skipped: " << line << endl;
            continue;
        }

        tokens.at(0) = toUpper(tokens.at(0));
        courseNumbers.push_back(tokens.at(0));
        validLines.push_back(tokens);
    }

    courseFile.close();

    for (vector<string> tokens : validLines) {
        bool validCourse = true;

        for (size_t i = 2; i < tokens.size(); ++i) {
            tokens.at(i) = toUpper(tokens.at(i));

            if (!tokens.at(i).empty() && !courseExists(courseNumbers, tokens.at(i))) {
                cout << "Error: Prerequisite " << tokens.at(i)
                     << " for " << tokens.at(0) << " was not found in the file." << endl;
                validCourse = false;
            }
        }

        if (!validCourse) {
            continue;
        }

        Course course;
        course.courseNumber = tokens.at(0);
        course.courseTitle = tokens.at(1);

        for (size_t i = 2; i < tokens.size(); ++i) {
            if (!tokens.at(i).empty()) {
                course.prerequisites.push_back(tokens.at(i));
            }
        }

        courseTree.insert(course);
    }

    return true;
}

// Prints one course and its prerequisites.
void printCourseInformation(const BinarySearchTree& courseTree, string courseNumber) {
    courseNumber = toUpper(trim(courseNumber));
    Course course = courseTree.search(courseNumber);

    if (course.courseNumber.empty()) {
        cout << courseNumber << " was not found." << endl;
        return;
    }

    cout << course.courseNumber << ", " << course.courseTitle << endl;

    if (course.prerequisites.empty()) {
        cout << "Prerequisites: None" << endl;
        return;
    }

    cout << "Prerequisites: ";

    for (size_t i = 0; i < course.prerequisites.size(); ++i) {
        Course prerequisite = courseTree.search(course.prerequisites.at(i));

        cout << course.prerequisites.at(i);

        if (!prerequisite.courseTitle.empty()) {
            cout << ", " << prerequisite.courseTitle;
        }

        if (i < course.prerequisites.size() - 1) {
            cout << "; ";
        }
    }

    cout << endl;
}

// Tries the common file names used for the ABCU course data.
bool loadDefaultCourses(BinarySearchTree& courseTree) {
    vector<string> fileNames = {
        "CS 300 ABCU_Advising_Program_Input.csv",
        "ABCU_Advising_Program_Input.csv",
        "ABCU Advising Program Input.csv"
    };

    for (const string& fileName : fileNames) {
        ifstream testFile(fileName);

        if (testFile.is_open()) {
            testFile.close();
            courseTree.clear();
            return loadCourses(fileName, courseTree);
        }
    }

    cout << "Error: Course input file was not found." << endl;
    return false;
}

void displayMenu() {
    cout << "1. Load Data Structure." << endl;
    cout << "2. Print Course List." << endl;
    cout << "3. Print Course." << endl;
    cout << "9. Exit" << endl;
    cout << "What would you like to do? ";
}

int main() {
    BinarySearchTree courseTree;
    bool dataLoaded = false;
    int choice = 0;

    cout << "Welcome to the course planner." << endl << endl;

    while (choice != 9) {
        displayMenu();

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input. Please enter a number." << endl << endl;
            continue;
        }

        cout << endl;

        switch (choice) {
        case 1:
            if (loadDefaultCourses(courseTree)) {
                dataLoaded = true;
                cout << "Data loaded successfully." << endl;
            }
            cout << endl;
            break;

        case 2:
            if (!dataLoaded) {
                dataLoaded = loadDefaultCourses(courseTree);
                cout << endl;
            }

            if (dataLoaded) {
                cout << "Here is a sample schedule:" << endl;
                courseTree.printCourseList();
            }
            cout << endl;
            break;

        case 3:
            if (!dataLoaded) {
                dataLoaded = loadDefaultCourses(courseTree);
                cout << endl;
            }

            if (dataLoaded) {
                string courseNumber;
                cout << "What course do you want to know about? ";
                cin >> courseNumber;
                cout << endl;
                printCourseInformation(courseTree, courseNumber);
            }
            cout << endl;
            break;

        case 9:
            cout << "Thank you for using the course planner!" << endl;
            break;

        default:
            cout << choice << " is not a valid option." << endl << endl;
            break;
        }
    }

    return 0;
}
