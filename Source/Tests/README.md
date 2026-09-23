# Unreal Engine Automation Tests — Source Integration

**Project**: Project Ascendant  
**Target Engine**: Unreal Engine 5.8.2

---

## Overview

All automated test classes in C++ use the Unreal Engine Automation Testing Framework.
Test files inside `Source/ProjectAscendant/Tests/` are automatically discovered when compiling in `Editor` or `Development` builds.

## Class Architecture

- **Unit Tests**: Use `IMPLEMENT_SIMPLE_AUTOMATION_TEST(ClassName, TestPath, TestFlags)`
- **Complex Tests**: Use `IMPLEMENT_COMPLEX_AUTOMATION_TEST(ClassName, TestPath, TestFlags)` for data-driven test generation.

```cpp
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FProjectAscendantSampleTest,
    "ProjectAscendant.Foundation.SampleVerification",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FProjectAscendantSampleTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("Sample check"), true);
    return true;
}

#endif
```
