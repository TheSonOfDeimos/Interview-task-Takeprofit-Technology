#include "threadSafeVector.hpp"
SafeVector::SafeVector()
    : vec_(),
    mut_(),
    cond_()
{
}

SafeVector::SafeVector(const SafeVector& orig)
    : vec_(orig.vec_),
    mut_(),
    cond_()
{
}

void SafeVector::insert(int in, const int index)
{
    std::lock_guard<std::mutex> lock(mut_);
    vec_[index] = std::move(in);
    cond_.notify_one();
}

void SafeVector::push_back(int in)
{
    std::lock_guard<std::mutex> lock(mut_);
    vec_.push_back(std::move(in));
    cond_.notify_one();
    
}

std::vector<int>::iterator SafeVector::begin()
{
    return vec_.begin();
}

std::vector<int>::iterator SafeVector::end()
{
    return vec_.end();
}

std::vector<int>& SafeVector::toVector()
{
    return vec_;
}

std::size_t SafeVector::size() const
{
    return vec_.size();
}

const int &SafeVector::at(std::size_t pos) const { 
    return vec_.at(pos);
}


