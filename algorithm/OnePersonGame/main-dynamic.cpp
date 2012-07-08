#include <cstdio>
#include <cstring>
#include <cassert>
#include <vector>
#include <algorithm>
using namespace std;

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
   DisjointSet(int n) : s_(n, -1)
   {
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
   int Normalize(int mask)
   {
      vector<int> groups(s_.size(), -1);
      int next = 0;
      int result = 0;
      for (int i = 0; i < static_cast<int>(s_.size()); i++)
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
   vector<int> s_;
};

class OnePersonGame
{
public:
   int solve(vector<vector<int>> && grid);
private:
   static const int dirs[][2];
   static const int ndirs;

   void Init(vector<Entry> *s);
   void Expand(vector<int> *choice, int r, int c, int from, int to, int totalScore, int stage, int mask);
   int SolveInternal();

   vector<vector<int>> grid_;
   int n_;
   int m_;
   int nstates_;
   vector<Entry> *prev_;
   vector<Entry> *next_;
};

const int OnePersonGame::dirs[][2] = {
   { 0, 0 },
   { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 },
   { 3, 1 }, { 4, 1 }, { 2, 4 }, { 2, 3 },
   { 3, 2 }, { 4, 2 }, { 1, 4 }, { 1, 3 },
   // ends
   { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 },
   // starts
   { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 },
};
const int OnePersonGame::ndirs = sizeof(OnePersonGame::dirs) / sizeof(*OnePersonGame::dirs);

void OnePersonGame::Init(vector<Entry> *s)
{
   for (int i = 0; i < nstates_; i++)
      s[i].clear();
}

void OnePersonGame::Expand(vector<int> *choice, int r, int c, int from, int to, int totalScore, int stage, int mask)
{
   if (r == n_)
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
         vector<Entry> const& parents = prev_[from + fromAdd];
         
         for (size_t i = 0; i < parents.size(); i++)
         {
            int currentScore = parents[i].score + totalScore;

            Entry const& fromEntry = parents[i];
            DisjointSet groups(n_);
            bool invalid = false;
            for (int j = 0; j < n_; j++)
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
            vector<Entry> & current = next_[to + toAdd];
            int toGroups = groups.Normalize(mask);
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
      }

      return;
   }

   for (int k = 0; k < ndirs; k++)
   {  
      if (grid_[r][c] == 0 && k > 0) break; 
      if (stage == 1 && dirs[k][0] == 0 && k > 0) break;
      if ((stage & 0x2) > 0 && dirs[k][1] == 0 && k > 0) break;

      if (r > 0 && 
         (choice[r - 1][0] == 3 && dirs[k][1] != 3 || choice[r - 1][1] == 4 && dirs[k][0] != 4))
         continue;
      if (dirs[k][0] == 4 && (r == 0 || choice[r - 1][1] != 4)) continue;
      if (dirs[k][1] == 3 && (r == 0 || choice[r - 1][0] != 3)) continue;
      if (dirs[k][1] == 4 && r == n_ - 1) continue;
      if (dirs[k][0] == 3 && r == n_ - 1) continue;

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
      Expand(choice, r + 1, c, from * 3 + fromBit, to * 3 + toBit, k == 0 ? totalScore : totalScore + grid_[r][c], nextStage, nextMask);
   }
}

int OnePersonGame::SolveInternal()
{
   Init(prev_);
   prev_[0].push_back(Entry(0, 0));

   vector<vector<int>> choice(n_, vector<int>(2));
   for (int c = m_ - 1; c >= 0; c--)
   {
      Init(next_);

      Expand(&choice[0], 0, c, 0, 0, 0, 0, 0);

      swap(prev_, next_);
   }

   assert(prev_[0].size() == 1 && prev_[0][0].score == 0); // the trivial empty path
   assert(prev_[1].empty()); // impossible for paths with 1 endpoint

   assert(prev_[2].size() <= 1);
   return prev_[2].empty() ? 0 : prev_[2][0].score;
}

int OnePersonGame::solve(vector<vector<int>> && grid)
{
   grid_ = move(grid);
   n_ = grid_.size();
   m_ = grid_[0].size();
   nstates_ = 1;
   for (int i = 0; i < n_ + 1; i++) nstates_ *= 3;
   vector<vector<Entry>> prev(nstates_);
   vector<vector<Entry>> next(nstates_);
   prev_ = &prev[0];
   next_ = &next[0];

   int maxi = 0;
   for (int i = 0; i < n_; i++)
   {
      for (int j = 0; j < m_; j++)
      {
         // one step case
         maxi = max(maxi, grid_[i][j]);
      }
   }

   maxi = max(maxi, SolveInternal());
   
   return maxi;
}

int main()
{
   int T;
   scanf("%d", &T);
   OnePersonGame solver;
   for (int t = 0; t < T; t++)
   {
      int n, m;
      scanf("%d %d", &n, &m);
      vector<vector<int>> grid(n, vector<int>(m));
      for (int i = 0; i < n; i++)
      for (int j = 0; j < m; j++)
      {
         scanf("%d", &grid[i][j]);
      }
      printf("%d\n", solver.solve(move(grid)));
   }
}
