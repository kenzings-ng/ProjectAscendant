// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PALootDistributionSubsystem.generated.h"

class APABaseCharacter;
class APALootDropletActor;

/**
 * FPAContributionRecord
 *
 * Bản ghi đóng góp sát thương của một người chơi trong một trận đánh Boss dã ngoại (AC-6).
 * Lưu trữ theo FGuid PlayerUID theo đúng chuẩn ADR-0001 và spec net-004.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAContributionRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Loot")
	FGuid PlayerUID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Loot")
	FString PlayerId = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Loot")
	float DamageDealt = 0.0f; // Tổng sát thương máu gây ra

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Loot")
	float PostureDamageDealt = 0.0f; // Tổng sát thương phá vỡ thế đứng gây ra

	UPROPERTY(BlueprintReadOnly, Category = "ProjectAscendant|Loot")
	TWeakObjectPtr<APABaseCharacter> PlayerCharacter = nullptr;

	FPAContributionRecord()
		: PlayerUID()
		, PlayerId(TEXT(""))
		, DamageDealt(0.0f)
		, PostureDamageDealt(0.0f)
		, PlayerCharacter(nullptr)
	{
	}

	FPAContributionRecord(const FGuid& InUID, const FString& InPlayerId, float InDamage, float InPostureDmg, APABaseCharacter* InChar)
		: PlayerUID(InUID)
		, PlayerId(InPlayerId)
		, DamageDealt(InDamage)
		, PostureDamageDealt(InPostureDmg)
		, PlayerCharacter(InChar)
	{
	}
};

/**
 * FPAPartyMemberInfo
 *
 * Thông tin thành viên tổ đội phục vụ tính toán chia sẻ EXP và kiểm tra khoảng cách (AC-8).
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAPartyMemberInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Party")
	FGuid PlayerUID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Party")
	FString PlayerId = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Party")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "ProjectAscendant|Party")
	TWeakObjectPtr<APABaseCharacter> PlayerCharacter = nullptr;

	FPAPartyMemberInfo()
		: PlayerUID()
		, PlayerId(TEXT(""))
		, Location(FVector::ZeroVector)
		, PlayerCharacter(nullptr)
	{
	}

	FPAPartyMemberInfo(const FString& InId, const FVector& InLoc, APABaseCharacter* InChar = nullptr)
		: PlayerUID()
		, PlayerId(InId)
		, Location(InLoc)
		, PlayerCharacter(InChar)
	{
		FGuid::Parse(InId, PlayerUID);
	}

	FPAPartyMemberInfo(const FGuid& InUID, const FString& InId, const FVector& InLoc, APABaseCharacter* InChar = nullptr)
		: PlayerUID(InUID)
		, PlayerId(InId)
		, Location(InLoc)
		, PlayerCharacter(InChar)
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPAOnInstancedLootSpawned, const FString&, EncounterId, const FString&, PlayerId, APALootDropletActor*, DropletActor);

/**
 * UPALootDistributionSubsystem
 *
 * World Subsystem quản lý thẩm quyền phân phối Chiến Lợi Phẩm Cá Nhân Hóa (Instanced Loot)
 * và Chia Sẻ Kinh Nghiệm Tổ Đội (Party EXP Sharing) trên Dedicated Server (Story 004 / net-004).
 *
 * Tuân thủ tuyệt đối:
 * - AC-6: Ngưỡng đóng góp nhận Loot: >= 5% Max HP HOẶC >= 10% Max Posture.
 * - AC-8: Chia sẻ EXP tổ đội trong bán kính 3000 cm kèm Morale Bonus (+10%, +20%, +35%).
 * - Guardrail: Thời gian sinh túi đồ <= 1.0ms trên Game Thread máy chủ.
 */
UCLASS()
class PROJECTASCENDANT_API UPALootDistributionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// -------------------------------------------------------------------------
	// Constants
	// -------------------------------------------------------------------------
	static constexpr float kBossLootContributionHPRatio = 0.05f;       // AC-6: >= 5% HP
	static constexpr float kBossLootContributionPostureRatio = 0.10f;  // AC-6: >= 10% Posture
	static constexpr float kPartyExpRadius = 3000.0f;                  // AC-8: 3000 cm bán kính chia EXP
	static constexpr float kMoraleBonus2P = 0.10f;                     // +10% cho 2 người
	static constexpr float kMoraleBonus3P = 0.20f;                     // +20% cho 3 người
	static constexpr float kMoraleBonus4P = 0.35f;                     // +35% cho 4 người

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// -------------------------------------------------------------------------
	// Contribution Tracking API (AC-6, FGuid-based)
	// -------------------------------------------------------------------------

	/**
	 * Ghi nhận lượng sát thương HP hoặc Posture của một người chơi (theo FGuid) lên Boss.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Loot")
	void RecordCombatContributionWithUID(
		const FString& EncounterId,
		const FGuid& PlayerUID,
		const FString& PlayerId,
		APABaseCharacter* PlayerChar,
		float HPDamage,
		float PostureDamage);

	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Loot")
	void RecordCombatContribution(
		const FString& EncounterId,
		const FString& PlayerId,
		APABaseCharacter* PlayerChar,
		float HPDamage,
		float PostureDamage);

	/**
	 * Lấy bản ghi đóng góp hiện tại của một người chơi trong trận đánh.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Loot")
	bool GetContributionRecordByUID(const FString& EncounterId, const FGuid& PlayerUID, FPAContributionRecord& OutRecord) const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Loot")
	bool GetContributionRecord(const FString& EncounterId, const FString& PlayerId, FPAContributionRecord& OutRecord) const;

	/**
	 * AC-6: Kiểm tra xem một người chơi có đủ điều kiện nhận Instanced Loot không.
	 * Điều kiện: DamageDealt / BossMaxHP >= 0.05f HOẶC PostureDamageDealt / BossMaxPosture >= 0.10f.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Loot")
	bool IsPlayerEligibleForLootByUID(
		const FString& EncounterId,
		const FGuid& PlayerUID,
		float BossMaxHP,
		float BossMaxPosture) const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Loot")
	bool IsPlayerEligibleForLoot(
		const FString& EncounterId,
		const FString& PlayerId,
		float BossMaxHP,
		float BossMaxPosture) const;

	/**
	 * Lấy danh sách tất cả PlayerUID đủ điều kiện nhận Loot từ trận đánh.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Loot")
	TArray<FGuid> GetEligiblePlayerUIDsForLoot(
		const FString& EncounterId,
		float BossMaxHP,
		float BossMaxPosture) const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Loot")
	TArray<FString> GetEligiblePlayersForLoot(
		const FString& EncounterId,
		float BossMaxHP,
		float BossMaxPosture) const;

	// -------------------------------------------------------------------------
	// Loot Distribution API (AC-6)
	// -------------------------------------------------------------------------

	/**
	 * Phân phối túi đồ Instanced Loot Droplet cho toàn bộ người chơi đạt chuẩn đóng góp.
	 * Mỗi túi đồ được gán chủ sở hữu độc quyền và hoàn toàn vô hình với người khác.
	 * @return Danh sách các Actor túi đồ đã được sinh ra.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Loot")
	TArray<APALootDropletActor*> DistributeInstancedLoot(
		const FString& EncounterId,
		float BossMaxHP,
		float BossMaxPosture,
		const FVector& DropOrigin,
		int32 BaseGoldReward,
		const TArray<FName>& DropItemPool);

	/**
	 * Xóa sạch dữ liệu đóng góp của một trận đánh sau khi kết thúc.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Loot")
	void ClearEncounterData(const FString& EncounterId);

	// -------------------------------------------------------------------------
	// Party EXP Sharing Math & API (AC-8)
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Party")
	static float GetMoraleBonus(int32 PartySize);

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Party")
	static int32 CalculatePartyMemberExp(int32 BaseMonsterExp, int32 TotalPartySize, int32 ValidMemberCount);

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Party")
	static bool IsPartyMemberInRange(const FVector& MemberLocation, const FVector& MobDeathLocation, float Radius = 3000.0f);

	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Party")
	TMap<FString, int32> DistributePartyExp(
		int32 BaseMonsterExp,
		const TArray<FPAPartyMemberInfo>& PartyMembers,
		const FVector& MonsterDeathLocation);

public:
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Loot|Events")
	FPAOnInstancedLootSpawned OnInstancedLootSpawned;

private:
	/**
	 * Bảng lưu trữ đóng góp theo từng trận đánh (chuẩn spec ADR-0001: TMap<FGuid, FContributionRecord>):
	 * EncounterId -> (PlayerUID -> FPAContributionRecord)
	 */
	TMap<FString, TMap<FGuid, FPAContributionRecord>> EncounterContributions;

	FGuid ResolveUID(const FString& PlayerId) const;
};
