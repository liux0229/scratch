#include <cstdio>
#include <cstring>
#include <cassert>
#include <vector>
#include <algorithm>
using namespace std;

const int N = 7;
const int S = 2187;

struct Entry
{
public:
   Entry(short g, int s) : groups(g), score(s) { }
   short groups;
   int score;
};

class DisjointSet
{
public:
   DisjointSet(int n)
   {
      memset(s_, -1, sizeof(*s_) * n);
   }
   int Find(int a)
   {
      if (s_[a] < 0) return a;
      else return s_[a] = Find(s_[a]);
   }
   bool Union(int a, int b)
   {
      int ra = Find(a);
      int rb = Find(b);
      if (ra == rb) 
         return false;

      if (s_[ra] > s_[rb]) swap(ra, rb);
      s_[ra] += s_[rb];
      s_[rb] = ra;
      return true;
   }
   short Normalize(int n, int mask)
   {
      int groups[N];
      memset(groups, -1, sizeof(*groups) * n);
      int next = 0;
      short result = 0;
      for (int i = 0; i < n; i++)
      {
         if (!(mask & 1 << i)) continue;
         int r = Find(i);
         if (groups[r] == -1)
         {
            assert(next < 4);
            groups[r] = next++;
         }
         result |= groups[r] << 2 * i;
      }
      return result;
   }
private:
   int s_[N];
};

int grid[N][N];
int n, m;
vector<Entry> states[2][S];

int dirs[][2] = {
   { 0, 0 },
   { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 },
   { 3, 1 }, { 4, 1 }, { 2, 4 }, { 2, 3 },
   { 3, 2 }, { 4, 2 }, { 1, 4 }, { 1, 3 }
};
const int ndirs = sizeof(dirs) / sizeof(*dirs);

void init(vector<Entry> *s)
{
   for (int i = 0; i < S; i++)
      s[i].clear();
}

void expand(int (*choice)[2], int r, int c, vector<Entry> *prev, vector<Entry> *next, int sr, int sc, int er, int ec)
{
   if (r == n)
   {
      int from = 0;
      int to = 0;
      int totalScore = 0;
      for (int i = 0; i < n; i++)
      {
         if (choice[i][0] != 0 || choice[i][1] != 0) totalScore += grid[i][c];

         int fromBit = 0;
         int toBit = 0;
         if (choice[i][0] == 1)
         {
            fromBit = 1;
         }
         else if (choice[i][1] == 2)
         {
            fromBit = 2;
         }

         if (choice[i][0] == 2)
         {
            toBit = 2;
         }
         else if (choice[i][1] == 1)
         {
            toBit = 1;
         }
         from = from * 3 + fromBit;
         to = to * 3 + toBit;
      }
      vector<Entry> const& parents = prev[from];
      vector<Entry> & current = next[to];
      for (size_t i = 0; i < parents.size(); i++)
      {
         int currentScore = parents[i].score + totalScore;

         Entry const& fromEntry = parents[i];
         DisjointSet groups(n);
         int mask = 0;
         bool invalid = false;
         for (int j = 0; j < n; j++)
         {
            if (choice[j][0] == 4) 
            {
               groups.Union(j, j - 1);
            }
            if (choice[j][0] == 1 || choice[j][1] == 2)
            {
               mask |= 1 << j;
               // find out whether the rhs has connected to anything above it
               int rhs = (fromEntry.groups >> j * 2) & 0x3;
               int connected = -1;
               for (int k = j - 1; k >= 0; k--)
               {
                  if (choice[k][0] == 1 || choice[k][1] == 2)
                  {
                     if (((fromEntry.groups >> k * 2) & 0x3) == rhs)
                     {
                        connected = k;
                        break;
                     }
                  }
               }
               if (connected != -1) 
               {
                  invalid |= !groups.Union(connected, j);
                  // this forms a closed loop, which is not allowed
                  if (invalid) break;
               }
            }
         }
         
         if (invalid) continue;

         short toGroups = groups.Normalize(n, mask);
         bool fnd = false;
         for (size_t j = 0; j < current.size(); j++)
         {
            if (current[j].groups == toGroups)
            {
               current[j].score = max(current[j].score, currentScore);
               fnd = true;
               break;
            }
         }
         if (!fnd)
         {
            current.push_back(Entry(toGroups, currentScore));
         }
      }

      return;
   }

   if (r == sr && c == sc)
   {
      choice[r][0] = 0;
      for (int k = 1; k <= 4; k++)
      {
         if (k == 3 && (r == 0 || choice[r - 1][0] != 3)) continue;
         if (k == 4 && r == n - 1) continue;
         if (r > 0 && 
             (choice[r - 1][0] == 3 && k != 3 || choice[r - 1][1] == 4))
            continue;

         choice[r][1] = k;
         expand(choice, r + 1, c, prev, next, sr, sc, er, ec);
      }
   }
   else if (r == er && c == ec)
   {
      choice[r][1] = 0;
      for (int k = 1; k <= 4; k++)
      {
         if (k == 4 && (r == 0 || choice[r - 1][1] != 4)) continue;
         if (k == 3 && r == n - 1) continue;
         if (r > 0 && 
             (choice[r - 1][0] == 3 || choice[r - 1][1] == 4 && k != 4))
            continue;
         choice[r][0] = k;
         expand(choice, r + 1, c, prev, next, sr, sc, er, ec);
      }
   }
   else
   {
      for (int k = 0; k < ndirs; k++)
      {  
         if (k > 0 && grid[r][c] == 0) continue;

         if (k == 0)
         {
            if (r > 0 && (choice[r - 1][0] == 3 || choice[r - 1][1] == 4)) continue;
            choice[r][0] = 0;
            choice[r][1] = 0;
         }
         else
         {
            if (r > 0 && 
                (choice[r - 1][0] == 3 && dirs[k][1] != 3 || choice[r - 1][1] == 4 && dirs[k][0] != 4))
                continue;
            if (dirs[k][0] == 4 && (r == 0 || choice[r - 1][1] != 4)) continue;
            if (dirs[k][1] == 3 && (r == 0 || choice[r - 1][0] != 3)) continue;
            if (dirs[k][1] == 4 && r == n - 1) continue;
            if (dirs[k][0] == 3 && r == n - 1) continue;
         
            choice[r][0] = dirs[k][0];
            choice[r][1] = dirs[k][1];
         }

         expand(choice, r + 1, c, prev, next, sr, sc, er, ec);
      }
   }
}

int solve(int sr, int sc, int er, int ec)
{
   init(states[0]);
   states[0][0].push_back(Entry(0, 0));

   vector<Entry> *prev = states[0];
   vector<Entry> *next = states[1];
   for (int c = m - 1; c >= 0; c--)
   {
      init(next);

      int choice[N][2];
      expand(choice, 0, c, prev, next, sr, sc, er, ec);

      swap(prev, next);
   }

   assert(prev[0].size() <= 1);
   return prev[0].empty() ? 0 : prev[0][0].score;
}

int solve()
{
   int maxi = 0;
   for (int i = 0; i < n; i++)
   {
      for (int j = 0; j < m; j++)
      {
         if (grid[i][j] == 0) continue;
         // one step case
         maxi = max(maxi, grid[i][j]);

         for (int a = 0; a < n; a++)
         {
            for (int b = 0; b < m; b++)
            {
               if (grid[a][b] == 0) continue;
               if (i * m + j < a * m + b)
               {
                  int result = solve(i, j, a, b);
                  printf("maxi=%d (%d %d %d %d)\n", result, i, j, a, b);
                  maxi = max(maxi, result);
               }
            }
         }
      }
   }
   return maxi;
}

int main()
{
   int T;
   scanf("%d", &T);
   for (int t = 0; t < T; t++)
   {
      scanf("%d %d", &n, &m);
      for (int i = 0; i < n; i++)
      for (int j = 0; j < m; j++)
         scanf("%d", grid[i] + j);
      printf("%d\n", solve());
   }
}