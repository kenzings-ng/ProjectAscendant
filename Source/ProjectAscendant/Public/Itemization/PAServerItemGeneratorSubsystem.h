// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Itemization/PAAffixTypes.h"
#include "Itemization/PASavedItemInstance.h"
#include "Inventory/PAInventoryTypes.h"
#include "PAServerItemGeneratorSubsystem.generated.h"

struct IConsoleCommand;

/**
 * UPAServerItemGeneratorSubsystem
 *
 * Subsystem phụ trách sinh vật phẩm sống động ngẫu nhiên có kiểm soát (Procedural Item & Affix Generation)
 * theo chuẩn Story item-002 (EPIC-ITEMIZATION-001, Sprint 6).
 *
 * Nguyên tắc kiến trúc:
 * 1. Dùng trực tiếp FPAAffixDefinitionRow::IsUnlockedAtForgeTier() để lọc pool affix theo Forge Tier hiện tại,
 *    không viết logic filter riêng.
 * 2. Phân bổ số dòng Affix theo Rarity:
 *    - Common: 0 affix.
 *    - Uncommon: 1-2 affixes.
 *    - Rare: 3 affixes (2 Prefixes + 1 Suffix).
 *    - Epic: 4 affixes (2 Prefixes + 2 Suffixes).
 *    - Legendary: 4 affixes (2 Prefixes + 2 Suffixes).
 * 3. Trần cứng (HardCap):
 *    - prefix_posture_dmg <= 35.0%
 *    - prefix_stagger_duration <= 0.5s
 *    (được đọc và áp dụng trực tiếp từ field HardCapPct của FPAAffixDefinitionRow).
 * 4. Không bao giờ sinh 2 affix trùng tên trên cùng 1 item.
 * 5. Cung cấp console command Cheat.GenerateItem <iLvl> <Rarity> [ForgeTier] để test nhanh.
 */
UCLASS()
class PROJECTASCENDANT_API UPAServerItemGeneratorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPAServerItemGeneratorSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Khởi tạo hoặc nạp bảng DataTable DT_AffixDefinitions */
	UFUNCTION(BlueprintCallable, Category = "Itemization|Generator")
	void InitializeAffixDefinitions(UDataTable* InDataTable = nullptr);

	/** Lấy danh sách định nghĩa affix đang nạp */
	const TArray<FPAAffixDefinitionRow>& GetAffixDefinitions() const { return CachedAffixDefinitions; }

	/**
	 * Sinh một phiên bản trang bị hoàn chỉnh (FPASavedItemInstance) với số dòng affix và giá trị theo chuẩn Story item-002
	 */
	UFUNCTION(BlueprintCallable, Category = "Itemization|Generator")
	FPASavedItemInstance GenerateItemInstance(
		FName ItemDefId,
		int32 ItemLevel,
		EPAItemRarity Rarity,
		EPAForgeTier ForgeTier = EPAForgeTier::None
	);

	/** Xác định bậc lò rèn mặc định dựa trên cấp độ trang bị (iLvl) */
	UFUNCTION(BlueprintPure, Category = "Itemization|Generator")
	static EPAForgeTier GetForgeTierFromItemLevel(int32 ItemLevel);

	/** Xác định bậc thuộc tính Affix (1..4) từ iLvl */
	UFUNCTION(BlueprintPure, Category = "Itemization|Generator")
	static int32 GetAffixTierFromItemLevel(int32 ItemLevel);

	/** Chuyển đổi Rarity Enum sang GameplayTag */
	UFUNCTION(BlueprintPure, Category = "Itemization|Generator")
	static FGameplayTag GetRarityTag(EPAItemRarity Rarity);

	/** Chuyển đổi chuỗi sang Rarity enum (hỗ trợ Cheat Console Command) */
	static EPAItemRarity ParseRarityFromString(const FString& InStr);

	/** Chuyển đổi chuỗi sang ForgeTier enum (hỗ trợ Cheat Console Command) */
	static EPAForgeTier ParseForgeTierFromString(const FString& InStr);

	/** Thực thi lệnh cheat Cheat.GenerateItem từ chuỗi tham số (có thể gọi trực tiếp trong test hoặc console) */
	static bool ExecuteCheatGenerateItem(
		const TArray<FString>& Args,
		UPAServerItemGeneratorSubsystem* Subsystem,
		FPASavedItemInstance* OutItem = nullptr
	);

private:
	UPROPERTY()
	TObjectPtr<UDataTable> AffixDataTable = nullptr;

	UPROPERTY()
	TArray<FPAAffixDefinitionRow> CachedAffixDefinitions;

	IConsoleCommand* CheatGenerateItemCmd = nullptr;

	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();
};
