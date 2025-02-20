#pragma once

namespace Reactive
{
  class Deferrable
  {
   public:
    Deferrable() = default;
    virtual void doDeferred() = 0;
  };
}