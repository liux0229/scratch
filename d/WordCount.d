import std.stdio;
import std.string;
import std.array;
import std.algorithm;
import std.regex;

void main() {
  uint[string] counts;
  foreach (line; stdin.byLine()) {
    string[] words = split(strip(line.idup), regex(r"[\W]+"));
    foreach (word; words) {
      // TODO: removechars does not support regex
      if (!word.empty) {
        ++counts[word];
      }
    }
  }
  string[] words = counts.keys;
  sort!((a, b) { return counts[a] > counts[b]; })(words);
  foreach (w; words) {
    writefln("%s: %s", w, counts[w]);
  }
}
