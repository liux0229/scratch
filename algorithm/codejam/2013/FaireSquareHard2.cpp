#include <iostream>
#include <cstring>
#include <cmath>
#include <string>
#include <queue>

namespace FairSquareHard2 {

using namespace std;

typedef long long int64_t;

class BigNum {
public:
	BigNum() {
		d_.resize(1, 0);
	}

	BigNum sqrt() const {
		// implement this
		return *this;
	}

	const vector<short>& digits() const { return d_; }

	size_t size() const { return d_.size(); }

private:
	// assume inputs are all digits
	void read(istream& in) {
		string s;
		in >> s;
		int start = 0;
		for (; start < static_cast<int>(s.size()) - 1; start++) {
			if (s[start] != '0') {
				break;
			}
		}
		vector<short> d;
		d.reserve(s.size() - start);
		for (int i = s.size() - 1; i >= start; i--) {
			d.push_back(s[i] - '0');
		}

		d_.swap(d);
	}

	friend istream& operator>>(istream& in, BigNum& n) { 
		n.read(in);
		return in;
	}

	void write(ostream& out) const {
		for (int i = d_.size() - 1; i >= 0; i--) {
			out << d_[i];
		}
	}

	friend ostream& operator<<(ostream& out, const BigNum& n) {
		n.write(out);
		return out;
	}

	vector<short> d_; 
};

// find all n <= x such that n is a parlindrome and n^2 is a parlindrome
int64_t f(const BigNum& x) {
	auto& ds = x.digits();
	int len = x.size();
	for (int i = len - 1; i >= 0; i--) {
		// digits chosen left to this position match those of x
		for (int d = 0; d < ds[i]; d++) {
			
		}
	}
}

int64_t solve(const BigNum& a, const BigNum &b)
{
	// implement square root
	BigNum ra = a.sqrt();
	BigNum rb = b.sqrt();
	
	// if ra^2 < a, we want to count [ra + 1, rb], and that translates to f(rb) - f(ra)
	return f(rb) - f(ra);
}

int main()
{
	int T;
	cin >> T;
	for (int i = 0; i < T; i++) {
		cout << "Case #" << (i + 1) << ": ";
		BigNum a, b;
		cin >> a >> b;
		cout << solve(a, b);
	}
	return 0;
}

}


