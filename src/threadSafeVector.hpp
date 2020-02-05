#ifndef threadSafeVector_hpp
#define threadSafeVector_hpp

#include <vector>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <string>

class SafeVector {
public:
    SafeVector();
    SafeVector(const SafeVector& orig);
    ~SafeVector() = default;

    void insert(int in, const int index);
    void push_back(int in);
    const int& at(std::size_t pos) const;
    std::size_t size() const;
    std::vector<int>::iterator begin();
    std::vector<int>::iterator end();
    std::vector<int>& toVector();

private:
    std::vector<int> vec_;
    std::mutex mut_;
    std::condition_variable cond_;
};

#endif /* threadSafeVector_hpp */
