// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "ActiveGameplayEffectHandle.h"
#include "Inventory/PAInventoryTypes.h"
#include "PAEquipmentComponent.generated.h"

class APABaseCharacter;
class UAbilitySystemComponent;
class UPAInventoryComponent;
class UItemStaticDataAsset;
class UGameplayEffect;
class UGameplayAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnItemEquipped, EPAEquipmentSlot, Slot, const FPAInventoryItemEntry&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnItemUnequipped, EPAEquipmentSlot, Slot, const FPAInventoryItemEntry&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnQuickbarSlotChanged, int32, QuickbarIndex, const FGuid&, ItemUID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnQuickbarUsed, int32, QuickbarIndex, float, HealthRestored);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnSkillBookLearned, FName, ItemId, TSubclassOf<UGameplayAbility>, AbilityClass);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnItemUseRejected, EPAItemUseError, ErrorCode);

/**
 * UPAEquipmentComponent
 *
 * Component quản lý Khung Trang Bị 6 ô (Paperdoll Equipment), tích hợp Gameplay Ability System (GAS),
 * khay phím tắt 1–4 (Quickbar Consumables) và đọc Sách Bí Kíp (Skill Books) (Story 003 / inv-003).
 *
 * Tuân thủ tuyệt đối:
 * - AC-1: Khung 6 ô trang bị (Mainhand, Offhand, Body Armor, Amulet, Ring 1, Ring 2).
 * - FastArray Serialization: Quản lý trang bị qua FPAEquipmentItemList thay vì raw TArray.
 * - AC-2: Mặc trang bị áp dụng dynamic UGameplayEffect vào ASC; tháo trang bị lập tức thu hồi effect handle.
 * - AC-3: Quickbar 1–4 uống bình dược phẩm: hành động 0.8s, giảm 30% tốc độ, trừ 1 bình, tự dọn khi hết stack.
 * - AC-4: Sách kỹ năng kiểm tra RequiredClassTag và cấm đọc trong lúc giao tranh (!State.InCombat); cấp UGameplayAbility.
 * - Client Error Notification: Client RPC phản hồi lỗi về UI.
 * - Guardrail: Thời gian xử lý Equip/Unequip <= 0.1ms trên Game Thread máy chủ.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPAEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPAEquipmentComponent();

	virtual void BeginPlay() override;

	// -------------------------------------------------------------------------
	// Constants
	// -------------------------------------------------------------------------
	static constexpr int32 kTotalEquipmentSlots = 6;
	static constexpr int32 kQuickbarSlotCount = 4;
	static constexpr float kDefaultPotionDuration = 0.8f;      // AC-3: 0.8s uống bình
	static constexpr float kDefaultSpeedDebuffRatio = 0.30f;    // AC-3: Giảm 30% tốc độ di chuyển

	// -------------------------------------------------------------------------
	// Paperdoll Equipment API (AC-1 & AC-2)
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Equipment")
	static bool IsSlotValid(EPAEquipmentSlot Slot);

	/**
	 * Kiểm tra xem một loại trang bị có tương thích với ô Paperdoll chỉ định hay không.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Equipment")
	static bool CanItemFitInSlot(const UItemStaticDataAsset* ItemData, EPAEquipmentSlot TargetSlot);

	/**
	 * Lấy thông tin trang bị hiện tại tại ô chỉ định (Raw pointer).
	 */
	const FPAInventoryItemEntry* GetEquippedItem(EPAEquipmentSlot Slot) const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Equipment")
	bool IsSlotOccupied(EPAEquipmentSlot Slot) const;

	/**
	 * AC-2: Mặc trang bị từ ba lô vào ô Paperdoll (Server-Authoritative).
	 * Áp dụng EquipGameplayEffect lên ASC và ghi nhận FActiveGameplayEffectHandle.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Equipment")
	bool EquipItem(int32 BackpackSlot, EPAEquipmentSlot TargetSlot);

	/**
	 * AC-2: Tháo trang bị từ ô Paperdoll cất lại vào ba lô (Server-Authoritative).
	 * Thu hồi FActiveGameplayEffectHandle và hoàn trả chỉ số tức thì.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Equipment")
	bool UnequipItem(EPAEquipmentSlot Slot, int32 TargetBackpackSlot = -1);

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Equipment")
	void Server_EquipItem(int32 BackpackSlot, EPAEquipmentSlot TargetSlot);

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Equipment")
	void Server_UnequipItem(EPAEquipmentSlot Slot, int32 TargetBackpackSlot);

	// -------------------------------------------------------------------------
	// Quickbar Consumable API (AC-3)
	// -------------------------------------------------------------------------

	/**
	 * Gán một vật phẩm trong túi đồ vào 1 trong 4 ô phím tắt Quickbar (chỉ số 0..3).
	 * Yêu cầu thẩm quyền máy chủ và xác thực vật phẩm thuộc danh mục Consumable (GDD §3.3).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Quickbar")
	bool AssignQuickbarSlot(int32 QuickbarIndex, const FGuid& ItemUID);

	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Quickbar")
	bool ClearQuickbarSlot(int32 QuickbarIndex);

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Quickbar")
	FGuid GetQuickbarItemUID(int32 QuickbarIndex) const;

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Quickbar")
	void Server_AssignQuickbarSlot(int32 QuickbarIndex, const FGuid& ItemUID);

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Quickbar")
	void Server_ClearQuickbarSlot(int32 QuickbarIndex);

	/**
	 * AC-3: Sử dụng bình tiêu hao tại ô Quickbar chỉ định (phím 1-4).
	 * Giảm 30% tốc độ di chuyển trong 0.8s, trừ 1 stack, và hồi máu sau 0.8s.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Quickbar")
	bool UseQuickbarSlot(int32 QuickbarIndex);

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Quickbar")
	void Server_UseQuickbarSlot(int32 QuickbarIndex);

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Quickbar")
	bool IsDrinkingPotion() const { return bIsDrinkingPotion; }

	// -------------------------------------------------------------------------
	// Skill Book API (AC-4)
	// -------------------------------------------------------------------------

	/**
	 * AC-4: Đọc Sách Kỹ Năng từ ba lô.
	 * Kiểm tra thẻ chức nghiệp (RequiredClassTag) và cấm đọc khi đang giao tranh (!State.InCombat).
	 * Cấp UGameplayAbility vào ASC khi thành công.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|SkillBook")
	bool UseSkillBook(int32 BackpackSlot, EPAItemUseError& OutError);

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|SkillBook")
	void Server_UseSkillBook(int32 BackpackSlot);

	/**
	 * Client RPC thông báo từ chối sử dụng vật phẩm / sách kỹ năng về Client UI.
	 */
	UFUNCTION(Client, Reliable, Category = "ProjectAscendant|Equipment")
	void Client_NotifyItemUseRejected(EPAItemUseError ErrorCode);

public:
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Equipment|Events")
	FPAOnItemEquipped OnItemEquipped;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Equipment|Events")
	FPAOnItemUnequipped OnItemUnequipped;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Quickbar|Events")
	FPAOnQuickbarSlotChanged OnQuickbarSlotChanged;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Quickbar|Events")
	FPAOnQuickbarUsed OnQuickbarUsed;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|SkillBook|Events")
	FPAOnSkillBookLearned OnSkillBookLearned;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Equipment|Events")
	FPAOnItemUseRejected OnItemUseRejected;

protected:
	/**
	 * Danh sách 6 ô trang bị Paperdoll (quản lý qua FastArray Serializer theo Control Manifest §1).
	 */
	UPROPERTY(Replicated)
	FPAEquipmentItemList EquipmentList;

	/** Danh sách 4 ô phím tắt Quickbar (chỉ số 0..3) */
	UPROPERTY(ReplicatedUsing = OnRep_QuickbarUIDs)
	TArray<FGuid> QuickbarItemUIDs;

	/** Trạng thái đang uống bình dược phẩm */
	UPROPERTY(ReplicatedUsing = OnRep_IsDrinkingPotion, BlueprintReadOnly, Category = "ProjectAscendant|Quickbar")
	bool bIsDrinkingPotion = false;

	/** Bộ theo dõi các Handle GameplayEffect đang cấp chỉ số cho từng slot trang bị (Server-Only) */
	TMap<EPAEquipmentSlot, FActiveGameplayEffectHandle> ActiveEquipEffectHandles;

	/** Bộ hẹn giờ cho hành động uống bình dược phẩm 0.8s */
	FTimerHandle TimerHandle_PotionDrink;

	/** Chỉ số ô Quickbar đang được sử dụng để uống bình */
	int32 ActiveDrinkingQuickbarIndex = INDEX_NONE;

	/** Lưu trữ thông tin lượng máu cần hồi khi uống xong */
	float PendingPotionHealthRestore = 0.0f;

	UFUNCTION()
	virtual void OnRep_QuickbarUIDs();

	UFUNCTION()
	virtual void OnRep_IsDrinkingPotion();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPAInventoryComponent* GetInventoryComponent() const;
	UAbilitySystemComponent* GetAbilitySystemComponent() const;
	APABaseCharacter* GetBaseCharacter() const;

	void CompletePotionDrinking();
	void ApplySpeedDebuff(bool bApply);
	FPAInventoryItemEntry* FindMutableEquippedEntry(EPAEquipmentSlot Slot);
};
