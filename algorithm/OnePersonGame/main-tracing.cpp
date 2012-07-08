#include <cstdio>
#include <cstring>
#include <cassert>
#include <vector>
#include <algorithm>
using namespace std;

const int N = 7;
const int S = 2187 * 3; // 3^N * 3

struct Entry
{
public:
   Entry(int g, int s) : groups(g), score(s) { }
   int groups;
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
   int Normalize(int n, int mask)
   {
      int groups[N];
      memset(groups, -1, sizeof(*groups) * n);
      int next = 0;
      int result = 0;
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

class Accumulator
{
public:
   Accumulator()
   {
      Clear();
   }
   void Store(size_t v)
   {
      average_ = average_ * count_ + v;
      average_ /= ++count_;
   }
   double GetAverage()
   {
      return average_;
   }
   void Clear()
   {
      average_ = 0;
      count_ = 0;
   }
private:
   double average_;
   size_t count_;
};

int grid[N][N];
int n, m;
vector<Entry> states[2][S];
vector<Entry> *Prev;
vector<Entry> *Next;

int dirs[][2] = {
   { 0, 0 },
   { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 },
   { 3, 1 }, { 4, 1 }, { 2, 4 }, { 2, 3 },
   { 3, 2 }, { 4, 2 }, { 1, 4 }, { 1, 3 },
   // ends
   { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 },
   // starts
   { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 },
};
const int ndirs = sizeof(dirs) / sizeof(*dirs);

Accumulator averageFetchLength, averageStoreLength;

void init(vector<Entry> *s)
{
   for (int i = 0; i < S; i++)
      s[i].clear();
}

void expand(int (*choice)[2], int r, int c, int from, int to, int totalScore, int stage, int mask)
{
   if (r == n)
   {
      from *= 3;
      to *= 3;
      int fromStart = 0, fromEnd = 2;
      if ((stage & 0x1) > 0)
      {
         // can only accept states with no start or end
         fromEnd = 0;
      }
      else if (stage == 0x2)
      {
         // has end in the column but no start, can only accept states with start (only)
         fromStart = 1;
         fromEnd = 1;
      }

      int toAdd = 0;
      if ((stage & 0x2) > 0) toAdd = 2;
      else if (stage == 0x1) toAdd = 1;

      for (int fromAdd = fromStart; fromAdd <= fromEnd; fromAdd++)
      {
         vector<Entry> const& parents = Prev[from + fromAdd];
         
         averageFetchLength.Store(parents.size());

         for (size_t i = 0; i < parents.size(); i++)
         {
            int currentScore = parents[i].score + totalScore;

            Entry const& fromEntry = parents[i];
            DisjointSet groups(n);
            bool invalid = false;
            for (int j = 0; j < n; j++)
            {
               if (choice[j][0] == 4 || choice[j][1] == 3) 
               {
                  groups.Union(j, j - 1);
               }
               if (choice[j][0] == 1 || choice[j][1] == 2)
               {
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
                     // this forms a closed loop, which is an invalid formation
                     if (invalid) break;
                  }
                  // else rhs does not connect to anything above it
               }
            }

            if (invalid) continue;

            toAdd = max(toAdd, fromAdd);
            vector<Entry> & current = Next[to + toAdd];
            int toGroups = groups.Normalize(n, mask);
            bool fnd = false;

            size_t storeLength = current.size();

            for (size_t j = 0; j < current.size(); j++)
            {
               if (current[j].groups == toGroups)
               {
                  current[j].score = max(current[j].score, currentScore);
                  fnd = true;
                  storeLength = j + 1;
                  break;
               }
            }
            if (!fnd)
            {
               current.push_back(Entry(toGroups, currentScore));
            }

            averageStoreLength.Store(storeLength);
         }
      }

      return;
   }

   for (int k = 0; k < ndirs; k++)
   {  
      if (grid[r][c] == 0 && k > 0) break; 
      if (stage == 1 && dirs[k][0] == 0 && k > 0) break;
      if ((stage & 0x2) > 0 && dirs[k][1] == 0 && k > 0) break;

      if (r > 0 && 
         (choice[r - 1][0] == 3 && dirs[k][1] != 3 || choice[r - 1][1] == 4 && dirs[k][0] != 4))
         continue;
      if (dirs[k][0] == 4 && (r == 0 || choice[r - 1][1] != 4)) continue;
      if (dirs[k][1] == 3 && (r == 0 || choice[r - 1][0] != 3)) continue;
      if (dirs[k][1] == 4 && r == n - 1) continue;
      if (dirs[k][0] == 3 && r == n - 1) continue;

      choice[r][0] = dirs[k][0];
      choice[r][1] = dirs[k][1];

      int fromBit = 0;
      int toBit = 0;

      if (choice[r][0] == 1)
      {
         fromBit = 1;
      }
      else if (choice[r][1] == 2)
      {
         fromBit = 2;
      }
      if (choice[r][0] == 2)
      {
         toBit = 2;
      }
      else if (choice[r][1] == 1)
      {
         toBit = 1;
      }

      int nextStage = stage;
      if (dirs[k][0] == 0 && k > 0) nextStage |= 0x1;
      else if (dirs[k][1] == 0 && k > 0) nextStage |= 0x2;

      int nextMask = mask;
      if (choice[r][1] == 1 || choice[r][0] == 2)
      {
         nextMask |= 1 << r;
      }
      expand(choice, r + 1, c, from * 3 + fromBit, to * 3 + toBit, k == 0 ? totalScore : totalScore + grid[r][c], nextStage, nextMask);
   }
}

int solveInternal()
{
   averageFetchLength.Clear();
   averageStoreLength.Clear();

   init(states[0]);
   states[0][0].push_back(Entry(0, 0));

   Prev = states[0];
   Next = states[1];
   for (int c = m - 1; c >= 0; c--)
   {
      init(Next);

      int choice[N][2];
      expand(choice, 0, c, 0, 0, 0, 0, 0);

      swap(Prev, Next);
   }

   assert(Prev[2].size() <= 1);
   return Prev[2].empty() ? 0 : Prev[2][0].score;
}

int solve()
{
   int maxi = 0;
   for (int i = 0; i < n; i++)
   {
      for (int j = 0; j < m; j++)
      {
         // one step case
         maxi = max(maxi, grid[i][j]);
      }
   }
   maxi = max(maxi, solveInternal());
   // trace stats
   printf("fetchLength=%lf storeLength=%lf\n", averageFetchLength.GetAverage(), averageStoreLength.GetAverage());
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
      {
         scanf("%d", grid[i] + j);
      }
      printf("%d\n", solve());
   }
}
