// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include <functional>
#include <unordered_map>
#include <any>

#include "TPRPGCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class ATPRPGSpellGrid;
class BP_4E_Base_ProjectileAlt;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ATPRPGCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATPRPGSpellGrid> BP_SpellGrid;

	/* Spawnable Spell Grid */
	UPROPERTY(BlueprintReadOnly, Category = Magic, meta = (AllowPrivateAccess = "true"))
	ATPRPGSpellGrid* PlayerSpellGrid;

	UPROPERTY(EditAnywhere, Category = Magic, meta = (AllowPrivateAccess = "true"));
	int SpellSpawnHeight;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> BP_FrostBolt;

	//UPROPERTY(BlueprintReadOnly, Category = Magic, meta = (AllowPrivateAccess = "true"))
	//BP_4E_Base_ProjectileAlt* FrostBolt;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Wizard Mapping Context*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* WizardBaseMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Dash Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateaccess = "true"))
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* StartCraftAction;

	// Wand Draw Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DrawAction;

	//UPROPERTY

	FVector2D hDirectionVector = FVector2D(0, 0);
	FTimerHandle DashDurationHandle;
	FTimerHandle DashTimerHandle;

	FVector LastMousePosition = FVector(0, 0, 0);

public:
	ATPRPGCharacter();
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void LookAtMouse();
			
	/** Called for dash action */
	void Dash(const FInputActionValue& Value);


	void FireProjectile();
	void StartCrafting();
	void EndCrafting();
	void DrawSpell();


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetupWizardActions(UInputComponent* WiardInputComponent);
	
	// To add mapping context
	virtual void BeginPlay();

	virtual void Tick(float DeltaTime);

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	enum ELookState
	{
		Default,
		Crafting,
		Aiming
	};
	ELookState currentLookState = ELookState::Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanDash = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int DashDistance = 1000;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DashDuration = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DashCooldown = 1.0f;

	void HaltMovementVelocity();
	void ResetCanDash();


	std::unordered_map<std::string, FString> AvailableSpells;
	const int MAX_SPELL_LENGTH = 5;
	FString SpellPoints = FString(TEXT(""));
	FString LastSpellPoint = FString(TEXT(""));

	void SetupInitialSpells();

	void CreateSpell();

	AActor* CurrentSpell;
	void Spell_FireBall();
	void Spell_FrostBolt();
};

