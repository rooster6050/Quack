// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Structs_Enums.h"
#include "QuackBoss.generated.h"

UCLASS()
class QUACK_API AQuackBoss : public AActor
{
	GENERATED_BODY()

private:
	// THIS CLASS NEEDS REFACTORING PROPERLY
	UPROPERTY(VisibleAnywhere, Category = "Shield")
		uint32 bShieldUp : 1;

	UPROPERTY(VisibleAnywhere, Category = "Shield")
		uint32 bMeleeAttacking : 1;

	UPROPERTY(VisibleAnywhere, Category = "Shield")
		int BileShotsFired = 1;

	UPROPERTY(EditAnywhere, Category = "Arm")
		float ArmDamage = 15.0f;

	UPROPERTY(EditAnywhere, Category = "Arm")
		float MeleeRangeDistanceThreshold = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Niall", meta = (AllowPrivateAccess = "true"))
		float RunningTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Niall", meta = (AllowPrivateAccess = "true"))
		float FireRate = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Niall", meta = (AllowPrivateAccess = "true"))
		float FireCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Niall", meta = (AllowPrivateAccess = "true"))
		float InitialFireCooldown;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Bullet Spawns")
		USceneComponent* MyRoot;

	UPROPERTY(VisibleDefaultsOnly, Category = "Bullet Spawns")
		USkeletalMeshComponent* MySkeletalMesh;

	UPROPERTY(VisibleDefaultsOnly, Category = "Chandelier")
		UStaticMeshComponent* Shield;

	UPROPERTY(VisibleDefaultsOnly, Category = "Chandelier")
		UStaticMeshComponent* Chandelier;

	UPROPERTY(VisibleDefaultsOnly, Category = "Bullet Spawns")
		class UArrowComponent* TailArrow;

	UPROPERTY(VisibleDefaultsOnly, Category = "Bullet Spawns")
		class UArrowComponent* MouthArrow;

	UPROPERTY(VisibleDefaultsOnly, Category = "Bullet Spawns")
		class UMinionFactoryComponent* MinionFactory;

	// Return true if should attack
	bool CheckForMeleeAttack();

	UFUNCTION()
		void EnableMelee();

	void BeginPipeDrain();

	void EndPipeDrain();

	UPROPERTY(VisibleAnywhere, Category = "AudioState")
		class AAudioManager* AudioManager;

public:	
	// Sets default values for this actor's properties
	AQuackBoss();

	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	void InitialiseStaticMeshes();
	void InitialiseSkeletalMeshes();
	void InitialiseArrowComponent();
	void ToggleShield(bool Visibility);
	void BeginAscend();
	void Ascend(float DeltaTime);
	void Descend(float DeltaTime);

	void InitialisePlayerCharacterReference();
	void InitialiseRockSpawnerReference();

	void ActivateSomeRocks();

	UFUNCTION()
		void OnTriggerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY()
	class AQuackCharacter* MyCharacter;
	UPROPERTY()
	class ARockSpawner* RockSpawner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Niall")
		FRotator HeadRot;

	void ResumeFighting();

	UPROPERTY(EditAnywhere, Category = "Custom Niall")
		float RecoilDuration = 2.0f;
	
	UFUNCTION()
		void ChangeBackToPrevious();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Niall")
		BossStates PreviousBossState;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Custom Niall")
	BossStates CurrentBossState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Niall")
	AnimationStates CurrentAnimationState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Niall")
		float KrisFloat;

	void ChangeState(BossStates DesiredState);
	void HandleStates(float DeltaTime);
	void ShouldEnterHealingPhase();
	void DestroyPreviousTargettedPipe();
	void SetPreviousPipe();

	UPROPERTY()
	TArray<UStaticMeshComponent*> StaticMeshes;
	UPROPERTY()
	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	UPROPERTY()
	TArray<UArrowComponent*> ProjectileSpawns;

	void SufferDamage(float Amount);
	void Regenerate(float DeltaTime);
	void CheckForDead();

	UFUNCTION(BlueprintCallable, Category = "Custom")
	FRotator RotateHeadToPlayer();

	void RotateTowardsPlayer();
	void RotateTowardsPipe();
	void LocateNearbyPipe();

	void CheckForPoisoned(float DeltaTime);

	void SlamGround();

	FTimerHandle BileTimer;

	UPROPERTY(EditAnywhere, Category = "Bile")
		float BileFireRate = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Bile")
		uint32 bIsBileSpitting : 1;
	void StartBileShot(float OverridenFireRate = 1.0f);
	UFUNCTION()
		void StopBileShot();
	UFUNCTION()
		void ShootBile();
		//void ShootBile(float DeltaTime);

	FTimerHandle TailTimer;

	UPROPERTY(EditAnywhere, Category = "Bile")
		float TailFireRate = 5.0f;
	UPROPERTY(EditAnywhere, Category = "Bile")
		uint32 bIsTailShooting : 1;
	void StartTailShot();
	UFUNCTION()
		void StopTailShot();
	UFUNCTION()
		void TailShoot();
	//void ShootFromTail(float DeltaTime);

	FTimerHandle AttackIterationTimer;
	UPROPERTY(EditAnywhere, Category = "Bile")
		float ChangeAttackPatternRate = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Bile")
		uint32 bUsingPrimaryAttack : 1;
	void BeginMultipleAttacksPattern();
	UFUNCTION()
		void ChangeAttack();
	void ResetMultipleAttacksPattern();

	void RotateTowardsWall();
	void LocateNearbyWall();
	void SmashWall();


	FTimerHandle WaveSpawnerTimer;
	UPROPERTY(VisibleAnywhere, Category = "Spawning")
		uint32 bIsSpawning : 1;
	UPROPERTY(VisibleAnywhere, Category = "Spawning")
		int WaveSpawnCounter = 0;
	UPROPERTY(EditAnywhere, Category = "Spawning")
		float WaveSpawnDelay = 10.0f;
	void BeginWaveSpawningCycle();
	UFUNCTION()
		void SpawnMinions();
	void ClearWaveSpawningTimer();

	UPROPERTY(VisibleAnywhere, Category = "Custom Niall")
		uint32 bPoisonned : 1;

	UPROPERTY(EditAnywhere, Category = "Custom Niall")
		TArray<class APipe*> Pipes;
	UPROPERTY(EditAnywhere, Category = "Custom Niall")
		TArray <TSubclassOf<class AQuackProjectile>> Projectile;
	UPROPERTY(EditAnywhere, Category = "Custom Niall")
		class APipe* TargettedPipe;
	UPROPERTY(EditAnywhere, Category = "Custom Niall")
		class APipe* PreviousPipe;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom Niall")
		FTransform CurrentTargettedPipeTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Niall")
		float BossRegenRate = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Niall")
		float BossHealth = 100.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom Niall")
		float MaxBossHealth = 100.0f;

	/*
	put above class
	https ://answers.unrealengine.com/questions/3742/what-is-proper-way-to-make-enum-for-bp-and-c-both.html
	 UENUM(BlueprintType)        //"BlueprintType" is essential specifier
 namespace EVictoryEnum
 {
     //256 entries max
     enum Type
     {
         VE_Dance     UMETA(DisplayName="Dance"),
         VE_Rain         UMETA(DisplayName="Rain"),
         VE_Song        UMETA(DisplayName="Song"),
         
         //~~~
         
         //256th entry
         VE_Max        UMETA(Hidden),
     };
 }
	*/

};
