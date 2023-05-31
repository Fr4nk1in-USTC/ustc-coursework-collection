#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_set>
#include <vector>

using std::endl;
using std::ifstream;
using std::istream;
using std::ofstream;
using std::ostream;
using std::sort;
using std::vector;

const int NUM_CASES = 10;

const char *INPUTS[] = {"../input/input0.txt", "../input/input1.txt",
                        "../input/input2.txt", "../input/input3.txt",
                        "../input/input4.txt", "../input/input5.txt",
                        "../input/input6.txt", "../input/input7.txt",
                        "../input/input8.txt", "../input/input9.txt"};

const char *OUTPUTS[] = {"../output/output0.txt", "../output/output1.txt",
                         "../output/output2.txt", "../output/output3.txt",
                         "../output/output4.txt", "../output/output5.txt",
                         "../output/output6.txt", "../output/output7.txt",
                         "../output/output8.txt", "../output/output9.txt"};

using shift_t   = vector<int>;
using request_t = vector<bool>;
using hashset   = std::unordered_set<int>;
using hashset_p = std::unique_ptr<hashset>;

ostream &operator<<(ostream &os, const shift_t &shift)
{
    for (size_t i = 0; i < shift.size() - 1; ++i) {
        os << shift[i] << ' ';
    }
    os << shift.back();
    return os;
}

ostream &operator<<(ostream &os, const request_t &request)
{
    for (size_t i = 0; i < request.size() - 1; ++i) {
        os << request[i] << ' ';
    }
    os << request.back();
    return os;
}

class solver
{
  public:
    solver() {}

    solver from_file(istream &file)
    {
        char stupid_comma;
        file >> staff_number >> stupid_comma;
        file >> day_number >> stupid_comma;
        file >> shift_per_day;

        total_shift_number = day_number * shift_per_day;
        min_shift_assigned = total_shift_number / staff_number;

        requests.resize(staff_number, request_t(total_shift_number, false));
        shifts.resize(total_shift_number, 0);
        staffs_.resize(staff_number, 0);
        staff_assigned.resize(staff_number, 0);
        staff_request.resize(staff_number, 0);

        // Parsing requests
        // Deal with the disgusting input format with comma as separator
        bool temp;
        for (auto &request : requests) {
            for (int i = 0; i < day_number; ++i) {
                for (int j = 0; j < shift_per_day - 1; ++j) {
                    file >> temp >> stupid_comma;
                    request[i * shift_per_day + j] = temp;
                }
                file >> temp;
                request[i * shift_per_day + shift_per_day - 1] = temp;
            }
        }

        for (int i = 0; i < staff_number; ++i) {
            for (int j = 0; j < total_shift_number; ++j) {
                staff_request[i] += requests[i][j];
            }
        }

        for (int i = 0; i < staff_number; ++i) {
            staffs_[i] = i;
        }

        return *this;
    }

    void solve() { backtrack(0); }

    void debug()
    {
        std::cout << "total shift number " << total_shift_number
                  << ", request satisfied " << request_satisfied << endl;
    }

    void to_file(ostream &file)
    {
        for (int i = 0; i < day_number; ++i) {
            for (int j = 0; j < shift_per_day - 1; ++j) {
                file << shifts[i * shift_per_day + j] << ',';
            }
            file << shifts[i * shift_per_day + shift_per_day - 1] << endl;
        }
        file << request_satisfied << endl;
    }

    bool verify()
    {
        int count = 0;
        for (int i = 0; i < total_shift_number; ++i) {
            count += requests[shifts[i] - 1][i];
        }
        if (count != request_satisfied) {
            std::cout << "Error: request_satisfied is not correct" << endl;
            return false;
        }
        for (int i = 0; i < total_shift_number - 1; ++i) {
            if (shifts[i] == shifts[i + 1]) {
                std::cout << "Error: two consecutive shifts are assigned to "
                             "the same staff"
                          << endl;
                return false;
            }
        }
        for (auto &assigned : staff_assigned) {
            if (assigned < min_shift_assigned) {
                std::cout << "Error: staff is assigned unfairly" << endl;
                return false;
            }
        }
        return true;
    }

  private:
    int staff_number;
    int day_number;
    int shift_per_day;
    int total_shift_number;
    int request_satisfied = 0;

    int min_shift_assigned;

    vector<request_t> requests;
    vector<int>       shifts;
    vector<int>       staff_assigned;
    vector<int>       staff_request;
    vector<int>       staffs_;

    void assign(int staff, int shift)
    {
        shifts[shift] = staff + 1;
        staff_assigned[staff]++;
    }

    void cancel(int staff, int shift)
    {
        shifts[shift] = 0;
        staff_assigned[staff]--;
    }

    bool valid(int staff, int shift)
    {
        return (shift == 0 or shifts[shift - 1] != staff + 1)
           and (shift == total_shift_number - 1
                or shifts[shift + 1] != staff + 1);
    }

    bool backtrack(int shift)
    {
        if (shift == total_shift_number) {
            return patch(0);
        }

        auto cmp = [this, shift](const int &a, const int &b) {
            if (requests[a][shift] != requests[b][shift]) {
                return requests[a][shift] == true;
            }
            if (staff_assigned[a] != staff_assigned[b]) {
                return staff_assigned[a] < staff_assigned[b];
            }
            return staff_request[a] < staff_request[b];
        };
        auto staffs = staffs_;
        sort(staffs.begin(), staffs.end(), cmp);

        int best_staff = staffs.front();
        if (not requests[best_staff][shift]) {
            return backtrack(shift + 1);
        }

        for (auto &staff : staffs) {
            if (not requests[staff][shift]) {
                return false;
            }
            if (not valid(staff, shift)) {
                continue;
            }
            assign(staff, shift);
            request_satisfied += requests[staff][shift];
            if (backtrack(shift + 1)) {
                return true;
            }
            request_satisfied -= requests[staff][shift];
            cancel(staff, shift);
        }

        return false;
    }

    bool patch(int shift)
    {
        while (shift < total_shift_number and shifts[shift]) {
            shift++;
        }
        if (shift == total_shift_number) {
            for (auto &assigned : staff_assigned) {
                if (assigned < min_shift_assigned) {
                    return false;
                }
            }
            return true;
        }

        auto cmp = [this](const int &a, const int &b) {
            return staff_assigned[a] < staff_assigned[b];
        };
        auto staffs = staffs_;
        sort(staffs.begin(), staffs.end(), cmp);

        for (auto &staff : staffs) {
            if (not valid(staff, shift)) {
                continue;
            }
            assign(staff, shift);
            request_satisfied += requests[staff][shift];
            if (patch(staff + 1)) {
                return true;
            }
            request_satisfied -= requests[staff][shift];
            cancel(staff, shift);
        }

        return false;
    }
};

int main()
{
    for (int i = 0; i < NUM_CASES; ++i) {
        ifstream input(INPUTS[i]);
        solver   s = solver().from_file(input);

        s.solve();

        std::cout << INPUTS[i] << ": ";
        s.debug();

        s.verify();

        ofstream output(OUTPUTS[i]);
        s.to_file(output);
    }
}
