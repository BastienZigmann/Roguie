# 🧩 Roguie — Top-Down Roguelike Dungeon Crawler (UE 5.5.4)

**Roguie** is a fast-paced, top-down roguelike game developed in **Unreal Engine 5.5.4** using a **mainly focused C++ architecture**. Feature focus is on **procedural generation**, **modular systems**, and **polished gameplay feel**.

> This game is a learning-focused, extendable foundation aimed at mastering clean C++ practices in UE5, while delivering a fluid and replayable roguelike experience.

---

## 🎮 Gameplay Overview

- **Core Loop**: Progress through procedurally generated dungeon floors while defeating enemies and collecting gameplay-altering items.
- **Combat**: 
  - Multiple weapon types (melee, ranged, magic) with unique combos.
  - Two active skills + one ultimate per weapon, with cooldown and conditional unlock mechanics.
  - Combat responsiveness and input chaining for combo fluidity.
- **Mobility**:
  - WASD movement (controller supported), with dash or teleport depending on weapon type.
- **Items**:
  - Frequent minor boosts, rare impactful items (stat changes, skills, visuals, downsides).
  - Mid-run weapon switching and evolution mechanics.
- **Enemies & Bosses**:
  - Modular Behavior-based enemy types (kite, tank, ranged, etc.).
  - Challenging, reactive boss fights.
- **Progression**:
  - Persistent stats (kills, floors cleared, unlocks) saved across runs.

---

## 🧠 Technical Focus

- Fully C++ structured gameplay systems with minimal Blueprint dependency.
- Strong emphasis on modularity, extensibility, and clean architecture.
- Procedural dungeon generation system with no template rooms.
- Modular enemy component system (AI behavior, movement, health, status effects).
- Custom animation systems and input buffer handling for smooth combat.

---

## ✅ Progress Tracking

### ✔️ **Completed**
- Custom C++ character class
- WASD movement + dash/teleport system (stack and cooldown-based)
- Controller support + input remapping foundations
- Weapon system (combo logic, cooldowns, per-weapon config)
- Input buffer and combo chaining
- Basic enemy AI with component-driven architecture
- Enemy health, damage, and death logic
- C++-based animation playback system (no direct Blueprint dependencies)
- Game architecture refactor into modular systems
- Project planning and development tracking infrastructure

### 🛠️ **In Progress**
- Status effect system (stun, stagger, knockback)
- Home made AI behavior states (patrol, chase, attack, idle) (no behavior tree)
- Weapon skill system (2 active + 1 ultimate per weapon)
- Dungeon content (enemy placement, loot spawns)

### 🔜 **Planned**
- Procedural dungeon generator (room structure, randomized layout, scalable logic)
- Item drop system and inventory
- Weapon evolution & skill replacement
- Player UI (health, stamina, cooldowns, inventory)
- Save/load progression system
- Menu systems (main menu, settings, pause menu)
- Meta progression tracking
- Sound system (SFX, music)
- Polish and balancing pass
