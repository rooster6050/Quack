// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Headers/Quack.h"
#include "Headers/Character/QuackCharacter.h"
#include "Headers/Misc/QuackProjectile.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "MotionControllerComponent.h"
#include "Headers/Misc/Pipe.h"
#include "Headers/Boss/QuackBoss.h"
#include "Headers/AIEnemies/QuackAIPawn.h"
#include "Headers/Misc/Rock.h"
#include "Headers/CustomComponents/GrabAndThrowComponent.h"
#include "Headers/Stations/HealthStation.h"
#include "Headers/CustomComponents/InteractComponent.h"
#include "Headers/Managers/TutorialManager.h"
#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AQuackCharacter

// AWAKE
AQuackCharacter::AQuackCharacter()
{

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	HarryAssaultRifle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AssaultRifle"));
	HarryAssaultRifle->bCastDynamicShadow = false;
	HarryAssaultRifle->CastShadow = false;
	HarryAssaultRifle->SetupAttachment(FP_Gun);

	KrisGravityGun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GravityGun"));
	KrisGravityGun->bCastDynamicShadow = false;
	KrisGravityGun->CastShadow = false;
	KrisGravityGun->SetupAttachment(FP_Gun);

	HarryGravGun = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HarryGravGun"));
	HarryGravGun->bCastDynamicShadow = false;
	HarryGravGun->CastShadow = false;
	HarryGravGun->SetupAttachment(FP_Gun);
	

	// Default offset from the character location for projectiles to spawn
	WeaponConfig.GunOffset = FVector(100.0f, 30.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.
	UWorld* const World = GetWorld();
	if (World != nullptr)
	{
		for (TActorIterator<AQuackBoss> ActorItr(World); ActorItr; ++ActorItr)
		{
			PlayerConfig.Boss = *ActorItr;
		}
	}
}

// Get Grab and Throw Component
void AQuackCharacter::InitialiseGrabberClass()
{
	TArray<UActorComponent*> Components = GetComponentsByClass(UGrabAndThrowComponent::StaticClass());
	if (Components.Num() == 0) return;
	for (size_t i = 0; i < Components.Num(); i++)
	{
		UGrabAndThrowComponent* ThisGrabber = Cast<UGrabAndThrowComponent>(Components[i]);
		if (ThisGrabber != nullptr)
		{
			GrabberClass = ThisGrabber;
		}
	}
}
// START
void AQuackCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	InitialiseGrabberClass();
	GetCharacterMovement()->MaxWalkSpeed = MovementConfig.InitialWalkSpeed;
	WeaponConfig.InitialFireCooldown = WeaponConfig.FireCooldown;
	PlayerConfig.MaxHealth = PlayerConfig.Health;
	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	CheckForMaxClip();
	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.

		//VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(true, false);
		WeaponConfig.bIsUsingGravityGun = false;
		HarryAssaultRifle->SetHiddenInGame(false, false);
		//KrisGravityGun->SetHiddenInGame(true, false);
		HarryGravGun->SetHiddenInGame(true, false);
		GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AQuackCharacter::OnTriggerEnter);
		GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &AQuackCharacter::OnTriggerExit);
}

// UPDATE
void AQuackCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Sprint();
	//GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("MouseDown: %s"), bMouseDown ? TEXT("True") : TEXT("False")));
	//GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("UsingGravGun: %s"), bIsUsingGravityGun ? TEXT("True") : TEXT("False")));
	if (WeaponConfig.bIsUsingGravityGun && WeaponConfig.bHasGravityGun)
	{
		if (GrabberClass != nullptr)
		{
			GrabberClass->TraceHandleLocation(PhysicsHandle, FirstPersonCameraComponent);
		}
	}
	else
	{
		SemiAutomaticShooting(DeltaTime);
	}

}

// Interact

void AQuackCharacter::Interact()
{
	if (InteractableActor != nullptr)
	{
		// ID represents what you interacted with
		if (InteractID == 0)
		{
			// Gun stuff
			WeaponConfig.bHasGravityGun = true;
			if (TutorialManager != nullptr)
			{
				//UE_LOG(LogTemp, Warning, TEXT("TrialGravGunDone"));
				TutorialManager->TrialGravGunPickUp();
			}
		}
		else if (InteractID == 1)
		{
			// Needle stuff
			PoisonConfig.bHasNeedle = true;
			if (TutorialManager != nullptr)
			{
				TutorialManager->TrialPoisonUsed();
			}
		}
		InteractID = -1;
		InteractableActor->Destroy();
		InteractableActor = nullptr;
	}
}

// TRIGGERS
void AQuackCharacter::OnTriggerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherbodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	APipe* Pipe = Cast<APipe>(OtherActor);
	if (Pipe != nullptr)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::Printf(TEXT("CurrentPipeShouldbeassigned")));
		PlayerConfig.CurrentPipe = Pipe;
		if (PlayerConfig.CurrentPipe->bNotABossPipe)
		{
			PlayerConfig.CurrentPipe->ToggleHighlight(true);
		}
	}

	AHealthStation* Regen = Cast<AHealthStation>(OtherActor);
	if (Regen != nullptr)
	{
		Regen->SetIsInZone(true);
	}

	if (OtherActor != nullptr)
	{
		UInteractComponent* InteractComp = Cast<UInteractComponent>(OtherActor->GetComponentByClass(UInteractComponent::StaticClass()));
		if (InteractComp != nullptr)
		{
			InteractableActor = OtherActor;
			InteractID = InteractComp->GetID();
		}
	}
}

void AQuackCharacter::OnTriggerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherbodyIndex)
{
	if (OtherActor != nullptr && OtherActor != this)
	{
		AHealthStation* Regen = Cast<AHealthStation>(OtherActor);
		if (Regen != nullptr)
		{
			Regen->SetIsInZone(false);
		}
		APipe* Pipe = Cast<APipe>(OtherActor);

		if (Pipe != nullptr)
		{
			if (PlayerConfig.CurrentPipe->bNotABossPipe)
			{
				PlayerConfig.CurrentPipe->ToggleHighlight(false);
			}
			PlayerConfig.CurrentPipe = nullptr;
		}
		
		UInteractComponent* InteractComp = Cast<UInteractComponent>(OtherActor->GetComponentByClass(UInteractComponent::StaticClass()));
		if (InteractComp != nullptr)
		{
			InteractableActor = nullptr;
			InteractID = -1;
		}
	}
}

// INPUT MANAGER
void AQuackCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("SwapGun", IE_Pressed, this, &AQuackCharacter::SwapGun);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AQuackCharacter::Reload);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AQuackCharacter::Interact);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AQuackCharacter::OnSprintBegin);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AQuackCharacter::OnSprintEnd);

	PlayerInputComponent->BindAction("MouseDown", IE_Pressed, this, &AQuackCharacter::MouseDown);
	PlayerInputComponent->BindAction("MouseUp", IE_Released, this, &AQuackCharacter::MouseUp);

	PlayerInputComponent->BindAction("Grab", IE_Pressed, this, &AQuackCharacter::Grab);

	PlayerInputComponent->BindAction("Poison", IE_Pressed, this, &AQuackCharacter::UsePoison);

	PlayerInputComponent->BindAxis("MoveForward", this, &AQuackCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AQuackCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AQuackCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AQuackCharacter::LookUpAtRate);
}

// GUN LOGIC
void AQuackCharacter::SwapGun()
{
	StopMuzzleFlash();
	//if (PlayerConfig.CurrentRock != nullptr) return;
	if (GrabberClass != nullptr)
	{
		
		if (GrabberClass->CurrentRock != nullptr) return;
	}

	if (WeaponConfig.bHasGravityGun)
		WeaponConfig.bIsUsingGravityGun = !WeaponConfig.bIsUsingGravityGun;

	if (WeaponConfig.bIsUsingGravityGun && WeaponConfig.bHasGravityGun)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Grav active, AR inactive, has grav and is using grav"));
		//FP_Gun->SetSkeletalMesh(FP_GravGun);
		HarryAssaultRifle->SetHiddenInGame(true, false);
		//KrisGravityGun->SetHiddenInGame(false, false);	
		HarryGravGun->SetHiddenInGame(false, false);

	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Grav inactive, AR active, doesnt have grav and isnt using grav"));
		//FP_Gun->SetSkeletalMesh(FP_RegularGun);
		HarryAssaultRifle->SetHiddenInGame(false, false);
		//KrisGravityGun->SetHiddenInGame(true, false);
		HarryGravGun->SetHiddenInGame(true, false);

	}

}

// GRAB AND THROW LOGIC

void AQuackCharacter::Grab()
{
	if (!WeaponConfig.bIsUsingGravityGun || !WeaponConfig.bHasGravityGun) return;
	if (GrabberClass != nullptr)
	{
		switch (GrabberClass->GetIsObjectHeld())
		{
			case true:
			{
				GrabberClass->Release(PhysicsHandle);
				break;
			}
			case false:
			{
				GrabberClass->Grab(this, PhysicsHandle, FirstPersonCameraComponent);
				break;
			}
		}
	}
}

// POISON LOGIC
void AQuackCharacter::UsePoison()
{
	//UE_LOG(LogTemp, Warning, TEXT("Pressed E Poison"));

	if (PlayerConfig.CurrentPipe != nullptr && 	PoisonConfig.bHasNeedle)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Pipe exists and has needle"));

		if (PlayerConfig.CurrentPipe->bNotABossPipe)
		{
			//UE_LOG(LogTemp, Warning, TEXT("This Ran Poison"));
			PlayerConfig.CurrentPipe->bPoisonedPipe = true;
			PlayerConfig.CurrentPipe->bNotABossPipe = false;
			PlayerConfig.CurrentPipe->ChangeMeshColour();
			PlayerConfig.CurrentPipe->SimulateDestroy();
			PlayerConfig.CurrentPipe->ToggleHighlight(false);
			PlayerConfig.CurrentPipe->OnPipeFinishedDraining.AddDynamic(TutorialManager, &ATutorialManager::RemoveAPipe);
			//TutorialManager->RemoveAPipe();
			return;
		}
	}
	// Must have poison vials
	if (PoisonConfig.PoisonVialsCount == 0) return;
	// Cant be using poison already
	if (PoisonConfig.bIsPoisoning) return;
	// Must be within the pipe radius
	if (!PoisonConfig.bCanPoisonPipe) return;
	// Reduce vial count
	if (PlayerConfig.Boss != nullptr)
	{
		if (PlayerConfig.CurrentPipe != nullptr && PlayerConfig.Boss->TargettedPipe != nullptr)
		{
			if (PlayerConfig.CurrentPipe->GetName() == PlayerConfig.Boss->TargettedPipe->GetName())
			{
				if (PlayerConfig.Boss->CurrentBossState == BossStates::E_HealingOne || PlayerConfig.Boss->CurrentBossState == BossStates::E_HealingTwo || PlayerConfig.Boss->CurrentBossState == BossStates::E_HealingThree || PlayerConfig.Boss->CurrentBossState == BossStates::E_HealingFour)
				{
					PlayerConfig.CurrentPipe->bPoisonedPipe = true;
					PlayerConfig.CurrentPipe->ChangeMeshColour();
					PlayerConfig.CurrentPipe->SimulateDestroy();
					PlayerConfig.CurrentPipe->ToggleHighlight(false);
					PlayerConfig.Boss->ChangeState(BossStates::E_Poisoned);
					if (PlayerConfig.Boss->Pipes.Num() == 0)
					{
						PlayerConfig.Boss->TargettedPipe = nullptr;
					}
					PoisonConfig.PoisonVialsCount--;
					UWorld* const World = GetWorld();
					if (World != nullptr)
					{
						PoisonConfig.bIsPoisoning = true;
						World->GetTimerManager().SetTimer(PoisonConfig.PoisonUsageDelay, this, &AQuackCharacter::PoisonCooldown, PoisonConfig.PoisonCD, false);
					}
				}
			}

		} 
	}
}

void AQuackCharacter::PoisonCooldown()
{
	UWorld* const World = GetWorld();
	if (World != nullptr)
	{
		PoisonConfig.bIsPoisoning = false;
		World->GetTimerManager().ClearTimer(PoisonConfig.PoisonUsageDelay);
		if (PlayerConfig.Boss != nullptr)
		{
			//PlayerConfig.Boss->DestroyPreviousTargettedPipe();
			PlayerConfig.Boss->LocateNearbyPipe();
			PlayerConfig.Boss->ChangeState(BossStates::E_Idle);
		}
	}
}

// HEALTH LOGIC
void AQuackCharacter::IncreaseHealth(float Amount)
{
	PlayerConfig.Health += Amount ;
	PlayerConfig.Health = FMath::Clamp(PlayerConfig.Health, 0.0f, PlayerConfig.MaxHealth);
}

void AQuackCharacter::IncreaseHealth(float Amount, float DeltaTime)
{
	PlayerConfig.Health += Amount * DeltaTime;
	PlayerConfig.Health = FMath::Clamp(PlayerConfig.Health, 0.0f, PlayerConfig.MaxHealth);
}

void AQuackCharacter::DecreaseHealth(float Amount)
{
	PlayerConfig.Health -= Amount;
	if (PlayerConfig.Health <= 0.0f)
	{
		// You Dead
		UGameplayStatics::OpenLevel(this, FName("MainMenu"), false);
	}
	PlayerConfig.Health = FMath::Clamp(PlayerConfig.Health, 0.0f, PlayerConfig.MaxHealth);
}

// SHOOTING AND WEAPON LOGIC
void AQuackCharacter::DecreaseAmmo(int Amount)
{
	WeaponConfig.Ammo -= Amount;
	if (WeaponConfig.Ammo <= 0)
	{
		Reload();
	}
}

void AQuackCharacter::IncreaseAmmo(int Amount)
{
	WeaponConfig.Ammo += Amount;
	CheckForMaxClip();
}

void AQuackCharacter::Reload()
{
	if (WeaponConfig.Ammo < WeaponConfig.MaxClipSize)
	{
		int RequiredAmmo = WeaponConfig.MaxClipSize - WeaponConfig.Ammo;
		if (WeaponConfig.ReservedAmmo >= RequiredAmmo)
		{
			WeaponConfig.Ammo += RequiredAmmo;
			WeaponConfig.ReservedAmmo -= RequiredAmmo;
		}
		else
		{
			WeaponConfig.Ammo += WeaponConfig.ReservedAmmo;
			WeaponConfig.ReservedAmmo = 0;
		}
	}

}

void AQuackCharacter::CheckForMaxClip()
{
	if (WeaponConfig.Ammo >= WeaponConfig.MaxClipSize)
	{
		WeaponConfig.ReservedAmmo += (WeaponConfig.Ammo - WeaponConfig.MaxClipSize);
		WeaponConfig.Ammo = WeaponConfig.MaxClipSize;
	}
}

void AQuackCharacter::Raycast()
{
	if (WeaponConfig.Ammo <= 0) return;
	FHitResult Hit;
	FVector StartTrace = FirstPersonCameraComponent->GetComponentLocation();
	FVector ForwardVector = FirstPersonCameraComponent->GetForwardVector();
	FVector EndTrace = StartTrace + (ForwardVector * PlayerConfig.RayLength);
	FCollisionQueryParams CQP;
	UWorld* const World = GetWorld();
	if (World != nullptr)
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC != nullptr)
		{
			if (WeaponConfig.CameraShake != nullptr)
			{
				PC->ClientPlayCameraShake(WeaponConfig.CameraShake, 1.0f);
			}
			if (WeaponConfig.Feedback != nullptr)
			{
				PC->ClientPlayForceFeedback(WeaponConfig.Feedback, false, "Weapon");
			}
			if (WeaponConfig.FireEffect != nullptr && WeaponConfig.MuzzleFlashParticleSystem == nullptr)
			{
				//const FRotator SpawnRotation = GetControlRotation();
				FName SocketName = FName(TEXT("MuzzleFlashSocket"));
				//const FVector Location = ((FP_Gun != nullptr) ? FP_Gun->GetSocketLocation(SocketName) : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);
				//UParticleSystemComponent* PS = UGameplayStatics::SpawnEmitterAttached(FireEffect, FP_Gun, SocketName, Location);
				//UParticleSystemComponent* PS = UGameplayStatics::SpawnEmitterAtLocation(this, FireEffect, GetCapsuleComponent()->GetComponentLocation() + (GetActorForwardVector() *  100.0f));
				//		UParticleSystemComponent* PS = UGameplayStatics::SpawnEmitterAtLocation(this, FireEffect, CurrentWeapon->GetMuzzleLocation());
				//UParticleSystemComponent* PS = UGameplayStatics::SpawnEmitterAttached(FireEffect, Mesh1P, SocketName);
				WeaponConfig.MuzzleFlashParticleSystem = UGameplayStatics::SpawnEmitterAttached(WeaponConfig.FireEffect, FP_Gun, SocketName);
				WeaponConfig.MuzzleFlashParticleSystem->SetWorldScale3D(FVector(0.75f));
			}

			if (WeaponConfig.FireTrailEffect != nullptr)
			{
				//const FVector Origin = FP_Gun->GetMuzzleLocation();

				// THIS IS IN THE ARMS SKELETON BY HIS RIGHT HAND
				// NEW ARMS, ADD SOCKET ETC
				const FVector Origin = FP_Gun->GetSocketLocation(FName("GripPoint"));
				UParticleSystemComponent* TrailPSC = UGameplayStatics::SpawnEmitterAtLocation(World, WeaponConfig.FireTrailEffect, Origin);
				if (TrailPSC)
				{
					TrailPSC->SetVectorParameter(FName(TEXT("ShockBeamEnd")), EndTrace);
				}
			}
		}
		if (World->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECC_Visibility, CQP))
		{
			//DrawDebugLine(GetWorld(), StartTrace, EndTrace, Colour, true);
			if (Hit.GetActor() != nullptr)
			{
				//print("Object Hit");
				AQuackBoss* Boss = Cast<AQuackBoss>(Hit.GetActor());
				AQuackAIPawn* Enemy = Cast<AQuackAIPawn>(Hit.GetActor());
				if (Boss != nullptr)
				{
					// Call take damage on the boss
					Boss->SufferDamage(WeaponConfig.GunDamage);
					Boss->ShouldEnterHealingPhase();
					if (WeaponConfig.HitEffect != nullptr)
					{
						UGameplayStatics::SpawnEmitterAtLocation(World, WeaponConfig.HitEffect, Hit.Location);
					}
				}
				else if (Enemy != nullptr)
				{
					Enemy->TakeDamages(WeaponConfig.GunDamage);
					if (WeaponConfig.HitEffect != nullptr)
					{
						UGameplayStatics::SpawnEmitterAtLocation(World, WeaponConfig.HitEffect, Hit.Location);
					}
				}

				//Hit->GetActor()->Destroy();
			}
		}
	}


	DecreaseAmmo(1);
	if (WeaponConfig.FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponConfig.FireSound, GetActorLocation(), GunShootVolume);
	}

	// try and play a firing animation if specified
	if (WeaponConfig.FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(WeaponConfig.FireAnimation, 1.f);
		}
	}
	//OnFire();
}

void AQuackCharacter::StopMuzzleFlash()
{
	if (WeaponConfig.MuzzleFlashParticleSystem != nullptr)
	{
		//UE_LOG(LogTemp, Warning, TEXT("MuzzleFlashDeactivated"));
		WeaponConfig.MuzzleFlashParticleSystem->DeactivateSystem();
		WeaponConfig.MuzzleFlashParticleSystem = nullptr;
	}
	//if (MuzzleFlashParticleSystemPistol != nullptr)
	//{
	//	MuzzleFlashParticleSystemPistol->DeactivateSystem();
	//	MuzzleFlashParticleSystemPistol = nullptr;
	//}
}

void AQuackCharacter::OnFire()
{
	// try and fire a projectile
	if (WeaponConfig.ProjectileClass.Num() <= 1) return;
	if (WeaponConfig.ProjectileClass[1] != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(WeaponConfig.GunOffset);

			// spawn the projectile at the muzzle
			World->SpawnActor<AQuackProjectile>(WeaponConfig.ProjectileClass[1], SpawnLocation, SpawnRotation);
			DecreaseAmmo(1);
			//Cast<AQuackProjectile>(ProjectileClass[Index])->GetProjectileMovement()->InitialSpeed = Cast<AQuackProjectile>(ProjectileClass[Index])->InitialVel;
			//Cast<AQuackProjectile>(ProjectileClass[Index])->GetProjectileMovement()->MaxSpeed = Cast<AQuackProjectile>(ProjectileClass[Index])->MaxVel;	
		}
	}

	// try and play the sound if specified
	if (WeaponConfig.FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponConfig.FireSound, GetActorLocation(), 0.0125f);
	}

	// try and play a firing animation if specified
	if (WeaponConfig.FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(WeaponConfig.FireAnimation, 1.f);
		}
	}
	//GetWorldTimerManager().SetTimer(FireRateDelay, this, &AQuackCharacter::ResetTimer, FireRate, true);
}

void AQuackCharacter::OnFire2()
{
	if (WeaponConfig.ProjectileClass.Num() == 0) return;
	// try and fire a projectile
	if (WeaponConfig.ProjectileClass[0] != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(WeaponConfig.GunOffset);

			// spawn the projectile at the muzzle
			World->SpawnActor<AQuackProjectile>(WeaponConfig.ProjectileClass[0], SpawnLocation, SpawnRotation);
			//Cast<AQuackProjectile>(ProjectileClass[Index])->GetProjectileMovement()->InitialSpeed = Cast<AQuackProjectile>(ProjectileClass[Index])->InitialVel;
			//Cast<AQuackProjectile>(ProjectileClass[Index])->GetProjectileMovement()->MaxSpeed = Cast<AQuackProjectile>(ProjectileClass[Index])->MaxVel;	
		}
	}

	// try and play the sound if specified
	if (WeaponConfig.FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponConfig.FireSound, GetActorLocation(), 0.1f);
	}

	// try and play a firing animation if specified
	if (WeaponConfig.FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(WeaponConfig.FireAnimation, 1.f);
		}
	}
	//GetWorldTimerManager().SetTimer(FireRateDelay, this, &AQuackCharacter::ResetTimer, FireRate);
}

void AQuackCharacter::SemiAutomaticShooting(float DeltaTime)
{
	UWorld* const World = GetWorld();
	if (World != nullptr)
	{
		if (WeaponConfig.bMouseDown && !WeaponConfig.bMouseUp)
		{
			WeaponConfig.RunningTime += DeltaTime;
			if (WeaponConfig.RunningTime >= WeaponConfig.FireCooldown)
			{
				Raycast();
				WeaponConfig.FireCooldown += WeaponConfig.FireRate;
			}
			//World->GetTimerManager().SetTimer(ContinuousShooting, this, &AQuackCharacter::Raycast, 0.5f, true);
		}
		else
		{
			StopMuzzleFlash();
			WeaponConfig.RunningTime = 0.0f;
			WeaponConfig.FireCooldown = WeaponConfig.InitialFireCooldown;
			//World->GetTimerManager().ClearTimer(ContinuousShooting); 
		}
	}
}

void AQuackCharacter::MouseDown()
{
	WeaponConfig.bMouseDown = true;
	WeaponConfig.bMouseUp = false;
	if (WeaponConfig.bIsUsingGravityGun && WeaponConfig.bHasGravityGun)
	{
		if (GrabberClass != nullptr)
		{
			GrabberClass->Throw(PhysicsHandle, FirstPersonCameraComponent, WeaponConfig.bMouseDown);
		}
	}
}

void AQuackCharacter::MouseUp()
{
	WeaponConfig.bMouseDown = false;
	WeaponConfig.bMouseUp = true;
}

// MOVEMENT AND CAMERA TURN
void AQuackCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AQuackCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AQuackCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AQuackCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

// SPRINTING
void AQuackCharacter::OnSprintBegin()
{
	MovementConfig.bIsSprinting = true;
}

void AQuackCharacter::OnSprintEnd()
{
	MovementConfig.bIsSprinting = false;
}

void AQuackCharacter::Sprint()
{
	if (MovementConfig.bIsSprinting)
	{
		//GEngine->AddOnScreenDebugMessage(-1, GetWorld()->GetDeltaSeconds(), FColor::Green, FString::Printf(TEXT("Sprinting")));
		GetCharacterMovement()->MaxWalkSpeed = MovementConfig.SprintSpeed;
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, GetWorld()->GetDeltaSeconds(), FColor::Green, FString::Printf(TEXT("Walking")));
		GetCharacterMovement()->MaxWalkSpeed = MovementConfig.InitialWalkSpeed;
	}
}
