// MIT License
// Copyright (c) 2024 Jonathan Lee Gunderson

#pragma once
#include <atomic>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestDoingCoolStuff.generated.h"


UCLASS(Blueprintable)
class DOINGCOOLSTUFF_API ADoingCoolStuff : public AActor
{
	GENERATED_BODY()
public:	

	// Sets default values for this actor's properties
	ADoingCoolStuff();

	UFUNCTION(BlueprintCallable)
	void DoStuff();

	UFUNCTION(BlueprintCallable)
	void EndWave();

	UFUNCTION(BlueprintCallable)
	void TogglePin(const int pin);


	UFUNCTION(BlueprintCallable)
	void SetPinState(const int pin, const bool bState);



	UPROPERTY(EditAnywhere)
	int I2C_BUs = 4;

	UPROPERTY(EditAnywhere)
	uint8 PCF_Address = -1; //0x27;

	UPROPERTY(EditAnywhere)
	int DELAY  = 100000;



private:
	std::atomic<bool>  isRunning;
	int InitITwoC();
	void WaveEffect();
	void singal_handler(int signmun);

	bool AutoFindPCFAddress();
	int i2c_fd;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason  ) override;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
