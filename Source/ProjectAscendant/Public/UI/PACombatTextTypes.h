// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PACombatTextTypes.generated.h"

/**
 * EPACombatTextType
 *
 * Loại sát thương hiển thị dưới dạng số nổi (Floating Combat Text) theo GDD combat-hud.md:
 * - NormalDamage: Số trắng viền đen, scale 1.0x, bay vọt lên rồi rơi nhẹ.
 * - CriticalDamage: Số cam vàng rực lửa, scale 1.5x, nảy mạnh (Bounce).
 * - PostureDamage: Số vàng kim nhỏ hơn, bay thẳng vào thanh Posture mục tiêu.
 * - PerfectDodgeCallout: Chữ "PERFECT!" thư pháp mạ vàng bạc, 0.50s trên đầu nhân vật.
 */
UENUM(BlueprintType)
enum class EPACombatTextType : uint8
{
	NormalDamage          = 0 UMETA(DisplayName = "Normal Damage (White)"),
	CriticalDamage        = 1 UMETA(DisplayName = "Critical Damage (Orange 1.5x)"),
	PostureDamage         = 2 UMETA(DisplayName = "Posture Damage (Gold)"),
	PerfectDodgeCallout   = 3 UMETA(DisplayName = "Perfect Dodge (PERFECT!)")
};

/**
 * FPACombatTextConfig
 *
 * Tham số cân chỉnh Floating Combat Text theo GDD combat-hud.md §Tuning Knobs.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPACombatTextConfig
{
	GENERATED_BODY()

	/** Vận tốc ban đầu Z (cm/s) — số bay lên */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatText|Config")
	float InitialVelocityZ = 180.0f;

	/** Gia tốc trọng lực Z (cm/s²) — kéo số rơi xuống */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatText|Config")
	float GravityZ = -300.0f;

	/** Thời gian tồn tại mỗi số sát thương (giây) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatText|Config")
	float DefaultLifetime = 0.60f;

	/** Thời gian tồn tại chữ "PERFECT!" (giây) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatText|Config")
	float PerfectDodgeLifetime = 0.50f;

	/** Biên độ offset hướng tâm (px) để tránh chồng chéo khi Cleave */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatText|Config")
	float RadialOffsetRange = 25.0f;

	/** Scale cho sát thương Crit (1.5x) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatText|Config")
	float CriticalScale = 1.5f;

	/** Scale cho sát thương thường (1.0x) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatText|Config")
	float NormalScale = 1.0f;

	/** Scale cho sát thương Posture (0.85x — nhỏ hơn thường) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatText|Config")
	float PostureScale = 0.85f;

	/** Scale cho "PERFECT!" callout */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatText|Config")
	float PerfectDodgeScale = 1.2f;

	/** Số instance tối đa trong pool */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatText|Config")
	int32 MaxPoolSize = 50;
};

/**
 * FPACombatTextInstance
 *
 * Một instance số sát thương nổi đang hoạt động.
 * Bao gồm vị trí 3D, vận tốc đạn đạo, thời gian sống, và nội dung hiển thị.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPACombatTextInstance
{
	GENERATED_BODY()

	/** Vị trí 3D hiện tại trong World Space */
	UPROPERTY(BlueprintReadOnly, Category = "CombatText|Instance")
	FVector WorldPosition = FVector::ZeroVector;

	/** Vận tốc hiện tại (cm/s) */
	UPROPERTY(BlueprintReadOnly, Category = "CombatText|Instance")
	FVector Velocity = FVector::ZeroVector;

	/** Thời gian đã trôi qua kể từ khi spawn */
	UPROPERTY(BlueprintReadOnly, Category = "CombatText|Instance")
	float Elapsed = 0.0f;

	/** Thời gian tồn tại tối đa */
	UPROPERTY(BlueprintReadOnly, Category = "CombatText|Instance")
	float MaxLifetime = 0.60f;

	/** Hệ số scale hiển thị (1.0 = thường, 1.5 = crit) */
	UPROPERTY(BlueprintReadOnly, Category = "CombatText|Instance")
	float Scale = 1.0f;

	/** Độ mờ hiện tại (1.0 → 0.0 fade out) */
	UPROPERTY(BlueprintReadOnly, Category = "CombatText|Instance")
	float Opacity = 1.0f;

	/** Nội dung text hiển thị (VD: "1234", "PERFECT!") */
	UPROPERTY(BlueprintReadOnly, Category = "CombatText|Instance")
	FString Text;

	/** Loại sát thương */
	UPROPERTY(BlueprintReadOnly, Category = "CombatText|Instance")
	EPACombatTextType Type = EPACombatTextType::NormalDamage;

	/** Instance đang được sử dụng? (pool management) */
	UPROPERTY(BlueprintReadOnly, Category = "CombatText|Instance")
	bool bActive = false;

	/** Vị trí ban đầu khi spawn (dùng tính toán ballistic) */
	UPROPERTY(BlueprintReadOnly, Category = "CombatText|Instance")
	FVector SpawnPosition = FVector::ZeroVector;

	/** Vận tốc ban đầu khi spawn (dùng tính toán ballistic) */
	UPROPERTY(BlueprintReadOnly, Category = "CombatText|Instance")
	FVector InitialVelocity = FVector::ZeroVector;

	FPACombatTextInstance() = default;

	/** Kiểm tra đã hết thời gian sống? */
	bool IsExpired() const
	{
		return Elapsed >= MaxLifetime;
	}

	/** Tỷ lệ thời gian sống đã trôi qua (0.0 → 1.0) */
	float GetLifetimeRatio() const
	{
		return MaxLifetime > 0.0f ? FMath::Clamp(Elapsed / MaxLifetime, 0.0f, 1.0f) : 1.0f;
	}
};

/**
 * FPACombatTextPool
 *
 * Object Pool quản lý các FPACombatTextInstance.
 * Tái sử dụng instance đã hết hạn thay vì cấp phát mới → zero GC spikes.
 * Thuần data, cho phép kiểm thử tự động 100% không phụ thuộc UMG.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPACombatTextPool
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatText|Pool")
	FPACombatTextConfig Config;

	UPROPERTY(BlueprintReadOnly, Category = "CombatText|Pool")
	TArray<FPACombatTextInstance> Instances;

	/** Bộ đếm spawn để tạo radial offset tự động */
	int32 SpawnCounter = 0;

	FPACombatTextPool()
	{
		SpawnCounter = 0;
	}

	// ===========================================================
	// Spawning
	// ===========================================================

	/**
	 * Spawn một instance số sát thương tại vị trí 3D.
	 * Trả về index của instance trong pool, hoặc INDEX_NONE nếu pool đầy.
	 */
	int32 SpawnCombatText(const FVector& WorldLocation, float Amount, EPACombatTextType Type)
	{
		int32 Index = AcquireSlot();
		if (Index == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		FPACombatTextInstance& Inst = Instances[Index];
		Inst.bActive = true;
		Inst.Elapsed = 0.0f;
		Inst.Type = Type;

		// Nội dung text
		if (Type == EPACombatTextType::PerfectDodgeCallout)
		{
			Inst.Text = TEXT("PERFECT!");
			Inst.MaxLifetime = Config.PerfectDodgeLifetime;
			Inst.Scale = Config.PerfectDodgeScale;
		}
		else
		{
			Inst.Text = FString::Printf(TEXT("%d"), FMath::RoundToInt(FMath::Abs(Amount)));
			Inst.MaxLifetime = Config.DefaultLifetime;

			switch (Type)
			{
			case EPACombatTextType::CriticalDamage:
				Inst.Scale = Config.CriticalScale;
				break;
			case EPACombatTextType::PostureDamage:
				Inst.Scale = Config.PostureScale;
				break;
			default:
				Inst.Scale = Config.NormalScale;
				break;
			}
		}

		Inst.Opacity = 1.0f;

		// Vận tốc ban đầu với radial offset để tránh chồng chéo
		const float RadialAngle = ComputeRadialAngle();
		const float OffsetX = Config.RadialOffsetRange * FMath::Cos(RadialAngle);
		const float OffsetY = Config.RadialOffsetRange * FMath::Sin(RadialAngle);

		Inst.SpawnPosition = WorldLocation;
		Inst.WorldPosition = WorldLocation;
		Inst.InitialVelocity = FVector(OffsetX, OffsetY, Config.InitialVelocityZ);
		Inst.Velocity = Inst.InitialVelocity;

		SpawnCounter++;

		return Index;
	}

	/**
	 * Spawn chữ "PERFECT!" trên đầu nhân vật.
	 */
	int32 SpawnPerfectDodgeCallout(const FVector& CharacterLocation)
	{
		// Offset lên trên đầu nhân vật (60 cm trên vị trí gốc)
		const FVector SpawnPos = CharacterLocation + FVector(0.0f, 0.0f, 60.0f);
		return SpawnCombatText(SpawnPos, 0.0f, EPACombatTextType::PerfectDodgeCallout);
	}

	// ===========================================================
	// Tick Update
	// ===========================================================

	/**
	 * Cập nhật tất cả instance đang hoạt động.
	 * Quỹ đạo đạn đạo: P(t) = P0 + V0*t + 0.5*g*t²
	 * Opacity fade out tuyến tính theo lifetime.
	 */
	void Update(float DeltaTime)
	{
		const FVector Gravity(0.0f, 0.0f, Config.GravityZ);

		for (FPACombatTextInstance& Inst : Instances)
		{
			if (!Inst.bActive)
			{
				continue;
			}

			Inst.Elapsed += DeltaTime;

			if (Inst.IsExpired())
			{
				Inst.bActive = false;
				Inst.Opacity = 0.0f;
				continue;
			}

			// Quỹ đạo đạn đạo chính xác: P(t) = P0 + V0*t + 0.5*g*t²
			const float t = Inst.Elapsed;
			Inst.WorldPosition = Inst.SpawnPosition
				+ Inst.InitialVelocity * t
				+ 0.5f * Gravity * t * t;

			// Cập nhật vận tốc cho reference: V(t) = V0 + g*t
			Inst.Velocity = Inst.InitialVelocity + Gravity * t;

			// Fade out tuyến tính: opacity giảm từ 1.0 → 0.0 theo lifetime
			Inst.Opacity = 1.0f - Inst.GetLifetimeRatio();
		}
	}

	// ===========================================================
	// Query
	// ===========================================================

	/** Số instance đang hoạt động */
	int32 GetActiveCount() const
	{
		int32 Count = 0;
		for (const FPACombatTextInstance& Inst : Instances)
		{
			if (Inst.bActive)
			{
				Count++;
			}
		}
		return Count;
	}

	/** Lấy danh sách instance đang hoạt động (cho UMG renderer) */
	void GetActiveInstances(TArray<const FPACombatTextInstance*>& OutActive) const
	{
		OutActive.Reset();
		for (const FPACombatTextInstance& Inst : Instances)
		{
			if (Inst.bActive)
			{
				OutActive.Add(&Inst);
			}
		}
	}

	/** Tổng số slot trong pool (bao gồm cả inactive) */
	int32 GetPoolSize() const
	{
		return Instances.Num();
	}

private:
	/**
	 * Tìm slot trống trong pool hoặc tạo mới nếu chưa đầy.
	 * Ưu tiên tái sử dụng slot inactive.
	 */
	int32 AcquireSlot()
	{
		// Ưu tiên tái sử dụng slot đã hết hạn
		for (int32 i = 0; i < Instances.Num(); ++i)
		{
			if (!Instances[i].bActive)
			{
				return i;
			}
		}

		// Pool chưa đầy → cấp phát slot mới
		if (Instances.Num() < Config.MaxPoolSize)
		{
			const int32 NewIndex = Instances.AddDefaulted();
			return NewIndex;
		}

		// Pool đầy: recycle instance cũ nhất (Elapsed cao nhất)
		int32 OldestIndex = 0;
		float MaxElapsed = -1.0f;
		for (int32 i = 0; i < Instances.Num(); ++i)
		{
			if (Instances[i].Elapsed > MaxElapsed)
			{
				MaxElapsed = Instances[i].Elapsed;
				OldestIndex = i;
			}
		}
		return OldestIndex;
	}

	/**
	 * Tính góc offset hướng tâm dựa trên SpawnCounter.
	 * Phân bố đều theo hình cánh quạt để tránh chồng chéo.
	 */
	float ComputeRadialAngle() const
	{
		// Golden angle distribution cho phân tán đều nhất
		constexpr float GoldenAngle = 2.399963f; // PI * (3 - sqrt(5))
		return SpawnCounter * GoldenAngle;
	}
};
