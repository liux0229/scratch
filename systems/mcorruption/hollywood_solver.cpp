// Example program
#include <iostream>
#include <string>
#include <algorithm>
#include <cassert>
#include <map>
#include <unordered_map>
#include <vector>
using namespace std;

unsigned short S(unsigned short x) {
    return (x >> 8) + ((x & 0xff) << 8);
}

void solve() {
    unsigned short r4 = 0, r6 = 0;
    // unsigned short r4 = 0x2aa2, r6 = 0x9341;
    unsigned short p[5] = { 0x21c2, 0x5e40, 0xfc31, 0x0 };
    // unsigned short p[5] = { 0x6df0, 0x0 };
    
    for (int i = 0; p[i]; ++i) {
        r4 += p[i];
        r4 = S(r4);
        r6 ^= p[i];
        swap(r4, r6);
        cout << hex << r4 << " " << r6 << endl;
    }
    
    // assert(r4 == 0xfeb1);
    // assert(r6 == 0x9298);
    cout << hex << r4 << " " << r6 << endl;
}

struct Hash {
   size_t operator()(pair<unsigned short, unsigned short> p) const {
      return (p.first << 16) + p.second;
   }
};

void bfs() {
    struct Step {
        unsigned short move;
	short steps;
    };
    unordered_map<pair<unsigned short, unsigned short>, Step, Hash> visited;
    vector<pair<unsigned short, unsigned short>> Q;
    Q.emplace_back(0x0, 0x0);
    visited.emplace(Q[0], Step{0, 0});
    
    for (size_t q = 0; q < Q.size(); ++q) {
	auto oldHead = Q[q];
        auto oldStep = visited[oldHead].steps;
	if (oldStep >= 2) {
	    break;
	}
	
        for (int i = 1; i < 0x10000; ++i) {
           auto head = oldHead;
            // cout << hex << head.first << " " << head.second << endl;

            unsigned short m = i;
            head.first += m;
            head.first = S(head.first);
            head.second ^= m;
            swap(head.first, head.second);

            auto it = visited.find(head);
            if (it == visited.end()) {
                visited.emplace(head, Step{m, (short)(oldStep + 1)});
                Q.emplace_back(head);
                if (Q.size() % 10000000 == 0) {
                   cout << Q.size() << endl;
                }
                if (Q.size() % 230000000 == 0) {
                  goto finished;
                }
                
#if 0
                if (head.first == 0xfeb1 && head.second == 0x9298) {
                    cout << "found" << endl;
                    return;
                }
#endif
            }
        }
    }

finished:

    auto reverse = [](unsigned short x, unsigned short y, unsigned short m) {
       swap(x, y);
       y ^= m;
       x = S(x);
       x -= m;
       return make_pair(x, y);
    };

    {
       unsigned short r4 = 0xfeb1, r6 = 0x9298;
       for (int i = 1; i < 0x10000; ++i) {
          unsigned short m = i;

          unsigned short x, y;
          tie(x, y) = reverse(r4, r6, m);

          if (visited.count(make_pair(x, y))) {
             cout << "------------" << endl;
             cout << hex << m << " " << x << " " << y << endl;
             while (x != 0 || y != 0) {
                cout << hex << visited[make_pair(x, y)].move << " => " << x << " " << y << endl;
                tie(x, y) = reverse(x, y, visited[make_pair(x, y)].move);
             }
             // cout << hex << visited[make_pair(x, y)].move << " " << m << " | " << "(" << x << "," << y << ")" << endl;
          }
       }
    }
}

int main()
{
   // bfs();
   cout << "------- verification ------" << endl;
   solve();
}

