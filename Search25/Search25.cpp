#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <map>

using namespace std;

vector<vector<int32_t>> decode_sum(int32_t s, int32_t N, const vector<vector<int32_t>>& v);
vector<vector<string>> sum_to_string(vector<int32_t> parts, const multimap<int32_t, string>& original);

constexpr bool is_unique_pair(const int32_t a, const int32_t b) {
    return (a & b) == 0;
}

string unencode(int32_t e) {
    string s;
    for (char i = 0; i < 26; i++) {
        if (e & (1 << i)) {
            s += 'a' + i;
        }
    }
    return s;
}

int32_t to_bitencoded(string s) {
    int t = 0;
    for (char c : s) {
        int d = c - 'a';
        t |= (1 << d);
    }
    return t;
    // cout << t << '\n';
}

int main()
{
    auto start = chrono::high_resolution_clock::now();

    ifstream myfile;
    myfile.open("words_alpha.txt");
    // myfile.open("valid-wordle-words.txt");
    
    multimap<int32_t, string> original;

    vector<vector<int32_t>> v(6);
    while (myfile) {
        string s;
        myfile >> s;
        // if (s == "vozhd" || s == "gymps") continue;
        if (s.length() == 5) {
            string s_copy = s;
            sort(s.begin(), s.end());
            if (s[0] != s[1] && s[1] != s[2] && s[2] != s[3] && s[3] != s[4]) {
                // v.push_back(s);
                int e = to_bitencoded(s);
                v[1].push_back(e);
                original.insert({ e, s_copy });
            }
        }
    }
    sort(v[1].begin(), v[1].end());
    v[1].erase(unique(v[1].begin(), v[1].end()), v[1].end());
    cout << v[1].size() << " words '" << unencode(v[1].front()) << "' to '" << unencode(v[1].back()) << "'" << '\n';

    for (int S = 2; S <= 5; S++) {
        cout << "S = " << S << '\n';
        int j_index_start = 0;
        for (const int32_t i : v[S-1]) {
            while (v[1][j_index_start] <= i) j_index_start++;
            //for (const int32_t j : v[1]) {
            for(int j_index = j_index_start; j_index < v[1].size(); j_index++) {
                int32_t j = v[1][j_index];
                if (is_unique_pair(i, j)) {
                    v[S].push_back(i | j);
                }
            }
        }
        vector<int32_t>* vx = &v[S];
        sort(vx->begin(), vx->end());
        vx->erase(unique(vx->begin(), vx->end()), vx->end());
        cout << vx->size() << " " << S << "-size words '" << unencode(vx->front()) << "' to '" << unencode(vx->back()) << "'" << '\n';
    }
    cout << '\n';

    // Rough and ready decode
    int count = 0;
    for (int x : v[5]) {
        int missing = (((1 << 26) - 1) - x);
        vector<vector<int32_t>> result = decode_sum(x, 5, v);
        for (auto sum : result) {
            vector<vector<string>> result1 = sum_to_string(sum, original);
            for (vector<string> parts : result1) {
                sort(parts.begin(), parts.end());
                for (string p : parts) {
                    cout << p << ", ";
                }
                cout << unencode(missing) << '\n';
                count++;
            }
        }
    }
    cout << count << " found" << endl;

    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
    cout << '\n' << duration.count() << "ms" << '\n';
}

vector<vector<string>> sum_to_string(vector<int32_t> parts, const multimap<int32_t, string> &original) {
    vector<vector<string>> result;
    if (parts.size() == 0) {
        result.push_back(vector<string>());
        return result;
    }

    int32_t x = parts.back(); parts.pop_back();
    vector<vector<string>> recursive_result = sum_to_string(parts, original);
    
    auto find_x = original.equal_range(x);
    for (auto it = find_x.first; it != find_x.second; it++) {
        string s = it->second;
        for (auto r : recursive_result) {
            result.push_back(r);
            result.back().push_back(s);
        }
    }
}

vector<vector<int32_t>> decode_sum(int32_t s, int32_t N, const vector<vector<int32_t>> &v) {
    vector<vector<int32_t>> result;
    if (N == 1) {
        result.push_back(vector<int32_t>(1, s));
        return result;
    }
    for (auto x : v[1]) {
        if (!((s | x) == s)) continue;
        int32_t y = s - x;
        if (!(x > y)) continue;
        // then s = x + y (and x > y)
        // need to check y is in the list v[N-1]
        
        auto const search = lower_bound(v[N - 1].begin(), v[N - 1].end(), y);
        if (search == v[N-1].end() || *search != y) continue;

        vector<vector<int32_t>> recursive_result = decode_sum(y, N-1, v);
        for (vector<int32_t> r : recursive_result) {
            r.push_back(x);
            result.push_back(r);
        }
    }
    return result;
}
