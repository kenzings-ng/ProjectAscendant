// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PACharacterSelectTypes.generated.h"

/**
 * EPACharacterClass
 *
 * Danh sách 3 chức nghiệp nền tảng cốt lõi trong Project Ascendant (foundational-classes.md).
 */
UENUM(BlueprintType)
enum class EPACharacterClass : uint8
{
	Vanguard UMETA(DisplayName = "Chiến Binh (Vanguard)"),
	Ranger   UMETA(DisplayName = "Du Hiệp (Ranger)"),
	Arcanist UMETA(DisplayName = "Thuật Sĩ (Arcanist)")
};

/**
 * FPACharacterClassInfo
 *
 * Cấu trúc dữ liệu mô tả chức nghiệp, chỉ số khởi đầu, phong cách và kỹ năng.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPACharacterClassInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|CharacterSelect")
	EPACharacterClass ClassType = EPACharacterClass::Vanguard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|CharacterSelect")
	FString ClassTag = TEXT("Class.Vanguard");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|CharacterSelect")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|CharacterSelect")
	FText Tagline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|CharacterSelect")
	FText LoreDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|CharacterSelect")
	FText PrimaryRole;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|CharacterSelect")
	FText PrimaryWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|CharacterSelect")
	float BaseHealth = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|CharacterSelect")
	float BaseStamina = 110.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|CharacterSelect")
	float BaseMana = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|CharacterSelect")
	float BasePosture = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|CharacterSelect")
	float MoveSpeed = 520.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|CharacterSelect")
	TArray<FString> KeyAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|CharacterSelect")
	FString SpritesheetAssetPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|CharacterSelect")
	FString AnimBlueprintPath;
};

class PROJECTASCENDANT_API FPACharacterClassRegistry
{
public:
	static FPACharacterClassInfo GetClassInfo(EPACharacterClass InClass);
	static TArray<FPACharacterClassInfo> GetAllClasses();
};
