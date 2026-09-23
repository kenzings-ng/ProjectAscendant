// Copyright Project Ascendant. All Rights Reserved.

#include "Economy/PACurrencyComponent.h"
#include "Net/UnrealNetwork.h"

// -----------------------------------------------------------------------------
// FPACurrencyEntry FastArray Callbacks
// -----------------------------------------------------------------------------

void FPACurrencyEntry::PreReplicatedRemove(const FPACurrencyList& InArraySerializer)
{
}

void FPACurrencyEntry::PostReplicatedAdd(const FPACurrencyList& InArraySerializer)
{
	if (InArraySerializer.OwnerComponent.IsValid())
	{
		InArraySerializer.OwnerComponent->OnReplicatedCurrencyChanged(CurrencyType, 0, Amount);
	}
}

void FPACurrencyEntry::PostReplicatedChange(const FPACurrencyList& InArraySerializer)
{
	if (InArraySerializer.OwnerComponent.IsValid())
	{
		InArraySerializer.OwnerComponent->OnReplicatedCurrencyChanged(CurrencyType, 0, Amount);
	}
}

// -----------------------------------------------------------------------------
// UPACurrencyComponent Implementation
// -----------------------------------------------------------------------------

UPACurrencyComponent::UPACurrencyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	Currencies.OwnerComponent = this;
	InitializeDefaultCurrencies();
}

void UPACurrencyComponent::BeginPlay()
{
	Super::BeginPlay();
	Currencies.OwnerComponent = this;
	InitializeDefaultCurrencies();
}

void UPACurrencyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPACurrencyComponent, Currencies);
}

void UPACurrencyComponent::InitializeDefaultCurrencies()
{
	if (Currencies.Entries.Num() == 0)
	{
		Currencies.Entries.Add(FPACurrencyEntry(EPACurrencyType::Gold, 0));
		Currencies.Entries.Add(FPACurrencyEntry(EPACurrencyType::AshShards, 0));
		Currencies.MarkArrayDirty();
	}
}

FPACurrencyEntry* UPACurrencyComponent::FindCurrencyEntryMutable(EPACurrencyType Type)
{
	for (FPACurrencyEntry& Entry : Currencies.Entries)
	{
		if (Entry.CurrencyType == Type)
		{
			return &Entry;
		}
	}
	return nullptr;
}

const FPACurrencyEntry* UPACurrencyComponent::FindCurrencyEntry(EPACurrencyType Type) const
{
	for (const FPACurrencyEntry& Entry : Currencies.Entries)
	{
		if (Entry.CurrencyType == Type)
		{
			return &Entry;
		}
	}
	return nullptr;
}

int64 UPACurrencyComponent::GetGold() const
{
	return GetCurrency(EPACurrencyType::Gold);
}

int64 UPACurrencyComponent::GetAshShards() const
{
	return GetCurrency(EPACurrencyType::AshShards);
}

int64 UPACurrencyComponent::GetCurrency(EPACurrencyType Type) const
{
	const FPACurrencyEntry* Entry = FindCurrencyEntry(Type);
	return Entry ? Entry->Amount : 0LL;
}

bool UPACurrencyComponent::HasSufficientCurrency(EPACurrencyType Type, int64 RequiredAmount) const
{
	if (RequiredAmount <= 0)
	{
		return true;
	}
	return GetCurrency(Type) >= RequiredAmount;
}

bool UPACurrencyComponent::AddCurrency(EPACurrencyType Type, int64 Amount, EPACurrencyTransactionError& OutError)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		OutError = EPACurrencyTransactionError::ServerRejected;
		OnCurrencyTransactionFailed.Broadcast(Type, OutError);
		return false;
	}

	if (Amount <= 0)
	{
		OutError = EPACurrencyTransactionError::InvalidAmount;
		OnCurrencyTransactionFailed.Broadcast(Type, OutError);
		return false;
	}

	FPACurrencyEntry* Entry = FindCurrencyEntryMutable(Type);
	if (!Entry)
	{
		InitializeDefaultCurrencies();
		Entry = FindCurrencyEntryMutable(Type);
	}

	if (!Entry)
	{
		OutError = EPACurrencyTransactionError::ServerRejected;
		OnCurrencyTransactionFailed.Broadcast(Type, OutError);
		return false;
	}

	const int64 MaxCapacity = PACurrencyLimits::GetMaxLimit(Type);
	const int64 OldAmount = Entry->Amount;
	const int64 NewAmount = FMath::Clamp(OldAmount + Amount, 0LL, MaxCapacity);
	const int64 Delta = NewAmount - OldAmount;

	Entry->Amount = NewAmount;
	Currencies.MarkItemDirty(*Entry);

	OutError = EPACurrencyTransactionError::None;
	OnCurrencyBalanceChanged.Broadcast(Type, NewAmount, Delta);
	return true;
}

bool UPACurrencyComponent::DeductCurrency(EPACurrencyType Type, int64 Amount, EPACurrencyTransactionError& OutError)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		OutError = EPACurrencyTransactionError::ServerRejected;
		OnCurrencyTransactionFailed.Broadcast(Type, OutError);
		return false;
	}

	if (Amount <= 0)
	{
		OutError = EPACurrencyTransactionError::InvalidAmount;
		OnCurrencyTransactionFailed.Broadcast(Type, OutError);
		return false;
	}

	FPACurrencyEntry* Entry = FindCurrencyEntryMutable(Type);
	if (!Entry || Entry->Amount < Amount)
	{
		OutError = EPACurrencyTransactionError::InsufficientFunds;
		OnCurrencyTransactionFailed.Broadcast(Type, OutError);
		return false;
	}

	const int64 OldAmount = Entry->Amount;
	const int64 NewAmount = OldAmount - Amount;

	Entry->Amount = NewAmount;
	Currencies.MarkItemDirty(*Entry);

	OutError = EPACurrencyTransactionError::None;
	OnCurrencyBalanceChanged.Broadcast(Type, NewAmount, -Amount);
	return true;
}

bool UPACurrencyComponent::TransferCurrencyTo(UPACurrencyComponent* TargetWallet, EPACurrencyType Type, int64 Amount, EPACurrencyTransactionError& OutError)
{
	if (!TargetWallet || TargetWallet == this)
	{
		OutError = EPACurrencyTransactionError::TargetNotFound;
		OnCurrencyTransactionFailed.Broadcast(Type, OutError);
		return false;
	}

	if (Amount <= 0)
	{
		OutError = EPACurrencyTransactionError::InvalidAmount;
		OnCurrencyTransactionFailed.Broadcast(Type, OutError);
		return false;
	}

	if (!HasSufficientCurrency(Type, Amount))
	{
		OutError = EPACurrencyTransactionError::InsufficientFunds;
		OnCurrencyTransactionFailed.Broadcast(Type, OutError);
		return false;
	}

	const int64 TargetMax = PACurrencyLimits::GetMaxLimit(Type);
	if (TargetWallet->GetCurrency(Type) + Amount > TargetMax)
	{
		OutError = EPACurrencyTransactionError::ExceedsMaxCapacity;
		OnCurrencyTransactionFailed.Broadcast(Type, OutError);
		return false;
	}

	// Atomic Execution
	EPACurrencyTransactionError DeductErr = EPACurrencyTransactionError::None;
	if (!DeductCurrency(Type, Amount, DeductErr))
	{
		OutError = DeductErr;
		return false;
	}

	EPACurrencyTransactionError AddErr = EPACurrencyTransactionError::None;
	if (!TargetWallet->AddCurrency(Type, Amount, AddErr))
	{
		// Rollback on target failure
		AddCurrency(Type, Amount, DeductErr);
		OutError = AddErr;
		return false;
	}

	OutError = EPACurrencyTransactionError::None;
	return true;
}

bool UPACurrencyComponent::HandlePvEDeathPenalty(int64& OutGoldLost)
{
	const int64 CurrentGold = GetGold();
	if (CurrentGold <= 0)
	{
		OutGoldLost = 0;
		return true;
	}

	// High-Stakes PvE Death Penalty: 50% Gold dropped into Ashen Remnant droplet.
	// Odd numbers drop ceil(CurrentGold / 2) to ensure remaining is floor(CurrentGold / 2).
	OutGoldLost = (CurrentGold + 1) / 2;

	EPACurrencyTransactionError Error = EPACurrencyTransactionError::None;
	return DeductCurrency(EPACurrencyType::Gold, OutGoldLost, Error);
}

void UPACurrencyComponent::OnReplicatedCurrencyChanged(EPACurrencyType Type, int64 OldAmount, int64 NewAmount)
{
	OnCurrencyBalanceChanged.Broadcast(Type, NewAmount, NewAmount - OldAmount);
}

// -----------------------------------------------------------------------------
// Server RPCs
// -----------------------------------------------------------------------------

void UPACurrencyComponent::Server_TransferCurrency_Implementation(UPACurrencyComponent* TargetWallet, EPACurrencyType Type, int64 Amount)
{
	EPACurrencyTransactionError Error;
	TransferCurrencyTo(TargetWallet, Type, Amount, Error);
}

bool UPACurrencyComponent::Server_TransferCurrency_Validate(UPACurrencyComponent* TargetWallet, EPACurrencyType Type, int64 Amount)
{
	return TargetWallet != nullptr && TargetWallet != this && Amount > 0;
}
