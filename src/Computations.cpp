#include <reactive/Computations.h>
#include "ComputationsImpl.h"

namespace Reactive
{

  Computations::Computations()
      : m_impl(std::make_shared<ComputationsImpl>())
  {
  }

  Computations::~Computations()
  {
    m_impl->clear();
  }

  void Computations::add(std::function<void()> &&cb) const
  {
    m_impl->add(std::move(cb));
  }

  void Computations::add(std::function<void()> &&cb, uint32_t depth) const
  {
    m_impl->add(std::move(cb), depth);
  }

}
