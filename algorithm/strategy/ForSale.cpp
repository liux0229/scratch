// Example program
#include <iostream>
#include <string>
#include <random>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

int N = 30;

int one() {
    vector<int> cards;
    for (int i = 1; i <= N; ++i) {
        cards.push_back(i);
    }
    random_shuffle(cards.begin(), cards.end());
    double avgSpread = 0;
    for (int i = 0; i < N; i += 4) {
        int spread = 0;
        sort(cards.begin() + i, cards.begin() + i + 4);
        for (int j = i; j < i + 4; ++j) {
            cout << cards[j] << " ";
        }
        for (int j = i; j < i + 3; ++j) {
            spread = max(spread, cards[j + 1] - cards[j]);
        }
        cout << "(" << spread << ")" << endl;
        avgSpread += spread;
    }
    return avgSpread / (N / 4);
}

void work() {
    int T = 1;
    double avg = 0;
    for (int i = 0; i < T; ++i) {
        // cout << one() << endl;
        avg += one();
    }
    cout << avg / T << endl;
}

int main()
{
    work();
}
