#if 0

#include <cstdio>
#include <algorithm>
using namespace std;

const int N = 100;

int R, C;
int B[N][N];
int T[N][N];

void read()
{
	scanf("%d %d", &R, &C);
	for (int i = 0; i < R; i++)
		for (int j = 0; j < C; j++)
			scanf("%d", &T[i][j]);
}

bool solve()
{
	for (int i = 0; i < R; i++) for (int j = 0; j < C; j++) B[i][j] = 100;
	for (int i = 0; i < R; i++) {
		int maxi = -1;
		for (int j = 0; j < C; j++) {
			maxi = max(maxi, T[i][j]);
		}
		for (int j = 0; j < C; j++) {
			B[i][j] = maxi;
		}
	}
	for (int i = 0; i < C; i++) {
		int maxi = -1;
		for (int j = 0; j < R; j++) {
			maxi = max(maxi, T[j][i]);
		}
		for (int j = 0; j < R; j++) {
			B[j][i] = min(B[j][i], maxi);
		}
	}
	for (int i = 0; i < R; i++) for (int j = 0; j < C; j++) 
		if (B[i][j] != T[i][j])
			return false;
	return true;
}

int main()
{
	int T;
	scanf("%d", &T);
	for (int t = 0; t < T; t++) {
		printf("Case #%d: ", t + 1);
		read();
		bool ret = solve();
		printf("%s\n", ret ? "YES" : "NO");
	}
}

#endif