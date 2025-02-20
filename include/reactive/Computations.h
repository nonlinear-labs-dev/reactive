#pragma once

#include <functional>
#include <memory>

namespace Reactive
{
  class ComputationsImpl;

  class Computations
  {
   public:
    Computations();
    virtual ~Computations();
    void add(std::function<void()> &&cb) const;

   private:
    std::shared_ptr<ComputationsImpl> m_impl;
  };
}
