// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UI/PACharacterSelectTypes.h"
#include "Engine/Texture2D.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPACharacterSelectTests
 *
 * Kiểm tra tự động cho hệ thống chọn nhân vật (Character Select):
 *  - Mỗi class nền tảng (Vanguard, Ranger, Arcanist) phải có đường dẫn texture riêng biệt.
 *  - Không được dùng fallback chung về T_Vanguard_Spritesheet.
 *  - Mỗi asset texture phải load thành công từ .uasset thật.
 *  - Cấu hình Texture Filter phải là TF_Nearest (Pixel Art chuẩn).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPACharacterSelectTests,
	"ProjectAscendant.UI.CharacterSelectTextures",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPACharacterSelectTests::RunTest(const FString& Parameters)
{
	const TArray<EPACharacterClass> ClassesToTest = {
		EPACharacterClass::Vanguard,
		EPACharacterClass::Ranger,
		EPACharacterClass::Arcanist
	};

	TMap<EPACharacterClass, FString> ClassPaths;

	for (EPACharacterClass ClassType : ClassesToTest)
	{
		const FPACharacterClassInfo Info = FPACharacterClassRegistry::GetClassInfo(ClassType);
		TestFalse(FString::Printf(TEXT("Class %d SpritesheetAssetPath must not be empty"), static_cast<int32>(ClassType)), Info.SpritesheetAssetPath.IsEmpty());

		// Verify asset exists and loads as a valid UTexture2D
		UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, *Info.SpritesheetAssetPath);
		TestNotNull(FString::Printf(TEXT("Class %d must load valid Texture2D from '%s'"), static_cast<int32>(ClassType), *Info.SpritesheetAssetPath), Texture);

		if (Texture)
		{
			// Verify pixel filter is Nearest
			TestEqual(FString::Printf(TEXT("Class %d Texture Filter must be TF_Nearest"), static_cast<int32>(ClassType)), Texture->Filter, TextureFilter::TF_Nearest);
		}

		ClassPaths.Add(ClassType, Info.SpritesheetAssetPath);
	}

	// Verify that each class has a DISTINCT spritesheet asset path (no fallback sharing)
	TestNotEqual(TEXT("Ranger and Vanguard must have distinct textures"),
		ClassPaths[EPACharacterClass::Ranger], ClassPaths[EPACharacterClass::Vanguard]);
	TestNotEqual(TEXT("Arcanist and Vanguard must have distinct textures"),
		ClassPaths[EPACharacterClass::Arcanist], ClassPaths[EPACharacterClass::Vanguard]);
	TestNotEqual(TEXT("Ranger and Arcanist must have distinct textures"),
		ClassPaths[EPACharacterClass::Ranger], ClassPaths[EPACharacterClass::Arcanist]);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
