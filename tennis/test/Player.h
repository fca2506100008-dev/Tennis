#pragma once
#include "Character.h"

class Player : public Character {
public:
    Player(Vector3 startPos);
    void Update(Ball& ball, float speedFactor = 1.0f) override;
    void DrawReachArea() const;
};