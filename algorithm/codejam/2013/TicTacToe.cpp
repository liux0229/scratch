#include <cstdio>

const int N = 4;

char B[N][N + 1];
int D[][2] = { { 0, 1 }, { 1, 0 }, { 1, 1 }, { -1, 1 } };

void read()
{
	for (int i = 0; i < N; i++) {
		scanf("%s", B[i]);
	}
}

inline bool match(char a, char b)
{
	return a != '.' && b != '.' && (a == 'T' || b == 'T' || a == b);
}

void solve()
{
	bool empty = false;
	bool X = false;
	bool O = false;
	for (int r = 0; r < N; r++) {
		for (int c = 0; c < N; c++) {
			if (B[r][c] == '.') {
				empty = true;
			}
			for (int d = 0; d < 4; d++) {
				int nr = r + D[d][0] * (N - 1);
				int nc = c + D[d][1] * (N - 1);
				if (nr >= 0 && nc >= 0 && nr < N && nc < N) {
					bool fail = false;
					char what = B[r][c];
					for (int i = 1; i < N; i++) {
						char next = B[r + D[d][0] * i][c + D[d][1] * i];
						if (!match(what, next)) {
							fail = true;
							break;
						}
						if (next != 'T') {
							what = next;
						}
					}
					if (!fail) {
						if (what == 'X' ) X = true;
						else O = true;
						goto end;
					}
				}
			}
		}
	}
end:
	if (X) {
		printf("X won");
	} else if (O) {
		printf("O won");
	} else if (empty) {
		printf("Game has not completed");
	} else {
		printf("Draw");
	}
}

int main()
{
	int T;
	scanf("%d", &T);
	for (int t = 0; t < T; t++) {
		printf("Case #%d: ", t + 1);
		read();
		solve();
		printf("\n");
	}
}

