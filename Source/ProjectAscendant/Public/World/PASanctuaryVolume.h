// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ActiveGameplayEffectHandle.h"
#include "PASanctuaryVolume.generated.h"

class USphereComponent;
class UAbilitySystemComponent;
class UGameplayEffect;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnZoneTransitionChanged, AActor*, TargetActor, FGameplayTag, NewZoneTag);

/**
 * FPASanctuaryLeashPipeline
 *
 * Bộ công thức toán học và logic trạng thái thuần túy (Pure Logic & Formulas)
 * cho Khu vực An toàn (Sanctuary Safezone), Biên giới kéo quái (Monster Leash Boundary),
 * và Phân định chuyển vùng (Zone Transition).
 *
 * Tuân thủ:
 *  - GDD: multiplayer-coop.md, game-concept.md
 *  - ADR-0001: Open World MMO Combat Networking
 *  - Story map-002 Acceptance Criteria (AC-1, AC-2, AC-3)
 */
struct PROJECTASCENDANT_API FPASanctuaryLeashPipeline
{
	static constexpr float kSanctuaryDefaultRadius = 1000.0f;     // AC-1: Bán kính vùng an toàn 1000cm
	static constexpr float kLeashDefaultDistance = 2500.0f;       // AC-2: Giới hạn kéo quái 2500cm khỏi spawn origin
	static constexpr float kLeashSpeedMultiplier = 1.5f;          // AC-2: Tốc độ chạy về vị trí gốc 1.5x
	static constexpr float kLeashArrivalTolerance = 50.0f;        // Cự ly hoàn tất hồi vị trí (50cm)

	/**
	 * AC-1: Kiểm tra xem vị trí đối tượng có nằm trong vùng an toàn Sanctuary hay không (mặt phẳng 2D).
	 */
	static bool IsInsideSanctuary(const FVector& ActorLocation, const FVector& SanctuaryLocation, float Radius = kSanctuaryDefaultRadius);

	/**
	 * AC-2: Kiểm tra xem khoảng cách hiện tại so với vị trí Spawn ban đầu có vượt quá biên giới Leash hay không.
	 */
	static bool IsBeyondLeashDistance(const FVector& CurrentLocation, const FVector& SpawnOrigin, float MaxDistance = kLeashDefaultDistance);

	/**
	 * AC-2: Tính toán tốc độ chạy về vị trí gốc của quái vật (1.5x walk speed).
	 */
	static float CalculateLeashReturnSpeed(float BaseWalkSpeed, float Multiplier = kLeashSpeedMultiplier);

	/**
	 * AC-2: Kiểm tra quái vật đã về đến vị trí gốc hay chưa.
	 */
	static bool HasArrivedAtOrigin(const FVector& CurrentLocation, const FVector& SpawnOrigin, float Tolerance = kLeashArrivalTolerance);

	/**
	 * AC-2: Tính toán hướng di chuyển quay về vị trí spawn (vector chuẩn hóa 2D).
	 */
	static FVector CalculateReturnDirection(const FVector& CurrentLocation, const FVector& SpawnOrigin);

	/**
	 * AC-1: Kiểm tra quyền kích hoạt kỹ năng tấn công khi đang trong Sanctuary.
	 */
	static bool CanActivateOffensiveAbilityInSanctuary(bool bIsInSanctuary);
};

/**
 * APASanctuaryVolume
 *
 * Actor vùng an toàn Sanctuary (Sanctuary Safezone Volume).
 * - AC-1: Bán kính 1000cm, gán thẻ State.InSanctuary và State.Invulnerable cho người chơi bên trong, chặn kích hoạt kỹ năng tấn công.
 * - AC-3: Kích hoạt thông báo chuyển vùng đồng bộ mạng qua Iris (Zone.Sanctuary, Zone.Wilderness).
 */
UCLASS()
class PROJECTASCENDANT_API APASanctuaryVolume : public AActor
{
	GENERATED_BODY()

public:
	APASanctuaryVolume();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Delegate thông báo chuyển vùng trên máy chủ và listeners cục bộ (AC-3) */
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|World|Zone")
	FPAOnZoneTransitionChanged OnZoneTransitionChanged;

	/** Bắt đầu chồng lấn vùng an toàn */
	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	/** Kết thúc chồng lấn vùng an toàn */
	UFUNCTION()
	void OnOverlapEnd(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	/** Kiểm tra xem một Actor có phải là người chơi hay không (AC-1) */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|World|Sanctuary")
	bool IsPlayerActor(const AActor* Actor) const;

	/** Kiểm tra xem một Actor hiện có đang ở trong Sanctuary hay không */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|World|Sanctuary")
	bool IsActorInsideSanctuary(const AActor* TargetActor) const;

	/** Trả về bán kính vùng an toàn hiện tại */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|World|Sanctuary")
	float GetSanctuaryRadius() const;

	/** Cập nhật bán kính vùng an toàn */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|World|Sanctuary")
	void SetSanctuaryRadius(float NewRadius);

	/** Trả về SphereComponent thành phần va chạm gốc */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|World|Sanctuary")
	USphereComponent* GetSphereComponent() const { return SphereComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|World|Sanctuary")
	TObjectPtr<USphereComponent> SphereComponent;

	/** Bán kính vùng an toàn cấu hình (AC-1: 1000cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|World|Sanctuary", meta = (ClampMin = "100.0", UIMin = "100.0"))
	float SanctuaryRadius = FPASanctuaryLeashPipeline::kSanctuaryDefaultRadius;

	/** Thẻ Gameplay Tag khu vực */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|World|Zone")
	FGameplayTag TagStateInSanctuary;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|World|Zone")
	FGameplayTag TagStateInvulnerable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|World|Zone")
	FGameplayTag TagZoneSanctuary;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|World|Zone")
	FGameplayTag TagZoneWilderness;

private:
	/** Bản đồ theo dõi GameplayEffectHandle của các Actor đang trong Sanctuary */
	TMap<TWeakObjectPtr<AActor>, FActiveGameplayEffectHandle> ActiveSanctuaryEffectHandles;

	/** Bản đồ theo dõi GameplayEffectHandle vùng Wilderness ngoài Sanctuary */
	TMap<TWeakObjectPtr<AActor>, FActiveGameplayEffectHandle> ActiveWildernessEffectHandles;

	/** Đối tượng GameplayEffect được cache sẵn nhằm tránh cấp phát heap runtime (Zero Runtime Allocations) */
	UPROPERTY()
	TObjectPtr<UGameplayEffect> CachedSanctuaryEffect;

	UPROPERTY()
	TObjectPtr<UGameplayEffect> CachedWildernessEffect;

	void InitCachedGameplayEffects();
	void ApplySanctuaryEffects(AActor* TargetActor);
	void RemoveSanctuaryEffects(AActor* TargetActor);
};
