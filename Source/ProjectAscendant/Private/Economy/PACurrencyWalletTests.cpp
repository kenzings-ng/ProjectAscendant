// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Economy/PACurrencyComponent.h"
#include "Economy/PACurrencyTypes.h"
#include "Character/PABaseCharacter.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPACurrencyWalletTests
 *
 * Automated unit and integration tests for Story econ-001 (Dual Currency Wallet & Transaction Engine):
 *  - AC-1: Dual Currency balances (Gold & Ash Shards), clamping to maximum limits, and FastArray delta serialization.
 *  - AC-2: Atomic transaction operations (Add, Deduct, Transfer) with anti-duping validation and insufficient funds guards.
 *  - AC-3: PvE Death Penalty dropping 50% Gold into an Ashen Remnant while preserving 100% Ash Shards.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPACurrencyWalletTests,
	"ProjectAscendant.Economy.CurrencyWallet",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPACurrencyWalletTests::RunTest(const FString& Parameters)
{
	// =========================================================================
	// AC-1: Dual Currency Balances & Limits
	// =========================================================================
	{
		UPACurrencyComponent* Wallet = NewObject<UPACurrencyComponent>();
		TestNotNull(TEXT("AC-1: UPACurrencyComponent instantiated successfully"), Wallet);

		if (Wallet)
		{
			// Default balances must be 0
			TestEqual(TEXT("AC-1: Default Gold balance is 0"), Wallet->GetGold(), 0LL);
			TestEqual(TEXT("AC-1: Default Ash Shards balance is 0"), Wallet->GetAshShards(), 0LL);

			// Add Gold within normal range
			EPACurrencyTransactionError ErrorCode = EPACurrencyTransactionError::None;
			bool bSuccess = Wallet->AddCurrency(EPACurrencyType::Gold, 1500, ErrorCode);
			TestTrue(TEXT("AC-1: AddCurrency 1500 Gold succeeds"), bSuccess);
			TestEqual(TEXT("AC-1: Gold balance is 1500"), Wallet->GetGold(), 1500LL);
			TestEqual(TEXT("AC-1: No error code on success"), ErrorCode, EPACurrencyTransactionError::None);

			// Add Ash Shards within normal range
			bSuccess = Wallet->AddCurrency(EPACurrencyType::AshShards, 42, ErrorCode);
			TestTrue(TEXT("AC-1: AddCurrency 42 Ash Shards succeeds"), bSuccess);
			TestEqual(TEXT("AC-1: Ash Shards balance is 42"), Wallet->GetAshShards(), 42LL);

			// Test Upper Limit Clamping: Gold max limit is 9,999,999
			Wallet->AddCurrency(EPACurrencyType::Gold, 50000000, ErrorCode);
			TestEqual(TEXT("AC-1: Gold clamped to MaxGold (9,999,999)"), Wallet->GetGold(), 9999999LL);

			// Test Upper Limit Clamping: Ash Shards max limit is 99,999
			Wallet->AddCurrency(EPACurrencyType::AshShards, 50000000, ErrorCode);
			TestEqual(TEXT("AC-1: Ash Shards clamped to MaxAshShards (99,999)"), Wallet->GetAshShards(), 99999LL);
		}
	}

	// =========================================================================
	// AC-2: Atomic Transaction Operations & Anti-Duping Safeguards
	// =========================================================================
	{
		UPACurrencyComponent* WalletA = NewObject<UPACurrencyComponent>();
		UPACurrencyComponent* WalletB = NewObject<UPACurrencyComponent>();
		TestNotNull(TEXT("AC-2: WalletA created"), WalletA);
		TestNotNull(TEXT("AC-2: WalletB created"), WalletB);

		if (WalletA && WalletB)
		{
			EPACurrencyTransactionError ErrorCode = EPACurrencyTransactionError::None;

			// Seed WalletA with 1,000 Gold
			WalletA->AddCurrency(EPACurrencyType::Gold, 1000, ErrorCode);

			// Deduct valid amount
			bool bDeductOk = WalletA->DeductCurrency(EPACurrencyType::Gold, 350, ErrorCode);
			TestTrue(TEXT("AC-2: Deduct 350 Gold succeeds"), bDeductOk);
			TestEqual(TEXT("AC-2: WalletA balance is 650 Gold"), WalletA->GetGold(), 650LL);

			// Attempt to deduct more than balance (Insufficient Funds)
			bDeductOk = WalletA->DeductCurrency(EPACurrencyType::Gold, 1000, ErrorCode);
			TestFalse(TEXT("AC-2: Deducting 1000 Gold fails due to insufficient funds"), bDeductOk);
			TestEqual(TEXT("AC-2: Error code is InsufficientFunds"), ErrorCode, EPACurrencyTransactionError::InsufficientFunds);
			TestEqual(TEXT("AC-2: WalletA balance unchanged at 650 Gold"), WalletA->GetGold(), 650LL);

			// Attempt invalid amount (zero or negative)
			bDeductOk = WalletA->DeductCurrency(EPACurrencyType::Gold, -50, ErrorCode);
			TestFalse(TEXT("AC-2: Deducting negative amount fails"), bDeductOk);
			TestEqual(TEXT("AC-2: Error code is InvalidAmount"), ErrorCode, EPACurrencyTransactionError::InvalidAmount);
			TestEqual(TEXT("AC-2: WalletA balance still 650 Gold"), WalletA->GetGold(), 650LL);

			// Atomic Transfer: WalletA (650) -> WalletB (0), amount 250
			bool bTransferOk = WalletA->TransferCurrencyTo(WalletB, EPACurrencyType::Gold, 250, ErrorCode);
			TestTrue(TEXT("AC-2: Transfer 250 Gold from WalletA to WalletB succeeds"), bTransferOk);
			TestEqual(TEXT("AC-2: WalletA balance is 400 Gold"), WalletA->GetGold(), 400LL);
			TestEqual(TEXT("AC-2: WalletB balance is 250 Gold"), WalletB->GetGold(), 250LL);

			// Atomic Transfer Failure: WalletA (400) -> WalletB (250), amount 500 (Insufficient Funds)
			bTransferOk = WalletA->TransferCurrencyTo(WalletB, EPACurrencyType::Gold, 500, ErrorCode);
			TestFalse(TEXT("AC-2: Transfer exceeding balance fails"), bTransferOk);
			TestEqual(TEXT("AC-2: Transfer error is InsufficientFunds"), ErrorCode, EPACurrencyTransactionError::InsufficientFunds);
			TestEqual(TEXT("AC-2: WalletA balance untouched at 400 Gold"), WalletA->GetGold(), 400LL);
			TestEqual(TEXT("AC-2: WalletB balance untouched at 250 Gold"), WalletB->GetGold(), 250LL);
		}
	}

	// =========================================================================
	// AC-3: PvE Death Penalty (50% Gold Loss, 100% Ash Shards Retained)
	// =========================================================================
	{
		UPACurrencyComponent* DeathWallet = NewObject<UPACurrencyComponent>();
		TestNotNull(TEXT("AC-3: DeathWallet created"), DeathWallet);

		if (DeathWallet)
		{
			EPACurrencyTransactionError ErrorCode = EPACurrencyTransactionError::None;
			DeathWallet->AddCurrency(EPACurrencyType::Gold, 1000, ErrorCode);
			DeathWallet->AddCurrency(EPACurrencyType::AshShards, 75, ErrorCode);

			int64 DroppedGold = 0;
			bool bDeathPenaltyApplied = DeathWallet->HandlePvEDeathPenalty(DroppedGold);
			TestTrue(TEXT("AC-3: PvE Death Penalty applied successfully"), bDeathPenaltyApplied);
			TestEqual(TEXT("AC-3: 50% of 1000 Gold dropped = 500 Gold"), DroppedGold, 500LL);
			TestEqual(TEXT("AC-3: Remaining Gold in wallet is 500"), DeathWallet->GetGold(), 500LL);
			TestEqual(TEXT("AC-3: Ash Shards 100% safeguarded at 75"), DeathWallet->GetAshShards(), 75LL);

			// Edge case: Odd number Gold (e.g. 501 Gold -> 50% floor is 250 remaining, 251 dropped)
			DeathWallet->AddCurrency(EPACurrencyType::Gold, 1, ErrorCode); // 501 Gold
			DeathWallet->HandlePvEDeathPenalty(DroppedGold);
			TestEqual(TEXT("AC-3: 501 Gold drops 251 Gold"), DroppedGold, 251LL);
			TestEqual(TEXT("AC-3: 250 Gold remaining"), DeathWallet->GetGold(), 250LL);

			// Edge case: 0 Gold -> 0 dropped
			DeathWallet->DeductCurrency(EPACurrencyType::Gold, 250, ErrorCode);
			DeathWallet->HandlePvEDeathPenalty(DroppedGold);
			TestEqual(TEXT("AC-3: 0 Gold drops 0"), DroppedGold, 0LL);
			TestEqual(TEXT("AC-3: 0 Gold remaining"), DeathWallet->GetGold(), 0LL);
		}
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
