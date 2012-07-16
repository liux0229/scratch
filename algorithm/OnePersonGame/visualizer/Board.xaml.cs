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
using System.Globalization;

namespace OnePersonGameVisualizer
{
    /// <summary>
    /// Interaction logic for Board.xaml
    /// </summary>
    partial class Board : UserControl
    {
        private const int CellWidth = 60;
        private const int CellHeight = 60;
        private const int CellBorderThickness = 2;
        private const int StrokeThickness = 2;
        private const int HH = CellHeight - CellBorderThickness * 2;
        private const int HM = HH / 2;
        private const int WW = CellWidth - CellBorderThickness * 2;
        private const int WM = WW / 2;
        private const int AL = 5;

        private int rows, cols;
        private Canvas[,] cells;
        private Grid[,] containers;

        public event Action<int, int, int> boardChanged;

        public Board()
        {
            InitializeComponent();
        }

        public void Clear()
        {
            board.Children.Clear();
            board.RowDefinitions.Clear();
            board.ColumnDefinitions.Clear();
        }

        public void ClearPath()
        {
            foreach (var canvas in cells)
            {
                canvas.Children.Clear();
            }
        }

        public void DrawGrid(int rows, int cols)
        {
            this.rows = rows;
            this.cols = cols;
            this.cells = new Canvas[rows, cols];
            this.containers = new Grid[rows, cols];

            for (int i = 0; i < rows; i++)
            {
                board.RowDefinitions.Add(new RowDefinition() { Height = new GridLength(CellHeight) });
            }
            for (int i = 0; i < cols; i++)
            {
                board.ColumnDefinitions.Add(new ColumnDefinition() { Width = new GridLength(CellWidth) });
            }

            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < cols; j++)
                {
                    var border = new Border() { BorderThickness = new Thickness(CellBorderThickness), BorderBrush = new SolidColorBrush(Colors.DarkCyan) };
                    Grid.SetRow(border, i);
                    Grid.SetColumn(border, j);
                    board.Children.Add(border);
                    var innerGrid = new Grid();
                    border.Child = innerGrid;
                    containers[i, j] = innerGrid;

                    var canvas = new Canvas { Background = new SolidColorBrush(Colors.Azure) };
                    innerGrid.Children.Add(canvas);
                    cells[i, j] = canvas;
                }
            }
        }

        public void DrawNumber(int r, int c, int number)
        {
            var grid = containers[r, c];
            var textBlock = new TextBlock 
            { 
                Text = number.ToString(CultureInfo.InvariantCulture),
                HorizontalAlignment = HorizontalAlignment.Center,
                VerticalAlignment = VerticalAlignment.Center,
                FontSize = 13,
                FontFamily = new FontFamily("Consolas")
            };
            textBlock.Opacity = 0.2;
            grid.Children.Add(textBlock);
            MouseButtonEventHandler handler = (s, e) =>
            {
                if (Keyboard.GetKeyStates(Key.LeftCtrl) == KeyStates.Down)
                {
                    var textBox = new TextBox
                    {
                        TextAlignment = TextAlignment.Center
                    };
                    textBox.LostFocus += (ss, ee) =>
                    {
                        int newNumber;
                        bool success = int.TryParse(textBox.Text, out newNumber);
                        grid.Children.Remove(textBox);
                        grid.Children.Add(textBlock);
                        if (success)
                        {
                            textBlock.Text = newNumber.ToString(CultureInfo.InvariantCulture);
                            if (boardChanged != null)
                            {
                                boardChanged(r, c, newNumber);
                            }
                        }
                    };
                    grid.Children.Add(textBox);
                    grid.Children.Remove(textBlock);
                    textBox.Focus();
                }
                e.Handled = true;
            };
            textBlock.MouseLeftButtonDown += handler;
            grid.MouseLeftButtonDown += handler;
            cells[r, c].MouseLeftButtonDown += handler;
        }

        public void DrawCell(int r, int c, Dir enter, Dir leave)
        {
            var children = cells[r, c].Children;
            children.Clear();
            switch (enter)
            {
                case Dir.Left:
                    DrawH2(children);
                    if (enter == leave) DrawLeftArrow(children);
                    else DrawLeftArrow(children, WM / 2 - AL / 2, 0);
                    break;
                case Dir.Right:
                    DrawH1(children);
                    if (enter == leave) DrawRightArrow(children);
                    else DrawRightArrow(children, -WM / 2 + AL / 2, 0);
                    break;
                case Dir.Up:
                    DrawV2(children);
                    if (enter == leave) DrawUpArrow(children);
                    else DrawUpArrow(children, 0, HM / 2 - AL / 2);
                    break;
                case Dir.Down:
                    DrawV1(children);
                    if (enter == leave) DrawDownArrow(children);
                    else DrawDownArrow(children, 0, -HM / 2 + AL / 2);
                    break;
            }
            switch (leave)
            {
                case Dir.Left:
                    DrawH1(children);
                    if (enter != leave) DrawLeftArrow(children, -WM / 2 - AL / 2, 0);
                    break;
                case Dir.Right:
                    DrawH2(children);
                    if (enter != leave) DrawRightArrow(children, WM / 2 + AL / 2, 0);
                    break;
                case Dir.Up:
                    DrawV1(children);
                    if (enter != leave) DrawUpArrow(children, 0, -HM / 2 - AL / 2);
                    break;
                case Dir.Down:
                    DrawV2(children);
                    if (enter != leave) DrawDownArrow(children, 0, HM / 2 + AL / 2);
                    break;
            }
        }

        private void DrawLeftArrow(UIElementCollection children, int dx = 0, int dy = 0)
        {
            DrawLine(children, new Line
            {
                X1 = WM,
                Y1 = HM,
                X2 = WM + AL,
                Y2 = HM + AL
            }, dx, dy);
            DrawLine(children, new Line
            {
                X1 = WM,
                Y1 = HM,
                X2 = WM + AL,
                Y2 = HM - AL
            }, dx, dy);
        }

        private void DrawRightArrow(UIElementCollection children, int dx = 0, int dy = 0)
        {
            DrawLine(children, new Line
            {
                X1 = WM,
                Y1 = HM,
                X2 = WM - AL,
                Y2 = HM + AL
            }, dx, dy);
            DrawLine(children, new Line
            {
                X1 = WM,
                Y1 = HM,
                X2 = WM - AL,
                Y2 = HM - AL
            }, dx, dy);
        }

        private void DrawUpArrow(UIElementCollection children, int dx = 0, int dy = 0)
        {
            DrawLine(children, new Line
            {
                X1 = WM,
                Y1 = HM,
                X2 = WM + AL,
                Y2 = HM + AL
            }, dx, dy);
            DrawLine(children, new Line
            {
                X1 = WM,
                Y1 = HM,
                X2 = WM - AL,
                Y2 = HM + AL
            }, dx, dy);
        }

        private void DrawDownArrow(UIElementCollection children, int dx = 0, int dy = 0)
        {
            DrawLine(children, new Line
            {
                X1 = WM,
                Y1 = HM,
                X2 = WM + AL,
                Y2 = HM - AL
            }, dx, dy);
            DrawLine(children, new Line
            {
                X1 = WM,
                Y1 = HM,
                X2 = WM - AL,
                Y2 = HM - AL
            }, dx, dy);
        }

        private void DrawH1(UIElementCollection children)
        {
            DrawLine(children, new Line
            {
                X1 = 0, Y1 = HM,
                X2 = WM, Y2 = HM
            });
        }

        private void DrawH2(UIElementCollection children)
        {
            DrawLine(children, new Line
            {
                X1 = WM, Y1 = HM,
                X2 = WW, Y2 = HM
            });
        }

        private void DrawV1(UIElementCollection children)
        {
            DrawLine(children, new Line
            {
                X1 = WM, Y1 = 0,
                X2 = WM, Y2 = HM
            });
        }

        private void DrawV2(UIElementCollection children)
        {
            DrawLine(children, new Line
            {
                X1 = WM, Y1 = HM,
                X2 = WM, Y2 = HH
            });
        }

        private void DrawLine(UIElementCollection children, Line line, int dx = 0, int dy = 0)
        {
            line.Stroke = new SolidColorBrush(Colors.Red);
            line.StrokeThickness = StrokeThickness;
            line.X1 += dx;
            line.X2 += dx;
            line.Y1 += dy;
            line.Y2 += dy;
            children.Add(line);
        }
    }
}
