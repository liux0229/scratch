#include <cstdio>
#include <cstring>
#include <cmath>

namespace FairSquareHard1 {

using namespace std;

const int N = 105;
const int MAX = 10000000;
// const int MAX = 40;

int match[MAX + 1];

bool isParlindrome(long long n)
{
	static int buf[N];
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

inline bool isMatch(long long x)
{
	bool ret = isParlindrome(x) && isParlindrome(x * x);
#if 0
	if (ret) {
		printf("%lld matches\n", x);
	}
#endif
	return ret;
}

void preprocess()
{
	for (long long i = 1; i <= MAX; i++) {
		match[i] = isMatch(i) + match[i - 1];
	}
}

long long solve(long long a, long long b)
{
	long long ra = static_cast<long long>(sqrt(a));
	long long rb = static_cast<long long>(sqrt(b));
	if (ra * ra != a) ++ra;
    return match[rb] - match[ra - 1];
}

int main()
{
	preprocess();
	int T;
	scanf("%d", &T);
	for (int t = 0; t < T; t++) {
		printf("Case #%d: ", t + 1);
		long long a, b;
		scanf("%lld %lld", &a, &b);
		long long ret = solve(a, b);
		printf("%lld\n", ret);
	}
	return 0;
}

}
