#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include <vector>
#include <memory>
#include "../GameObject/Player.h"
#include "../GameObject/BulletSpawner.h"
#include "../System/CollisionSystem.h"
#include "../GameObject/GameObject.h"
#include "../GameObject/EnemyManager.h"
#include "../GameObject/Boss.h"  // 添加Boss头文件

namespace Game {

class GameScene {
public:
    // 游戏状态枚举
    enum class GameState {
        CHOOSE,
        NORMAL,    // 普通阶段
        BOSS,      // Boss战阶段
        VICTORY,   // 胜利
        GAMEOVER   // 失败
    };

    enum class Difficulty {
        EASY,
        NORMAL,
        HARD
    };
private:
    
    GameState currentState_ = GameState::CHOOSE;
    Difficulty currentDifficulty_ = Difficulty::NORMAL;
    int menuSelected_ = 1; // 0=Easy,1=Normal,2=Hard
    bool menuConfirmed_ = false;

    std::vector<std::unique_ptr<GameObject>> objects_;
    std::vector<std::unique_ptr<Bullet>> bullets_;
    BulletManager bulletManager_;
    EnemyManager enemyManager_;
    Player player_;
    float bulletTimer_ = 0.0f;
    const float BULLET_INTERVAL = 0.5f;
    int collisionCount_ = 0;
    int score_ = 0;
    bool gameOver_ = false;
    float playerFireTimer_ = 0.0f;
    const float PLAYER_FIRE_INTERVAL = 0.1f;
    RenderCore::Vector2f m_playerPos;
    bool bossDeathChecked = false;
    
    // Boss相关
    std::unique_ptr<Boss> boss_;  // Boss对象
    bool bossSpawned_ = false;    // 是否已生成Boss
    const int BOSS_SCORE_THRESHOLD = 2000; // Boss出现分数阈值
    
    
public:
    GameScene() : enemyManager_(bulletManager_.GetPool()), m_playerPos({400, 500}) {}
    void Init();
    void Update(float deltaTime);
    void Render(RenderCore::RenderEngine& engine);

    // 获得分数
    int GetScore() const { return score_; };

    
    void SpawnBullet(const RenderCore::Vector2f& pos, 
                     const RenderCore::Vector2f& vel,
                     bool isPlayerBullet = false);
    
    void CleanupInactive();
    Player& GetPlayer() { return player_; }
    const std::vector<std::unique_ptr<Bullet>>& GetBullets() const { return bullets_; }
    BulletManager& GetBulletManager() { return bulletManager_; }
    GameState GetState(){return currentState_;}

private:
    void HandlePlayerShooting(float deltaTime);
    void CheckCollisions();
    void RenderHUD(RenderCore::RenderEngine& engine);
    void SpawnBoss();  // 生成Boss
    void UpdateNormalPhase(float deltaTime);  // 更新普通阶段
    void UpdateBossPhase(float deltaTime);    // 更新Boss阶段
    void InitPlayerLives();

    // 菜单相关
    void RenderMenu(RenderCore::RenderEngine& engine);
    void HandleMenuInput();
    // 难度系数分发
    float GetEnemySpawnInterval() const;
    float GetBulletSpeedScale() const;
    int GetBossHealth() const;
    float GetBossFireInterval() const;
};

} // namespace Game

#endif