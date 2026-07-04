#pragma once

#include <memory>
#include <vector>

namespace Reactive
{
  class Deferrable;

  class Deferrer
  {
   public:
    Deferrer();
    ~Deferrer();

    static void add(std::shared_ptr<Deferrable> pending);
    static void remove(const std::shared_ptr<Deferrable>& pending);
    [[nodiscard]] const std::vector<std::weak_ptr<Deferrable>>& getPending() const;

   private:
    std::vector<std::weak_ptr<Deferrable>> m_pending;

    friend struct DeferrerTester;
  };
}
