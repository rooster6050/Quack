// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "QuackCharacter.generated.h"

class UInputComponent;

USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		bool bIsUsingGravityGun;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		bool bHasGravityGun;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		bool bMouseDown;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		bool bMouseUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		int MaxClipSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		int Ammo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		int ReservedAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		float FireRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		float FireCooldown;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		float InitialFireCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		float GunDamage;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditAnywhere, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		TArray<TSubclassOf<class AQuackProjectile>> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UCameraShake> CameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		UForceFeedbackEffect* Feedback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		class UParticleSystem* FireEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		class UParticleSystem* HitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		class UParticleSystem* FireTrailEffect;

	UPROPERTY()
		UParticleSystemComponent* MuzzleFlashParticleSystem;

	float RunningTime;

	FTimerHandle ContinuousShooting;

	FTimerHandle FireRateDelay;
	/** defaults */
	FWeaponData()
	{
		bIsUsingGravityGun = false;
		bHasGravityGun = false;
		bMouseDown = false;
		bMouseUp = true;
		MaxClipSize = 30;
		Ammo = 30;
		ReservedAmmo = 60;
		FireRate = 0.15f;
		FireCooldown = 0.0f;
		InitialFireCooldown = 0.0f;
		GunDamage = 15.0f;
		RunningTime = 0.0f;
	}
};

USTRUCT(BlueprintType)
struct FPoisonData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		bool bHasNeedle;

	// Used in boss class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		bool bIsPoisoning;

	// Used in pipe class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		bool bCanPoisonPipe;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		float PoisonCD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		int PoisonVialsCount;

	FTimerHandle PoisonUsageDelay;

	/** defaults */
	FPoisonData()
	{
		bHasNeedle = false;
		bIsPoisoning = false;
		bCanPoisonPipe = false;
		PoisonCD = 2.5f;
		PoisonVialsCount = 4;
	}
};

USTRUCT(BlueprintType)
struct FMovementData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		float SprintSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		float InitialWalkSpeed;

	UPROPERTY(EditAnywhere, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		uint32 bIsSprinting : 1;

	FMovementData()
	{
		SprintSpeed = 1500.0f;
		InitialWalkSpeed = 1000.0f;
		bIsSprinting = false;
	}
};

USTRUCT(BlueprintType)
struct FPlayerData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		float Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		float MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		float RayLength;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom", meta = (AllowPrivateAccess = "true"))
		class APipe* CurrentPipe;

	UPROPERTY(VisibleAnywhere, Category = "Custom")
		class AQuackBoss* Boss;

	FPlayerData()
	{
		Health = 30.0f;
		MaxHealth = 30.0f;
		RayLength = 5000.0f;
	}
};

UCLASS(config=Game)
class AQuackCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "Volume")
		float GunShootVolume = 0.5f;
	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
		class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USkeletalMeshComponent* FP_Gun;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(EditAnywhere, Category = "Custom Niall")
		class USkeletalMesh* FP_RegularGun;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(EditAnywhere, Category = "Custom Niall")
		class USkeletalMesh* FP_GravGun;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class UStaticMeshComponent* HarryAssaultRifle;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class UStaticMeshComponent* HarryGravGun;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USkeletalMeshComponent* KrisGravityGun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USceneComponent* FP_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY()
		class UPhysicsHandleComponent* PhysicsHandle;

	UPROPERTY(VisibleAnywhere, Category = "Interact")
		AActor* InteractableActor;

	UPROPERTY(VisibleAnywhere, Category = "Interact")
		int InteractID = -1;

	UPROPERTY(EditAnywhere, Category = "Tutorials")
		class ATutorialManager* TutorialManager;

	void CheckForMaxClip();

	UFUNCTION()
		void Interact();

	UFUNCTION()
		void SwapGun();

	// Grabbing and Throwing
	UFUNCTION()
		void Grab();

	UFUNCTION()
		void OnSprintBegin();
	
	UFUNCTION()
		void OnSprintEnd();

	void Sprint();

	UPROPERTY()
		UPrimitiveComponent* HitComponent;

	UFUNCTION()
		void MouseDown();

	UFUNCTION()
		void MouseUp();

	void SemiAutomaticShooting(float DeltaTime);

	void PoisonCooldown();

	UFUNCTION()
		void UsePoison();

	UFUNCTION()
		void Reload();

	UFUNCTION()
		void OnTriggerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherbodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnTriggerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherbodyIndex);

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float BaseLookUpRate;

	void StopMuzzleFlash();
public:
	AQuackCharacter();

	virtual void BeginPlay();

	virtual void Tick(float DeltaTime) override;

	void DecreaseAmmo(int Amount);

	void IncreaseAmmo(int Amount);

	void IncreaseHealth(float Amount);
	void IncreaseHealth(float Amount, float DeltaTime);

	void DecreaseHealth(float Amount);

	FORCEINLINE float GetCurrentHealth() const { return PlayerConfig.Health; }

	FORCEINLINE float GetMaxHealth() const { return PlayerConfig.MaxHealth; }

	UPROPERTY()
		class UGrabAndThrowComponent* GrabberClass;

	// Used in boss class
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BossHP;

	/** poison data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom")
		FPoisonData PoisonConfig = FPoisonData();

protected:

	void InitialiseGrabberClass();

	/** weapon data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom")
		FWeaponData WeaponConfig = FWeaponData();

	/** movement data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom")
		FMovementData MovementConfig = FMovementData();

	/** movement data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom")
		FPlayerData PlayerConfig = FPlayerData();

	void Raycast();

	/** Fires a projectile. */
	void OnFire();

	void OnFire2();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

