#include <vector>
#include <memory>
#include <tuple>
#include <limits>
#include <iostream>
#include <string>
#include <map>
#include <iomanip>

using namespace std;

enum class Feature {
  Easy,
  AI,
  System,
  Thy, // not sure what this stands for
  Morning,
};

const map<Feature, string> featureNames {
    { Feature::Easy, "Easy" },
    { Feature::AI, "AI" },
    { Feature::System, "System" },
    { Feature::Thy, "Thy" },
    { Feature::Morning, "Morning" },
};

const int totalFeatures = featureNames.size();

using FeatureValues = vector < bool > ;

struct Example {
  FeatureValues features;
  bool label; // like, hate
};

using Examples = vector < Example >;

class Node {
public:
  virtual ~Node() {}
  bool virtual predict(const FeatureValues& features) const = 0;
  
  void print() {
    print(0);
  }

  virtual void print(int indent) = 0;
};

using NodePtr = unique_ptr < Node > ;
using DecisionTree = NodePtr;

class BranchNode : public Node {
public:
  BranchNode(Feature f, NodePtr l, NodePtr r) : feature(f), left(move(l)), right(move(r)) { }
  bool predict(const FeatureValues& features) const override {
    if (features[static_cast<int>(feature)]) {
      return left->predict(features);
    } else {
      return right->predict(features);
    }
  }
  
private:
  void print(int indent) override {
    cout << setw(indent) << featureNames.at(feature) << endl;
    left->print(indent + 10);
    right->print(indent + 10);
  }
  
  Feature feature;
  NodePtr left;
  NodePtr right;
};

class Leaf : public Node {
public:
  Leaf(bool p) : prediction(p) {}
  bool predict(const FeatureValues& features) const override {
    return prediction;
  }

private:
  void print(int indent) override {
    cout << setw(indent) << (prediction ? "like" : "hate") << endl;
  }

  bool prediction;
};

struct Config {
  explicit Config(int d = numeric_limits<int>::max()) : maxDepth(d) { }

  Config decrementDepth() const {
    return Config {maxDepth - 1};
  }

  int maxDepth;
};

pair<Examples, Examples> partition(const Examples& examples, int feature) {
  Examples left, right;
  for (const auto& e : examples) {
    if (e.features[feature]) {
      left.push_back(e);
    } else {
      right.push_back(e);
    }
  }
  return make_pair(move(left), move(right));
}

// <prediction, error>
pair<bool, size_t> getPrediction(const Examples& examples) {
  size_t positive = 0;
  for (auto& e : examples) {
    positive += e.label;
  }
  auto negative = examples.size() - positive;
  if (positive >= negative) {
    return make_pair(true, negative);
  } else {
    return make_pair(false, positive);
  }
}

DecisionTree train(Examples examples, Config config = Config {}) {
  auto prediction = getPrediction(examples);
  if (prediction.second == 0 || config.maxDepth <= 0) {
    // all examples have the same label
    return make_unique<Leaf>(prediction.first);
  }
  
  // greedily build out the decision tree
  // note that setting best score to prediction.second 
  // (the prediction without splitting) is wrong.
  // The reason is that there could be a minority of examples
  // that could not be "separated out" through a single
  // splitting.
  size_t bestScore = numeric_limits<size_t>::max();
  int bestFeature = -1;

  for (int i = 0; i < totalFeatures; ++i) {
    Examples left, right;
    tie(left, right) = partition(examples, i);

    if (left.empty() || right.empty()) {
      // the partition does not have any value
      continue;
    }

    auto L = getPrediction(left);
    auto R = getPrediction(right);
    auto score = L.second + R.second;
    if (score < bestScore) {
      bestScore = score;
      bestFeature = i;
    }
  }

  if (bestFeature == -1) {
    // we cannot further partition the examples
    // predict to the best of our capability
    return make_unique<Leaf>(prediction.first);
  }

  Examples left, right;
  tie(left, right) = partition(examples, bestFeature);
  return make_unique<BranchNode>(static_cast<Feature>(bestFeature), 
                                 train(left, config.decrementDepth()), 
                                 train(right, config.decrementDepth()));
}

double test(const Examples& examples, const DecisionTree& tree) {
  size_t error = 0;
  for (auto& e : examples) {
    bool prediction = tree->predict(e.features);
    error += prediction != e.label;
  }
  return 1.0 * error / examples.size();
}

int main() {
  Examples examples {
      { { 1, 1, 0, 1, 0 }, true },
      { { 1, 1, 0, 1, 0 }, true },
      { { 0, 1, 0, 0, 0 }, true },
      { { 0, 0, 0, 1, 0 }, true },
      { { 0, 1, 1, 0, 1 }, true },
      { { 1, 1, 0, 0, 0 }, true },
      { { 1, 1, 0, 1, 0 }, true },
      { { 0, 1, 0, 1, 0 }, true },
      { { 0, 0, 0, 0, 1 }, true },
      { { 1, 0, 0, 1, 1 }, true },
      { { 0, 1, 0, 1, 0 }, true },
      { { 1, 1, 1, 1, 1 }, true },
      { { 1, 1, 1, 0, 1 }, false },
      { { 0, 0, 1, 1, 0 }, false },
      { { 0, 0, 1, 0, 1 }, false },
      { { 1, 0, 1, 0, 1 }, false },
      { { 0, 0, 1, 1, 0 }, false },
      { { 0, 1, 1, 0, 1 }, false },
      { { 1, 0, 1, 0, 0 }, false },
      { { 1, 0, 1, 0, 1 }, false },
  };

  auto tree = train(examples, Config {3});
  tree->print();
  cout << test(examples, tree) << endl;
}