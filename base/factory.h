// Basic Factory class for creating instances of a specific class.

#ifndef BASE_FACTORY_H_
#define BASE_FACTORY_H_

// A generic factory for create instances of type T.
template <typename T> class Factory {
public:
  Factory(const std::string &name, std::function<T *()> new_fn)
      : name_(name), new_fn_(new_fn) {}

  // Name of the instance type.
  const std::string &name() const { return name_; }

  // Creates and returns an instance of T.
  std::unique_ptr<T> operator()() const {
    return std::unique_ptr<T>(new_fn_());
  }

private:
  std::string name_;
  std::function<T *()> new_fn_;
};

#endif /* BASE_FACTORY_H_ */
