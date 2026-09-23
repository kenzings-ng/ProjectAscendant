// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PAAccountSubsystem.generated.h"

/**
 * EPAAuthLoginState
 *
 * Trạng thái phiên xác thực người chơi trong Game Instance.
 */
UENUM(BlueprintType)
enum class EPAAuthLoginState : uint8
{
	LoggedOut       UMETA(DisplayName = "Chưa Đăng Nhập"),
	Authenticating  UMETA(DisplayName = "Đang Kết Nối / Xác Thực"),
	LoggedIn        UMETA(DisplayName = "Đã Đăng Nhập"),
	Error           UMETA(DisplayName = "Lỗi Xác Thực")
};

/**
 * EPAAuthErrorCode
 *
 * Mã lỗi chi tiết khi đăng nhập hoặc đăng ký tài khoản.
 */
UENUM(BlueprintType)
enum class EPAAuthErrorCode : uint8
{
	None                    UMETA(DisplayName = "Không Có Lỗi"),
	InvalidEmail            UMETA(DisplayName = "Email Không Hợp Lệ"),
	PasswordTooShort        UMETA(DisplayName = "Mật Khẩu Quá Ngắn"),
	PasswordMismatch        UMETA(DisplayName = "Mật Khẩu Xác Nhận Không Khớp"),
	UserNotFound            UMETA(DisplayName = "Không Tìm Thấy Tài Khoản"),
	IncorrectPassword       UMETA(DisplayName = "Sai Mật Khẩu"),
	AccountAlreadyExists    UMETA(DisplayName = "Tài Khoản Đã Tồn Tại"),
	UnknownError            UMETA(DisplayName = "Lỗi Không Xác Định")
};

/**
 * FPAAccountProfile
 *
 * Hồ sơ định danh của người chơi sau khi đăng nhập thành công.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAAccountProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|Account")
	FString AccountID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|Account")
	FString Email;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|Account")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|Account")
	FString AuthToken;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|Account")
	bool bIsDevProfile = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|Account")
	FDateTime CreatedAt = FDateTime::UtcNow();

	FPAAccountProfile() = default;

	FPAAccountProfile(const FString& InId, const FString& InEmail, const FString& InName, const FString& InToken, bool bInDev = false)
		: AccountID(InId)
		, Email(InEmail)
		, DisplayName(InName)
		, AuthToken(InToken)
		, bIsDevProfile(bInDev)
		, CreatedAt(FDateTime::UtcNow())
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnLoginStateChanged, EPAAuthLoginState, NewState, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnLoginSuccess, const FPAAccountProfile&, Profile);

/**
 * UPAAccountSubsystem
 *
 * GameInstance Subsystem quản lý định danh người chơi, đăng nhập, đăng ký,
 * lưu phiên làm việc (Remember Me) và chế độ Fast Playtest phục vụ thử nghiệm nội bộ.
 */
UCLASS()
class PROJECTASCENDANT_API UPAAccountSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Đăng ký tài khoản mới bằng Email và Mật khẩu.
	 * Yêu cầu: Email hợp lệ, Mật khẩu >= 6 ký tự và trùng với Xác nhận mật khẩu.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ascendant|Account")
	bool RegisterAccount(const FString& InEmail, const FString& InPassword, const FString& InConfirmPassword, FString& OutError);

	/**
	 * Đăng nhập bằng Email và Mật khẩu.
	 * Nếu bRememberMe = true, lưu phiên để tự động đăng nhập ở lần mở game sau.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ascendant|Account")
	bool LoginWithCredentials(const FString& InEmail, const FString& InPassword, bool bRememberMe, FString& OutError);

	/**
	 * Chế độ 1-Click Fast Playtest:
	 * Tự động tạo profile nhanh với tên InDevDisplayName (hoặc tạo ngẫu nhiên Tester_N).
	 * Giúp mở nhiều client trên cùng một máy để test coop/combat ngay tức thì.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ascendant|Account")
	bool LoginFastPlaytest(const FString& InDevDisplayName);

	/**
	 * Đăng xuất và dọn dẹp profile hiện tại.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ascendant|Account")
	void Logout();

	/**
	 * Kiểm tra xem có phiên đăng nhập đã được lưu (Remember Me) hay không.
	 */
	UFUNCTION(BlueprintPure, Category = "Ascendant|Account")
	bool HasSavedSession() const { return bHasSavedSession; }

	/**
	 * Tự động đăng nhập bằng phiên đã lưu.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ascendant|Account")
	bool AutoLoginWithSavedSession();

	/**
	 * Xóa sạch phiên đăng nhập đã lưu.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ascendant|Account")
	void ClearSavedSession();

	/** Lấy thông tin hồ sơ tài khoản đang hoạt động */
	UFUNCTION(BlueprintPure, Category = "Ascendant|Account")
	const FPAAccountProfile& GetActiveProfile() const { return ActiveProfile; }

	/** Trạng thái đăng nhập hiện tại */
	UFUNCTION(BlueprintPure, Category = "Ascendant|Account")
	EPAAuthLoginState GetLoginState() const { return CurrentState; }

	/** Kiểm tra xem người chơi đã đăng nhập thành công hay chưa */
	UFUNCTION(BlueprintPure, Category = "Ascendant|Account")
	bool IsLoggedIn() const { return CurrentState == EPAAuthLoginState::LoggedIn; }

	/** Thiết lập chức nghiệp được chọn (ví dụ: Class.Vanguard, Class.Ranger, Class.Arcanist) */
	UFUNCTION(BlueprintCallable, Category = "Ascendant|Account")
	void SetSelectedCharacterClass(FName InClass) { SelectedCharacterClass = InClass; }

	/** Lấy chức nghiệp được chọn */
	UFUNCTION(BlueprintPure, Category = "Ascendant|Account")
	FName GetSelectedCharacterClass() const { return SelectedCharacterClass; }

	// --- Thẩm Tra Dữ Liệu (Static Helpers) ---

	/** Kiểm tra cú pháp Email có hợp lệ không (phải chứa @ và domain) */
	UFUNCTION(BlueprintPure, Category = "Ascendant|Account|Validation")
	static bool IsValidEmail(const FString& InEmail);

	/** Kiểm tra độ dài mật khẩu tối thiểu (>= 6 ký tự) */
	UFUNCTION(BlueprintPure, Category = "Ascendant|Account|Validation")
	static bool IsValidPassword(const FString& InPassword, FString& OutRequirementError);

	/** Sinh Session Auth Token ngẫu nhiên chuẩn hóa */
	UFUNCTION(BlueprintPure, Category = "Ascendant|Account|Validation")
	static FString GenerateAuthToken(const FString& AccountId);

public:
	/** Sự kiện phát đi khi trạng thái đăng nhập thay đổi */
	UPROPERTY(BlueprintAssignable, Category = "Ascendant|Account|Events")
	FPAOnLoginStateChanged OnLoginStateChanged;

	/** Sự kiện phát đi khi đăng nhập thành công */
	UPROPERTY(BlueprintAssignable, Category = "Ascendant|Account|Events")
	FPAOnLoginSuccess OnLoginSuccess;

private:
	void SetLoginState(EPAAuthLoginState NewState, const FString& ErrorMessage = TEXT(""));

private:
	/** Trạng thái xác thực hiện tại */
	UPROPERTY(Transient)
	EPAAuthLoginState CurrentState = EPAAuthLoginState::LoggedOut;

	/** Hồ sơ tài khoản đang hoạt động */
	UPROPERTY(Transient)
	FPAAccountProfile ActiveProfile;

	/** Hồ sơ phiên đã lưu (Remember Me) */
	UPROPERTY(Transient)
	FPAAccountProfile SavedSessionProfile;

	UPROPERTY(Transient)
	bool bHasSavedSession = false;

	/** Chức nghiệp nhân vật đang được chọn */
	UPROPERTY(Transient)
	FName SelectedCharacterClass = FName(TEXT("Class.Vanguard"));

	/** Bộ nhớ lưu trữ giả lập các tài khoản đã đăng ký (Email -> Password) */
	TMap<FString, FString> RegisteredCredentials;

	/** Bộ nhớ lưu trữ hồ sơ tài khoản (Email -> Profile) */
	TMap<FString, FPAAccountProfile> RegisteredProfiles;

	/** Đếm số lượng Dev Tester profile đã tạo */
	int32 DevTesterCounter = 1;
};
