#ifndef SKILL_H
#define SKILL_H

#include <string>
#include <vector>

class Character; // ���� ������ ���� ��ȯ ���� ����
class GameState;

class Skill {
public:
    std::string name;
    std::string description;
    int apCost;

    Skill(std::string n, std::string desc, int ap)
        : name(n), description(desc), apCost(ap) {}
    virtual ~Skill() {}

    // �� �ٽ�: �� �Լ��� ��ӹ޾� �ڽ� Ŭ�������� ��¥ ��ų ȿ���� ������
    // caster: ��ų�� ���� ����, state: ���̳� �� ��Ͽ� �����ϱ� ���� ���� ����
    virtual bool execute(Character& caster, GameState& state) = 0;
};

#endif