#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <cctype>
using namespace std;

// ----------------- Structures -----------------
struct Event {
    int id{};
    string name, type, date, time, venue, coordinator;

    string toCSV() const {
        return to_string(id) + "," + name + "," + type + "," + date + "," + time + "," + venue + "," + coordinator;
    }

    static Event fromCSV(const string &line) {
        Event e;
        vector<string> parts;
        string token;
        stringstream ss(line);
        while (getline(ss, token, ',')) parts.push_back(token);
        if (parts.size() >= 7) {
            try { e.id = stoi(parts[0]); }
            catch (...) { e.id = 0; } // if invalid ID (like "12A"), ignore that row
            e.name = parts[1];
            e.type = parts[2];
            e.date = parts[3];
            e.time = parts[4];
            e.venue = parts[5];
            e.coordinator = parts[6];
        }
        return e;
    }

    void display() const {
        cout << left << setw(4) << id << setw(25) << name << setw(12)
             << type << setw(12) << date << setw(8) << time
             << setw(15) << venue << setw(15) << coordinator << endl;
    }
};

struct Participant {
    string name, rollNo, dept;
    int eventId{};
    string toCSV() const { return name + "," + rollNo + "," + dept + "," + to_string(eventId); }

    static Participant fromCSV(const string &line) {
        Participant p;
        vector<string> parts;
        string token;
        stringstream ss(line);
        while (getline(ss, token, ',')) parts.push_back(token);
        if (parts.size() >= 4) {
            p.name = parts[0];
            p.rollNo = parts[1];
            p.dept = parts[2];
            try { p.eventId = stoi(parts[3]); } catch (...) { p.eventId = -1; }
        }
        return p;
    }

    void display() const {
        cout << left << setw(20) << name << setw(12) << rollNo
             << setw(12) << dept << setw(6) << eventId << endl;
    }
};

struct User {
    string username, password, role;
    string toCSV() const { return username + "," + password + "," + role; }

    static User fromCSV(const string &line) {
        User u;
        vector<string> parts;
        string token;
        stringstream ss(line);
        while (getline(ss, token, ',')) parts.push_back(token);
        if (parts.size() >= 3) {
            u.username = parts[0];
            u.password = parts[1];
            u.role = parts[2];
        }
        return u;
    }
};

// ----------------- Event Manager -----------------
class EventManager {
private:
    vector<Event> events;
    vector<Participant> participants;
    vector<User> users;
    int nextEventId = 1;

    const string eventsFile = "events.csv";
    const string participantsFile = "participants.csv";
    const string usersFile = "users.csv";

    string currentUser = "";
    string currentRole = "guest";

public:
    EventManager() {
        loadFromFiles();
        if (users.empty()) {
            cout << "No users found. Creating default admin (admin/admin)\n";
            users.push_back({"admin", "admin", "admin"});
            saveUsers();
        }
    }

    // ----------------- File I/O -----------------
    void loadFromFiles() {
        events.clear();
        participants.clear();
        users.clear();

        ifstream fin(eventsFile);
        string line;
        int maxId = 0;
        while (getline(fin, line)) {
            Event e = Event::fromCSV(line);
            if (e.id) {
                events.push_back(e);
                if (e.id > maxId) maxId = e.id;
            }
        }
        nextEventId = maxId + 1;

        ifstream fin2(participantsFile);
        while (getline(fin2, line)) {
            Participant p = Participant::fromCSV(line);
            if (p.eventId != -1) participants.push_back(p);
        }

        ifstream fin3(usersFile);
        while (getline(fin3, line)) {
            User u = User::fromCSV(line);
            if (!u.username.empty()) users.push_back(u);
        }
    }

    void saveUsers() {
        ofstream fout(usersFile, ios::trunc);
        for (auto &u : users) fout << u.toCSV() << endl;
    }

    void saveToFiles() {
        ofstream fout(eventsFile, ios::trunc);
        for (auto &e : events) fout << e.toCSV() << endl;

        ofstream fout2(participantsFile, ios::trunc);
        for (auto &p : participants) fout2 << p.toCSV() << endl;

        saveUsers();
    }

    // ----------------- User Registration -----------------
    void registerNewUser() {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        User u;
        cout << "\n=== User Registration ===\n";
        cout << "Username: ";
        getline(cin, u.username);
        cout << "Password: ";
        getline(cin, u.password);
        cout << "Role (admin/guest): ";
        getline(cin, u.role);

        if (u.role != "admin" && u.role != "guest") {
            cout << "Invalid role. Defaulting to guest.\n";
            u.role = "guest";
        }

        users.push_back(u);
        saveUsers();
        cout << "User registered successfully!\n";
    }

    // ----------------- Authentication -----------------
    bool login() {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string u, p;
        cout << "Username: ";
        getline(cin, u);
        cout << "Password: ";
        getline(cin, p);
        for (auto &usr : users) {
            if (usr.username == u && usr.password == p) {
                currentUser = u;
                currentRole = usr.role;
                cout << "Login successful! Role: " << currentRole << "\n";
                return true;
            }
        }
        cout << "Login failed.\n";
        return false;
    }

    void logout() {
        currentUser = "";
        currentRole = "guest";
    }

    // ----------------- Admin Features -----------------
    void addEvent() {
        Event e;
        e.id = nextEventId++;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Event Name: ";
        getline(cin, e.name);
        cout << "Type: ";
        getline(cin, e.type);
        cout << "Date (YYYY-MM-DD): ";
        getline(cin, e.date);
        cout << "Time (HH:MM): ";
        getline(cin, e.time);
        cout << "Venue: ";
        getline(cin, e.venue);
        cout << "Coordinator: ";
        getline(cin, e.coordinator);

        events.push_back(e);
        saveToFiles();
        cout << "✅ Event added successfully!\n";
    }

    void showEvents() {
        if (events.empty()) {
            cout << "No events found.\n";
            return;
        }
        cout << left << setw(4) << "ID" << setw(25) << "Name" << setw(12) << "Type"
             << setw(12) << "Date" << setw(8) << "Time" << setw(15) << "Venue"
             << setw(15) << "Coordinator" << endl;
        cout << string(90, '-') << endl;
        for (auto &e : events) e.display();
    }

    void registerParticipant() {
        Participant p;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Name: ";
        getline(cin, p.name);
        cout << "Roll No: ";
        getline(cin, p.rollNo);
        cout << "Dept: ";
        getline(cin, p.dept);
        cout << "Event ID: ";
        cin >> p.eventId;

        bool exists = false;
        for (auto &e : events) if (e.id == p.eventId) exists = true;

        if (!exists) {
            cout << "❌ Invalid Event ID!\n";
            return;
        }

        participants.push_back(p);
        saveToFiles();
        cout << "✅ Registered successfully!\n";
    }

    // ----------------- Panels -----------------
    void adminPanel() {
        while (true) {
            cout << "\n======= ADMIN PANEL =======\n"
                 << "1. Add Event\n2. View Events\n3. Register Participant\n"
                 << "4. Logout\nChoice: ";
            int ch; cin >> ch;
            switch (ch) {
                case 1: addEvent(); break;
                case 2: showEvents(); break;
                case 3: registerParticipant(); break;
                case 4: logout(); return;
                default: cout << "Invalid choice.\n";
            }
        }
    }

    void guestPanel() {
        while (true) {
            cout << "\n======= GUEST PANEL =======\n"
                 << "1. View Events\n2. Register Participant\n3. Exit\nChoice: ";
            int ch; cin >> ch;
            switch (ch) {
                case 1: showEvents(); break;
                case 2: registerParticipant(); break;
                case 3: return;
                default: cout << "Invalid choice.\n";
            }
        }
    }

    // ----------------- Main Menu -----------------
    void run() {
        while (true) {
            cout << "\n======= EVENT MANAGEMENT =======\n"
                 << "1. Login\n2. Register New User\n3. Guest Access\n4. Exit\nChoice: ";
            int ch; cin >> ch;
            switch (ch) {
                case 1: if (login()) {
                            if (currentRole == "admin") adminPanel();
                            else guestPanel();
                        } break;
                case 2: registerNewUser(); break;
                case 3: guestPanel(); break;
                case 4: cout << "Exiting...\n"; return;
                default: cout << "Invalid!\n";
            }
        }
    }
};

// ----------------- Main -----------------
int main() {
    EventManager em;
    em.run();
    return 0;
}
// ----------------- End of main.cpp -----------------