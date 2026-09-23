// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AscendantAttributeSet.generated.h"

/**
 * ATTRIBUTE_ACCESSORS
 * Macro chuẩn của Unreal Engine Gameplay Ability System (GAS) để tự động sinh các hàm
 * Getter, Setter, Initter và Property Getter cho từng FGameplayAttributeData.
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * UAscendantAttributeSet
 *
 * AttributeSet cơ sở cho toàn bộ thực thể trong Project Ascendant (Người chơi, Quái vật, Boss).
 * Quản lý 4 nhóm chỉ số sống còn theo GDD attributes-system.md & ADR-0002:
 *  1. Sinh mệnh: Health, MaxHealth
 *  2. Năng lượng kỹ năng: Mana, MaxMana
 *  3. Thể lực hành động: Stamina, MaxStamina, StaminaRegenRate
 *  4. Chiến đấu & Phá thế: Posture, MaxPosture, PostureDecayRate, IFrameDuration, MoveSpeed
 *
 * Toàn bộ thuộc tính được đồng bộ mạng qua Dedicated Server và Iris Replication với macro GAMEPLAYATTRIBUTE_REPNOTIFY.
 */
UCLASS()
class PROJECTASCENDANT_API UAscendantAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAscendantAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// -------------------------------------------------------------------------
	// Combat Delegates (Broadcast khi cạn máu hoặc vỡ thế Posture)
	// -------------------------------------------------------------------------
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnOutOfHealthDelegate, AActor* /* InstigatorActor */);
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPostureBrokenDelegate, AActor* /* InstigatorActor */);

	FOnOutOfHealthDelegate OnOutOfHealth;
	FOnPostureBrokenDelegate OnPostureBroken;

	/** Cờ trạng thái ngăn phát lặp sự kiện cạn kiệt sinh mệnh */
	bool bOutOfHealthBroadcasted = false;

	/** Cờ trạng thái ngăn phát lặp sự kiện vỡ thế đứng */
	bool bPostureBrokenBroadcasted = false;

	/** Thẩm định và phát sự kiện hết máu với cơ chế chống lặp */
	void EvaluateOutOfHealthBroadcast(AActor* InstigatorActor = nullptr)
	{
		if (GetHealth() <= 0.0f)
		{
			if (!bOutOfHealthBroadcasted)
			{
				bOutOfHealthBroadcasted = true;
				OnOutOfHealth.Broadcast(InstigatorActor);
			}
		}
		else
		{
			bOutOfHealthBroadcasted = false;
		}
	}

	/** Thẩm định và phát sự kiện vỡ thế đứng với cơ chế chống lặp */
	void EvaluatePostureBrokenBroadcast(AActor* InstigatorActor = nullptr)
	{
		if (GetPosture() >= GetMaxPosture())
		{
			if (!bPostureBrokenBroadcasted)
			{
				bPostureBrokenBroadcasted = true;
				OnPostureBroken.Broadcast(InstigatorActor);
			}
		}
		else
		{
			bPostureBrokenBroadcasted = false;
		}
	}

	// -------------------------------------------------------------------------
	// 1. Health & Survival (Sinh Mệnh)
	// -------------------------------------------------------------------------

	/** Máu hiện tại. Về 0 kích hoạt chết / tử trận. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "ProjectAscendant|Attributes|Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAscendantAttributeSet, Health)

	/** Máu tối đa của thực thể. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "ProjectAscendant|Attributes|Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAscendantAttributeSet, MaxHealth)

	// -------------------------------------------------------------------------
	// 2. Mana & Skills (Năng Lượng Phép Thuật)
	// -------------------------------------------------------------------------

	/** Năng lượng hiện tại dùng để kích hoạt Gameplay Abilities. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "ProjectAscendant|Attributes|Mana")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UAscendantAttributeSet, Mana)

	/** Năng lượng tối đa. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "ProjectAscendant|Attributes|Mana")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UAscendantAttributeSet, MaxMana)

	// -------------------------------------------------------------------------
	// 3. Stamina & Mobility (Thể Lực Hành Động)
	// -------------------------------------------------------------------------

	/** Thể lực dùng cho Lướt né đòn (25 điểm) hoặc Tụ lực đánh (20 điểm). Về 0 gây Kiệt Sức. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "ProjectAscendant|Attributes|Stamina")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UAscendantAttributeSet, Stamina)

	/** Thể lực tối đa (mặc định 100). */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "ProjectAscendant|Attributes|Stamina")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UAscendantAttributeSet, MaxStamina)

	/** Tốc độ hồi phục thể lực mỗi giây (mặc định 45 điểm/giây). */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_StaminaRegenRate, Category = "ProjectAscendant|Attributes|Stamina")
	FGameplayAttributeData StaminaRegenRate;
	ATTRIBUTE_ACCESSORS(UAscendantAttributeSet, StaminaRegenRate)

	// -------------------------------------------------------------------------
	// 4. Posture & Break Mechanism (Thanh Thế Đứng & Phá Vỡ Tư Thế)
	// -------------------------------------------------------------------------

	/**
	 * Thanh Thế Đứng (Cơ chế tích lũy ngược từ 0 đến MaxPosture).
	 * Khi Posture đạt 100% MaxPosture -> Đối tượng rơi vào trạng thái Choáng vỡ thế (Staggered).
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Posture, Category = "ProjectAscendant|Attributes|Posture")
	FGameplayAttributeData Posture;
	ATTRIBUTE_ACCESSORS(UAscendantAttributeSet, Posture)

	/** Ngưỡng chịu tải thế đứng tối đa (Người chơi: 100, Boss: 1000+). */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxPosture, Category = "ProjectAscendant|Attributes|Posture")
	FGameplayAttributeData MaxPosture;
	ATTRIBUTE_ACCESSORS(UAscendantAttributeSet, MaxPosture)

	/** Tốc độ hạ nhiệt / tự hồi phục thế đứng sau thời gian trễ (mặc định 20 điểm/giây). */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PostureDecayRate, Category = "ProjectAscendant|Attributes|Posture")
	FGameplayAttributeData PostureDecayRate;
	ATTRIBUTE_ACCESSORS(UAscendantAttributeSet, PostureDecayRate)

	// -------------------------------------------------------------------------
	// 5. Combat Parameters (Tham Số Chiến Đấu Cơ Sở & Phòng Ngự)
	// -------------------------------------------------------------------------

	/** Chỉ số Giáp (Armor) dùng để giảm trừ sát thương nhận vào theo công thức AC-1: 100 / (100 + Armor). */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "ProjectAscendant|Attributes|Combat")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UAscendantAttributeSet, Armor)

	/** Sức mạnh tấn công cơ bản (AttackPower / RawDamage) của thực thể. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPower, Category = "ProjectAscendant|Attributes|Combat")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UAscendantAttributeSet, AttackPower)

	/** Lực đánh phá thế cơ sở (BaseStagger) gây áp lực lên thanh Posture mục tiêu. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseStagger, Category = "ProjectAscendant|Attributes|Combat")
	FGameplayAttributeData BaseStagger;
	ATTRIBUTE_ACCESSORS(UAscendantAttributeSet, BaseStagger)

	/** Hệ số thưởng phá thế (StaggerBonus, ví dụ +0.3 khi có dầu tẩm Dược sư). */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_StaggerBonus, Category = "ProjectAscendant|Attributes|Combat")
	FGameplayAttributeData StaggerBonus;
	ATTRIBUTE_ACCESSORS(UAscendantAttributeSet, StaggerBonus)

	/** Thời lượng bất tử (I-Frame) tính bằng giây khi thực hiện Dash (mặc định 0.28s). */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IFrameDuration, Category = "ProjectAscendant|Attributes|Combat")
	FGameplayAttributeData IFrameDuration;
	ATTRIBUTE_ACCESSORS(UAscendantAttributeSet, IFrameDuration)

	/** Vận tốc di chuyển cơ bản (mặc định 550 cm/s). */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeed, Category = "ProjectAscendant|Attributes|Combat")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UAscendantAttributeSet, MoveSpeed)

protected:
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	virtual void OnRep_Mana(const FGameplayAttributeData& OldMana);

	UFUNCTION()
	virtual void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana);

	UFUNCTION()
	virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

	UFUNCTION()
	virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);

	UFUNCTION()
	virtual void OnRep_StaminaRegenRate(const FGameplayAttributeData& OldStaminaRegenRate);

	UFUNCTION()
	virtual void OnRep_Posture(const FGameplayAttributeData& OldPosture);

	UFUNCTION()
	virtual void OnRep_MaxPosture(const FGameplayAttributeData& OldMaxPosture);

	UFUNCTION()
	virtual void OnRep_PostureDecayRate(const FGameplayAttributeData& OldPostureDecayRate);

	UFUNCTION()
	virtual void OnRep_Armor(const FGameplayAttributeData& OldArmor);

	UFUNCTION()
	virtual void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);

	UFUNCTION()
	virtual void OnRep_BaseStagger(const FGameplayAttributeData& OldBaseStagger);

	UFUNCTION()
	virtual void OnRep_StaggerBonus(const FGameplayAttributeData& OldStaggerBonus);

	UFUNCTION()
	virtual void OnRep_IFrameDuration(const FGameplayAttributeData& OldIFrameDuration);

	UFUNCTION()
	virtual void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);
};
