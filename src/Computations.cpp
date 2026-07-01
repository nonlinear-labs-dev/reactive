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
    if(m_impl)
      m_impl->cancelPending();
  }

  void Computations::add(std::function<void()> &&cb) const
  {
    m_impl->add(std::move(cb));
  }

  void Computations::cancelPending() const
  {
    m_impl->cancelPending();
  }

}
