// Copyright Project Ascendant. All Rights Reserved.

#include "Inventory/PAEquipmentComponent.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAItemStaticDataAsset.h"
#include "Character/PABaseCharacter.h"
#include "Combat/AscendantAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/World.h"

UPAEquipmentComponent::UPAEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	EquipmentList.OwnerComponent = this;
	QuickbarItemUIDs.Init(FGuid(), kQuickbarSlotCount);
	bIsDrinkingPotion = false;
	ActiveDrinkingQuickbarIndex = INDEX_NONE;
	PendingPotionHealthRestore = 0.0f;
}

void UPAEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	EquipmentList.OwnerComponent = this;
}

bool UPAEquipmentComponent::IsSlotValid(EPAEquipmentSlot Slot)
{
	return Slot >= EPAEquipmentSlot::Mainhand && Slot <= EPAEquipmentSlot::Ring2;
}

bool UPAEquipmentComponent::CanItemFitInSlot(const UItemStaticDataAsset* ItemData, EPAEquipmentSlot TargetSlot)
{
	if (!ItemData || !IsSlotValid(TargetSlot))
	{
		return false;
	}

	if (ItemData->Category != EPAItemCategory::Equipment)
	{
		return false;
	}

	if (ItemData->AllowedEquipmentSlot == TargetSlot)
	{
		return true;
	}

	// Nhẫn có thể lắp vào Ring1 hoặc Ring2
	if (ItemData->AllowedEquipmentSlot == EPAEquipmentSlot::Ring1 || ItemData->AllowedEquipmentSlot == EPAEquipmentSlot::Ring2)
	{
		return TargetSlot == EPAEquipmentSlot::Ring1 || TargetSlot == EPAEquipmentSlot::Ring2;
	}

	return false;
}

const FPAInventoryItemEntry* UPAEquipmentComponent::GetEquippedItem(EPAEquipmentSlot Slot) const
{
	if (!IsSlotValid(Slot))
	{
		return nullptr;
	}

	const int32 TargetSlotIndex = static_cast<int32>(Slot);
	for (const FPAInventoryItemEntry& Entry : EquipmentList.Items)
	{
		if (Entry.SlotIndex == TargetSlotIndex && Entry.StaticData != nullptr)
		{
			return &Entry;
		}
	}

	return nullptr;
}

FPAInventoryItemEntry* UPAEquipmentComponent::FindMutableEquippedEntry(EPAEquipmentSlot Slot)
{
	if (!IsSlotValid(Slot))
	{
		return nullptr;
	}

	const int32 TargetSlotIndex = static_cast<int32>(Slot);
	for (FPAInventoryItemEntry& Entry : EquipmentList.Items)
	{
		if (Entry.SlotIndex == TargetSlotIndex && Entry.StaticData != nullptr)
		{
			return &Entry;
		}
	}

	return nullptr;
}

bool UPAEquipmentComponent::IsSlotOccupied(EPAEquipmentSlot Slot) const
{
	return GetEquippedItem(Slot) != nullptr;
}

UPAInventoryComponent* UPAEquipmentComponent::GetInventoryComponent() const
{
	if (AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UPAInventoryComponent>();
	}
	return nullptr;
}

UAbilitySystemComponent* UPAEquipmentComponent::GetAbilitySystemComponent() const
{
	if (const APABaseCharacter* BaseChar = GetBaseCharacter())
	{
		return BaseChar->GetAbilitySystemComponent();
	}

	if (AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UAbilitySystemComponent>();
	}

	return nullptr;
}

APABaseCharacter* UPAEquipmentComponent::GetBaseCharacter() const
{
	return Cast<APABaseCharacter>(GetOwner());
}

bool UPAEquipmentComponent::EquipItem(int32 BackpackSlot, EPAEquipmentSlot TargetSlot)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (!IsSlotValid(TargetSlot))
	{
		Client_NotifyItemUseRejected(EPAItemUseError::InvalidSlot);
		return false;
	}

	UPAInventoryComponent* Inv = GetInventoryComponent();
	if (!Inv)
	{
		return false;
	}

	const FPAInventoryItemEntry* BackpackItem = Inv->GetItemAtSlot(BackpackSlot);
	if (!BackpackItem || !BackpackItem->StaticData)
	{
		Client_NotifyItemUseRejected(EPAItemUseError::ItemNotFound);
		return false;
	}

	if (!CanItemFitInSlot(BackpackItem->StaticData, TargetSlot))
	{
		Client_NotifyItemUseRejected(EPAItemUseError::InvalidSlot);
		return false;
	}

	// Nếu slot trang bị đã có đồ sẵn -> tháo đồ cũ ra ba lô trước
	if (IsSlotOccupied(TargetSlot))
	{
		if (!UnequipItem(TargetSlot, BackpackSlot))
		{
			return false;
		}
	}

	// Sao lưu thông tin trang bị trước khi trừ khỏi ba lô
	UItemStaticDataAsset* StaticData = BackpackItem->StaticData;
	const FPAItemInstanceData DynamicData = BackpackItem->DynamicData;
	const FGuid ItemUID = BackpackItem->ItemInstanceUID;
	const FName ItemDefId = BackpackItem->ItemDefId;

	// Xóa 1 vật phẩm khỏi ba lô
	Inv->RemoveItemFromSlot(BackpackSlot, 1);

	// Gán vào khung FastArray Paperdoll
	const int32 SlotIndex = static_cast<int32>(TargetSlot);
	FPAInventoryItemEntry NewEquipEntry(SlotIndex, ItemDefId, StaticData, 1, ItemUID);
	NewEquipEntry.DynamicData = DynamicData;

	EquipmentList.Items.Add(NewEquipEntry);
	EquipmentList.MarkItemDirty(EquipmentList.Items.Last());

	// AC-2: Áp dụng GameplayEffect lên ASC
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC && StaticData->EquipGameplayEffect)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(GetOwner());

		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(StaticData->EquipGameplayEffect, 1.0f, Context);
		if (SpecHandle.IsValid())
		{
			const FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			ActiveEquipEffectHandles.Add(TargetSlot, ActiveHandle);
		}
	}

	OnItemEquipped.Broadcast(TargetSlot, EquipmentList.Items.Last());
	return true;
}

bool UPAEquipmentComponent::UnequipItem(EPAEquipmentSlot Slot, int32 TargetBackpackSlot)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (!IsSlotValid(Slot) || !IsSlotOccupied(Slot))
	{
		return false;
	}

	UPAInventoryComponent* Inv = GetInventoryComponent();
	if (!Inv)
	{
		return false;
	}

	int32 DestinationSlot = TargetBackpackSlot;
	if (!Inv->IsValidSlotIndex(DestinationSlot) || Inv->IsSlotOccupied(DestinationSlot))
	{
		DestinationSlot = Inv->FindFirstEmptySlot();
	}

	if (DestinationSlot == INDEX_NONE)
	{
		return false;
	}

	const int32 TargetSlotIndex = static_cast<int32>(Slot);
	int32 FoundIndex = INDEX_NONE;
	for (int32 i = 0; i < EquipmentList.Items.Num(); ++i)
	{
		if (EquipmentList.Items[i].SlotIndex == TargetSlotIndex)
		{
			FoundIndex = i;
			break;
		}
	}

	if (FoundIndex == INDEX_NONE)
	{
		return false;
	}

	FPAInventoryItemEntry ItemToUnequip = EquipmentList.Items[FoundIndex];

	// AC-2: Thu hồi GameplayEffect handle tương ứng
	if (ActiveEquipEffectHandles.Contains(Slot))
	{
		const FActiveGameplayEffectHandle Handle = ActiveEquipEffectHandles[Slot];
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			if (Handle.IsValid())
			{
				ASC->RemoveActiveGameplayEffect(Handle);
			}
		}
		ActiveEquipEffectHandles.Remove(Slot);
	}

	// Đưa lại vào ba lô (bảo toàn ItemInstanceUID cũ)
	Inv->AddItemToSlot(DestinationSlot, ItemToUnequip.StaticData, 1, ItemToUnequip.DynamicData, ItemToUnequip.ItemInstanceUID);

	// Dọn sạch ô FastArray
	EquipmentList.Items.RemoveAt(FoundIndex);
	EquipmentList.MarkArrayDirty();

	OnItemUnequipped.Broadcast(Slot, ItemToUnequip);
	return true;
}

bool UPAEquipmentComponent::Server_EquipItem_Validate(int32 BackpackSlot, EPAEquipmentSlot TargetSlot)
{
	return BackpackSlot >= 0 && IsSlotValid(TargetSlot);
}

void UPAEquipmentComponent::Server_EquipItem_Implementation(int32 BackpackSlot, EPAEquipmentSlot TargetSlot)
{
	EquipItem(BackpackSlot, TargetSlot);
}

bool UPAEquipmentComponent::Server_UnequipItem_Validate(EPAEquipmentSlot Slot, int32 TargetBackpackSlot)
{
	return IsSlotValid(Slot);
}

void UPAEquipmentComponent::Server_UnequipItem_Implementation(EPAEquipmentSlot Slot, int32 TargetBackpackSlot)
{
	UnequipItem(Slot, TargetBackpackSlot);
}

bool UPAEquipmentComponent::AssignQuickbarSlot(int32 QuickbarIndex, const FGuid& ItemUID)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (QuickbarIndex < 0 || QuickbarIndex >= kQuickbarSlotCount)
	{
		return false;
	}

	// Xác thực vật phẩm thuộc danh mục Consumables (GDD §3.3)
	if (const UPAInventoryComponent* Inv = GetInventoryComponent())
	{
		const int32 InvSlot = Inv->FindSlotByItemUID(ItemUID);
		if (InvSlot != INDEX_NONE)
		{
			if (const FPAInventoryItemEntry* Entry = Inv->GetItemAtSlot(InvSlot))
			{
				if (!Entry->StaticData || Entry->StaticData->Category != EPAItemCategory::Consumable)
				{
					Client_NotifyItemUseRejected(EPAItemUseError::NotConsumable);
					return false;
				}
			}
		}
	}

	QuickbarItemUIDs[QuickbarIndex] = ItemUID;
	OnQuickbarSlotChanged.Broadcast(QuickbarIndex, ItemUID);
	return true;
}

bool UPAEquipmentComponent::ClearQuickbarSlot(int32 QuickbarIndex)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (QuickbarIndex < 0 || QuickbarIndex >= kQuickbarSlotCount)
	{
		return false;
	}

	QuickbarItemUIDs[QuickbarIndex].Invalidate();
	OnQuickbarSlotChanged.Broadcast(QuickbarIndex, FGuid());
	return true;
}

FGuid UPAEquipmentComponent::GetQuickbarItemUID(int32 QuickbarIndex) const
{
	if (QuickbarIndex >= 0 && QuickbarIndex < kQuickbarSlotCount)
	{
		return QuickbarItemUIDs[QuickbarIndex];
	}
	return FGuid();
}

bool UPAEquipmentComponent::Server_AssignQuickbarSlot_Validate(int32 QuickbarIndex, const FGuid& ItemUID)
{
	return QuickbarIndex >= 0 && QuickbarIndex < kQuickbarSlotCount && ItemUID.IsValid();
}

void UPAEquipmentComponent::Server_AssignQuickbarSlot_Implementation(int32 QuickbarIndex, const FGuid& ItemUID)
{
	AssignQuickbarSlot(QuickbarIndex, ItemUID);
}

bool UPAEquipmentComponent::Server_ClearQuickbarSlot_Validate(int32 QuickbarIndex)
{
	return QuickbarIndex >= 0 && QuickbarIndex < kQuickbarSlotCount;
}

void UPAEquipmentComponent::Server_ClearQuickbarSlot_Implementation(int32 QuickbarIndex)
{
	ClearQuickbarSlot(QuickbarIndex);
}

bool UPAEquipmentComponent::UseQuickbarSlot(int32 QuickbarIndex)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (QuickbarIndex < 0 || QuickbarIndex >= kQuickbarSlotCount)
	{
		Client_NotifyItemUseRejected(EPAItemUseError::EmptyQuickbarSlot);
		return false;
	}

	if (bIsDrinkingPotion)
	{
		Client_NotifyItemUseRejected(EPAItemUseError::AlreadyDrinking);
		return false;
	}

	const FGuid ItemUID = QuickbarItemUIDs[QuickbarIndex];
	if (!ItemUID.IsValid())
	{
		Client_NotifyItemUseRejected(EPAItemUseError::EmptyQuickbarSlot);
		return false;
	}

	UPAInventoryComponent* Inv = GetInventoryComponent();
	if (!Inv)
	{
		return false;
	}

	// Khắc phục smell Feature Envy: Gọi trực tiếp hàm tra cứu từ UPAInventoryComponent
	const int32 InvSlot = Inv->FindSlotByItemUID(ItemUID);
	if (InvSlot == INDEX_NONE)
	{
		ClearQuickbarSlot(QuickbarIndex);
		Client_NotifyItemUseRejected(EPAItemUseError::ItemNotFound);
		return false;
	}

	const FPAInventoryItemEntry* Entry = Inv->GetItemAtSlot(InvSlot);
	if (!Entry || !Entry->StaticData || Entry->StaticData->Category != EPAItemCategory::Consumable)
	{
		Client_NotifyItemUseRejected(EPAItemUseError::NotConsumable);
		return false;
	}

	UItemStaticDataAsset* ConsumableData = Entry->StaticData;
	ActiveDrinkingQuickbarIndex = QuickbarIndex;
	PendingPotionHealthRestore = ConsumableData->ConsumableHealthRestore;

	// AC-3: Giảm stack count đi 1
	const int32 NewStack = Entry->StackCount - 1;
	Inv->RemoveItemFromSlot(InvSlot, 1);

	// Nếu hết stack -> tự động dọn sạch ô quickbar
	if (NewStack <= 0)
	{
		ClearQuickbarSlot(QuickbarIndex);
	}

	// AC-3: Giảm 30% tốc độ di chuyển trong 0.8s
	bIsDrinkingPotion = true;
	ApplySpeedDebuff(true);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle_PotionDrink);
		World->GetTimerManager().SetTimer(
			TimerHandle_PotionDrink,
			this,
			&UPAEquipmentComponent::CompletePotionDrinking,
			ConsumableData->ConsumableDuration,
			false
		);
	}
	else
	{
		CompletePotionDrinking();
	}

	return true;
}

void UPAEquipmentComponent::CompletePotionDrinking()
{
	bIsDrinkingPotion = false;
	ApplySpeedDebuff(false);

	// AC-3: Hồi phục sinh mệnh qua GAS Attribute
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->ApplyModToAttributeUnsafe(UAscendantAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, PendingPotionHealthRestore);
	}
	else if (APABaseCharacter* BaseChar = GetBaseCharacter())
	{
		if (UAscendantAttributeSet* AttrSet = BaseChar->GetAttributeSet())
		{
			const float CurrentHP = AttrSet->GetHealth();
			const float MaxHP = AttrSet->GetMaxHealth();
			AttrSet->SetHealth(FMath::Clamp(CurrentHP + PendingPotionHealthRestore, 0.0f, MaxHP));
		}
	}

	OnQuickbarUsed.Broadcast(ActiveDrinkingQuickbarIndex, PendingPotionHealthRestore);

	PendingPotionHealthRestore = 0.0f;
	ActiveDrinkingQuickbarIndex = INDEX_NONE;
}

void UPAEquipmentComponent::ApplySpeedDebuff(bool bApply)
{
	if (APABaseCharacter* BaseChar = GetBaseCharacter())
	{
		const float BaseSpeed = BaseChar->GetBaseMoveSpeed();
		if (bApply)
		{
			BaseChar->SetBaseMoveSpeed(BaseSpeed * (1.0f - kDefaultSpeedDebuffRatio));
		}
		else
		{
			BaseChar->SetBaseMoveSpeed(BaseSpeed / (1.0f - kDefaultSpeedDebuffRatio));
		}
	}
}

bool UPAEquipmentComponent::Server_UseQuickbarSlot_Validate(int32 QuickbarIndex)
{
	return QuickbarIndex >= 0 && QuickbarIndex < kQuickbarSlotCount;
}

void UPAEquipmentComponent::Server_UseQuickbarSlot_Implementation(int32 QuickbarIndex)
{
	UseQuickbarSlot(QuickbarIndex);
}

bool UPAEquipmentComponent::UseSkillBook(int32 BackpackSlot, EPAItemUseError& OutError)
{
	OutError = EPAItemUseError::None;

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	UPAInventoryComponent* Inv = GetInventoryComponent();
	if (!Inv)
	{
		OutError = EPAItemUseError::ItemNotFound;
		Client_NotifyItemUseRejected(OutError);
		return false;
	}

	const FPAInventoryItemEntry* Entry = Inv->GetItemAtSlot(BackpackSlot);
	if (!Entry || !Entry->StaticData)
	{
		OutError = EPAItemUseError::ItemNotFound;
		Client_NotifyItemUseRejected(OutError);
		return false;
	}

	if (Entry->StaticData->Category != EPAItemCategory::SkillBook)
	{
		OutError = EPAItemUseError::NotSkillBook;
		Client_NotifyItemUseRejected(OutError);
		return false;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

	// AC-4: Kiểm tra trạng thái giao tranh (Cấm đọc sách khi đang có tag State.InCombat)
	static const FGameplayTag TagInCombat = FGameplayTag::RequestGameplayTag(FName("State.InCombat"), false);
	if (ASC && TagInCombat.IsValid() && ASC->HasMatchingGameplayTag(TagInCombat))
	{
		OutError = EPAItemUseError::NotInCombatRequired;
		Client_NotifyItemUseRejected(OutError);
		return false;
	}

	// AC-4: Kiểm tra chức nghiệp (RequiredClassTag)
	const FGameplayTag& RequiredTag = Entry->StaticData->RequiredClassTag;
	if (RequiredTag.IsValid())
	{
		if (!ASC || !ASC->HasMatchingGameplayTag(RequiredTag))
		{
			OutError = EPAItemUseError::ClassMismatch;
			Client_NotifyItemUseRejected(OutError);
			return false;
		}
	}

	// Hợp lệ: Trừ 1 cuốn sách khỏi ba lô và cấp UGameplayAbility vào ASC
	UItemStaticDataAsset* BookData = Entry->StaticData;
	Inv->RemoveItemFromSlot(BackpackSlot, 1);

	if (ASC && BookData->GrantedAbilityClass)
	{
		ASC->GiveAbility(FGameplayAbilitySpec(BookData->GrantedAbilityClass, 1, INDEX_NONE, GetOwner()));
	}

	OnSkillBookLearned.Broadcast(BookData->ItemId, BookData->GrantedAbilityClass);
	return true;
}

bool UPAEquipmentComponent::Server_UseSkillBook_Validate(int32 BackpackSlot)
{
	return BackpackSlot >= 0;
}

void UPAEquipmentComponent::Server_UseSkillBook_Implementation(int32 BackpackSlot)
{
	EPAItemUseError IgnoredError;
	UseSkillBook(BackpackSlot, IgnoredError);
}

void UPAEquipmentComponent::Client_NotifyItemUseRejected_Implementation(EPAItemUseError ErrorCode)
{
	OnItemUseRejected.Broadcast(ErrorCode);
}

void UPAEquipmentComponent::OnRep_QuickbarUIDs()
{
}

void UPAEquipmentComponent::OnRep_IsDrinkingPotion()
{
}

void UPAEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UPAEquipmentComponent, EquipmentList, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UPAEquipmentComponent, QuickbarItemUIDs, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UPAEquipmentComponent, bIsDrinkingPotion, COND_None);
}
