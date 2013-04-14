#if 0

#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <queue>
using namespace std;

const int N = 105;
const int MAX_HALF_LEN = 50;

string square(const string& x)
{
	static char buf[N];
	memset(buf, 0, sizeof buf);
	int len = x.size();
	for (int i = 0; i < len; i++) {
		for (int j = 0; j < len; j++) {
			int p = x[i] * x[j];
			buf[i + j] += p;
			buf[i + j + 1] += buf[i + j] / 10;
			buf[i + j] %= 10;
		}
	}
	int retLen = buf[len * 2 - 1] == 0 ? len * 2 - 1 : len * 2;
	string ret(retLen, static_cast<char>(0));
	for (int i = 0; i < retLen; i++) ret[i] = buf[i];
	return ret;
}

bool LE(const string& a, const string& b)
{
	int la = a.size(), lb = b.size();
	if (la < lb) return true;
	else if (la > lb) return false;
	for (int i = la - 1; i >= 0; i--) {
		if (a[i] < b[i]) return true;
		else if (a[i] > b[i]) return false;
	}
	return true;
}

void Print(const string& x)
{
	string v(x);
	for (int i = 0; i < v.size(); ++i) v[i] += '0';
	printf("%s\n", v.c_str());
}

bool isMatch(const string &x, const string&a, const string &b)
{
	int len = x.size();
	if (x[len - 1] == 0) return false;

	// Print(x);

	string x2 = square(x);
	return LE(a, x2) && LE(x2, b);
}

bool twoExpandable(const string& x)
{
	int oneCount = 0;
	for (int i = 0; i < x.size(); ++i) {
		if (x[i] == 0) continue;
		else if (x[i] == 1) {
			if (++oneCount > 1) {
				return false;
			}
		} else {
			return false;
		}
	}
	return true;
}

long long solve(const string& a, const string& b)
{
	const char c0 = static_cast<char>(0);
	const char c1 = static_cast<char>(1);
	const char c2 = static_cast<char>(2);
	const char c3 = static_cast<char>(3);
	queue<string> q;
	q.push("");
	q.push(string(1, c0));
	q.push(string(1, c1));
	q.push(string(1, c2));
	q.push(string(1, c3));
	long long ret = 0;
	while (!q.empty()) {
		string x = q.front();
		q.pop();
#if 0
		if (isMatch(x, a, b)) {
			++ret;
		}
#endif
		++ret;
		if (x.size() >= MAX_HALF_LEN - 1) continue;
		char last = x[x.size() - 1];
		if ((last == c2 && x.size() > 1) || last == c3) continue;
		q.push(c0 + x + c0);
		q.push(c1 + x + c1);
		if (twoExpandable(x)) {
			q.push(c2 + x + c2);
		}
	}
	printf("finishes\n");
	return ret;
}

int main()
{
	int T;
	scanf("%d", &T);
	for (int t = 0; t < T; t++) {
		printf("Case #%d: ", t + 1);
		char a[N], b[N];
		scanf("%s %s", a, b);
		long long ret = solve(a, b);
		printf("%lld\n", ret);
	}
}

#endif