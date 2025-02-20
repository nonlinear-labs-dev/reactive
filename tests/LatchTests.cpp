#include <catch2/catch_all.hpp>
#include <reactive/Computations.h>
#include <reactive/Latch.h>
#include <reactive/Var.h>

using namespace Reactive;

TEST_CASE("Latch")
{
  struct User
  {
    const std::string &getValue() const
    {
      return m_latch.doLatch([this] {
        m_executionCount++;
        return std::to_string(m_value);
      });
    }

    Var<int> m_value { 0 };
    mutable Latch<std::string> m_latch;
    mutable int m_executionCount = 0;
  };

  Computations computations;
  User user;

  WHEN("Latch is read and none of the used vars ever changed")
  {
    auto val = user.getValue();

    THEN("attached computation is executed exactly once")
    {
      CHECK(user.m_executionCount == 1);

      THEN("latch produces correct value")
      {
        CHECK(val == "0");
      }

      AND_WHEN("latch is read again")
      {
        val = user.getValue();

        THEN("attached computation is not executed again")
        {
          CHECK(user.m_executionCount == 1);

          AND_THEN("latch produces correct value")
          {
            CHECK(val == "0");
          }
        }
      }
    }
  }

  WHEN("Vars used by latch are changed")
  {
    user.m_value = 2;

    THEN("latch produces correct value by executing producer once")
    {
      CHECK(user.getValue() == "2");
      CHECK(user.m_executionCount == 1);

      AND_WHEN("latch is changed again")
      {
        user.m_value = 3;

        THEN("latch produces correct value by executing producer again")
        {
          CHECK(user.getValue() == "3");
          CHECK(user.m_executionCount == 2);
        }
      }

      AND_WHEN("latch is set to same value")
      {
        user.m_value = 2;

        THEN("latch produces correct value without executing producer again")
        {
          CHECK(user.getValue() == "2");
          CHECK(user.m_executionCount == 1);
        }
      }
    }
  }

  WHEN("Computations are nested and inner computation uses reactive var")
  {
    int outerCounter = 0;
    int innerCounter = 0;
    std::string readValue;

    Computations outer;
    Computations inner;

    outer.add([&] {
      outerCounter++;

      inner.add([&] {
        innerCounter++;
        readValue = user.getValue();
      });
    });

    REQUIRE(innerCounter == 1);
    REQUIRE(outerCounter == 1);

    AND_WHEN("reactive var is changed")
    {
      user.m_value = 1;

      THEN("only inner computation was executed")
      {
        CHECK(innerCounter == 2);
        CHECK(outerCounter == 1);
      }

      WHEN("value of latch is read")
      {
        CHECK(user.getValue() == "1");

        THEN("inner computation was executed")
        {
          CHECK(innerCounter == 2);
          CHECK(outerCounter == 1);
        }
      }
    }
  }
}

TEST_CASE("Computation sorting")
{
  WHEN("Computations are nested")
  {
    int outerCalls = 0;
    int innerCalls = 0;

    Var<int> usedFromOuter;
    Var<int> usedFromInner;

    Computations outer;
    std::unique_ptr<Computations> inner;

    outer.add([&] {
      outerCalls++;
      usedFromOuter.get();

      inner = std::make_unique<Computations>();

      inner->add([&] {
        innerCalls++;
        usedFromInner.get();
      });
    });

    AND_WHEN("Both computations are invalidated, inner first")
    {
      {
        Deferrer deferred;
        usedFromInner = 1;
        usedFromOuter = 1;
      }

      THEN("Both computations are executed only twice")
      {
        /*
         * If computations were not sorted from outside-in, then innermost
         * computation would be calculated first (usedFromInner was modified first).
         * But we already know that outer computation is invalid, so could be the
         * data references by inner. Also, it saves a useless execution of inner.
         */
        CHECK(outerCalls == 2);
        CHECK(innerCalls == 2);
      }
    }
  }
}
