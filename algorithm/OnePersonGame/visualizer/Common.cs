using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace OnePersonGameVisualizer
{
    // TODO: why does this have to be public?
    public enum Dir
    {
        None,
        Left,
        Right,
        Up,
        Down
    }

    static class Dirs
    {
        public static Dir[,] Values =
        {
            { Dir.None, Dir.None },
            { Dir.Left, Dir.Left },
            { Dir.Left, Dir.Up },
            { Dir.Left, Dir.Down },
            { Dir.Right, Dir.Right },
            { Dir.Right, Dir.Up },
            { Dir.Right, Dir.Down },
            { Dir.Up, Dir.Up },
            { Dir.Up, Dir.Left },
            { Dir.Up, Dir.Right },
            { Dir.Down, Dir.Down },
            { Dir.Down, Dir.Left },
            { Dir.Down, Dir.Right },
            { Dir.Left, Dir.None },
            { Dir.Right, Dir.None },
            { Dir.Up, Dir.None },
            { Dir.Down, Dir.None },
            { Dir.None, Dir.Left },
            { Dir.None, Dir.Right },
            { Dir.None, Dir.Up },
            { Dir.None, Dir.Down }
        };
    }
}
