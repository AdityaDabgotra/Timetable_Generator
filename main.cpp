#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <tuple>

using namespace std;

struct Teacher{
    string name;
    string subject;
    string type;
    int totalClasses;
    int classDuration;
    vector<pair<int, pair<int, int>>> schedule; // day, start time, end time
};

struct TimeSlot{
    int day;
    int start;
    int end;
    bool occupied;
};

vector<string> days = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};
vector<Teacher> teachers;
vector<vector<TimeSlot>> timetable;

int convertTime(const string& time){
    int hours, minutes;
    char period;
    char colon;
    stringstream ss(time);
    ss >> hours >> colon >> minutes >> period;

    if (period == 'P' && hours != 12) hours += 12;
    if (period == 'A' && hours == 12) hours = 0;
    return hours * 60 + minutes;
}

string formatTime(int minutes){
    int hours = minutes / 60;
    int mins = minutes % 60;
    string period = (hours >= 12) ? "PM" : "AM";
    
    if (hours > 12) hours -= 12;
    if (hours == 0) hours = 12;
    
    stringstream ss;
    ss << setw(2) << setfill('0') << hours << ":" 
       << setw(2) << setfill('0') << mins << " " << period;
    return ss.str();
}

void generateTimetable(int startTime, int endTime, int baseDuration, int lunchStart, int lunchEnd) {
    timetable.clear();

    for (int day = 0; day < days.size(); day++){
        vector<TimeSlot> daySlots;
        
        // Morning session
        int currentTime = startTime;
        while (currentTime + baseDuration <= lunchStart){
            daySlots.push_back({day, currentTime, currentTime + baseDuration, false});
            currentTime += baseDuration;
        }
        
        // Afternoon session
        currentTime = lunchEnd;
        while (currentTime + baseDuration <= endTime){
            daySlots.push_back({day, currentTime, currentTime + baseDuration, false});
            currentTime += baseDuration;
        }
        
        timetable.push_back(daySlots);
    }
}

bool assignClasses(Teacher& teacher, int baseDuration){
    int slotsPerClass = teacher.classDuration / baseDuration;
    vector<tuple<int, int, int>> availableBlocks; // day, startSlotIndex, slotsPerClass

    for (int day = 0; day < timetable.size(); day++){
        vector<TimeSlot>& daySlots = timetable[day];
        for (int slotIdx = 0; slotIdx < daySlots.size(); slotIdx++){
            bool canAssign = true;
            for (int i = 0; i < slotsPerClass; i++){
                int currentSlot = slotIdx + i;
                if (currentSlot >= daySlots.size() || daySlots[currentSlot].occupied){
                    canAssign = false;
                    break;
                }
            }
            if (canAssign){
                availableBlocks.emplace_back(day, slotIdx, slotsPerClass);
            }
        }
    }

    if (availableBlocks.size() < teacher.totalClasses) return false;

    random_shuffle(availableBlocks.begin(), availableBlocks.end());
    
    for (int i = 0; i < teacher.totalClasses; i++) {
        int day = get<0>(availableBlocks[i]);
        int startSlotIdx = get<1>(availableBlocks[i]);
        int numSlots = get<2>(availableBlocks[i]);

        for (int j = 0; j < numSlots; j++) {
            timetable[day][startSlotIdx + j].occupied = true;
        }

        int startTime = timetable[day][startSlotIdx].start;
        int endTime = timetable[day][startSlotIdx + numSlots - 1].end;
        teacher.schedule.push_back({day, {startTime, endTime}});
    }
    return true;
}

void printTimetable(){
    for (int day = 0; day < days.size(); day++){
        cout << "\n" << days[day] << ":\n";
        cout << "-----------------------------\n";
        
        for (auto& slot : timetable[day]){
            cout << formatTime(slot.start) << " - " << formatTime(slot.end) << ": ";
            bool classFound = false;
            
            for (auto& teacher : teachers){
                for (auto& tslot : teacher.schedule){
                    if (tslot.first == day && 
                        tslot.second.first <= slot.start && 
                        tslot.second.second >= slot.end){
                        cout << teacher.name << " (" << teacher.subject << ") ";
                        classFound = true;
                        break;
                    }
                }
                if (classFound) break;
            }
            if (!classFound) cout << "Free";
            cout << "\n";
        }
    }
}

int main() {
    srand(time(0));
    
    // Input timetable parameters
    string startStr, endStr, lunchStartStr, lunchEndStr;
    int baseDuration;
    
    cout << "Enter start time (HH:MM AM/PM): ";
    getline(cin, startStr);
    cout << "Enter end time (HH:MM AM/PM): ";
    getline(cin, endStr);
    cout << "Enter lunch break start time (HH:MM AM/PM): ";
    getline(cin, lunchStartStr);
    cout << "Enter lunch break end time (HH:MM AM/PM): ";
    getline(cin, lunchEndStr);
    cout << "Enter theory class duration (minutes): ";
    cin >> baseDuration;
    cin.ignore();

    int startTime = convertTime(startStr);
    int endTime = convertTime(endStr);
    int lunchStart = convertTime(lunchStartStr);
    int lunchEnd = convertTime(lunchEndStr);

    generateTimetable(startTime, endTime, baseDuration, lunchStart, lunchEnd);

    // Input teachers
    while (true) {
        Teacher teacher;
        cout << "\nEnter teacher's name (or 'done' to finish): ";
        getline(cin, teacher.name);
        if (teacher.name == "done") break;

        cout << "Enter subject: ";
        getline(cin, teacher.subject);

        cout << "Theory or Practical? (t/p): ";
        string type;
        getline(cin, type);
        transform(type.begin(), type.end(), type.begin(), ::tolower);

        if (type == "t") {
            teacher.type = "theory";
            teacher.classDuration = baseDuration;
        } else {
            teacher.type = "practical";
            bool valid = false;
            do {
                cout << "Enter lab duration (minutes, multiple of " << baseDuration << "): ";
                cin >> teacher.classDuration;
                cin.ignore();
                if (teacher.classDuration % baseDuration != 0) {
                    cout << "Duration must be a multiple of " << baseDuration << "!\n";
                } else {
                    valid = true;
                }
            } while (!valid);
        }

        cout << "Enter number of classes: ";
        cin >> teacher.totalClasses;
        cin.ignore();

        teachers.push_back(teacher);
    }

    // Assign classes
    bool allAssigned = true;
    for (auto& teacher : teachers) {
        if (!assignClasses(teacher, baseDuration)) {
            cout << "Failed to assign all classes for " << teacher.name << endl;
            allAssigned = false;
        }
    }

    if (!allAssigned) {
        cout << "\nTimetable generation failed due to insufficient slots.\n";
        return 1;
    }

    // Print timetable
    cout << "\nGenerated Timetable:\n";
    printTimetable();

    return 0;
}