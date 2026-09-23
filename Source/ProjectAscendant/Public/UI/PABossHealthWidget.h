// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/PABossHUDTypes.h"
#include "PABossHealthWidget.generated.h"

class UAscendantAttributeSet;
class UAbilitySystemComponent;

/**
 * UPABossHealthWidget
 *
 * Widget hiển thị thanh máu Boss, thanh Posture, và trạng thái bộ phận.
 * Bind trực tiếp vào GAS delegates của Boss Actor (event-driven, zero polling).
 *
 * Tham chiếu GDD: design/gdd/combat-hud.md §Boss Cluster
 * ADR-0001: Server-authoritative boss stats
 * ADR-0002: Event-driven GAS attribute delegates
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTASCENDANT_API UPABossHealthWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ===========================================================
	// Lifecycle
	// ===========================================================

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ===========================================================
	// Boss Target Binding
	// ===========================================================

	/**
	 * Gán Boss Target và bind vào GAS AttributeSet delegates.
	 * @param InBossActor   Boss Actor có AbilitySystemComponent
	 * @param InBossName    Tên hiển thị Boss
	 * @param InBossLevel   Cấp độ Boss
	 * @param InPartIds     Danh sách bộ phận có thể gãy (VD: Horn, Tail)
	 */
	UFUNCTION(BlueprintCallable, Category = "BossHUD")
	void SetBossTarget(AActor* InBossActor, const FString& InBossName, int32 InBossLevel, const TArray<FName>& InPartIds);

	/** Hủy binding khi Boss bị hạ hoặc rời khỏi encounter */
	UFUNCTION(BlueprintCallable, Category = "BossHUD")
	void ClearBossTarget();

	// ===========================================================
	// Event Handlers (GAS Attribute Delegates)
	// ===========================================================

	/** Được gọi khi Health Boss thay đổi (GAS OnAttributeChanged) */
	void HandleBossHealthChanged(const struct FOnAttributeChangeData& ChangeData);

	/** Được gọi khi MaxHealth Boss thay đổi */
	void HandleBossMaxHealthChanged(const struct FOnAttributeChangeData& ChangeData);

	/** Được gọi khi Posture Boss thay đổi */
	void HandleBossPostureChanged(const struct FOnAttributeChangeData& ChangeData);

	/** Được gọi khi MaxPosture Boss thay đổi */
	void HandleBossMaxPostureChanged(const struct FOnAttributeChangeData& ChangeData);

	// ===========================================================
	// Manual Event Triggers (từ GameplayAbility hoặc Server RPC)
	// ===========================================================

	/** Kích hoạt trạng thái Stagger (Posture vỡ 100%) */
	UFUNCTION(BlueprintCallable, Category = "BossHUD")
	void HandlePostureBroken();

	/** Đánh dấu bộ phận bị gãy */
	UFUNCTION(BlueprintCallable, Category = "BossHUD")
	void HandlePartBroken(FName PartId);

	// ===========================================================
	// Blueprint Getters
	// ===========================================================

	UFUNCTION(BlueprintPure, Category = "BossHUD")
	float GetBossHPPercent() const { return Model.GetHPPercent(); }

	UFUNCTION(BlueprintPure, Category = "BossHUD")
	float GetBossPosturePercent() const { return Model.GetPosturePercent(); }

	UFUNCTION(BlueprintPure, Category = "BossHUD")
	bool IsStaggerFlashing() const { return Model.IsStaggerFlashing(); }

	UFUNCTION(BlueprintPure, Category = "BossHUD")
	bool IsBlinkVisible() const { return Model.bBlinkVisible; }

	UFUNCTION(BlueprintPure, Category = "BossHUD")
	int32 GetCurrentPhase() const { return Model.GetCurrentPhase(); }

	UFUNCTION(BlueprintPure, Category = "BossHUD")
	const FString& GetBossName() const { return Model.BossName; }

	UFUNCTION(BlueprintPure, Category = "BossHUD")
	int32 GetBossLevel() const { return Model.BossLevel; }

	UFUNCTION(BlueprintPure, Category = "BossHUD")
	bool IsPartBroken(FName PartId) const { return Model.IsPartBroken(PartId); }

	UFUNCTION(BlueprintPure, Category = "BossHUD")
	bool IsExecutionReticleVisible() const { return Model.bExecutionReticleVisible; }

	UFUNCTION(BlueprintPure, Category = "BossHUD")
	FVector2D GetReticleScreenPosition() const { return Model.ReticleScreenPosition; }

	UFUNCTION(BlueprintPure, Category = "BossHUD")
	void GetPhaseNotchPercents(TArray<float>& OutNotches) const { Model.GetPhaseNotchPercents(OutNotches); }

	UFUNCTION(BlueprintPure, Category = "BossHUD")
	const FPABossHUDModel& GetModel() const { return Model; }

	// ===========================================================
	// Delegates cho Blueprint Event Binding
	// ===========================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossStaggerStarted);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossStaggerEnded);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossPartBroken, FName, PartId);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossPhaseChanged, int32, NewPhase);

	UPROPERTY(BlueprintAssignable, Category = "BossHUD|Events")
	FOnBossStaggerStarted OnBossStaggerStarted;

	UPROPERTY(BlueprintAssignable, Category = "BossHUD|Events")
	FOnBossStaggerEnded OnBossStaggerEnded;

	UPROPERTY(BlueprintAssignable, Category = "BossHUD|Events")
	FOnBossPartBroken OnBossPartBroken;

	UPROPERTY(BlueprintAssignable, Category = "BossHUD|Events")
	FOnBossPhaseChanged OnBossPhaseChanged;

protected:
	/** Data model thuần túy — tách biệt khỏi UMG để kiểm thử tự động */
	UPROPERTY(BlueprintReadOnly, Category = "BossHUD")
	FPABossHUDModel Model;

	/** Pha chiến đấu trước đó (phát hiện chuyển pha) */
	int32 PreviousPhase = 0;

	/** Boss Actor hiện tại */
	UPROPERTY()
	TWeakObjectPtr<AActor> BossActor;

	/** Cached ASC reference */
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> BossASC;

	/** Cập nhật Model và phát broadcast nếu chuyển pha */
	void UpdateModel();
};
