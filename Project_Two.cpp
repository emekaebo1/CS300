#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// Stores one course and the course numbers it requires as prerequisites.
struct Course {
    string courseNumber;
    string courseTitle;
    vector<string> prerequisites;
};

// Removes whitespace at the beginning and end of a string.
string trim(const string& value) {
    const string whitespace = " \t\r\n";
    const size_t first = value.find_first_not_of(whitespace);

    if (first == string::npos) {
        return "";
    }

    const size_t last = value.find_last_not_of(whitespace);
    return value.substr(first, last - first + 1);
}

// Splits a CSV row on commas. This matches the supplied course file,
// which uses separate columns and does not contain quoted commas.
vector<string> splitCsvRow(const string& line) {
    vector<string> fields;
    string field;
    stringstream row(line);

    while (getline(row, field, ',')) {
        fields.push_back(trim(field));
    }

    // Preserve a final empty field when the row ends with a comma.
    if (!line.empty() && line.back() == ',') {
        fields.push_back("");
    }

    return fields;
}

// Finds a course by its course number. Returns nullptr when not found.
const Course* findCourse(const vector<Course>& courses,
    const string& courseNumber) {
    for (const Course& course : courses) {
        if (course.courseNumber == courseNumber) {
            return &course;
        }
    }

    return nullptr;
}

// Loads course records from a CSV file into the vector.
// Existing course data is replaced only if the file opens successfully.
void loadCourses(const string& filename, vector<Course>& courses) {
    ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        cout << "Error: Could not open file \"" << filename << "\".\n";
        cout << "Check the filename and make sure the file is in the "
            << "program's working directory.\n";
        return;
    }

    vector<Course> loadedCourses;
    string line;
    int lineNumber = 0;
    int skippedRows = 0;

    while (getline(inputFile, line)) {
        ++lineNumber;

        if (trim(line).empty()) {
            continue;
        }

        vector<string> fields = splitCsvRow(line);

        // The input format is course number, title, and up to two prerequisites.
        if (fields.size() < 2 || fields.size() > 4 ||
            fields[0].empty() || fields[1].empty()) {
            cout << "Warning: Skipping invalid row " << lineNumber << ".\n";
            ++skippedRows;
            continue;
        }

        Course course;
        course.courseNumber = fields[0];
        course.courseTitle = fields[1];

        for (size_t i = 2; i < fields.size(); ++i) {
            if (!fields[i].empty()) {
                course.prerequisites.push_back(fields[i]);
            }
        }

        // Avoid storing duplicate course numbers.
        if (findCourse(loadedCourses, course.courseNumber) != nullptr) {
            cout << "Warning: Skipping duplicate course number "
                << course.courseNumber << " on row " << lineNumber << ".\n";
            ++skippedRows;
            continue;
        }

        loadedCourses.push_back(course);
    }

    inputFile.close();

    if (loadedCourses.empty()) {
        cout << "Error: No valid course records were loaded. "
            << "The existing course data was not changed.\n";
        return;
    }

    courses = loadedCourses;
    cout << "Successfully loaded " << courses.size() << " course(s).\n";

    if (skippedRows > 0) {
        cout << "Note: " << skippedRows << " invalid or duplicate row(s) "
            << "were skipped.\n";
    }
}

// Sorts courses by course number and prints the complete course list.
void printCourseList(vector<Course>& courses) {
    if (courses.empty()) {
        cout << "No courses are loaded. Choose option 1 first.\n";
        return;
    }

    sort(courses.begin(), courses.end(),
        [](const Course& first, const Course& second) {
            return first.courseNumber < second.courseNumber;
        });

    cout << "\nCourse List (alphanumeric order)\n";
    cout << "--------------------------------\n";

    for (const Course& course : courses) {
        cout << course.courseNumber << " : " << course.courseTitle << '\n';
    }

    cout << '\n';
}

// Displays a selected course and the numbers and titles of its prerequisites.
void printCourseInformation(const vector<Course>& courses) {
    if (courses.empty()) {
        cout << "No courses are loaded. Choose option 1 first.\n";
        return;
    }

    string courseNumber;
    cout << "Enter course number: ";
    getline(cin, courseNumber);
    courseNumber = trim(courseNumber);

    const Course* course = findCourse(courses, courseNumber);

    if (course == nullptr) {
        cout << "Error: Course " << courseNumber
            << " was not found. Check the course number and try again.\n";
        return;
    }

    cout << "\n" << course->courseNumber << " : "
        << course->courseTitle << '\n';

    if (course->prerequisites.empty()) {
        cout << "Prerequisites: None\n";
        return;
    }

    cout << "Prerequisites:\n";

    for (const string& prerequisiteNumber : course->prerequisites) {
        const Course* prerequisite = findCourse(courses, prerequisiteNumber);

        if (prerequisite != nullptr) {
            cout << "  " << prerequisite->courseNumber << " : "
                << prerequisite->courseTitle << '\n';
        }
        else {
            cout << "  " << prerequisiteNumber
                << " : Title unavailable (course not found in loaded data)\n";
        }
    }

    cout << '\n';
}

// Displays the menu and processes the user's choices.
int main() {
    vector<Course> courses;
    string choice;

    cout << "Welcome to the ABCU Course Advising Program.\n";

    while (true) {
        cout << "\nMenu\n";
        cout << "  1. Load course data\n";
        cout << "  2. Print course list\n";
        cout << "  3. Print course information\n";
        cout << "  9. Exit\n";
        cout << "Enter your choice: ";

        if (!getline(cin, choice)) {
            cout << "\nInput ended. Exiting program.\n";
            break;
        }

        choice = trim(choice);

        if (choice == "1") {
            string filename;
            cout << "Enter the course data filename: ";
            getline(cin, filename);
            filename = trim(filename);

            if (filename.empty()) {
                cout << "Error: Filename cannot be empty.\n";
                continue;
            }

            loadCourses(filename, courses);
        }
        else if (choice == "2") {
            printCourseList(courses);
        }
        else if (choice == "3") {
            printCourseInformation(courses);
        }
        else if (choice == "9") {
            cout << "Thank you for using the ABCU Course Advising Program. "
                << "Goodbye!\n";
            break;
        }
        else {
            cout << "Invalid menu choice. Enter 1, 2, 3, or 9.\n";
        }
    }

    return 0;
}
