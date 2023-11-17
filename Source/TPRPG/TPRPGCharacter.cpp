// Copyright Epic Games, Inc. All Rights Reserved.

#include "TPRPGCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/KismetMathLibrary.h"
#include "TPRPGSpellGrid.h"
#include "SpellPoint.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

#define ECC_SpellPoint ECollisionChannel::ECC_GameTraceChannel2

//////////////////////////////////////////////////////////////////////////
// ATPRPGCharacter

ATPRPGCharacter::ATPRPGCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	//CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	//CameraBoom->SetupAttachment(RootComponent);
	//CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	//CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	//FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	//FollowCamera->SetupAttachment(RootComponent);
	//FollowCamera->SetWorldLocation(FVector(-283.0, 0, 850.0));
	//FollowCamera->SetRelativeRotation(FRotator(0, -70, 0).Quaternion());
	//FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	//FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ATPRPGCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		PlayerController->SetShowMouseCursor(true);

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			Subsystem->AddMappingContext(WizardBaseMappingContext, 1);
		}

		SetupInitialSpells();
	}
}

void ATPRPGCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LookAtMouse();
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATPRPGCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATPRPGCharacter::Move);

		// Looking
		//EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATPRPGCharacter::Look);

		SetupWizardActions(PlayerInputComponent);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ATPRPGCharacter::SetupWizardActions(UInputComponent* WiardInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(WiardInputComponent)) 
	{
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &ATPRPGCharacter::Dash);

		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ATPRPGCharacter::FireProjectile);

		EnhancedInputComponent->BindAction(StartCraftAction, ETriggerEvent::Started, this, &ATPRPGCharacter::StartCrafting);
		EnhancedInputComponent->BindAction(StartCraftAction, ETriggerEvent::Completed, this, &ATPRPGCharacter::EndCrafting);

		EnhancedInputComponent->BindAction(DrawAction, ETriggerEvent::Triggered, this, &ATPRPGCharacter::DrawSpell);
	}
}

void ATPRPGCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	hDirectionVector = MovementVector;

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ATPRPGCharacter::Look(const FInputActionValue& Value)
{
	//// input is a Vector2D
	//FVector2D LookAxisVector = Value.Get<FVector2D>();

	//if (Controller != nullptr)
	//{
	//	// add yaw and pitch input to controller
	//	/*AddControllerYawInput(LookAxisVector.X);
	//	AddControllerPitchInput(LookAxisVector.Y);*/

	//}
}

void ATPRPGCharacter::LookAtMouse()
{
	if (Controller != nullptr)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
		{
			FHitResult result;
			PlayerController->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Camera), true, result);

			double LookToMouseRotation = 0;
			switch (currentLookState)
			{
			case(ELookState::Default):
				LookToMouseRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), result.Location).Yaw;
				break;
			case(ELookState::Crafting):
				LookToMouseRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LastMousePosition).Yaw;
				break;
			case(ELookState::Aiming):
				LookToMouseRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), result.Location).Yaw;
				break;
			}

			SetActorRotation(FRotator(GetActorRotation().Pitch, LookToMouseRotation, GetActorRotation().Roll));
		}
	}
}

void ATPRPGCharacter::Dash(const FInputActionValue& Value)
{
	if (Controller != nullptr && bCanDash == true)
	{
		bCanDash = false;
		LaunchCharacter(FVector(hDirectionVector.Y, (hDirectionVector.X), 0) * DashDistance, true, true);

		GetWorldTimerManager().SetTimer(DashDurationHandle, this, &ATPRPGCharacter::HaltMovementVelocity, 1.0f, false, DashDuration);


		GetWorldTimerManager().SetTimer(DashTimerHandle, this, &ATPRPGCharacter::ResetCanDash, 1.0f, false, DashCooldown);
	}
}

// PLAN: If spell is a projectile, player will be able to call this function on "left-click" to fire the projectile in the direction theyre aiming with their mouse
void ATPRPGCharacter::FireProjectile()
{
	currentLookState = ELookState::Default;
	UE_LOG(LogTemplateCharacter, Error, TEXT("PROJECTILE FIRED"));

}

// PLAN:: Some kind of spell grid/spell circle will be created on the ground and the player will be able to move their mouse over certain points which should light up to create patterns.
// Each attack will have a different pattern (maybe) and show a different colour
void ATPRPGCharacter::StartCrafting()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (currentLookState == ELookState::Default)
		{
			currentLookState = ELookState::Crafting;
			float TimeScale = 0.65f;
			GetWorld()->GetWorldSettings()->SetTimeDilation(TimeScale);

			FHitResult result;
			PlayerController->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Camera), true, result);

			LastMousePosition = result.Location;

			PlayerSpellGrid = GetWorld()->SpawnActor<ATPRPGSpellGrid>(BP_SpellGrid, LastMousePosition, FRotator(0, 0, 0));
		}
	}
}

void ATPRPGCharacter::EndCrafting()
{
	if (currentLookState == ELookState::Crafting)
	{
		currentLookState = ELookState::Default;
	}

	SpellPoints.Empty(0);
	LastSpellPoint.Empty(0);
	PlayerSpellGrid->Destroy();
	GetWorld()->GetWorldSettings()->SetTimeDilation(1.0f);

}

void ATPRPGCharacter::DrawSpell()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (currentLookState == ELookState::Crafting)
		{
			FHitResult result;
			PlayerController->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Camera), true, result);
			
			if (result.GetComponent()->GetOwner()->Implements<USpellPoint>())
			{
				UE_LOG(LogTemplateCharacter, Error, TEXT("IMPLEMENTS INTERFACE"));

				AActor* HitSpellPoint = result.GetComponent()->GetOwner();
				ISpellPoint* SpellPointInterface = Cast<ISpellPoint>(HitSpellPoint);															// Object interface we are using currently (hit spellpoint)

				FString PointNum = SpellPointInterface->Execute_EnablePoint(HitSpellPoint);

				if (PointNum != LastSpellPoint)
				{
					//UE_LOG(LogTemplateCharacter, Error, TEXT("COLLIDED WITH SPELLPOINT %s"), *result.GetComponent()->GetOwner()->GetFName().ToString());
					UE_LOG(LogTemplateCharacter, Error, TEXT("COLLIDED WITH SPELLPOINT %s"), *PointNum);
					//SpellPoints.Append(result.GetComponent()->GetOwner()->GetFName().ToString());
					SpellPoints.Append(PointNum);
				}

				LastSpellPoint = PointNum;
			}

			if (SpellPoints.Len() >= MAX_SPELL_LENGTH)
			{
				CreateSpell();
			}
		}
	}
}

void ATPRPGCharacter::HaltMovementVelocity()
{
	GetCharacterMovement()->Velocity *= 0;
}

void ATPRPGCharacter::ResetCanDash()
{
	bCanDash = true;
}

void ATPRPGCharacter::SetupInitialSpells()
{
	AvailableSpells["Fireball"] = FString(TEXT("27542"));
	AvailableSpells["Frostbolt"] = FString(TEXT("27452"));
	AvailableSpells["Lightningbolt"] = FString(TEXT("76321"));
}

void ATPRPGCharacter::CreateSpell()
{
	if (SpellPoints == AvailableSpells["Fireball"])
	{
		currentLookState = ELookState::Aiming;
		Spell_FireBall();
	}
	else if (SpellPoints == AvailableSpells["Frostbolt"])
	{
		currentLookState = ELookState::Aiming;
		Spell_FrostBolt();
	}
	else if (SpellPoints == AvailableSpells["Lightningbolt"])
	{
		currentLookState = ELookState::Aiming;
		Spell_LightningBolt();
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("INVALID SPELL COMBINATION"));
	}

	EndCrafting();
}

void ATPRPGCharacter::Spell_FireBall()
{
	UE_LOG(LogTemplateCharacter, Error, TEXT("Fire Ball!"));
	AActor* Fireball = GetWorld()->SpawnActor<AActor>(BP_Fireball, FVector(LastMousePosition.X, LastMousePosition.Y, LastMousePosition.Z + SpellSpawnHeight), FRotator(0, 0, 0));
}

void ATPRPGCharacter::Spell_FrostBolt()
{
	UE_LOG(LogTemplateCharacter, Error, TEXT("Frost Bolt!"));
	AActor* Frostbolt = GetWorld()->SpawnActor<AActor>(BP_Frostbolt, FVector(LastMousePosition.X, LastMousePosition.Y, LastMousePosition.Z + SpellSpawnHeight), FRotator(0, 0, 0));
}

void ATPRPGCharacter::Spell_LightningBolt()
{
	UE_LOG(LogTemplateCharacter, Error, TEXT("Lightning Bolt!"));
	AActor* Lightningbolt = GetWorld()->SpawnActor<AActor>(BP_Lightningbolt, FVector(LastMousePosition.X, LastMousePosition.Y, LastMousePosition.Z), FRotator(0, 0, 0));
}