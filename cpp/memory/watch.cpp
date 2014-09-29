#include <iostream>
#include <functional>
#include <string>
#include <functional>
#include <memory>
#include <atomic>

using namespace std;

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class Watch {
 public:
  using Callback = std::function<void ()>;
  Watch(Callback callback) : callback_(callback) {
  }
  void start() { 
    /* start listening to some external event to trigger notify() asynchronously */ 
  }
  
 private:
  /* triggered asynchronously by an external event */
  void notify() {
    callback_();
    /* 1) access to data fields */
  }
  
  Callback callback_;
};

class Watcher {
 public:
  void createWatch() {
    auto callback = [this] {
      createWatch();
      /* 2 */
      postProcess();
    };
    watch_ = make_unique<Watch>(callback);
    watch_->start();
  }
 private:
  void postProcess() {
    /* access to data fields */
  }
  
  std::unique_ptr<Watch> watch_;
};

int main()
{
   return 0;
}
