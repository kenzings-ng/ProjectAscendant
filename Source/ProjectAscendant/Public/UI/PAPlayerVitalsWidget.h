// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/PAPlayerVitalsTypes.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "PAPlayerVitalsWidget.generated.h"

class UAscendantAttributeSet;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnHealthPercentChanged, float, NewPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnGhostHealthPercentChanged, float, NewGhostPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnStaminaPercentChanged, float, NewPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnManaPercentChanged, float, NewPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnStaminaStateChanged, EPAStaminaBarState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnLowHealthVignetteChanged, bool, bActive, float, HeartbeatBPM);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnExhaustionVignetteChanged, bool, bActive);

/**
 * UPAPlayerVitalsWidget
 *
 * UserWidget quản lý cụm thông tin sinh tử người chơi (Player Cluster) theo GDD combat-hud.md:
 * - Đồng bộ trực tiếp với UAscendantAttributeSet qua delegates của GAS.
 * - Thanh máu bóng mờ (Catch-up Ghost Bar) trễ 0.40s trước khi co lại.
 * - Thanh thể lực phản hồi tức thời: Lóe sáng màu vàng kim (0.20s) khi né chuẩn, xám tro khi kiệt sức.
 * - Hiệu ứng viền màn hình nhịp tim dồn dập (60-100 BPM) khi Máu < 20% và viền xám tro khi kiệt sức.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTASCENDANT_API UPAPlayerVitalsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPAPlayerVitalsWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// -------------------------------------------------------------------------
	// GAS Initialization & Bindings
	// -------------------------------------------------------------------------

	/**
	 * AC-1: Khởi tạo liên kết sự kiện với AbilitySystemComponent và AttributeSet.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|UI|Vitals")
	void BindToAttributeSet(UAscendantAttributeSet* InAttributeSet, UAbilitySystemComponent* InASC);

	/** Cập nhật thủ công các giá trị cho Automation Tests hoặc giả lập */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|UI|Vitals")
	void SetManualVitals(float Health, float MaxHealth, float Stamina, float MaxStamina, float Mana, float MaxMana);

	/**
	 * AC-3: Kích hoạt hiệu ứng né hoàn hảo (Golden Flash 0.20s + hoàn trả 15 Stamina).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|UI|Vitals")
	void TriggerPerfectDodge();

	/**
	 * AC-3 / AC-4: Cập nhật trạng thái kiệt sức (State.Exhausted).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|UI|Vitals")
	void SetExhausted(bool bExhausted);

	/**
	 * Cập nhật toán học mô hình dữ liệu (có thể gọi trực tiếp trong headless tests).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|UI|Vitals")
	void UpdateModel(float DeltaTime);

	// -------------------------------------------------------------------------
	// Blueprint Getters for UMG Bindings
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|UI|Vitals")
	float GetHealthPercent() const { return Model.GetHealthPercent(); }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|UI|Vitals")
	float GetGhostHealthPercent() const { return Model.GetGhostHealthPercent(); }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|UI|Vitals")
	float GetStaminaPercent() const { return Model.GetStaminaPercent(); }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|UI|Vitals")
	float GetManaPercent() const { return Model.GetManaPercent(); }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|UI|Vitals")
	EPAStaminaBarState GetStaminaBarState() const { return Model.StaminaState; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|UI|Vitals")
	bool IsLowHealthVignetteActive() const { return Model.bLowHealthVignetteActive; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|UI|Vitals")
	float GetHeartbeatBPM() const { return Model.CurrentHeartbeatBPM; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|UI|Vitals")
	bool IsExhaustionVignetteActive() const { return Model.bExhaustionVignetteActive; }

	const FPAVitalsModel& GetModel() const { return Model; }
	FPAVitalsModel& GetMutableModel() { return Model; }

public:
	// -------------------------------------------------------------------------
	// Blueprint Assignable Delegates
	// -------------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|UI|Vitals")
	FPAOnHealthPercentChanged OnHealthPercentChanged;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|UI|Vitals")
	FPAOnGhostHealthPercentChanged OnGhostHealthPercentChanged;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|UI|Vitals")
	FPAOnStaminaPercentChanged OnStaminaPercentChanged;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|UI|Vitals")
	FPAOnManaPercentChanged OnManaPercentChanged;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|UI|Vitals")
	FPAOnStaminaStateChanged OnStaminaStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|UI|Vitals")
	FPAOnLowHealthVignetteChanged OnLowHealthVignetteChanged;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|UI|Vitals")
	FPAOnExhaustionVignetteChanged OnExhaustionVignetteChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|UI|Vitals")
	FPAVitalsModel Model;

	TWeakObjectPtr<UAscendantAttributeSet> BoundAttributeSet;
	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;

	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
	void OnStaminaChanged(const FOnAttributeChangeData& Data);
	void OnMaxStaminaChanged(const FOnAttributeChangeData& Data);
	void OnManaChanged(const FOnAttributeChangeData& Data);
	void OnMaxManaChanged(const FOnAttributeChangeData& Data);
	void OnExhaustionTagChanged(const FGameplayTag Tag, int32 NewCount);
};
