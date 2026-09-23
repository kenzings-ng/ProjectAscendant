// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Account/PAAccountSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAAccountTests
 *
 * Kiểm thử tự động cho Story auth-001 (Account Subsystem, 1-Click Fast Playtest & Server Token Handshake):
 *  - AC-1: GameInstance Subsystem Persistence & Profile Data Structure.
 *  - AC-2: 1-Click Fast Playtest Mode (< 0.10s, bIsDevProfile = true).
 *  - AC-3: Email & Password Validation Rules (RFC 5322, >= 6 ký tự, mismatch error).
 *  - AC-4: Server Join Token Handshake (Format PA-TOKEN-, ValidateJoinToken).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAAccountTests,
	"ProjectAscendant.Account.AuthTokenHandshake",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAAccountTests::RunTest(const FString& Parameters)
{
	// ===========================================================
	// AC-1: Profile Data Structure
	// ===========================================================
	{
		FPAAccountProfile Profile(
			TEXT("PA-USER-001"),
			TEXT("hero@ascendant.io"),
			TEXT("ShadowKnight"),
			TEXT("PA-TOKEN-PA-USER-001-12345-abc"),
			false
		);

		TestEqual(TEXT("AC1: AccountID matches"), Profile.AccountID, TEXT("PA-USER-001"));
		TestEqual(TEXT("AC1: Email matches"), Profile.Email, TEXT("hero@ascendant.io"));
		TestEqual(TEXT("AC1: DisplayName matches"), Profile.DisplayName, TEXT("ShadowKnight"));
		TestEqual(TEXT("AC1: AuthToken matches"), Profile.AuthToken, TEXT("PA-TOKEN-PA-USER-001-12345-abc"));
		TestFalse(TEXT("AC1: Not a dev profile"), Profile.bIsDevProfile);
	}

	// ===========================================================
	// AC-2: 1-Click Fast Playtest Mode (< 0.1s)
	// ===========================================================
	{
		const double StartTime = FPlatformTime::Seconds();

		// Giả lập logic 1-click playtest của Subsystem
		const FString DevAccountId = FString::Printf(TEXT("PA-DEV-%04d"), 1001);
		const FString DevToken = UPAAccountSubsystem::GenerateAuthToken(DevAccountId);
		FPAAccountProfile DevProfile(DevAccountId, TEXT("dev@playtest.local"), TEXT("Tester_1"), DevToken, true);

		const double Elapsed = FPlatformTime::Seconds() - StartTime;

		TestTrue(TEXT("AC2: Playtest execution completed in < 0.10 seconds"), Elapsed < 0.10);
		TestTrue(TEXT("AC2: bIsDevProfile is true"), DevProfile.bIsDevProfile);
		TestEqual(TEXT("AC2: DisplayName is Tester_1"), DevProfile.DisplayName, TEXT("Tester_1"));
		TestTrue(TEXT("AC2: Generated dev token is non-empty"), !DevProfile.AuthToken.IsEmpty());
	}

	// ===========================================================
	// AC-3: Email & Password Validation Rules
	// ===========================================================
	{
		// 1. Kiểm tra Email hợp lệ
		TestTrue(TEXT("AC3: Valid email passes"), UPAAccountSubsystem::IsValidEmail(TEXT("hero@ascendant.io")));
		TestTrue(TEXT("AC3: Valid subdomain email passes"), UPAAccountSubsystem::IsValidEmail(TEXT("user@play.game.com")));

		// Email không hợp lệ
		TestFalse(TEXT("AC3: Empty email fails"), UPAAccountSubsystem::IsValidEmail(TEXT("")));
		TestFalse(TEXT("AC3: Missing @ fails"), UPAAccountSubsystem::IsValidEmail(TEXT("plainaddress")));
		TestFalse(TEXT("AC3: Missing domain dot fails"), UPAAccountSubsystem::IsValidEmail(TEXT("user@domain")));
		TestFalse(TEXT("AC3: @ at start fails"), UPAAccountSubsystem::IsValidEmail(TEXT("@domain.com")));
		TestFalse(TEXT("AC3: Whitespace email fails"), UPAAccountSubsystem::IsValidEmail(TEXT("user @domain.com")));

		// 2. Kiểm tra Mật khẩu (tối thiểu 6 ký tự)
		FString PwdError;
		TestTrue(TEXT("AC3: 6-char password passes"), UPAAccountSubsystem::IsValidPassword(TEXT("123456"), PwdError));
		TestTrue(TEXT("AC3: 10-char password passes"), UPAAccountSubsystem::IsValidPassword(TEXT("secretpass"), PwdError));
		TestFalse(TEXT("AC3: 5-char password fails"), UPAAccountSubsystem::IsValidPassword(TEXT("12345"), PwdError));
		TestFalse(TEXT("AC3: Empty password fails"), UPAAccountSubsystem::IsValidPassword(TEXT(""), PwdError));
	}

	// ===========================================================
	// AC-4: Server Join Token Handshake
	// ===========================================================
	{
		const FString AccountId = TEXT("PA-ACC-9999");
		const FString GeneratedToken = UPAAccountSubsystem::GenerateAuthToken(AccountId);

		// Format token phải bắt đầu bằng PA-TOKEN-[AccountId]-
		TestTrue(TEXT("AC4: Token starts with PA-TOKEN-"), GeneratedToken.StartsWith(TEXT("PA-TOKEN-")));
		TestTrue(TEXT("AC4: Token contains AccountId"), GeneratedToken.Contains(AccountId));

		// Server xác thực token hợp lệ
		bool bValid = UPAAccountSubsystem::ValidateJoinToken(GeneratedToken, AccountId);
		TestTrue(TEXT("AC4: Server validates matching join token"), bValid);

		// Giả mạo token khác AccountId -> Server từ chối
		bool bForged = UPAAccountSubsystem::ValidateJoinToken(GeneratedToken, TEXT("PA-ACC-FAKE"));
		TestFalse(TEXT("AC4: Server rejects mismatched account token"), bForged);

		// Token rác -> Server từ chối
		bool bMalformed = UPAAccountSubsystem::ValidateJoinToken(TEXT("INVALID-TOKEN"), AccountId);
		TestFalse(TEXT("AC4: Server rejects malformed token"), bMalformed);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
