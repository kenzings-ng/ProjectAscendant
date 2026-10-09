// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Character/PAPaperdollTypes.h"
#include "Inventory/PAInventoryTypes.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * X7 — Tag class chuẩn Class.Line.<Nhánh>.<Class> và tên độ hiếm trang bị không chứa "Tier".
 *
 *  - Đủ 16 tag class (DECISIONS.md §2, §8) được đăng ký trong Config/DefaultGameplayTags.ini.
 *  - Các tag cũ bị cấm (Class.Vanguard, Class.Ranger, Class.Arcanist, Class.Acolyte) KHÔNG được đăng ký.
 *  - FPAPaperdollConstants::GetTagForClass trả về đúng tag Class.Line.* cho từng class; không dựng "Class.<Tên>".
 *  - DisplayName của EPAItemRarity không chứa "Tier" (DECISIONS.md §1).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAClassLineTagsTest,
	"ProjectAscendant.Foundation.Character.ClassLineTags",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAClassLineTagsTest::RunTest(const FString& Parameters)
{
	struct FExpectedClass
	{
		const TCHAR* ClassName;
		const TCHAR* TagName;
	};

	// Nguồn: DECISIONS.md §2 (16 class) + §8 (Class.Line.<Nhánh>.<Class>, Apex = Class.Line.Apex.GodSlayer).
	const FExpectedClass Expected[] = {
		{ TEXT("Vanguard"),       TEXT("Class.Line.Guard.Vanguard") },
		{ TEXT("Templar"),        TEXT("Class.Line.Guard.Templar") },
		{ TEXT("Berserker"),      TEXT("Class.Line.Guard.Berserker") },
		{ TEXT("Swordmaster"),    TEXT("Class.Line.Guard.Swordmaster") },
		{ TEXT("DragonKnight"),   TEXT("Class.Line.Guard.DragonKnight") },
		{ TEXT("VoidBlade"),      TEXT("Class.Line.Guard.VoidBlade") },
		{ TEXT("Ranger"),         TEXT("Class.Line.Scout.Ranger") },
		{ TEXT("Shadowblade"),    TEXT("Class.Line.Scout.Shadowblade") },
		{ TEXT("PhantomStalker"), TEXT("Class.Line.Scout.PhantomStalker") },
		{ TEXT("Arcanist"),       TEXT("Class.Line.Caster.Arcanist") },
		{ TEXT("Elementalist"),   TEXT("Class.Line.Caster.Elementalist") },
		{ TEXT("Chronomancer"),   TEXT("Class.Line.Caster.Chronomancer") },
		{ TEXT("Acolyte"),        TEXT("Class.Line.Faith.Acolyte") },
		{ TEXT("Inquisitor"),     TEXT("Class.Line.Faith.Inquisitor") },
		{ TEXT("Seraph"),         TEXT("Class.Line.Faith.Seraph") },
		{ TEXT("GodSlayer"),      TEXT("Class.Line.Apex.GodSlayer") },
	};

	// -------------------------------------------------------------------------
	// 1. 16 tag Class.Line.* đã đăng ký và khớp helper
	// -------------------------------------------------------------------------
	TestEqual(TEXT("Expected table has 16 classes"), static_cast<int32>(UE_ARRAY_COUNT(Expected)), 16);
	TestEqual(TEXT("GetAllClassTagNames returns 16 tags"), FPAPaperdollConstants::GetAllClassTagNames().Num(), 16);

	for (const FExpectedClass& Entry : Expected)
	{
		const FName TagName(Entry.TagName);
		const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName, false);
		TestTrue(FString::Printf(TEXT("%s is registered"), Entry.TagName), Tag.IsValid());
		TestTrue(FString::Printf(TEXT("%s listed by GetAllClassTagNames"), Entry.TagName),
			FPAPaperdollConstants::GetAllClassTagNames().Contains(TagName));

		const FGameplayTag Mapped = FPAPaperdollConstants::GetTagForClass(FName(Entry.ClassName));
		TestTrue(FString::Printf(TEXT("GetTagForClass(%s) is valid"), Entry.ClassName), Mapped.IsValid());
		TestEqual(FString::Printf(TEXT("GetTagForClass(%s) == %s"), Entry.ClassName, Entry.TagName),
			Mapped.GetTagName(), TagName);
		TestEqual(FString::Printf(TEXT("GetClassNameFromTag(%s) == %s"), Entry.TagName, Entry.ClassName),
			FPAPaperdollConstants::GetClassNameFromTag(Mapped), FName(Entry.ClassName));
	}

	// Tên có khoảng trắng (như trong GDD) vẫn ánh xạ đúng.
	TestEqual(TEXT("GetTagForClass(\"Dragon Knight\")"),
		FPAPaperdollConstants::GetTagForClass(FName(TEXT("Dragon Knight"))).GetTagName(), FName(TEXT("Class.Line.Guard.DragonKnight")));
	TestEqual(TEXT("GetTagForClass(\"God Slayer\")"),
		FPAPaperdollConstants::GetTagForClass(FName(TEXT("God Slayer"))).GetTagName(), FName(TEXT("Class.Line.Apex.GodSlayer")));

	// Tên không thuộc 16 class: không tự tạo tag mới.
	TestFalse(TEXT("Unknown class name yields no tag"),
		FPAPaperdollConstants::GetTagForClass(FName(TEXT("NotAClass"))).IsValid());

	// -------------------------------------------------------------------------
	// 2. Tag cũ bị cấm không được đăng ký (DECISIONS.md §8)
	// -------------------------------------------------------------------------
	const TCHAR* BannedTags[] = {
		TEXT("Class.Vanguard"),
		TEXT("Class.Ranger"),
		TEXT("Class.Arcanist"),
		TEXT("Class.Acolyte"),
	};
	for (const TCHAR* Banned : BannedTags)
	{
		TestFalse(FString::Printf(TEXT("Banned tag %s is NOT registered"), Banned),
			FGameplayTag::RequestGameplayTag(FName(Banned), false).IsValid());
	}

	// Mọi tag con trực tiếp của "Class" chỉ được là "Class.Line" (không còn Class.<Tên>, Class.TierX, Class.RankX).
	{
		FGameplayTagContainer AllTags;
		// Chỉ lấy tag đăng ký tường minh (ini/native); tag cha ngầm định "Class", "Class.Line", "Class.Line.<Nhánh>" bị loại.
		UGameplayTagsManager::Get().RequestAllGameplayTags(AllTags, /*OnlyIncludeDictionaryTags=*/true);
		int32 ClassLineLeafCount = 0;
		for (const FGameplayTag& Tag : AllTags)
		{
			const FString TagStr = Tag.ToString();
			if (!TagStr.StartsWith(TEXT("Class.")))
			{
				continue;
			}
			TestTrue(FString::Printf(TEXT("Class tag %s uses Class.Line.* form"), *TagStr),
				TagStr.StartsWith(TEXT("Class.Line.")));

			TArray<FString> Parts;
			TagStr.ParseIntoArray(Parts, TEXT("."));
			TestEqual(FString::Printf(TEXT("Class tag %s has exactly 4 segments"), *TagStr), Parts.Num(), 4);
			if (Parts.Num() == 4)
			{
				++ClassLineLeafCount;
			}
		}
		TestEqual(TEXT("Exactly 16 explicit Class.Line.<Line>.<Class> tags registered"), ClassLineLeafCount, 16);
	}

	// -------------------------------------------------------------------------
	// 3. EPAItemRarity: tên hiển thị không chứa "Tier" (DECISIONS.md §1)
	// -------------------------------------------------------------------------
	{
		const UEnum* RarityEnum = StaticEnum<EPAItemRarity>();
		TestNotNull(TEXT("EPAItemRarity is reflected"), RarityEnum);
		if (RarityEnum)
		{
			const TCHAR* ExpectedNames[] = { TEXT("Common"), TEXT("Uncommon"), TEXT("Rare"), TEXT("Epic"), TEXT("Legendary") };
			const EPAItemRarity Values[] = { EPAItemRarity::Common, EPAItemRarity::Uncommon, EPAItemRarity::Rare, EPAItemRarity::Epic, EPAItemRarity::Legendary };
			for (int32 i = 0; i < UE_ARRAY_COUNT(Values); ++i)
			{
				const FString DisplayName = RarityEnum->GetDisplayNameTextByValue(static_cast<int64>(Values[i])).ToString();
				TestEqual(FString::Printf(TEXT("EPAItemRarity display name for %s"), ExpectedNames[i]), DisplayName, FString(ExpectedNames[i]));
			}
			for (int32 Index = 0; Index < RarityEnum->NumEnums() - 1; ++Index)
			{
				const FString DisplayName = RarityEnum->GetDisplayNameTextByIndex(Index).ToString();
				TestFalse(FString::Printf(TEXT("EPAItemRarity '%s' has no 'Tier'"), *DisplayName), DisplayName.Contains(TEXT("Tier")));
			}
		}
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
