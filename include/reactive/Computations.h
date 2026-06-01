#pragma once

#include <functional>
#include <memory>

namespace Reactive
{
  class Computations
  {
   public:
    Computations();
    virtual ~Computations();
    void add(std::function<void()> &&cb) const;

   private:
    struct Impl;
    std::shared_ptr<Impl> m_impl;
  };
}
