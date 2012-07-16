using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace OnePersonGameVisualizer
{
    class Entry
    {
        public Entry(int g, int s, List<Dir[,]> sol)
        {
            groups = g;
            score = s;
            solution = sol;
        }

        public int groups { get; private set; }
        public int score { get; set; }
        public List<Dir[,]> solution { get; set; }
    }

    class DisjointSet
    {
        public DisjointSet(int n)
        {
            s_ = new int[n];
            for (int i = 0; i < n; i++)
                s_[i] = -1;
        }

        public int Find(int a)
        {
            if (s_[a] < 0) return a;
            else return s_[a] = Find(s_[a]);
        }

        public bool Union(int a, int b)
        {
            int ra = Find(a);
            int rb = Find(b);
            if (ra == rb)
                return false;

            if (s_[ra] > s_[rb])
            {
                int tmp = ra;
                ra = rb;
                rb = tmp;
            }

            s_[ra] += s_[rb];
            s_[rb] = ra;
            return true;
        }

        public int Normalize(int mask)
        {
            int[] groups = new int[s_.Length];
            for (int i = 0; i < groups.Length; i++)
                groups[i] = -1;

            int next = 0;
            int result = 0;
            for (int i = 0; i < s_.Length; i++)
            {
                if ((mask & 1 << i) == 0) continue;
                int r = Find(i);
                if (groups[r] == -1)
                {
                    groups[r] = next++;
                }
                result |= groups[r] << 2 * i;
            }
            return result;
        }

        private int[] s_;
    }

    class OnePersonGameSolver
    {
        public Solution Solve(int[,] grid)
        {
            InitSolver(grid);

            for (int c = m_ - 1; c >= 0; c--)
            {
                SolveColumn(c);
            }

            return prev_[2].Count == 0 ? new Solution() : new Solution(prev_[2][0].score, prev_[2][0].solution);
        }

        public void BeginSolve(int[,] grid)
        {
            InitSolver(grid);
            currentColumn_ = grid_.GetLength(1) - 1;
        }

        public void BeginSolveNextColumn(Action<Solution> callback)
        {
            if (currentColumn_ == -1) return;

            SolveColumn(currentColumn_);
            --currentColumn_;

            int bestScore;
            List<Dir[,]> bestSolution;

            if (currentColumn_ == -1)
            {
                if (prev_[2].Count == 0)
                {
                    bestScore = 0;
                    bestSolution = new List<Dir[,]>();
                }
                else
                {
                    bestScore = prev_[2][0].score;
                    bestSolution = prev_[2][0].solution;
                }
            }
            else
            {
                bestScore = 0;
                bestSolution = new List<Dir[,]>();

                for (int i = 0; i < nstates_; i++)
                {
                    for (int j = 0; j < prev_[i].Count; j++)
                    {
                        if (prev_[i][j].score > bestScore)
                        {
                            bestScore = prev_[i][j].score;
                            bestSolution = prev_[i][j].solution;
                        }
                    }
                }
            }

            callback(new Solution(bestScore, bestSolution));
        }

        private void InitSolver(int[,] grid)
        {
            grid_ = grid;
            n_ = grid_.GetLength(0);
            m_ = grid_.GetLength(1);
            nstates_ = 1;
            for (int i = 0; i < n_ + 1; i++) nstates_ *= 3;
            prev_ = new List<Entry>[nstates_];
            next_ = new List<Entry>[nstates_];
            for (int i = 0; i < nstates_; i++)
            {
                prev_[i] = new List<Entry>();
                next_[i] = new List<Entry>();
            }

            InitStates(prev_);
            prev_[0].Add(new Entry(0, 0, new List<Dir[,]>()));
        }

        private void InitStates(List<Entry>[] s)
        {
            for (int i = 0; i < nstates_; i++)
                s[i].Clear();
        }

        private void SolveColumn(int c)
        {
            InitStates(next_);
            Dir[,] choice = new Dir[n_, 2];
            Expand(choice, 0, c, 0, 0, 0, 0, 0);

            var tmp = prev_;
            prev_ = next_;
            next_ = tmp;
        }

        private void Expand(Dir[,] choice, int r, int c, int from, int to, int totalScore, int stage, int mask)
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
                    List<Entry> parents = prev_[from + fromAdd];

                    for (int i = 0; i < parents.Count; i++)
                    {
                        int currentScore = parents[i].score + totalScore;

                        DisjointSet groups = new DisjointSet(n_);
                        bool invalid = false;
                        for (int j = 0; j < n_; j++)
                        {
                            if (choice[j, 0] == Dir.Down || choice[j, 1] == Dir.Up)
                            {
                                groups.Union(j, j - 1);
                            }
                            if (choice[j, 0] == Dir.Left || choice[j, 1] == Dir.Right)
                            {
                                // find out whether the rhs has connected to anything above it
                                int rhs = (parents[i].groups >> j * 2) & 0x3;
                                int connected = -1;
                                for (int k = j - 1; k >= 0; k--)
                                {
                                    if (choice[k, 0] == Dir.Left || choice[k, 1] == Dir.Right)
                                    {
                                        if (((parents[i].groups >> k * 2) & 0x3) == rhs)
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

                        toAdd = Math.Max(toAdd, fromAdd);
                        List<Entry> current = next_[to + toAdd];
                        int toGroups = groups.Normalize(mask);
                        bool fnd = false;
                        var currentSolution = parents[i].solution.Select(s => s).ToList();
                        currentSolution.Add((Dir[,])choice.Clone());
                        for (int j = 0; j < current.Count; j++)
                        {
                            if (current[j].groups == toGroups)
                            {
                                if (currentScore > current[j].score)
                                {
                                    current[j].score = currentScore;
                                    current[j].solution = currentSolution;
                                }
                                fnd = true;
                                break;
                            }
                        }
                        if (!fnd)
                        {
                            current.Add(new Entry(toGroups, currentScore, currentSolution));
                        }
                    }
                }

                return;
            }

            for (int k = 0; k < Dirs.Values.GetLength(0); k++)
            {
                if (grid_[r, c] == 0 && k > 0) break;
                if (stage == 1 && Dirs.Values[k, 0] == Dir.None && k > 0) break;
                if ((stage & 0x2) > 0 && Dirs.Values[k, 1] == Dir.None && k > 0) break;

                if (r > 0 &&
                (choice[r - 1, 0] == Dir.Up && Dirs.Values[k, 1] != Dir.Up || choice[r - 1, 1] == Dir.Down && Dirs.Values[k, 0] != Dir.Down))
                    continue;
                if (Dirs.Values[k, 0] == Dir.Down && (r == 0 || choice[r - 1, 1] != Dir.Down)) continue;
                if (Dirs.Values[k, 1] == Dir.Up && (r == 0 || choice[r - 1, 0] != Dir.Up)) continue;
                if (Dirs.Values[k, 1] == Dir.Down && r == n_ - 1) continue;
                if (Dirs.Values[k, 0] == Dir.Up && r == n_ - 1) continue;

                choice[r, 0] = Dirs.Values[k, 0];
                choice[r, 1] = Dirs.Values[k, 1];

                int fromBit = 0;
                int toBit = 0;

                if (choice[r, 0] == Dir.Left)
                {
                    fromBit = 1;
                }
                else if (choice[r, 1] == Dir.Right)
                {
                    fromBit = 2;
                }
                if (choice[r, 0] == Dir.Right)
                {
                    toBit = 2;
                }
                else if (choice[r, 1] == Dir.Left)
                {
                    toBit = 1;
                }

                int nextStage = stage;
                if (Dirs.Values[k, 0] == Dir.None && k > 0) nextStage |= 0x1;
                else if (Dirs.Values[k, 1] == Dir.None && k > 0) nextStage |= 0x2;

                int nextMask = mask;
                if (choice[r, 1] == Dir.Left || choice[r, 0] == Dir.Right)
                {
                    nextMask |= 1 << r;
                }
                Expand(choice, r + 1, c, from * 3 + fromBit, to * 3 + toBit, k == 0 ? totalScore : totalScore + grid_[r, c], nextStage, nextMask);
            }
        }

        int[,] grid_;
        int n_;
        int m_;
        int nstates_;
        List<Entry>[] prev_;
        List<Entry>[] next_;
        int currentColumn_;
    }

    class Solution
    {
        public Solution()
        {
            score = 0;
            board = new List<Dir[,]>();
        }

        public Solution(int s, List<Dir[,]> b)
        {
            score = s;
            board = b;
        }

        public int score { get; private set; }
        public List<Dir[,]> board { get; private set; }
    }
}
