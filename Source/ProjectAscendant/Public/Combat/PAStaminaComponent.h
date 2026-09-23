// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PAStaminaComponent.generated.h"

class UAbilitySystemComponent;
class UAscendantAttributeSet;

/**
 * FPAStaminaPipeline
 *
 * Bộ công thức toán học và máy trạng thái thể lực thuần tuý (Pure Logic & Formulas) cho Project Ascendant.
 * Tách biệt logic kinh doanh khỏi Engine World để phục vụ cả việc chạy Unit Test tự động tốc độ cao
 * lẫn việc quản trị vòng đời thể lực trên Server/Client.
 *
 * Tuân thủ:
 *  - GDD attributes-system.md & TR-attr-003
 *  - ADR-0002: GAS Integration Strategy
 *  - Control Manifest: Không bao giờ thăm dò trong Tick(), sử dụng Timer / GameplayTags.
 */
struct PROJECTASCENDANT_API FPAStaminaPipeline
{
	static constexpr float kDefaultDashCost = 25.0f;
	static constexpr float kDefaultRegenDelay = 0.6f;
	static constexpr float kDefaultRegenRate = 45.0f;
	static constexpr float kStandardExhaustionDuration = 1.5f;
	static constexpr float kDesperationExhaustionDuration = 2.2f;
	static constexpr float kRecoveryThresholdPct = 0.30f; // 30% MaxStamina
	static constexpr float kMoveSpeedPenaltyPct = 0.25f;  // Giảm 25% tốc độ

	/**
	 * AC-1: Tiêu hao thể lực và kiểm tra Cú Lướt Tuyệt Vọng (Desperation Roll).
	 *
	 * @param CurrentStamina Thể lực hiện có trước khi hành động.
	 * @param Cost Chi phí thể lực của kỹ năng (Mặc định Dash: 25.0f).
	 * @param OutRemainingStamina Thể lực còn lại sau khi trừ.
	 * @param bOutIsDesperationRoll Đánh dấu kích hoạt Desperation Roll (khi Stamina > 0 nhưng < Cost).
	 * @return true nếu hành động được phép thực hiện; false nếu Stamina <= 0 (bị chặn).
	 */
	static bool ConsumeStamina(float CurrentStamina, float Cost, float& OutRemainingStamina, bool& bOutIsDesperationRoll);

	/**
	 * AC-2: Tính toán thể lực hồi phục sau khoảng thời gian trôi qua với độ trễ 0.6s.
	 *
	 * @param StartStamina Thể lực tại mốc bắt đầu trễ.
	 * @param MaxStamina Thể lực tối đa.
	 * @param RegenRate Tốc độ hồi phục (mặc định 45.0 điểm/s).
	 * @param ElapsedTime Tổng thời gian đã trôi qua kể từ đòn tiêu hao cuối cùng.
	 * @param RegenDelay Độ trễ trước khi bắt đầu hồi (mặc định 0.6s).
	 * @return Lượng thể lực tại thời điểm ElapsedTime.
	 */
	static float CalculateStaminaAfterTime(float StartStamina, float MaxStamina, float RegenRate, float ElapsedTime, float RegenDelay);

	/**
	 * AC-3: Lấy thời lượng trạng thái Kiệt Sức tương ứng.
	 *
	 * @param bIsDesperationRoll Có bắt nguồn từ cú lướt tuyệt vọng hay không.
	 * @return 2.2s nếu là Desperation Roll; 1.5s nếu là kiệt sức thông thường.
	 */
	static float GetExhaustionDuration(bool bIsDesperationRoll);

	/**
	 * AC-3: Kiểm tra điều kiện giải trừ trạng thái Kiệt Sức.
	 * Chỉ được thoát khi: Hết thời gian phạt (1.5s hoặc 2.2s) VÀ Thể lực hồi vượt 30% MaxStamina.
	 *
	 * @param CurrentStamina Thể lực hiện tại.
	 * @param MaxStamina Thể lực tối đa.
	 * @param ElapsedExhaustionTime Thời gian đã chịu trạng thái kiệt sức.
	 * @param RequiredDuration Thời gian phạt bắt buộc (1.5s hoặc 2.2s).
	 * @param ThresholdPct Ngưỡng thể lực yêu cầu để hồi phục (mặc định 0.30f).
	 * @return true nếu đủ điều kiện thoát Kiệt Sức.
	 */
	static bool CanRecoverFromExhaustion(float CurrentStamina, float MaxStamina, float ElapsedExhaustionTime, float RequiredDuration, float ThresholdPct = kRecoveryThresholdPct);

	/**
	 * AC-3: Tính toán vận tốc sau khi áp dụng hình phạt Kiệt Sức (-25%).
	 */
	static float CalculateExhaustedMoveSpeed(float BaseMoveSpeed);
};

/**
 * UPAStaminaComponent
 *
 * Quản lý chu trình tiêu hao, trễ hồi phục và trạng thái Kiệt Sức cho nhân vật.
 * Vận hành bằng FTimerManager không dùng Tick() theo quy tắc Control Manifest.
 * Giao tiếp trực tiếp với Gameplay Ability System (GAS) và gắn/xóa thẻ GameplayTag:
 *  - State.Exhausted: Trạng thái kiệt sức làm giảm 25% tốc độ.
 *  - Ability.Block.Dash: Khóa toàn bộ kỹ năng lướt né đòn.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPAStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPAStaminaComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExhaustionStateChanged, bool, bIsExhausted);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaConsumed, float, ConsumedAmount, bool, bWasDesperationRoll);

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Stamina")
	FOnExhaustionStateChanged OnExhaustionStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Stamina")
	FOnStaminaConsumed OnStaminaConsumed;

	// -------------------------------------------------------------------------
	// Core Operations (AC-1, AC-2, AC-3)
	// -------------------------------------------------------------------------

	/**
	 * AC-1: Thử tiêu hao thể lực cho cú lướt né đòn (Dash).
	 * Nếu thành công, tự động đặt lại độ trễ hồi phục 0.6s.
	 *
	 * @param bOutDesperationRoll Trả về true nếu đây là Desperation Roll.
	 * @return true nếu cho phép lướt (kể cả Desperation Roll); false nếu Stamina <= 0 hoặc đang kiệt sức.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Stamina")
	bool TryConsumeStaminaForDash(bool& bOutDesperationRoll);

	/**
	 * Tiêu hao thể lực thông thường (cho tấn công tụ lực, v.v.).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Stamina")
	void ConsumeStamina(float Amount);

	/** Trả về cờ đang trong trạng thái Kiệt Sức. */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Stamina")
	bool IsExhausted() const { return bIsExhausted; }

	/** Trả về cờ xem hành động lướt có đang bị khóa hay không. */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Stamina")
	bool IsDashBlocked() const;

	/** Lấy lượng thể lực hiện tại từ AttributeSet. */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Stamina")
	float GetCurrentStamina() const;

	/** Lấy lượng thể lực tối đa từ AttributeSet. */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Stamina")
	float GetMaxStamina() const;

	/** Kích hoạt chủ động trạng thái kiệt sức (cho unit tests hoặc debuff từ ngoài). */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Stamina")
	void ApplyExhaustion(bool bFromDesperationRoll);

	/** Buộc giải trừ trạng thái kiệt sức. */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Stamina")
	void ClearExhaustion();

protected:
	/** Khởi động bộ đếm trễ hồi phục 0.6s (AC-2) */
	void StartRegenDelayTimer();

	/** Bắt đầu chu trình hồi thể lực tuần hoàn (timer 0.05s) sau khi hết trễ 0.6s */
	void OnRegenDelayExpired();

	/** Hàm hồi thể lực định kỳ được kích hoạt bởi FTimerHandle */
	void HandleActiveRegenTick();

	/** Xử lý khi hết thời gian phạt kiệt sức (1.5s hoặc 2.2s) */
	void OnExhaustionDurationExpired();

	/** Kiểm tra xem đã đủ điều kiện thoát kiệt sức chưa (đã hết giờ VÀ stamina >= 30%) */
	void CheckExhaustionRecovery();

	/** Cập nhật GameplayTags trên AbilitySystemComponent */
	void UpdateExhaustionTags(bool bEnable);

protected:
	/** Chi phí thể lực của hành động Dash (mặc định 25.0 điểm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Stamina|Config")
	float DashStaminaCost = 25.0f;

	/** Độ trễ trước khi bắt đầu hồi phục thể lực (mặc định 0.6 giây) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Stamina|Config")
	float StaminaRegenDelay = 0.6f;

	/** Khoảng thời gian nhịp đếm timer hồi thể lực (0.05s = 20Hz, mượt mà và nhẹ nhàng) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Stamina|Config")
	float RegenTickInterval = 0.05f;

	/** Cờ trạng thái kiệt sức hiện tại */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Stamina|State")
	bool bIsExhausted = false;

	/** Đánh dấu trạng thái kiệt sức gây ra bởi Desperation Roll */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Stamina|State")
	bool bIsDesperationRoll = false;

	/** Đánh dấu thời gian phạt cố định đã trôi qua xong (chỉ còn chờ hồi > 30% stamina) */
	bool bExhaustionTimerFinished = false;

	/** Timer đếm trễ 0.6s trước khi hồi */
	FTimerHandle RegenDelayTimerHandle;

	/** Timer định kỳ nhồi thể lực */
	FTimerHandle RegenTickTimerHandle;

	/** Timer đếm thời lượng kiệt sức (1.5s hoặc 2.2s) */
	FTimerHandle ExhaustionTimerHandle;

private:
	/** Lấy con trỏ AbilitySystemComponent từ Owner */
	UAbilitySystemComponent* GetOwnerASC() const;

	/** Lấy con trỏ AttributeSet từ Owner */
	UAscendantAttributeSet* GetOwnerAttributeSet() const;
};
