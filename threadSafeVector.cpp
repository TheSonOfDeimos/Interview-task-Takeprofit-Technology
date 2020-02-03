#include "threadSafeVector.hpp"
/*SafeVector::SafeVector()
    : vec(),
    mut(),
    cond()
{
}

SafeVector::SafeVector(const SafeVector& orig)
    : vec(orig.vec),
    mut(),
    cond()
{
}
*/
void SafeVector::insert(int in, const int index)
{
    std::lock_guard<std::mutex> lock(mut);
    vec[index] = std::move(in);
    cond.notify_one();
}

void SafeVector::push_back(int in)
{
    std::lock_guard<std::mutex> lock(mut);
    vec.push_back(std::move(in));
    cond.notify_one();
    
}

int& SafeVector::operator[](const int index)
{
    return vec[index];
}

std::vector<int>::iterator SafeVector::begin()
{
    return vec.begin();
}

std::vector<int>::iterator SafeVector::end()
{
    return vec.end();
}

std::vector<int>& SafeVector::toVector()
{
    return vec;
}
