#include "Boss.h"
#include "GameState.h"
#include <cstdlib> // rand()
#include <algorithm>

// =========================================================
// [�巡�� ���� ��ų ������]
// =========================================================

// 1. ���׿�: �� �߽� ���� 5x5 ���� ����
class MeteorSkill : public Skill {
public:
    MeteorSkill() : Skill("Meteor", "Deals heavy damage in a 5x5 area.", 0) {}
    bool execute(Character& caster, GameState& state) override {
        state.setLastMessage("Dragon casts METEOR! The sky is falling!");
        // Ÿ�� �߽��� ������ �Ʊ����� ��ų� �߾����� ���� (���⼱ ������ �Ʊ� Ÿ��)
        auto allies = state.getAllies();
        if (allies.empty()) return false;

        Character* target = allies[rand() % allies.size()];
        int tx = target->x;
        int ty = target->y;

        for (auto a : allies) {
            if (a->isAlive() && std::abs(a->x - tx) <= 2 && std::abs(a->y - ty) <= 2) {
                a->takeDamage(10); // �⺻ ���ݷ� �ִ�ġ
                state.addHitEffect(a->x, a->y, 10);
            }
        }
        return true;
    }
};

// 2. �巡�� �Ǿ�: �� ���� ��� �� �ൿ�Ҵ�
class DragonFearSkill : public Skill {
public:
    DragonFearSkill() : Skill("Dragon Fear", "Stuns all enemies for 1 turn.", 0) {}
    bool execute(Character& caster, GameState& state) override {
        state.setLastMessage("ROAR! Dragon Fear paralyzes everyone!");
        for (auto a : state.getAllies()) {
            if (a->isAlive()) {
                a->isStunned = true; // ���� �Ͽ� ������ �� ���� ����
            }
        }
        return true;
    }
};

// 3. ����: ���� ���� Ư�� ���� ������ ��ü ����
class EarthquakeSkill : public Skill {
public:
    EarthquakeSkill() : Skill("Earthquake", "Linear damage across the map.", 0) {}
    bool execute(Character& caster, GameState& state) override {
        state.setLastMessage("Dragon slams the ground! EARTHQUAKE!");
        // ��: Y�� ������ ����
        for (auto a : state.getAllies()) {
            if (a->isAlive() && (a->x >= 4 && a->x <= 10)) { // ���� �ʺ��� ���� ����
                a->takeDamage(8);
                state.addHitEffect(a->x, a->y, 8);
            }
        }
        return true;
    }
};

// 4. ������: �¿� �о�� (�˹�+����)
class WingFlapSkill : public Skill {
public:
    WingFlapSkill() : Skill("Wing Flap", "Knocks back and stuns adjacent enemies on Left/Right.", 0) {}
    bool execute(Character& caster, GameState& state) override {
        // �α� ������ ����(����) ȿ���� ���ԵǾ����� ����
        state.setLastMessage("Dragon flaps its massive wings! You are knocked back and STUNNED!");

        for (auto a : state.getAllies()) {
            if (!a->isAlive()) continue;

            // �巡���� Y�� ���� ����(4~10) �ȿ� �ִ��� Ȯ��
            if (a->y >= 4 && a->y <= 10) {
                // ������ �ٷ� ����(x == 3)�� �� �پ����� ��
                if (a->x == 3) {
                    a->x = std::max(0, a->x - 2); // 2ĭ �������� �˹�
                    a->takeDamage(5);             // ������ 5
                    state.addHitEffect(a->x, a->y, 5);
                    a->isStunned = true;          // �� [�߰�] �˹�� �� 1�� ����!
                }
                // ������ �ٷ� ������(x == 11)�� �� �پ����� ��
                else if (a->x == 11) {
                    a->x = std::min(14, a->x + 2); // 2ĭ ���������� �˹�
                    a->takeDamage(5);              // ������ 5
                    state.addHitEffect(a->x, a->y, 5);
                    a->isStunned = true;           // �� [�߰�] �˹�� �� 1�� ����!
                }
            }
        }
        return true;
    }
};


// =========================================================
// [DragonBoss ���� ������]
// =========================================================

DragonBoss::DragonBoss()
    : Character("Dragon", CharacterClass::BOSS_DRAGON, 7, 7, 100, 10, 0, 7, false),
    scaleTurnCounter(0), currentWeakness(WeakPoint::NONE),
    lastSkillName("") // �� [�߰�] ó���� �� ��ų�� �����Ƿ� �� ���ڿ�
{
    def = 3; // �⺻ ���� 3
    isStatusImmune = true; // �� [�нú�: ����] �����̻� �鿪 ���� Ȱ��ȭ!

    // �巡�� ���� ��ų ����
    skills.push_back(new MeteorSkill());
    skills.push_back(new DragonFearSkill());
    skills.push_back(new EarthquakeSkill());
    skills.push_back(new WingFlapSkill());
}

// 7x7 ���� ���� (���߾� 7,7 ����: 4~10 ���� ��ü ����)
bool DragonBoss::isOccupying(int tx, int ty) const {
    return (tx >= 4 && tx <= 10 && ty >= 4 && ty <= 10);
}

// �ǰ� ���� �� ����(����) �нú� ����
// 1. ��ǥ ��� ���� Ÿ�� �ý��� ����
bool DragonBoss::takeDamage(int dmg, int targetX, int targetY) {
    int finalDmg = dmg;
    isStunned = false; // ���� �нú�: �����̻� �鿪

    // �Ʊ��� ������ ��ǥ�� ��Ȯ�� ������ ��ü�� �ִ� Ÿ������ �˻�
    bool hitWeakness = isWeakPointTile(targetX, targetY);

    if (hitWeakness) {
        finalDmg *= 2; // ���� ���� ����: ������ �����ϰ� 2�� ġ��Ÿ ������!
    }
    else {
        finalDmg = (finalDmg - def > 1) ? (finalDmg - def) : 1; // �Ϲ� ������ ���� 3 ����
    }

    hp -= finalDmg;
    return hitWeakness; // ���� ���� ���θ� �����Ͽ� UI �α׿� �ݿ��ǵ��� ��
};

// ���� ���� ����
void DragonBoss::generateWeakPoint(GameState& state) {
    int r = rand() % 4;
    switch (r) {
    case 0: currentWeakness = WeakPoint::UP;    state.setLastMessage("Dragon exposes its Reverse Scale on the TOP!"); break;
    case 1: currentWeakness = WeakPoint::DOWN;  state.setLastMessage("Dragon exposes its Reverse Scale on the BOTTOM!"); break;
    case 2: currentWeakness = WeakPoint::LEFT;  state.setLastMessage("Dragon exposes its Reverse Scale on the LEFT!"); break;
    case 3: currentWeakness = WeakPoint::RIGHT; state.setLastMessage("Dragon exposes its Reverse Scale on the RIGHT!"); break;
    }
}

// ������ �� �ൿ AI
bool DragonBoss::processBossTurn(GameState& state) {
    if (!isAlive()) return false;

    // =========================================================
    // 1. ����(����) 3�� ���� AI ����
    // =========================================================
    if (currentWeakness == WeakPoint::NONE) {
        generateWeakPoint(state);
        scaleTurnCounter = 1; // 1��° ���� ����
    }
    else {
        scaleTurnCounter++;
        // ��Ȯ�� 3�� ���� ������ �� 4��°�� �� �� ���ο� ��ġ�� ���ġ
        if (scaleTurnCounter > 3) {
            generateWeakPoint(state);
            scaleTurnCounter = 1; // ī���� ���� �� �ٽ� 1��° ���� ����
        }
    }

    // =========================================================
    // 2. ������(Wing Flap) �ߵ��� ���� �翷 ��ĵ AI ����
    // =========================================================
    bool isEnemyAdjacentLR = false; // �� �ߺ� ���� ������ ���� ���� (���� �ϳ��� �����մϴ�)
    for (auto a : state.getAllies()) {
        if (a->isAlive() && a->y >= 4 && a->y <= 10) {
            // �ٷ� ����(3)�̳� ������(11)�� �Ʊ��� �ִ��� Ȯ��
            if (a->x == 3 || a->x == 11) {
                isEnemyAdjacentLR = true;
                break;
            }
        }
    }

    // =========================================================
    // 3. ��Ȳ�� ���� ���� ��ų ���� ó��
    // =========================================================
    if (isEnemyAdjacentLR) {
        // �翷�� �Ʊ��� �� �پ��ִٸ� ������ '������' �ߵ�
        for (auto s : skills) {
            if (s->name == "Wing Flap") {
                s->execute(*this, state);
                lastSkillName = s->name;
                return true;
            }
        }
    }

    // �翷�� �ƹ��� ���ٸ� �������� ������ ������ ��ų(���׿�, ���� ��) �� ������ ����
    std::vector<Skill*> availableSkills;
    for (auto s : skills) {
        if (s->name != "Wing Flap" && s->name != lastSkillName) {
            availableSkills.push_back(s);
        }
    }

    if (!availableSkills.empty()) {
        int skillIndex = rand() % availableSkills.size();
        availableSkills[skillIndex]->execute(*this, state);

        // �� ��ų ���! (���� �Ͽ� �� ������)
        lastSkillName = availableSkills[skillIndex]->name;
    }

    return true;
}