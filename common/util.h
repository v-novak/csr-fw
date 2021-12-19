#ifndef UTIL_H
#define UTIL_H

template <typename T>
class smoothened
{
  T val;
  T max_d;

public:
  smoothened(T initial_value, T max_delta): val(initial_value), max_d(max_delta) {}
  smoothened() = delete;
  ~smoothened() {}

  void set(T value) {
    if (value - val > max_d)
      val += max_d;
    else if (val - value > max_d)
      val -= max_d;
    else
      val = value;
  }

  T get() {
    return val;
  }

  operator T() const {
    return val;
  }

  smoothened& operator= (const T& value) {
    set(value);
    return *this;
  }
};

template <typename T>
T restrict(T value, T min_value, T max_value, T zero_threshold)
{
    if (value >= -zero_threshold && value <= zero_threshold ) return (T)0;
    else if (value > max_value) return max_value;
    else if (value < min_value) return min_value;
    else return value;
}

#endif // UTIL_H
