// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PALootDropletActor.generated.h"

class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnLootDropletClaimed, const FString&, PlayerId, int32, GoldAmount);

/**
 * APALootDropletActor
 *
 * Actor đại diện cho Túi Chiến Lợi Phẩm Cá Nhân Rơi (Instanced Loot Droplet).
 * Tuân thủ quy tắc AC-6 và ADR-0001:
 * - Thuộc sở hữu độc quyền của một người chơi (OwningPlayerUID / OwningPlayerController).
 * - Hoàn toàn vô hình và không thể nhặt bởi bất kỳ ai khác (chống Ninja Looting tuyệt đối).
 * - Cắt lọc mạng Iris / Net Relevancy: Ghi đè IsNetRelevantFor để ngăn chặn hoàn toàn việc
 *   gói tin và Actor được sinh trên máy khách của người chơi khác.
 * - Server-Authoritative: Chỉ Dedicated Server mới có thẩm quyền xác nhận nhặt đồ.
 */
UCLASS()
class PROJECTASCENDANT_API APALootDropletActor : public AActor
{
	GENERATED_BODY()

public:
	APALootDropletActor();

	virtual void BeginPlay() override;

	/**
	 * AC-6 & ADR-0001: Lọc kết nối mạng mức độ Actor (True Network Relevance Isolation).
	 * Chỉ gửi gói tin sinh Actor và đồng bộ dữ liệu tới duy nhất PlayerController sở hữu.
	 */
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

	// -------------------------------------------------------------------------
	// Initialization API (Server-Authoritative)
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Loot")
	void InitializeDropletWithUID(
		const FGuid& InPlayerUID,
		const FString& InPlayerId,
		APlayerController* InPC,
		const FString& InEncounterId,
		int32 InGoldReward,
		const TArray<FName>& InItemIds);

	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Loot")
	void InitializeDroplet(
		const FString& InPlayerId,
		APlayerController* InPC,
		const FString& InEncounterId,
		int32 InGoldReward,
		const TArray<FName>& InItemIds);

	// -------------------------------------------------------------------------
	// Queries & Visibility
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Loot")
	FGuid GetOwningPlayerUID() const { return OwningPlayerUID; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Loot")
	FString GetOwningPlayerId() const { return OwningPlayerId; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Loot")
	FString GetDropSourceEncounterId() const { return DropSourceEncounterId; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Loot")
	int32 GetGoldReward() const { return GoldReward; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Loot")
	const TArray<FName>& GetDroppedItemIds() const { return DroppedItemIds; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Loot")
	bool IsClaimed() const { return bIsClaimed; }

	/**
	 * AC-6: Kiểm tra quyền nhìn thấy và tương tác với túi đồ này.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Loot")
	bool CanPlayerSeeOrInteractByUID(const FGuid& InPlayerUID) const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Loot")
	bool CanPlayerSeeOrInteract(const FString& InPlayerId) const;

	/**
	 * Kiểm tra xem Actor có liên kết với PlayerController cục bộ này hay không.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Loot")
	bool IsVisibleToLocalController(const APlayerController* LocalPC) const;

	// -------------------------------------------------------------------------
	// Claim API (Server-Authoritative)
	// -------------------------------------------------------------------------

	/**
	 * Nhặt chiến lợi phẩm trong túi đồ.
	 * Dedicated Server kiểm tra toàn quyền thẩm quyền.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Loot")
	bool TryClaimLoot(const FString& InPlayerId, int32& OutGold, TArray<FName>& OutItems);

	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Loot")
	bool TryClaimLootByUID(const FGuid& InPlayerUID, int32& OutGold, TArray<FName>& OutItems);

public:
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Loot|Events")
	FPAOnLootDropletClaimed OnLootClaimed;

protected:
	/** GUID duy nhất của người chơi sở hữu túi đồ này (AC-6) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ProjectAscendant|Loot")
	FGuid OwningPlayerUID;

	/** Chuỗi định danh của người chơi sở hữu */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ProjectAscendant|Loot")
	FString OwningPlayerId = TEXT("");

	/** Mã nhận diện trận đánh Boss sinh ra túi đồ này */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ProjectAscendant|Loot")
	FString DropSourceEncounterId = TEXT("");

	/** Lượng vàng thưởng chứa trong túi */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ProjectAscendant|Loot")
	int32 GoldReward = 0;

	/** Danh sách ID vật phẩm rơi trong túi */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ProjectAscendant|Loot")
	TArray<FName> DroppedItemIds;

	/** Cờ đánh dấu túi đồ đã được nhận thưởng */
	UPROPERTY(ReplicatedUsing = OnRep_IsClaimed, BlueprintReadOnly, Category = "ProjectAscendant|Loot")
	bool bIsClaimed = false;

	/** Thời gian tồn tại mặc định của túi đồ rơi ngoài hoang dã: 1800.0s (30 phút) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Loot")
	float DropletLifespanSeconds = 1800.0f;

	/** Con trỏ tới PlayerController sở hữu phục vụ lọc kết nối */
	UPROPERTY()
	TWeakObjectPtr<APlayerController> OwningPlayerController = nullptr;

	UFUNCTION()
	virtual void OnRep_IsClaimed();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
