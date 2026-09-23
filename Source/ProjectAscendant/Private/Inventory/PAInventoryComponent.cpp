// Copyright Project Ascendant. All Rights Reserved.

#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAItemStaticDataAsset.h"
#include "Net/UnrealNetwork.h"

// -----------------------------------------------------------------------------
// FPAInventoryItemEntry Callbacks
// -----------------------------------------------------------------------------

void FPAInventoryItemEntry::PreReplicatedRemove(const FPAInventoryItemList& InArraySerializer)
{
	if (InArraySerializer.OwnerComponent.IsValid())
	{
		InArraySerializer.OwnerComponent->HandleItemRemoved(*this);
	}
}

void FPAInventoryItemEntry::PostReplicatedAdd(const FPAInventoryItemList& InArraySerializer)
{
	if (InArraySerializer.OwnerComponent.IsValid())
	{
		InArraySerializer.OwnerComponent->HandleItemAdded(*this);
	}
}

void FPAInventoryItemEntry::PostReplicatedChange(const FPAInventoryItemList& InArraySerializer)
{
	if (InArraySerializer.OwnerComponent.IsValid())
	{
		InArraySerializer.OwnerComponent->HandleItemChanged(*this);
	}
}

// -----------------------------------------------------------------------------
// UPAInventoryComponent Implementation
// -----------------------------------------------------------------------------

UPAInventoryComponent::UPAInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	CurrentMaxSlots = kBaseInventorySlots; // AC-1: Khởi tạo 30 ô
	InventoryList.OwnerComponent = this;
}

void UPAInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	InventoryList.OwnerComponent = this;
}

bool UPAInventoryComponent::UnlockMoreSlots(int32 AdditionalSlots)
{
	if (CurrentMaxSlots >= kMaxInventorySlots || AdditionalSlots <= 0)
	{
		return false;
	}

	const int32 NewCapacity = FMath::Clamp(CurrentMaxSlots + AdditionalSlots, kBaseInventorySlots, kMaxInventorySlots);
	if (NewCapacity != CurrentMaxSlots)
	{
		CurrentMaxSlots = NewCapacity;
		OnInventoryCapacityChanged.Broadcast(CurrentMaxSlots);
		return true;
	}

	return false;
}

FPAInventoryItemEntry* UPAInventoryComponent::FindMutableEntryAtSlot(int32 SlotIndex)
{
	for (FPAInventoryItemEntry& Entry : InventoryList.Items)
	{
		if (Entry.SlotIndex == SlotIndex)
		{
			return &Entry;
		}
	}
	return nullptr;
}

const FPAInventoryItemEntry* UPAInventoryComponent::GetItemAtSlot(int32 SlotIndex) const
{
	for (const FPAInventoryItemEntry& Entry : InventoryList.Items)
	{
		if (Entry.SlotIndex == SlotIndex)
		{
			return &Entry;
		}
	}
	return nullptr;
}

int32 UPAInventoryComponent::FindFirstEmptySlot() const
{
	TSet<int32> OccupiedSlots;
	OccupiedSlots.Reserve(InventoryList.Items.Num());

	for (const FPAInventoryItemEntry& Entry : InventoryList.Items)
	{
		OccupiedSlots.Add(Entry.SlotIndex);
	}

	for (int32 i = 0; i < CurrentMaxSlots; ++i)
	{
		if (!OccupiedSlots.Contains(i))
		{
			return i;
		}
	}

	return INDEX_NONE;
}

bool UPAInventoryComponent::TryAddItem(UItemStaticDataAsset* ItemData, int32 Quantity, int32& OutRemainingQuantity)
{
	OutRemainingQuantity = Quantity;

	if (!ItemData || Quantity <= 0)
	{
		return false;
	}

	const FName ItemKey = ItemData->ItemId.IsNone() ? ItemData->GetFName() : ItemData->ItemId;
	const int32 MaxStack = ItemData->MaxStackSize > 0
		? ItemData->MaxStackSize
		: UItemStaticDataAsset::GetDefaultMaxStackForCategory(ItemData->Category);

	const int32 StartingQuantity = Quantity;

	if (MaxStack > 1)
	{
		for (FPAInventoryItemEntry& Entry : InventoryList.Items)
		{
			if (Entry.ItemDefId == ItemKey && Entry.StackCount < MaxStack)
			{
				const int32 AvailableSpace = MaxStack - Entry.StackCount;
				const int32 AmountToAdd = FMath::Min(Quantity, AvailableSpace);

				Entry.StackCount += AmountToAdd;
				Quantity -= AmountToAdd;

				InventoryList.MarkItemDirty(Entry);
				OnInventorySlotUpdated.Broadcast(Entry.SlotIndex, Entry.StackCount);

				if (Quantity <= 0)
				{
					OutRemainingQuantity = 0;
					return true;
				}
			}
		}
	}

	while (Quantity > 0)
	{
		const int32 EmptySlot = FindFirstEmptySlot();
		if (EmptySlot == INDEX_NONE)
		{
			break;
		}

		const int32 AmountToSlot = FMath::Min(Quantity, MaxStack);

		FPAInventoryItemEntry NewEntry;
		NewEntry.SlotIndex = EmptySlot;
		NewEntry.ItemInstanceUID = FGuid::NewGuid();
		NewEntry.ItemDefId = ItemKey;
		NewEntry.StaticData = ItemData;
		NewEntry.StackCount = AmountToSlot;
		NewEntry.DynamicData = FPAItemInstanceData();

		InventoryList.Items.Add(NewEntry);
		InventoryList.MarkItemDirty(InventoryList.Items.Last());

		OnInventorySlotUpdated.Broadcast(EmptySlot, AmountToSlot);

		Quantity -= AmountToSlot;
	}

	OutRemainingQuantity = Quantity;
	return Quantity < StartingQuantity;
}

bool UPAInventoryComponent::AddItemToSlot(int32 TargetSlot, UItemStaticDataAsset* ItemData, int32 Quantity, const FPAItemInstanceData& DynamicData, const FGuid& InItemUID)
{
	if (!IsValidSlotIndex(TargetSlot) || !ItemData || Quantity <= 0)
	{
		return false;
	}

	const FName ItemKey = ItemData->ItemId.IsNone() ? ItemData->GetFName() : ItemData->ItemId;
	const int32 MaxStack = ItemData->MaxStackSize > 0
		? ItemData->MaxStackSize
		: UItemStaticDataAsset::GetDefaultMaxStackForCategory(ItemData->Category);

	if (Quantity > MaxStack)
	{
		return false;
	}

	if (FPAInventoryItemEntry* Existing = FindMutableEntryAtSlot(TargetSlot))
	{
		Existing->ItemDefId = ItemKey;
		Existing->StaticData = ItemData;
		Existing->StackCount = Quantity;
		Existing->DynamicData = DynamicData;
		if (InItemUID.IsValid())
		{
			Existing->ItemInstanceUID = InItemUID;
		}
		InventoryList.MarkItemDirty(*Existing);
		OnInventorySlotUpdated.Broadcast(TargetSlot, Quantity);
	}
	else
	{
		FPAInventoryItemEntry NewEntry;
		NewEntry.SlotIndex = TargetSlot;
		NewEntry.ItemInstanceUID = InItemUID.IsValid() ? InItemUID : FGuid::NewGuid();
		NewEntry.ItemDefId = ItemKey;
		NewEntry.StaticData = ItemData;
		NewEntry.StackCount = Quantity;
		NewEntry.DynamicData = DynamicData;

		InventoryList.Items.Add(NewEntry);
		InventoryList.MarkItemDirty(InventoryList.Items.Last());
		OnInventorySlotUpdated.Broadcast(TargetSlot, Quantity);
	}

	return true;
}

bool UPAInventoryComponent::RemoveItemFromSlot(int32 SlotIndex, int32 QuantityToRemove)
{
	if (!IsValidSlotIndex(SlotIndex) || QuantityToRemove <= 0)
	{
		return false;
	}

	for (int32 i = 0; i < InventoryList.Items.Num(); ++i)
	{
		if (InventoryList.Items[i].SlotIndex == SlotIndex)
		{
			if (InventoryList.Items[i].StackCount <= QuantityToRemove)
			{
				InventoryList.Items.RemoveAt(i);
				InventoryList.MarkArrayDirty();
				OnInventorySlotUpdated.Broadcast(SlotIndex, 0);
			}
			else
			{
				InventoryList.Items[i].StackCount -= QuantityToRemove;
				InventoryList.MarkItemDirty(InventoryList.Items[i]);
				OnInventorySlotUpdated.Broadcast(SlotIndex, InventoryList.Items[i].StackCount);
			}
			return true;
		}
	}

	return false;
}

int32 UPAInventoryComponent::FindSlotByItemUID(const FGuid& ItemUID) const
{
	if (!ItemUID.IsValid())
	{
		return INDEX_NONE;
	}

	for (const FPAInventoryItemEntry& Entry : InventoryList.Items)
	{
		if (Entry.ItemInstanceUID == ItemUID)
		{
			return Entry.SlotIndex;
		}
	}

	return INDEX_NONE;
}

int32 UPAInventoryComponent::GetItemCount(FName ItemId) const
{
	int32 Total = 0;
	for (const FPAInventoryItemEntry& Entry : InventoryList.Items)
	{
		if (Entry.ItemDefId == ItemId)
		{
			Total += Entry.StackCount;
		}
	}
	return Total;
}

bool UPAInventoryComponent::ConsumeItemQuantity(FName ItemId, int32 QuantityToConsume)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (QuantityToConsume <= 0 || ItemId.IsNone())
	{
		return false;
	}

	if (GetItemCount(ItemId) < QuantityToConsume)
	{
		return false;
	}

	int32 RemainingToConsume = QuantityToConsume;
	for (int32 i = InventoryList.Items.Num() - 1; i >= 0 && RemainingToConsume > 0; --i)
	{
		if (InventoryList.Items[i].ItemDefId == ItemId)
		{
			const int32 InStack = InventoryList.Items[i].StackCount;
			const int32 SlotIdx = InventoryList.Items[i].SlotIndex;

			if (InStack <= RemainingToConsume)
			{
				RemainingToConsume -= InStack;
				InventoryList.Items.RemoveAt(i);
				InventoryList.MarkArrayDirty();
				OnInventorySlotUpdated.Broadcast(SlotIdx, 0);
			}
			else
			{
				InventoryList.Items[i].StackCount -= RemainingToConsume;
				RemainingToConsume = 0;
				InventoryList.MarkItemDirty(InventoryList.Items[i]);
				OnInventorySlotUpdated.Broadcast(SlotIdx, InventoryList.Items[i].StackCount);
			}
		}
	}

	return RemainingToConsume == 0;
}

bool UPAInventoryComponent::UpdateItemDynamicData(int32 SlotIndex, const FPAItemInstanceData& NewDynamicData)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (!IsValidSlotIndex(SlotIndex))
	{
		return false;
	}

	if (FPAInventoryItemEntry* Entry = FindMutableEntryAtSlot(SlotIndex))
	{
		Entry->DynamicData = NewDynamicData;
		InventoryList.MarkItemDirty(*Entry);
		OnInventorySlotUpdated.Broadcast(SlotIndex, Entry->StackCount);
		return true;
	}

	return false;
}

void UPAInventoryComponent::ClearInventory()
{
	InventoryList.Items.Empty();
	InventoryList.MarkArrayDirty();
	OverflowStash.Empty();
}

// -----------------------------------------------------------------------------
// Server-Authoritative Transaction RPCs & Duplication Safeguards (Story 002)
// -----------------------------------------------------------------------------

bool UPAInventoryComponent::Server_MoveItem_Validate(int32 SourceSlot, int32 TargetSlot, FGuid ExpectedItemUID, int64 TransactionID)
{
	return true;
}

void UPAInventoryComponent::Server_MoveItem_Implementation(int32 SourceSlot, int32 TargetSlot, FGuid ExpectedItemUID, int64 TransactionID)
{
	if (TransactionID > 0 && TransactionID <= LastProcessedTransactionID)
	{
		Client_NotifyTransactionRejected(TransactionID, EPAInventoryTransactionError::ReplayDetected);
		return;
	}

	const bool bSuccess = MoveItem(SourceSlot, TargetSlot, ExpectedItemUID, TransactionID);
	if (!bSuccess)
	{
		Client_NotifyTransactionRejected(TransactionID, EPAInventoryTransactionError::ServerRejected);
	}
	else if (TransactionID > 0)
	{
		LastProcessedTransactionID = TransactionID;
	}
}

bool UPAInventoryComponent::MoveItem(int32 SourceSlot, int32 TargetSlot, FGuid ExpectedItemUID, int64 TransactionID)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (!IsValidSlotIndex(SourceSlot) || !IsValidSlotIndex(TargetSlot) || SourceSlot == TargetSlot)
	{
		return false;
	}

	FPAInventoryItemEntry* SourceEntry = FindMutableEntryAtSlot(SourceSlot);
	if (!SourceEntry)
	{
		// Ô nguồn không có vật phẩm (ngăn chặn duplicate khi spam packet)
		return false;
	}

	if (ExpectedItemUID.IsValid() && SourceEntry->ItemInstanceUID != ExpectedItemUID)
	{
		// UID không khớp -> giao dịch lỗi thời hoặc gian lận
		return false;
	}

	if (SourceEntry->DynamicData.bIsLocked)
	{
		// AC-3: Vật phẩm đang bị khóa không được phép di chuyển/hoán đổi
		return false;
	}

	FPAInventoryItemEntry* TargetEntry = FindMutableEntryAtSlot(TargetSlot);

	// Trường hợp 1: Ô đích rỗng -> Di chuyển thẳng sang ô đích
	if (!TargetEntry)
	{
		SourceEntry->SlotIndex = TargetSlot;
		InventoryList.MarkItemDirty(*SourceEntry);

		OnInventorySlotUpdated.Broadcast(SourceSlot, 0);
		OnInventorySlotUpdated.Broadcast(TargetSlot, SourceEntry->StackCount);
		OnItemMoved.Broadcast(SourceSlot, TargetSlot);
		return true;
	}

	if (TargetEntry->DynamicData.bIsLocked)
	{
		// Ô đích cũng không được phép hoán đổi nếu đang bị khóa
		return false;
	}

	// Trường hợp 2: Ô đích có cùng loại vật phẩm và có thể cộng dồn
	const int32 MaxStack = SourceEntry->StaticData
		? (SourceEntry->StaticData->MaxStackSize > 0 ? SourceEntry->StaticData->MaxStackSize : UItemStaticDataAsset::GetDefaultMaxStackForCategory(SourceEntry->StaticData->Category))
		: 1;

	if (SourceEntry->ItemDefId == TargetEntry->ItemDefId && MaxStack > 1 && TargetEntry->StackCount < MaxStack)
	{
		const int32 Space = MaxStack - TargetEntry->StackCount;
		const int32 AmountToMerge = FMath::Min(SourceEntry->StackCount, Space);

		TargetEntry->StackCount += AmountToMerge;
		SourceEntry->StackCount -= AmountToMerge;

		InventoryList.MarkItemDirty(*TargetEntry);
		OnInventorySlotUpdated.Broadcast(TargetSlot, TargetEntry->StackCount);

		if (SourceEntry->StackCount <= 0)
		{
			RemoveItemFromSlot(SourceSlot, AmountToMerge);
		}
		else
		{
			InventoryList.MarkItemDirty(*SourceEntry);
			OnInventorySlotUpdated.Broadcast(SourceSlot, SourceEntry->StackCount);
		}

		OnItemMoved.Broadcast(SourceSlot, TargetSlot);
		return true;
	}

	// Trường hợp 3: Ô đích có vật phẩm khác -> Hoán đổi nguyên tử 2 ô (Atomic Swap)
	SourceEntry->SlotIndex = TargetSlot;
	TargetEntry->SlotIndex = SourceSlot;

	InventoryList.MarkItemDirty(*SourceEntry);
	InventoryList.MarkItemDirty(*TargetEntry);

	OnInventorySlotUpdated.Broadcast(SourceSlot, TargetEntry->StackCount);
	OnInventorySlotUpdated.Broadcast(TargetSlot, SourceEntry->StackCount);
	OnItemMoved.Broadcast(SourceSlot, TargetSlot);

	return true;
}

bool UPAInventoryComponent::Server_SplitStack_Validate(int32 SourceSlot, int32 SplitAmount, int32 TargetSlot, int64 TransactionID)
{
	return true;
}

void UPAInventoryComponent::Server_SplitStack_Implementation(int32 SourceSlot, int32 SplitAmount, int32 TargetSlot, int64 TransactionID)
{
	if (TransactionID > 0 && TransactionID <= LastProcessedTransactionID)
	{
		Client_NotifyTransactionRejected(TransactionID, EPAInventoryTransactionError::ReplayDetected);
		return;
	}

	const bool bSuccess = SplitStack(SourceSlot, SplitAmount, TargetSlot, TransactionID);
	if (!bSuccess)
	{
		Client_NotifyTransactionRejected(TransactionID, EPAInventoryTransactionError::ServerRejected);
	}
	else if (TransactionID > 0)
	{
		LastProcessedTransactionID = TransactionID;
	}
}

bool UPAInventoryComponent::SplitStack(int32 SourceSlot, int32 SplitAmount, int32 TargetSlot, int64 TransactionID)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (!IsValidSlotIndex(SourceSlot) || !IsValidSlotIndex(TargetSlot) || SourceSlot == TargetSlot)
	{
		return false;
	}

	FPAInventoryItemEntry* SourceEntry = FindMutableEntryAtSlot(SourceSlot);
	if (!SourceEntry || SplitAmount <= 0 || SplitAmount >= SourceEntry->StackCount)
	{
		// Lượng tách không hợp lệ hoặc vượt quá số lượng hiện có
		return false;
	}

	if (SourceEntry->DynamicData.bIsLocked)
	{
		// Vật phẩm bị khóa không được phép tách chồng
		return false;
	}

	if (GetItemAtSlot(TargetSlot) != nullptr)
	{
		// Ô đích phải là ô trống mới được phép tách chồng vào
		return false;
	}

	// Trừ bớt số lượng ô nguồn
	SourceEntry->StackCount -= SplitAmount;
	InventoryList.MarkItemDirty(*SourceEntry);
	OnInventorySlotUpdated.Broadcast(SourceSlot, SourceEntry->StackCount);

	// Tạo phiên bản mới tại ô đích với constructor tập trung
	FPAInventoryItemEntry NewEntry(TargetSlot, SourceEntry->ItemDefId, SourceEntry->StaticData, SplitAmount);
	NewEntry.DynamicData = SourceEntry->DynamicData;

	InventoryList.Items.Add(NewEntry);
	InventoryList.MarkItemDirty(InventoryList.Items.Last());
	OnInventorySlotUpdated.Broadcast(TargetSlot, SplitAmount);

	OnItemSplit.Broadcast(SourceSlot, TargetSlot);
	return true;
}

void UPAInventoryComponent::Client_NotifyTransactionRejected_Implementation(int64 TransactionID, EPAInventoryTransactionError ErrorCode)
{
	OnTransactionRejected.Broadcast(TransactionID, ErrorCode);
}

// -----------------------------------------------------------------------------
// QoL Item Lock & Junk Flags (Story 002 / AC-3)
// -----------------------------------------------------------------------------

bool UPAInventoryComponent::SetItemLocked(int32 SlotIndex, bool bLocked)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (FPAInventoryItemEntry* Entry = FindMutableEntryAtSlot(SlotIndex))
	{
		Entry->DynamicData.bIsLocked = bLocked;
		InventoryList.MarkItemDirty(*Entry);
		OnInventorySlotUpdated.Broadcast(SlotIndex, Entry->StackCount);
		return true;
	}
	return false;
}

bool UPAInventoryComponent::ToggleItemJunk(int32 SlotIndex)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (FPAInventoryItemEntry* Entry = FindMutableEntryAtSlot(SlotIndex))
	{
		Entry->DynamicData.bIsJunk = !Entry->DynamicData.bIsJunk;
		InventoryList.MarkItemDirty(*Entry);
		OnInventorySlotUpdated.Broadcast(SlotIndex, Entry->StackCount);
		return true;
	}
	return false;
}

bool UPAInventoryComponent::CanDropOrSellItem(int32 SlotIndex) const
{
	if (const FPAInventoryItemEntry* Entry = GetItemAtSlot(SlotIndex))
	{
		// AC-3: Vật phẩm đang khóa (bIsLocked == true) tuyệt đối không được bán hoặc vứt
		return !Entry->DynamicData.bIsLocked;
	}
	return false;
}

// -----------------------------------------------------------------------------
// Overflow Stash Routing (Story 002 / AC-4)
// -----------------------------------------------------------------------------

bool UPAInventoryComponent::RouteToOverflowStash(UItemStaticDataAsset* ItemData, int32 Quantity, const FPAItemInstanceData& DynamicData)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (!ItemData || Quantity <= 0 || OverflowStash.Num() >= kMaxOverflowStashSlots)
	{
		return false;
	}

	// AC-4: Chỉ định tuyến vào Hòm Đệm Lửa Trại khi ba lô chính thức đã đầy (30/30)
	if (FindFirstEmptySlot() != INDEX_NONE)
	{
		return false;
	}

	// AC-4: Chỉ định tuyến vật phẩm Tier Rare (3) trở lên vào Hòm Đệm Lửa Trại
	if (ItemData->RarityTier < EPAItemRarity::Rare)
	{
		return false;
	}

	const FName DefId = ItemData->ItemId.IsNone() ? ItemData->GetFName() : ItemData->ItemId;
	FPAInventoryItemEntry Entry(OverflowStash.Num(), DefId, ItemData, Quantity);
	Entry.DynamicData = DynamicData;

	OverflowStash.Add(Entry);
	OnOverflowStashItemAdded.Broadcast(Entry);
	return true;
}

bool UPAInventoryComponent::ClaimOverflowStashItem(int32 StashIndex)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (!OverflowStash.IsValidIndex(StashIndex))
	{
		return false;
	}

	const int32 EmptySlot = FindFirstEmptySlot();
	if (EmptySlot == INDEX_NONE)
	{
		// Túi đồ vẫn đầy, không thể nhận
		return false;
	}

	const FPAInventoryItemEntry StashEntry = OverflowStash[StashIndex];
	OverflowStash.RemoveAt(StashIndex);

	return AddItemToSlot(EmptySlot, StashEntry.StaticData, StashEntry.StackCount, StashEntry.DynamicData, StashEntry.ItemInstanceUID);
}

// -----------------------------------------------------------------------------
// Replication Callbacks
// -----------------------------------------------------------------------------

void UPAInventoryComponent::HandleItemAdded(const FPAInventoryItemEntry& Entry)
{
	OnInventorySlotUpdated.Broadcast(Entry.SlotIndex, Entry.StackCount);
}

void UPAInventoryComponent::HandleItemChanged(const FPAInventoryItemEntry& Entry)
{
	OnInventorySlotUpdated.Broadcast(Entry.SlotIndex, Entry.StackCount);
}

void UPAInventoryComponent::HandleItemRemoved(const FPAInventoryItemEntry& Entry)
{
	OnInventorySlotUpdated.Broadcast(Entry.SlotIndex, 0);
}

void UPAInventoryComponent::OnRep_CurrentMaxSlots()
{
	OnInventoryCapacityChanged.Broadcast(CurrentMaxSlots);
}

void UPAInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPAInventoryComponent, InventoryList);
	DOREPLIFETIME(UPAInventoryComponent, CurrentMaxSlots);
	DOREPLIFETIME_CONDITION(UPAInventoryComponent, OverflowStash, COND_OwnerOnly);
}
