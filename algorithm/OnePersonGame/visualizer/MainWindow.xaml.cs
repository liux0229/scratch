using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO;
using System.Threading.Tasks;
using System.Diagnostics;

namespace OnePersonGameVisualizer
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private int[,] input;

        private OnePersonGameSolver solver;

        private int nextColumn;

        private bool boardChanged;

        public MainWindow()
        {
            InitializeComponent();
            SetupDragDrop();
        }

        private void SetupDragDrop()
        {
            AllowDrop = true;
            Drop += (s, e) =>
            {
                var files = ((System.Windows.DataObject)e.Data).GetFileDropList();
                foreach (string filePath in files)
                {
                    ReadInput(filePath);
                    DrawBoard();
                    break;
                }
            };

            // other inits
            board.boardChanged += (r, c, number) =>
            {
                if (input != null)
                {
                    input[r, c] = number;
                    boardChanged = true;
                }
            };
        }

        private void ReadInput(string filePath)
        {
            using (var textReader = new StreamReader(File.OpenRead(filePath)))
            {
                string line = textReader.ReadLine();
                int[] ints = parseInts(line);
                int rows = ints[0], cols = ints[1];
                int[,] values = new int[rows, cols];
                for (int r = 0; r < rows; r++)
                {
                    line = textReader.ReadLine();
                    ints = parseInts(line);
                    for (int c = 0; c < cols; c++)
                    {
                        values[r, c] = ints[c];
                    }
                }
                input = values;
            }

            // initialize game state
            nextColumn = -1;
            status.Text = "";
            boardChanged = false;
        }

        private int[] parseInts(string line)
        {
            string[] units = line.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
            return (from unit in units select int.Parse(unit)).ToArray();
        }

        private void DrawBoard()
        {
            if (input == null) return;

            this.board.Clear();

            int rows = input.GetLength(0), cols = input.GetLength(1);
            this.board.DrawGrid(rows, cols);
            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < cols; j++)
                {
                    board.DrawNumber(i, j, input[i, j]);
                }
            }
        }

        private void startClick(object sender, RoutedEventArgs e)
        {
            if (input == null) return;

            Stopwatch timer = new Stopwatch();
            var task = new Task<Solution>(() =>
            {
                DisplayResult("Start solving...");
                timer.Start();
                nextColumn = -1;
                boardChanged = false;
                solver = new OnePersonGameSolver();
                return solver.Solve(input);
            });
            task.ContinueWith(result =>
            {
                ProcessSolution(result.Result, timer.Elapsed);
            });
            task.Start();
        }

        private void ProcessSolution(Solution solution, TimeSpan? elapsed)
        {
            Dispatcher.BeginInvoke((Action)(() =>
            {
                DrawSolution(solution);
            }));
            DisplayResult(string.Format("max={0}{1}", solution.score,
                elapsed != null ? string.Format(" spent {0} s", elapsed.Value.TotalSeconds) : ""));
        }

        private void startColumnClick(object sender, RoutedEventArgs e)
        {
            if (input == null) return;

            if (nextColumn == -1 || boardChanged)
            {
                board.ClearPath();
                nextColumn = input.GetLength(1) - 1;
                boardChanged = false;
                solver = new OnePersonGameSolver();
                solver.BeginSolve(input);
            };

            var task = new Task(() =>
            {
                DisplayResult("Start solving next column...");
                solver.BeginSolveNextColumn(sol =>
                {
                    ProcessSolution(sol, null);
                });
            });

            --nextColumn;
            task.Start();
        }

        private void DrawSolution(Solution solution)
        {
            int startCol = input.GetLength(1) - solution.board.Count;
            for (int r = 0; r < input.GetLength(0); r++)
            {
                for (int c = startCol; c < input.GetLength(1); c++)
                {
                    var choice = solution.board[solution.board.Count - 1 - (c - startCol)];
                    board.DrawCell(r, c, choice[r, 0], choice[r, 1]);
                }
            }
        }

        private void DisplayResult(string result)
        {
            Dispatcher.BeginInvoke((Action)(() =>
            {
                this.status.Text = result;
            }));
        }
    }
}
