// Copyright Project Ascendant. All Rights Reserved.

#include "Account/PAAccountSubsystem.h"
#include "Misc/Guid.h"

void UPAAccountSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentState = EPAAuthLoginState::LoggedOut;
	bHasSavedSession = false;
	DevTesterCounter = 1;

	// Khởi tạo sẵn một tài khoản thử nghiệm mặc định để nhà phát triển có thể đăng nhập test ngay
	const FString DefaultEmail = TEXT("tester@ascendant.com");
	const FString DefaultPassword = TEXT("ascendant123");
	const FString DefaultAccountId = TEXT("PA-ACC-0001-DEFAULT");
	const FString DefaultDisplayName = TEXT("AscendantKnight");
	const FString DefaultToken = GenerateAuthToken(DefaultAccountId);

	RegisteredCredentials.Add(DefaultEmail, DefaultPassword);
	RegisteredProfiles.Add(DefaultEmail, FPAAccountProfile(DefaultAccountId, DefaultEmail, DefaultDisplayName, DefaultToken, false));
}

void UPAAccountSubsystem::Deinitialize()
{
	Logout();
	RegisteredCredentials.Empty();
	RegisteredProfiles.Empty();

	Super::Deinitialize();
}

void UPAAccountSubsystem::SetLoginState(EPAAuthLoginState NewState, const FString& ErrorMessage)
{
	CurrentState = NewState;
	OnLoginStateChanged.Broadcast(NewState, ErrorMessage);
}

bool UPAAccountSubsystem::RegisterAccount(
	const FString& InEmail,
	const FString& InPassword,
	const FString& InConfirmPassword,
	FString& OutError)
{
	SetLoginState(EPAAuthLoginState::Authenticating);

	// 1. Thẩm tra định dạng Email
	if (!IsValidEmail(InEmail))
	{
		OutError = TEXT("Định dạng email không hợp lệ. Vui lòng nhập đúng cấu trúc (ví dụ: player@example.com).");
		SetLoginState(EPAAuthLoginState::Error, OutError);
		return false;
	}

	// 2. Thẩm tra độ mạnh mật khẩu
	if (!IsValidPassword(InPassword, OutError))
	{
		SetLoginState(EPAAuthLoginState::Error, OutError);
		return false;
	}

	// 3. Thẩm tra khớp mật khẩu xác nhận
	if (!InPassword.Equals(InConfirmPassword, ESearchCase::CaseSensitive))
	{
		OutError = TEXT("Mật khẩu xác nhận không khớp với mật khẩu đã nhập.");
		SetLoginState(EPAAuthLoginState::Error, OutError);
		return false;
	}

	// 4. Kiểm tra tài khoản đã tồn tại chưa
	if (RegisteredCredentials.Contains(InEmail))
	{
		OutError = TEXT("Tài khoản với email này đã tồn tại trong hệ thống.");
		SetLoginState(EPAAuthLoginState::Error, OutError);
		return false;
	}

	// 5. Đăng ký thành công và tạo Profile
	const FString NewAccountId = FString::Printf(TEXT("PA-ACC-%s"), *FGuid::NewGuid().ToString(EGuidFormats::Short));
	
	// Trích xuất tên hiển thị ban đầu từ phần trước dấu @ của email
	FString AutoDisplayName = InEmail;
	int32 AtIndex = INDEX_NONE;
	if (InEmail.FindChar(TEXT('@'), AtIndex))
	{
		AutoDisplayName = InEmail.Left(AtIndex);
	}

	const FString NewAuthToken = GenerateAuthToken(NewAccountId);
	FPAAccountProfile NewProfile(NewAccountId, InEmail, AutoDisplayName, NewAuthToken, false);

	RegisteredCredentials.Add(InEmail, InPassword);
	RegisteredProfiles.Add(InEmail, NewProfile);

	SetLoginState(EPAAuthLoginState::LoggedOut);
	OutError = TEXT("");
	return true;
}

bool UPAAccountSubsystem::LoginWithCredentials(
	const FString& InEmail,
	const FString& InPassword,
	bool bRememberMe,
	FString& OutError)
{
	SetLoginState(EPAAuthLoginState::Authenticating);

	if (!IsValidEmail(InEmail))
	{
		OutError = TEXT("Email không hợp lệ.");
		SetLoginState(EPAAuthLoginState::Error, OutError);
		return false;
	}

	// 1. Kiểm tra tài khoản có tồn tại không
	if (!RegisteredCredentials.Contains(InEmail))
	{
		OutError = TEXT("Tài khoản không tồn tại. Vui lòng đăng ký tài khoản mới.");
		SetLoginState(EPAAuthLoginState::Error, OutError);
		return false;
	}

	// 2. Kiểm tra mật khẩu
	const FString StoredPassword = RegisteredCredentials[InEmail];
	if (!StoredPassword.Equals(InPassword, ESearchCase::CaseSensitive))
	{
		OutError = TEXT("Mật khẩu không chính xác. Vui lòng thử lại.");
		SetLoginState(EPAAuthLoginState::Error, OutError);
		return false;
	}

	// 3. Đăng nhập thành công -> Cập nhật Token mới và ActiveProfile
	FPAAccountProfile& Profile = RegisteredProfiles[InEmail];
	Profile.AuthToken = GenerateAuthToken(Profile.AccountID);
	ActiveProfile = Profile;

	if (bRememberMe)
	{
		SavedSessionProfile = ActiveProfile;
		bHasSavedSession = true;
	}

	SetLoginState(EPAAuthLoginState::LoggedIn);
	OnLoginSuccess.Broadcast(ActiveProfile);
	OutError = TEXT("");
	return true;
}

bool UPAAccountSubsystem::LoginFastPlaytest(const FString& InDevDisplayName)
{
	SetLoginState(EPAAuthLoginState::Authenticating);

	FString DisplayName = InDevDisplayName.TrimStartAndEnd();
	if (DisplayName.IsEmpty())
	{
		DisplayName = FString::Printf(TEXT("Tester_%d"), DevTesterCounter++);
	}

	const FString DevAccountId = FString::Printf(TEXT("PA-DEV-%04d"), FMath::RandRange(1000, 9999));
	const FString DevToken = GenerateAuthToken(DevAccountId);

	ActiveProfile = FPAAccountProfile(DevAccountId, TEXT("dev@playtest.local"), DisplayName, DevToken, true);

	SetLoginState(EPAAuthLoginState::LoggedIn);
	OnLoginSuccess.Broadcast(ActiveProfile);
	return true;
}

void UPAAccountSubsystem::Logout()
{
	ActiveProfile = FPAAccountProfile();
	SetLoginState(EPAAuthLoginState::LoggedOut);
}

bool UPAAccountSubsystem::AutoLoginWithSavedSession()
{
	if (!bHasSavedSession || SavedSessionProfile.AccountID.IsEmpty())
	{
		return false;
	}

	SetLoginState(EPAAuthLoginState::Authenticating);

	// Làm mới Token cho phiên đăng nhập tự động
	SavedSessionProfile.AuthToken = GenerateAuthToken(SavedSessionProfile.AccountID);
	ActiveProfile = SavedSessionProfile;

	SetLoginState(EPAAuthLoginState::LoggedIn);
	OnLoginSuccess.Broadcast(ActiveProfile);
	return true;
}

void UPAAccountSubsystem::ClearSavedSession()
{
	bHasSavedSession = false;
	SavedSessionProfile = FPAAccountProfile();
}

bool UPAAccountSubsystem::IsValidEmail(const FString& InEmail)
{
	const FString Trimmed = InEmail.TrimStartAndEnd();
	if (Trimmed.Len() < 5)
	{
		return false;
	}

	// Kiểm tra không chứa khoảng trắng
	if (Trimmed.Contains(TEXT(" ")))
	{
		return false;
	}

	int32 AtIndex = INDEX_NONE;
	if (!Trimmed.FindChar(TEXT('@'), AtIndex))
	{
		return false;
	}

	// Ký tự @ không được nằm ở đầu hoặc cuối
	if (AtIndex == 0 || AtIndex == Trimmed.Len() - 1)
	{
		return false;
	}

	// Sau ký tự @ phải có ít nhất 1 dấu chấm cho domain
	const FString DomainPart = Trimmed.Mid(AtIndex + 1);
	int32 DotIndex = INDEX_NONE;
	if (!DomainPart.FindChar(TEXT('.'), DotIndex))
	{
		return false;
	}

	// Dấu chấm không được nằm ngay sau @ hoặc ở cuối cùng
	if (DotIndex == 0 || DotIndex == DomainPart.Len() - 1)
	{
		return false;
	}

	return true;
}

bool UPAAccountSubsystem::IsValidPassword(const FString& InPassword, FString& OutRequirementError)
{
	constexpr int32 kMinPasswordLength = 6;
	if (InPassword.Len() < kMinPasswordLength)
	{
		OutRequirementError = FString::Printf(TEXT("Mật khẩu phải có độ dài tối thiểu %d ký tự."), kMinPasswordLength);
		return false;
	}

	OutRequirementError = TEXT("");
	return true;
}

FString UPAAccountSubsystem::GenerateAuthToken(const FString& AccountId)
{
	const int64 UnixTimestamp = FDateTime::UtcNow().ToUnixTimestamp();
	const FString RandomSecret = FGuid::NewGuid().ToString(EGuidFormats::Short);
	return FString::Printf(TEXT("PA-TOKEN-%s-%lld-%s"), *AccountId, UnixTimestamp, *RandomSecret);
}
