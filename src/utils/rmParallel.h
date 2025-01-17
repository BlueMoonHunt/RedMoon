#pragma once
#include <functional>

namespace rm
{
    void parExecution(size_t totalWork, std::function<void(size_t,size_t)> function);
} // namespace rm
