#include <cstdio>
#include <cstring>
#include <cmath>
using namespace std;

const int N = 105;

bool isParlindrome(int n)
{
	static char buf[N];
	if (n <= 0) return false;
	int p = 0;
	while (n > 0) {
		buf[p++] = n % 10;
		n /= 10;
	}
	for (int i = 0; i < p / 2; i++) {
		if (buf[i] != buf[p - 1 - i]) {
			return false;
		}
	}
	return true;
}

int toNum(const char *s, int len)
{
	int ret = 0;
	int p = 1;
	for (int i = len - 1; i >= 0; i--) {
		ret += p * (s[i] - '0');
		p *= 10;
	}
	return ret;
}

int solve(const char* a, const char *b)
{
	int la = strlen(a);
	int lb = strlen(b);
	int na = toNum(a, la);
	int nb = toNum(b, lb);
	int ret = 0;
	for (int i = na; i <= nb; i++) {
		if (!isParlindrome(i)) {
			continue;
		}
		int r = static_cast<int>(sqrt(i));
		if (r * r == i && isParlindrome(r)) {
			++ret;
		}
	}
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
		int ret = solve(a, b);
		printf("%d\n", ret);
	}
}
