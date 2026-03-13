#define _USE_MATH_DEFINES
#include "GameScene.h"
#include "../System/InputManager.h"
#include "vector.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include "../Utils/GameFont.h"

namespace Game {

void GameScene::Init() {
    // 启动即进入菜单
    currentState_ = GameState::CHOOSE;
    menuSelected_ = 1;
    menuConfirmed_ = false;
    player_.SetPosition({400, 500});
    score_ = 0;
    bossSpawned_ = false;
    boss_ = nullptr;
}

void GameScene::Update(float deltaTime) {
    if (currentState_ == GameState::VICTORY || currentState_ == GameState::GAMEOVER) {
        return; // 游戏结束不再更新
    }

    if (currentState_ == GameState::CHOOSE) {
        HandleMenuInput();
        return;
    }

    // 1. 玩家输入处理
    InputManager& input = InputManager::Instance();
    RenderCore::Vector2f playerVel = {0, 0};
    const float speed = 5.0f;
    
    if (input.IsKeyPressed(SDL_SCANCODE_A)) {
        playerVel[0] = -speed;
    }
    if (input.IsKeyPressed(SDL_SCANCODE_D)) {
        playerVel[0] = speed;
    }
    if (input.IsKeyPressed(SDL_SCANCODE_W)) {
        playerVel[1] = -speed;
    }
    if (input.IsKeyPressed(SDL_SCANCODE_S)) {
        playerVel[1] = speed;
    }
    player_.SetVelocity(playerVel);
    player_.Update(deltaTime);
    m_playerPos = player_.GetPosition();

    // 2. 处理射击
    HandlePlayerShooting(deltaTime);
    
    // 3. 根据状态更新游戏逻辑
    switch (currentState_) {
        case GameState::NORMAL:
            UpdateNormalPhase(deltaTime);
            break;
        case GameState::BOSS:
            UpdateBossPhase(deltaTime);
            break;
        default:
            break;
    }
    
    // 4. 碰撞检测
    CheckCollisions();
}

void GameScene::HandleMenuInput() {
    InputManager& input = InputManager::Instance();
    // 上下选择
    if (input.IsKeyPressed(SDL_SCANCODE_W) || input.IsKeyPressed(SDL_SCANCODE_UP)) {
        if (menuSelected_ > 0) menuSelected_--;
    }
    if (input.IsKeyPressed(SDL_SCANCODE_S) || input.IsKeyPressed(SDL_SCANCODE_DOWN)) {
        if (menuSelected_ < 2) menuSelected_++;
    }
    // 回车选择
    if ((input.IsKeyPressed(SDL_SCANCODE_RETURN) || input.IsKeyPressed(SDL_SCANCODE_SPACE)) && !menuConfirmed_) {
        menuConfirmed_ = true;
        switch(menuSelected_) {
            case 0: currentDifficulty_ = Difficulty::EASY;   break;
            case 1: currentDifficulty_ = Difficulty::NORMAL; break;
            case 2: currentDifficulty_ = Difficulty::HARD;   break;
        }
        // 初始化难度参数
        InitPlayerLives();
        enemyManager_.SetDifficulty((Game::EnemyManager::Difficulty)currentDifficulty_);
        // 其它管理器、Boss可用难度getter传递
        currentState_ = GameState::NORMAL;
        std::cout << "选择难度: " << (menuSelected_==0?"Easy":menuSelected_==1?"Normal":"Hard") << std::endl;
        std::cout << "========= 游戏开始 =========" << std::endl;
    }
}

void GameScene::InitPlayerLives() {
    // 血量按难度设定
    switch(currentDifficulty_) {
        case Difficulty::EASY:   player_.SetInitialLives(4); break;
        case Difficulty::NORMAL: player_.SetInitialLives(3); break;
        case Difficulty::HARD:   player_.SetInitialLives(2); break;
    }
}

// 分发难度参数
float GameScene::GetEnemySpawnInterval() const {
    switch(currentDifficulty_) {
        case Difficulty::EASY:   return 5.0f;
        case Difficulty::NORMAL: return 3.0f;
        case Difficulty::HARD:   return 2.0f;
    }
    return 3.0f;
}
float GameScene::GetBulletSpeedScale() const {
    switch(currentDifficulty_) {
        case Difficulty::EASY:   return 2.0f;
        case Difficulty::NORMAL: return 3.5f;
        case Difficulty::HARD:   return 5.0f;
    }
    return 3.5f;
}
int GameScene::GetBossHealth() const {
    switch(currentDifficulty_) {
        case Difficulty::EASY:   return 100;
        case Difficulty::NORMAL: return 150;
        case Difficulty::HARD:   return 200;
    }
    return 150;
}
float GameScene::GetBossFireInterval() const {
    switch(currentDifficulty_) {
        case Difficulty::EASY:   return 0.8f;
        case Difficulty::NORMAL: return 0.5f;
        case Difficulty::HARD:   return 0.3f;
    }
    return 0.5f;
}

void GameScene::UpdateNormalPhase(float deltaTime) {
    // 更新子弹和敌机
    bulletManager_.Update(deltaTime);
    enemyManager_.Update(deltaTime, m_playerPos);
    
    // 检查是否达到Boss出现条件
    if (score_ >= BOSS_SCORE_THRESHOLD && !bossSpawned_) {
        currentState_ = GameState::BOSS;
        enemyManager_.GetPool().ForEachActive([](Enemy& enemy) {
            enemy.SetActive(false); // 清除所有敌机
        });
        std::cout << "========= Boss出现！ =========" << std::endl;
        SpawnBoss();
    }
}

void GameScene::UpdateBossPhase(float deltaTime) {
    if (!boss_) return;

    // ✅ 先更新Boss（如果还活着）
    if (boss_->IsActive()) {
        boss_->Update(deltaTime);
        bulletManager_.Update(deltaTime);
    }

    // ✅ 独立检查：即使IsActive为false，也要检测是否死亡
    // 注意：必须在状态未改变时检查一次
    if (boss_->IsDead() && !bossDeathChecked) {
        bossDeathChecked = true;
        currentState_ = GameState::VICTORY;
        std::cout << "========= 胜利！Boss被击败！ =========" << std::endl;
        std::cout << "最终得分: " << score_ << std::endl;
        
        // 清理Boss子弹
        bulletManager_.GetPool().ForEachActive([](Bullet& bullet) {
            if (!bullet.Getisplayerbullet()) {
                bullet.SetActive(false);
            }
        });
    }
    bossDeathChecked = false;
}

void GameScene::SpawnBoss() {
    // 配置Boss的三个阶段
    std::vector<BossPhase> phases;
    
    // ✅ 阶段1: 100%血量 - 密集圆形弹幕 + 3段直线循环移动
    BossPhase phase1;
    phase1.phaseStartHealth = 100.0f;
    phase1.pattern.type = BulletPattern::Type::CIRCULAR;
    phase1.pattern.count = 18;              // 从12增加到18
    phase1.pattern.speed = GetBulletSpeedScale();
    phase1.pattern.angle = 0.0f;
    phase1.pattern.angleDelta = 30.0f * (M_PI / 180.0f); // ✅ 每秒旋转30度
    phase1.fireInterval = GetBossFireInterval();             // 从1.5秒缩短到0.8秒
    
    // ✅ 3段直线轨迹：下→左→右→上 循环
    phase1.trajectories.push_back({TrajectoryType::LINEAR, {400, -50}, {400, 150}});
    phase1.trajectories.push_back({TrajectoryType::LINEAR, {400, 150}, {150, 150}});
    phase1.trajectories.push_back({TrajectoryType::LINEAR, {150, 150}, {400, -50}});
    phase1.trajectorySwitchInterval = 4.0f; // 每4秒切换轨迹
    
    // ✅ 阶段2: 50%血量 - 螺旋弹幕 + 2段圆弧移动
    BossPhase phase2;
    phase2.phaseStartHealth = 75.0f;
    phase2.pattern.type = BulletPattern::Type::SPIRAL;
    phase2.pattern.speed = GetBulletSpeedScale();
    phase2.pattern.spiralSpeed = 270.0f;    // 从180增加到270度/秒
    phase2.pattern.spiralRadius = 40.0f;    // 从30增加到40
    phase2.fireInterval = GetBossFireInterval();             // 从0.8秒缩短到0.5秒
    
    // ✅ 2段圆弧轨迹：左右半圆
    phase2.trajectories.push_back({TrajectoryType::ARC, {400, 150}, {200, 150}, {300, 250}});
    phase2.trajectories.push_back({TrajectoryType::ARC, {200, 150}, {400, 150}, {300, 250}});
    phase2.trajectorySwitchInterval = 4.0f;
    
    // ✅ 阶段3: 50%血量 - 追踪 + 贝塞尔曲线
    BossPhase phase3;
    phase3.phaseStartHealth = 50.0f;
    phase3.pattern.type = BulletPattern::Type::HOMING;
    phase3.pattern.speed = GetBulletSpeedScale();            // 从4增加到5
    phase3.fireInterval = 0.25f;             // 从0.5秒缩短到0.3秒
    
    // ✅ 3段贝塞尔轨迹：复杂S形移动
    phase3.trajectories.push_back({TrajectoryType::BEZIER, {400, 150}, {200, 250}, {{300, 200}, {100, 300}}});
    phase3.trajectories.push_back({TrajectoryType::BEZIER, {200, 250}, {600, 250}, {{300, 300}, {500, 200}}});
    phase3.trajectories.push_back({TrajectoryType::BEZIER, {600, 250}, {400, 150}, {{500, 300}, {300, 200}}});
    phase3.trajectorySwitchInterval = 4.0f;

    // ✅ 阶段4: 25-0% - 激光+混合弹幕
    BossPhase phase4;
    phase4.phaseStartHealth = 25.0f; // 最后25%
    phase4.pattern.type = BulletPattern::Type::LASER;
    phase4.pattern.laserLength = 550.0f;
    phase4.pattern.laserWidth = 5.0f;
    phase4.pattern.warningDuration = 1.5f;
    phase4.pattern.fireDuration = 1.0f;
    phase4.fireInterval = 3.5f;
    phase4.pattern.laserCount = 5;
    phase4.pattern.laserAngleSpread = 60.0f * (M_PI / 180.0f);
    
    phases.push_back(phase1);
    phases.push_back(phase2);
    phases.push_back(phase3);
    phases.push_back(phase4);
    
    // 创建Boss
    boss_ = std::make_unique<Boss>(RenderCore::Vector2f{400, -50}, bulletManager_.GetPool());
    boss_->Initialize({400, -50}, phases, bulletManager_.GetPool(), &m_playerPos);
    boss_->SetActive(true);
    bossSpawned_ = true;
}

void GameScene::HandlePlayerShooting(float deltaTime) {
    InputManager& input = InputManager::Instance();
    
    if (input.IsKeyPressed(SDL_SCANCODE_SPACE)) {
        playerFireTimer_ += deltaTime;
        if (playerFireTimer_ >= PLAYER_FIRE_INTERVAL) {
            playerFireTimer_ = 0.0f;
            
            RenderCore::Vector2f playerPos = player_.GetPosition();
            bulletManager_.Spawn({playerPos[0], playerPos[1] - 12.0f}, 
                               {0, -8.0f}, true);
        }
    } else {
        playerFireTimer_ = PLAYER_FIRE_INTERVAL;
    }
}

void GameScene::Render(RenderCore::RenderEngine& engine) {
    if (currentState_ == GameState::CHOOSE) {
        RenderMenu(engine);
        return;
    }
    // 渲染玩家
    player_.Render(engine);
    
    // 渲染子弹
    bulletManager_.Render(engine);
    
    // 根据状态渲染不同对象
    if (currentState_ == GameState::NORMAL || currentState_ == GameState::BOSS) {
        enemyManager_.Render(engine);
        if (boss_ && boss_->IsActive()) {
            boss_->Render(engine);
        }
    }
    
    // 渲染HUD
    RenderHUD(engine);
}

// 菜单渲染函数
void GameScene::RenderMenu(RenderCore::RenderEngine& engine) {
    // 背景
    engine.set_pen_options({ .color = RenderCore::Colors::DarkGray, .fill_color = RenderCore::Colors::DarkGray });
    engine.add_primitive(RenderCore::make_rectangle({150,200}, {650,450}));
    // 标题
    engine.set_pen_options({ .color = RenderCore::Colors::White, .fill_color = RenderCore::Colors::White });
    engine.add_primitive(RenderCore::make_rectangle({160,210}, {640,240}));
    GameFont::Instance().RenderText(engine, "CHOOSE DIFFICULTY", {210, 220}, RenderCore::Colors::Black, 1.6f);
    // 选项
    const char* opts[3] = { "Easy（3命,慢弹，Boss血低）", "Normal（2命,中弹，Boss血中）", "Hard（1命,快弹，Boss血高）" };
    for(int i=0;i<3;++i) {
        engine.set_pen_options({
            .color = menuSelected_==i ? RenderCore::Colors::Yellow : RenderCore::Colors::White,
            .fill_color = menuSelected_==i ? RenderCore::Colors::Yellow : RenderCore::Colors::White
        });
        engine.add_primitive(RenderCore::make_rectangle({200,250 + i*50}, {600,290 + i*50}));

        // 由于无法直接显示文字，可用线条或小块模拟高亮和位置
    }
    GameFont::Instance().RenderText(engine, "EASY",   {300, 260}, menuSelected_==0 ? RenderCore::Colors::Cyan : RenderCore::Colors::Red, 1.5f);
    GameFont::Instance().RenderText(engine, "NORMAL", {300, 310}, menuSelected_==1 ? RenderCore::Colors::Cyan : RenderCore::Colors::Red, 1.5f);
    GameFont::Instance().RenderText(engine, "HARD",   {300, 360}, menuSelected_==2 ? RenderCore::Colors::Cyan : RenderCore::Colors::Red, 1.5f);
    // 提示
    engine.set_pen_options({ .color = RenderCore::Colors::Red, .fill_color = RenderCore::Colors::Red });
    engine.add_primitive(RenderCore::make_rectangle({250,400},{550,435}));
    GameFont::Instance().RenderText(engine, "ENTER TO START", {260, 410}, RenderCore::Colors::Green, 1.2f);
}

void GameScene::CheckCollisions() {
    CircleCollider playerCollider(player_.GetHitCenter(), player_.GetHitRadius());
    
    // 1. 敌人子弹 vs 玩家
    bulletManager_.GetPool().ForEachActive([this, &playerCollider](Bullet& bullet) {
        if (!bullet.IsActive() || bullet.Getisplayerbullet()) return;                                                                                   

        // ✅ 检查是否为安全状态的预警激光
        if (!bullet.IsDangerous()) return;
        
        // ✅ 激光使用AABB碰撞，其他子弹用圆形
        bool hit = false;
        if (bullet.IsLaser()) {
            LineSegmentCollider laserCollider = bullet.GetLaserSegment();
            hit = CollisionSystem::Check(laserCollider, playerCollider);
        } else {
            CircleCollider bulletCollider(bullet.GetPosition(), bullet.GetRadius());
            hit = CollisionSystem::Check(playerCollider, bulletCollider);
        }
        
        // 玩家血量系统：被击中后处理
        if (hit && !player_.IsInvincible()) {
            bullet.SetActive(false);
            player_.TakeDamage();
            std::cout << "玩家被弹幕击中！剩余生命: " << player_.GetLives() << std::endl;
            if (player_.GetLives() <= 0) {
                currentState_ = GameState::GAMEOVER;
                std::cout << "========= 失败！你被击中（三次）GAMEOVER！ =========" << std::endl;
            }
        }
    });
    
    // 2. 玩家子弹 vs 敌机
    bulletManager_.GetPool().ForEachActive([this,&playerCollider](Bullet& bullet) {
        if (!bullet.IsActive() || !bullet.Getisplayerbullet()) return;
        
        CircleCollider bulletCollider(bullet.GetPosition(), bullet.GetRadius());
        
        // 普通敌机碰撞
        enemyManager_.GetPool().ForEachActive([this, &bullet, &bulletCollider](Enemy& enemy) {
            if (!enemy.IsActive()) return;
            
            AABBCollider enemyCollider(enemy.GetBounds());
            if (CollisionSystem::Check(enemyCollider, bulletCollider)) {
                enemy.TakeDamage(33.4f);
                bullet.SetActive(false);
                
                if (enemy.IsDead()) {
                    score_ += 100;
                    std::cout << "敌机被摧毁！得分: " << score_ << " (距离Boss还有 " 
                              << (BOSS_SCORE_THRESHOLD - score_) << " 分)" << std::endl;
                }
            }
        });
        
        // Boss碰撞检测
        if (boss_ && boss_->IsActive() && !boss_->IsDead()) {
            AABBCollider bossCollider(boss_->GetBounds());
            if (CollisionSystem::Check(bossCollider, bulletCollider)) {
                boss_->TakeDamage(1.0f); // Boss更耐打
                bullet.SetActive(false);
                std::cout << "Boss剩余血量: " << boss_->GetHealth() << std::endl;
            }
        }
    });
    
    // 3. 敌机 vs 玩家碰撞
    enemyManager_.GetPool().ForEachActive([this, &playerCollider](Enemy& enemy) {
        if (!enemy.IsActive()) return;
        
        AABBCollider enemyCollider(enemy.GetBounds());
        // 玩家血量系统：与敌机相撞扣一次
        if (CollisionSystem::Check(enemyCollider, playerCollider) && !player_.IsInvincible()) {
            player_.TakeDamage();
            std::cout << "玩家与敌机相撞！剩余生命: " << player_.GetLives() << std::endl;
            if (player_.GetLives() <= 0) {
                currentState_ = GameState::GAMEOVER;
                std::cout << "========= 失败！你与敌机相撞！GAMEOVER！ =========" << std::endl;
                std::cout << "最终得分: " << score_ << std::endl;
            }
        }
    });
    
    // 4. Boss vs 玩家碰撞
    if (boss_ && boss_->IsActive() && !boss_->IsDead()) {
        AABBCollider bossCollider(boss_->GetBounds());
        if (CollisionSystem::Check(bossCollider, playerCollider) && !player_.IsInvincible()) {
            player_.TakeDamage();
            std::cout << "玩家与Boss相撞！剩余生命: " << player_.GetLives() << std::endl;
            if (player_.GetLives() <= 0) {
                currentState_ = GameState::GAMEOVER;
                std::cout << "========= 失败！你与Boss相撞！GAMEOVER！ =========" << std::endl;
                std::cout << "最终得分: " << score_ << std::endl;
            }
        }
    }
}

void GameScene::RenderHUD(RenderCore::RenderEngine& engine) {

    // 生命图标渲染（左上角爱心）
    int lives = player_.GetLives();
    int maxLives = player_.GetMaxLives();

    for (int i = 0; i < maxLives; ++i) {
        int x = 80 + i * 28; int y = 45;
        bool activeLife = (i < lives);

        // 爱心绘制（两个矩形+下方三角组合）
        engine.set_pen_options({
            .color = activeLife ? RenderCore::Colors::Blue : RenderCore::Colors::DarkGray,
            .fill_color = activeLife ? RenderCore::Colors::Blue : RenderCore::Colors::DarkGray
        });
        // 左半心
        engine.add_primitive(RenderCore::make_rectangle({x, y}, {x + 10, y + 14}));
        // 右半心
        engine.add_primitive(RenderCore::make_rectangle({x + 10, y}, {x + 20, y + 14}));
        // 下方三角
        RenderCore::Polygon tri;
        tri.emplace_back(x, y + 10);
        tri.emplace_back(x + 20, y + 10);
        tri.emplace_back(x + 10, y + 26);
        engine.add_primitive(tri);
    }
    
    // 绘制状态提示
    if (currentState_ == GameState::NORMAL) {
        // 普通阶段：显示进度条
        int bossProgress = (score_ * 100) / BOSS_SCORE_THRESHOLD;
        engine.set_pen_options({
            .color = RenderCore::Colors::Yellow,
            .fill_color = RenderCore::Colors::Yellow
        });
        engine.add_primitive(RenderCore::make_rectangle(
            {10, 10},
            {210, 18}
        ));
        
        engine.set_pen_options({
            .color = RenderCore::Colors::Red,
            .fill_color = RenderCore::Colors::Red
        });
        engine.add_primitive(RenderCore::make_rectangle(
            {10, 10},
            {10 + (bossProgress * 2), 18}
        ));
    } else if (currentState_ == GameState::BOSS) {
        // Boss阶段：显示Boss血量
        if (boss_ && boss_->IsActive()) {
            float bossHealthPercent = boss_->GetHealth() / 100.0f;
            engine.set_pen_options({
                .color = RenderCore::Colors::Red,
                .fill_color = RenderCore::Colors::Red
            });
            engine.add_primitive(RenderCore::make_rectangle(
                {300, 10},
                {500, 20}
            ));
            
            engine.set_pen_options({
                .color = RenderCore::Colors::Green,
                .fill_color = RenderCore::Colors::Green
            });
            engine.add_primitive(RenderCore::make_rectangle(
                {300, 10},
                {300 + static_cast<int>(200 * bossHealthPercent), 20}
            ));
        }
    } else if (currentState_ == GameState::GAMEOVER) {
        // 失败提示
        engine.set_pen_options({
            .color = RenderCore::Colors::Blue,
            .fill_color = RenderCore::Colors::Blue
        });
        engine.add_primitive(RenderCore::make_rectangle({250, 250}, {550, 350}));
        
        engine.set_pen_options({
            .color = RenderCore::Colors::White,
        });
        engine.add_primitive(RenderCore::make_rectangle({260, 260}, {540, 340}));
        GameFont::Instance().RenderText(engine, "GAME", {320, 270}, RenderCore::Colors::Red, 1.5f);
        GameFont::Instance().RenderText(engine, "OVER", {325, 300}, RenderCore::Colors::Red, 1.5f);
        
        // 显示重新开始提示
        GameFont::Instance().RenderText(engine, "ESC TO FRONTPAGE", {335, 330}, RenderCore::Colors::Yellow, 1.5f);

    } else if (currentState_ == GameState::VICTORY) {
        // 胜利提示
        engine.set_pen_options({
            .color = RenderCore::Colors::Green,
            .fill_color = RenderCore::Colors::Green
        });
        engine.add_primitive(RenderCore::make_rectangle({250, 250}, {550, 350}));
        
        engine.set_pen_options({
            .color = RenderCore::Colors::White,
        });
        engine.add_primitive(RenderCore::make_rectangle({260, 260}, {540, 340}));
        GameFont::Instance().RenderText(engine, "YOU WIN", {355, 270}, RenderCore::Colors::Green, 2.2f);
        GameFont::Instance().RenderText(engine, "ESC TO FRONTPAGE", {340, 310}, RenderCore::Colors::Yellow, 1.5f);
    }
}

} // namespace Game