#include <iostream>
#include <string>
#include <map>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <functional>

using namespace std;

char comp(char x) {
    static char tbl[128];
    tbl['A'] = 'T';
    tbl['T'] = 'A';
    tbl['C'] = 'G';
    tbl['G'] = 'C';
    return tbl[x];
}

string complement(string text) {
    string ret;
    for (auto x : text) {
        ret.push_back(comp(x));
    }
    for (size_t i = 0; i < ret.size() / 2; ++i) {
        swap(ret[i], ret[ret.size() - 1 - i]);
    }
    return ret;
}

size_t mostFrequent(string text, size_t k) {
    map<string, size_t> M;
    for (size_t i = 0; i + k <= text.size(); ++i) {
        ++M[text.substr(i, k)];
    }
    size_t max = 0;
    for (auto& kv : M) {
        if (kv.second > max) {
            max = kv.second;
        }
    }
#if 1
    for (auto& kv : M) {
        if (max == kv.second) {
            cout << kv.first << " ";
        }
    }
    cout << endl;
#endif
    return max;
}

char randOne() {
    const char* s = "ATCG";
    return s[rand() % 4];
}

size_t simulateOne() {
    string text;
    for (int i = 0; i < 500; ++i) {
        text.push_back(randOne());
    }
    return mostFrequent(text, 9);
}

void simulate() {
    int K = 100000;

    map<size_t, size_t> M;
    for (int i = 0; i < K; ++i) {
        ++M[simulateOne()];
    }

    for (auto& kv : M) {
        cout << kv.first << " " << 1.0 * kv.second / K * 100 << "%" << endl;
    }
}

void locate(string text, string p) {
    auto m = p.size();
    for (size_t i = 0; i + m <= text.size(); ++i) {
        if (text.substr(i, m) == p) {
            cout << i << " ";
        }
    }
    cout << endl;
}

void locateComp(string text, string p) {
    auto m = p.size();
    auto cp = complement(p);
    for (size_t i = 0; i + m <= text.size(); ++i) {
        auto sub = text.substr(i, m);
        if (sub == p || sub == cp) {
            cout << i << " ";
        }
    }
    cout << endl;
}

size_t hamming(string s, string t) {
    size_t ret = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        ret += s[i] != t[i];
    }
    return ret;
}

void locateApprox(string text, string p, size_t d) {
    auto m = p.size();
    for (size_t i = 0; i + m <= text.size(); ++i) {
        if (hamming(text.substr(i, m), p) <= d) {
            cout << i << " ";
        }
    }
    cout << endl;
}

size_t countApprox(string text, string p, size_t d) {
    size_t ret = 0;
    auto m = p.size();
    for (size_t i = 0; i + m <= text.size(); ++i) {
        ret += hamming(text.substr(i, m), p) <= d;
    }
    return ret;
}

// t - occurs
void findClump(string text, size_t k, size_t L, size_t t) {
    map<string, vector<size_t>> M;
    for (size_t i = 0; i + k <= text.size(); ++i) {
        M[text.substr(i, k)].push_back(i);
    }
    vector<string> ret;
    for (auto& kv : M) {
        auto& lst = kv.second;
        // iterate though all possible lumps
        for (size_t i = 0; i + t <= lst.size(); ++i) {
            auto range = lst[i + t - 1] + k - lst[i];
            if (range <= L) {
                ret.push_back(kv.first);
                break;
            }
        }
    }
    for (auto& x : ret) {
        cout << x << " ";
    }
    cout << endl;
}

void skew(string text) {
    int s = 0;
    cout << s << " ";
    for (auto x : text) {
        if (x == 'G') ++s;
        else if (x == 'C') --s;
        cout << s << " ";
    }
    cout << endl;
}

void minSkew(string text) {
    int s = 0;
    vector<int> all;
    all.push_back(s);
    for (auto x : text) {
        if (x == 'G') ++s;
        else if (x == 'C') --s;
        all.push_back(s);
    }
    auto min = *std::min_element(all.begin(), all.end());
    for (size_t i = 0; i < all.size(); ++i) {
        if (all[i] == min) {
            cout << i << " ";
        }
    }
    cout << endl;
}

void mutateInternal(vector<string>& ret, string& base, size_t d, size_t s) {
    if (s == base.size() || d == 0) {
        ret.push_back(base);
        return;
    }

    static const string n = "ATCG";
    auto old = base[s];
    for (auto x : n) {
        base[s] = x; 
        mutateInternal(ret, base, d - (old != x), s + 1);
    }
    base[s] = old;
}

vector<string> mutate(string base, size_t d) {
    vector<string> ret;
    mutateInternal(ret, base, d, 0);
    return ret;
}

void mostFrequentApprox(string text, size_t k, size_t d, bool comp = false) {
    map<string, size_t> M;
    for (size_t i = 0; i + k <= text.size(); ++i) {
        auto base = text.substr(i, k);
        for (auto& s : mutate(base, d)) {
            ++M[s];
            ++M[complement(s)];
        }
    }
    size_t max = 0;
    for (auto& kv : M) {
        if (kv.second > max) {
            max = kv.second;
        }
    }
    for (auto& kv : M) {
        if (max == kv.second) {
            cout << kv.first << " ";
        }
    }
    cout << endl;
}

const char* decodeBuf(int n, int len) {
    static char buf[128];
    for (int i = 0; i < len; ++i) {
        buf[len - 1 - i] = n % 2 == 0 ? '0' : '1';
        n /= 2;
    }
    buf[len] = 0;
    return buf;
}

string decode(int n, int len) {
    return decodeBuf(n, len);
}

size_t countPattern(size_t n, size_t alpha, string pat, size_t t) {
    // ignore alpha & t for now.
    size_t ret = 0;

    for (int i = 0; i < 1 << n; ++i) {
        cout << i << " " << decode(i, n);
        if (decode(i, n).find(pat) != string::npos) {
            cout << " 1" << endl;
            ++ret;
        } else {
            cout << " 0" << endl;
        }
        // cout << i << endl;
    }

    return ret;
}

int main() {
    cout << countApprox("TACGCATTACAAAGCACA", "AA", 1) << endl;
}
