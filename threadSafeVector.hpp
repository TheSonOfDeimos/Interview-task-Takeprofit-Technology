#ifndef threadSafeVector_hpp
#define threadSafeVector_hpp

#include <vector>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <string>

class SafeVector {
public:
    SafeVector()
        : vec(),
        mut(),
        cond()
    {
    }
    
    SafeVector(const SafeVector& orig)
        : vec(orig.vec),
        mut(),
        cond()
    {
    }
    
    ~SafeVector()
    {

    }

    void insert(int in, const int index);
    void push_back(int in);
    int& operator[](const int index);
    std::vector<int>::iterator begin();
    std::vector<int>::iterator end();
    std::vector<int>& toVector();

private:
    std::vector<int> vec;
    std::mutex mut;
    std::condition_variable cond;
};

#endif /* threadSafeVector_hpp */
