#include "Computations.h"
#include "ComputationsImpl.h"

namespace Reactive
{

  Computations::Computations()
      : m_impl(std::make_shared<ComputationsImpl>())
  {
  }

  Computations::~Computations() = default;

  void Computations::add(std::function<void()> &&cb) const
  {
    m_impl->add(std::move(cb));
  }

}
