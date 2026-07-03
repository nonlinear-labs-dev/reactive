#pragma once

#include <functional>
#include <memory>
#include <cstdint>

namespace Reactive
{
  class ComputationsImpl;

  class Computations
  {
   public:
    Computations();
    virtual ~Computations();
    void add(std::function<void()> &&cb) const;
    void add(std::function<void()> &&cb, uint32_t depth) const;

   private:
    std::shared_ptr<ComputationsImpl> m_impl;
  };
}
