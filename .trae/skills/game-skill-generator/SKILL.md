---
name: "game-skill-generator"
description: "Generates a full UE GAS gameplay skill scaffold (C++ classes + config skeleton + editor checklist) for the GroundBlast project. Invoke when user asks to create/add a new game skill/ability, or says '我要加一个新技能'."
---

# GroundBlast 游戏技能生成器 (Game Skill Generator)

## 项目背景与约束（MUST READ FIRST）

本工具为 **GroundBlast (UE GAS Demo) 项目**专用，遵循**"不改现有架构、复用现有基类"**的原则。现有架构采用：
- **基类层级**：`UGameplayAbility` → `UDemoCharacterGameplayAbility`（见 [DemoCharacterGameplayAbility.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/DemoCharacterGameplayAbility.h)） → `UDemoCharacterHoldingAbility`（蓄力/选目标子类，见 [DemoCharacterHoldingAbility.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/DemoCharacterHoldingAbility.h)）
- **数据驱动**：每个技能配对一个 `USkillConfig` 子类（DataAsset），通过 `FGameplayAbilitySpec::SourceObject` 注入（见 [SkillConfig.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/DataAssets/SkillConfig.h)）
- **技能装配**：通过 `FCharacterSkillSlotsRow` 数据表行装配（见 [CharacterSkillSlotsRow.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/DataTables/CharacterSkillSlotsRow.h)），`UDemoAbilitySettings::CharacterSkillTable` 指定全局DT
- **技能授权绑定**：`ADemoCharacter::InitializeSkillDataFromDataTable()` → `OnSkillConfigsLoaded()` → `GrantAndBind` lambda（见 [DemoCharacter.cpp#L446-L495](file:///d:/GroundBlast/Source/Demo/Gameplay/Character/DemoCharacter.cpp#L446-L495)）
- **现有技能参考**（按"相似度/实现复杂度"分类，注意：**只有两个基类**，Complex不是基类，只是 NeedComplexLogic=Yes 时的复杂度标签）：
  - **瞬发型（Instant，基类=UDemoCharacterGameplayAbility）**：`ChaGA_Melee`、`ChaGA_Dash`、`ChaGA_HealthRegen`、`ChaGA_AIMelee`
  - **蓄力/选目标型（Holding，基类=UDemoCharacterHoldingAbility）**：`ChaGA_GroundBlast`（唯一继承HoldingAbility的现有技能）
  - **复杂型（Complex，基类仍=UDemoCharacterGameplayAbility，但 NeedComplexLogic=Yes）**：`ChaGA_FireBlast`（双GameplayEvent监听Pull→Push）、`ChaGA_Laser`（持续通道+MP属性变化委托）

**禁止**：修改上述任何现有基类、修改数据表结构、引入Modifer体系、重构InstantAbility模板类。——保持"原架构实现"。

---

## 触发条件（When to Invoke）

当用户出现以下任一意图时立即调用本Skill：
1. 直接说"我要做一个新技能/新Ability / 加一个XX技能"
2. 描述技能效果并问"怎么实现/怎么加"（如"我想做一个冰霜新星技能，3米范围冰冻减速"）
3. 提到"技能骨架 / 技能脚手架 / skill scaffold / ability boilerplate"

---

## 执行步骤（Standard Workflow）

### Step 1：向用户收集 7 个参数（Missing → AskUserQuestion）

执行前必须集齐以下 7 个字段，缺任何一个都先问：

| # | 参数名 | 枚举/类型 | 说明 |
|---|--------|----------|------|
| 1 | `SkillName` | 英文大驼峰，如 `FrostNova` / `ThunderStrike` / `HealingCircle` | 技能的英文代号（所有文件命名以此为前缀），不要用中文 |
| 2 | `BehaviorMode` | 枚举二选一：`InstantBase`（普通基类）/ `HoldingBase`（蓄力/选目标基类） | **只决定继承哪一个基类**。InstantBase → 继承 `UDemoCharacterGameplayAbility`（90%普通技能选这个）；HoldingBase → 继承 `UDemoCharacterHoldingAbility`（需要蓄力/选位置/用户二次确认才选）。注意：不再有 Complex 这个基类选项 |
| 3 | `NeedComplexLogic` | `Yes / No`（与BehaviorMode正交独立） | **控制生成骨架的复杂度，不影响基类选择**。Yes = 除 Activated/Ended 外，还生成 OnGiveAbility + UAbilityTask_WaitGameplayEvent 回调 + 自定义委托绑定等（对应现有 FireBlast / Laser 级别）；No = 只生成最基础的 Activated + Ended 两个重写函数（对应 Dash/Melee/HealthRegen 级别） |
| 4 | `NeedCustomConfig` | `Yes / No` | 技能是否需要**超出 `USkillConfig` 基类字段**（SummonItemClass / AbilityMaterialInstance / FireMontage）之外的专属配置字段？（例：Dash需要ForwardImpulse→Yes；Melee不需要→No） |
| 5 | `CustomConfigFields` | 字段列表（当NeedCustomConfig=Yes时必填）| 形如 `["float DamageRadius=300.f", "TSubclassOf<UGameplayEffect> FreezeEffect", "FGameplayTag ExplosionEventTag"]` |
| 6 | `SimilarExistingSkill` | 从现有7个里选一个最像的：`Melee / Dash / HealthRegen / GroundBlast / FireBlast / Laser / AIMelee` | 代码生成器会用它的 .h/.cpp 作为骨架模板，替换关键符号 |
| 7 | `DisplayName` | 中文名，如"冰霜新星" / "雷霆一击" | 仅用于编辑器里DataAsset的显示名/UI |

### Step 2：生成 C++ 代码文件（核心产出）

在 [Source/Demo/Gameplay/Abilities/](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/) 目录下生成（或按需生成子集）：

#### 2a) 生成 `ChaGA_{SkillName}.h`
**继承规则（只看 BehaviorMode，与NeedComplexLogic无关）**：
- BehaviorMode=InstantBase → 继承 `UDemoCharacterGameplayAbility`
- BehaviorMode=HoldingBase → 继承 `UDemoCharacterHoldingAbility`

**骨架要求**（必须对齐现有同类技能的结构，以 SimilarExistingSkill 为模板，符号替换）：
- `GENERATED_BODY()`
- 声明并重写核心虚函数（**由两个参数共同决定重写组合**）：
  - **基础组合（NeedComplexLogic=No 时适用）**：
    - BehaviorMode=InstantBase（参考 [ChaGA_Dash.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/ChaGA_Dash.h) 或 [ChaGA_HealthRegen.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/ChaGA_HealthRegen.h)）：通常重写 `OnDemoCharacterGameplayAbilityActivated` + `PreDemoCharacterGameplayAbilityEnded`；有激活条件时再加 `CanActivateAbility`；有特殊动画时再加 `PlayFireMontage()`
    - BehaviorMode=HoldingBase（参考 `ChaGA_GroundBlast.h` + `DemoCharacterHoldingAbility.h`）：重写 `OnDemoCharacterGameplayAbilityActivated` + `PreDemoCharacterGameplayAbilityEnded` + `OnSkillConfirmed()`
  - **复杂扩展（NeedComplexLogic=Yes 时，在上述基础上追加，而非替换）**：
    - 追加声明 `virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;`（参考 [ChaGA_FireBlast.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/ChaGA_FireBlast.h) 里的用法，用于提前缓存Config或绑定Attribute变化委托）
    - 追加需要的 `UAbilityTask_WaitGameplayEvent` 动态回调 UFUNCTION（如 `OnWaitXxxEventTaskReady`）、以及Attribute变化回调（如 `OnMPAttributeChanged`，参考 [ChaGA_Laser.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/ChaGA_Laser.h)）
    - 追加需要的Transient缓存成员（如 `TArray<ADemoCharacter*> CacheOverlapedEnemies`、`ALaserActor* SpawnedLaserActor` 等状态流转变量）
- Transient 成员：缓存 `U{SkillName}Config* {SkillName}Config = nullptr;`（如果 NeedCustomConfig=Yes）

#### 2b) 生成 `ChaGA_{SkillName}.cpp`
**骨架要求**：
- include 顺序必须与现有技能一致（先自己的.h，再DemoAbilitySystemComponent.h、Character.h等）
- `OnDemoCharacterGameplayAbilityActivated` 中**先调用 `Super::`**，然后：
  - 第一步：`Cast<U{SkillName}Config>(RoleSkillConfig)` 拿到专属配置并校验非空
  - 第二步：写 `UE_LOG(LogTemp, Log, TEXT("[{SkillName}] activated"));`
  - 第三步：**留一个 `// TODO: {SkillName} 具体业务逻辑` 注释占位**，下面加一两句最典型的逻辑（如：ApplyEffectToSelf / AddImpulse / SpawnActor 等），不要写死复杂业务
- `PreDemoCharacterGameplayAbilityEnded` 中**先写清理逻辑**，再 `Super::`
- `CanActivateAbility`（如需）中先 `Super::` 再做条件检查
- 所有日志输出要包含技能名

#### 2c) （当 NeedCustomConfig=Yes 时）在 [SkillConfig.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/DataAssets/SkillConfig.h) 中追加新 Config 子类
**不要删改原有子类**，只在文件末尾追加：

```cpp
UCLASS(BlueprintType)
class U{SkillName}Config : public USkillConfig
{
    GENERATED_BODY()
public:
    // 把 CustomConfigFields 展开在这里
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "{SkillName}")
    float DamageRadius = 300.f;
    // ...
};
```
对应 [SkillConfig.cpp](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/DataAssets/SkillConfig.cpp) **不需要修改**（只有一行 `#include "SkillConfig.h"`，保持原样即可）

### Step 3：一次性产出所有代码，不要分步问

参数集齐后，**一次调用完成**：
- Write / Edit 工具写入 C++ 源文件（ChaGA_{SkillName}.h/.cpp）
- （NeedCustomConfig=Yes 时）Edit 工具在 SkillConfig.h 末尾追加 U{SkillName}Config 子类定义

**不要**先生成h再问要不要生成cpp，也不要输出编辑器操作指南——本 Skill 职责范围仅限 C++ 代码生成。

---

## 已生成代码的风格规范（Coding Style）

严格对齐项目现有风格，**不能出现个人风格**：

- include 顺序：先自己的`"ChaGA_Xxx.h"` → 再系统头`<CoreMinimal.h>`等 → 再项目相对路径`"Gameplay/Abilities/..."` → 最后 UE 模块头（`Components/...`、`Abilities/Tasks/...`）
- UCLASS/UFUNCTION/UPROPERTY 宏的括号里不换行，保持 `UCLASS()`
- 函数大括号与函数名同行（和现有代码一致）、指针星号靠近类型（`UDemoCharacter* Character`，不是`UDemoCharacter *Character`）
- Transient 指针成员都加 `UPROPERTY(Transient)` 防止 GC 回收
- 所有 `GetOwnerCharacter()` / `RoleSkillConfig` / `GetAbilitySystemComponent()` 拿到指针后**第一时间 if(nullptr) return**，绝不裸用
- Server/Client 权限判断：写 GE 用 `GetNetMode() < NM_Client`（服务端+Standalone），UI/音效/输入用 `GetNetMode() != NM_DedicatedServer`（客户端+Standalone）——严格和现有代码保持，保持一致
- 命名严格对齐现有：类前缀 `UChaGA_`，Config前缀 `U`，成员函数名按现有虚函数约定

---

## 参考文件索引（Code References，快速跳转）

| 作用 | 路径 |
|------|------|
| Ability 基类 | [DemoCharacterGameplayAbility.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/DemoCharacterGameplayAbility.h) / .cpp |
| Holding 蓄力子类 | [DemoCharacterHoldingAbility.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/DemoCharacterHoldingAbility.h) / .cpp |
| SkillConfig 体系 | [SkillConfig.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/DataAssets/SkillConfig.h) |
| 技能装配数据表 | [CharacterSkillSlotsRow.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/DataTables/CharacterSkillSlotsRow.h) |
| 技能Grant&Bind逻辑 | [DemoCharacter.cpp#L446-L495](file:///d:/GroundBlast/Source/Demo/Gameplay/Character/DemoCharacter.cpp#L446-L495) |
| 全局DT配置 | [DemoAbilitySettings.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Settings/DemoAbilitySettings.h) |
| Instant型技能模板1（Dash） | [ChaGA_Dash.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/ChaGA_Dash.h) / [ChaGA_Dash.cpp](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/ChaGA_Dash.cpp) |
| Instant型技能模板2（HealthRegen，含CanActivate） | [ChaGA_HealthRegen.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/ChaGA_HealthRegen.h) / [ChaGA_HealthRegen.cpp](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/ChaGA_HealthRegen.cpp) |
| Instant型技能模板3（Melee，含PlayFireMontage重写） | [ChaGA_Melee.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/ChaGA_Melee.h) / [ChaGA_Melee.cpp](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/ChaGA_Melee.cpp) |
| Complex型技能模板（FireBlast双GameplayEvent） | [ChaGA_FireBlast.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/ChaGA_FireBlast.h) / [ChaGA_FireBlast.cpp](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/ChaGA_FireBlast.cpp) |
| Complex型技能模板（Laser通道+MP监听） | [ChaGA_Laser.h](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/ChaGA_Laser.h) / [ChaGA_Laser.cpp](file:///d:/GroundBlast/Source/Demo/Gameplay/Abilities/ChaGA_Laser.cpp) |

