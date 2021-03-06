#include <chrono>
#include <cstring>
#include <deque>
#include <exception>
#include <map>
#include <random>
#include <set>
#include <vector>

#include "MyStrategy.hpp"
#include <cassert>
MyStrategy::MyStrategy() {}

class HfsmNode;

enum class NodeType
{
	HfsmNode,
	MinerNode,
	Repairer,
	BuilderNode,
	DaemonNode,
	GuardNode,
	AttackNode,
	EscaperNode
};

namespace HfsmData
{
	int minerCounter = 0;
	std::map<int, std::shared_ptr<HfsmNode>> hfsmStates;
	std::map<int, Entity> viewById;
	PlayerView playerView;
	Player player;
	EntityProperties BUILDER_UNIT;
	EntityProperties HOUSE;
	EntityProperties MELEE_BASE;
	EntityProperties MELEE_UNIT;
	EntityProperties RANGED_BASE;
	EntityProperties RANGED_UNIT;
	EntityProperties RESOURCE;
	EntityProperties WALL;
	EntityProperties TURRET;
	bool vis[305][305], visM[305][305], fog[305][305];
	int dis(Entity x, Entity y)
	{
		return abs(x.position.x - y.position.x) + abs(x.position.y - y.position.y);
	}
	int dis(Vec2Int x, Vec2Int y)
	{
		return abs(x.x - y.x) + abs(x.y - y.y);
	}
	bool canFull(Vec2Int pos, int size)
	{
		for (int i = pos.x; i < pos.x + size; i++)
		{
			for (int j = pos.y; j < pos.y + size; j++)
			{
				if (!fog[i][j])
					return 0;
				if (vis[i][j])
					return 0;
				if (i < 0 || i >= playerView.mapSize)
					return 0;
				if (j < 0 || j >= playerView.mapSize)
					return 0;
			}
		}
		return 1;
	}
	void makeFull(Vec2Int pos, int size)
	{
		for (int i = pos.x; i < pos.x + size; i++)
			for (int j = pos.y; j < pos.y + size; j++)
				vis[i][j] = 1;
	}
	bool canFullM(Vec2Int pos, int size)
	{
		for (int i = pos.x; i < pos.x + size; i++)
		{
			for (int j = pos.y; j < pos.y + size; j++)
			{
				if (!fog[i][j])
					return 0;
				if (visM[i][j])
					return 0;
				if (i < 0 || i >= playerView.mapSize)
					return 0;
				if (j < 0 || j >= playerView.mapSize)
					return 0;
			}
		}
		return 1;
	}
	void makeFullM(Vec2Int pos, int size)
	{
		for (int i = pos.x; i < pos.x + size; i++)
			for (int j = pos.y; j < pos.y + size; j++)
				visM[i][j] = 1;
	}
	Vec2Int finRound(Vec2Int pos, int size, Vec2Int self)
	{
		std::vector<Vec2Int> v;
		for (int p = 0; p < size; p++)
		{
			if (HfsmData::canFull({ pos.x - 1, pos.y + p }, 1))
				v.push_back(Vec2Int(pos.x - 1, pos.y + p));
			if (HfsmData::canFull({ pos.x + p, pos.y - 1 }, 1))
				v.push_back(Vec2Int(pos.x + p, pos.y - 1));
			if (HfsmData::canFull({ pos.x + size, pos.y + p }, 1))
				v.push_back(Vec2Int(pos.x + size, pos.y + p));
			if (HfsmData::canFull({ pos.x + p, pos.y + size }, 1))
				v.push_back(Vec2Int(pos.x + p, pos.y + size));
		}
		if (!v.size())
			return Vec2Int(0, 0);
		sort(v.begin(), v.end(),
			[=](Vec2Int a, Vec2Int b) { return HfsmData::dis(a, self) > HfsmData::dis(b, self); });
		return v.back();
	}
	Vec2Int finRound(Vec2Int pos, int size)
	{
		for (int p = 0; p < size; p++)
		{
			if (HfsmData::canFull({ pos.x - 1, pos.y + p }, 1))
				return Vec2Int(pos.x - 1, pos.y + p);
			if (HfsmData::canFull({ pos.x + p, pos.y - 1 }, 1))
				return Vec2Int(pos.x + p, pos.y - 1);
			if (HfsmData::canFull({ pos.x + size, pos.y + p }, 1))
				return Vec2Int(pos.x + size, pos.y + p);
			if (HfsmData::canFull({ pos.x + p, pos.y + size }, 1))
				return Vec2Int(pos.x + p, pos.y + size);
		}
		return Vec2Int(0, 0);
	}
	Vec2Int finRoundM(Vec2Int pos, int size, Vec2Int self)
	{
		std::vector<Vec2Int> v;
		for (int p = 0; p < size; p++)
		{
			if (HfsmData::canFullM({ pos.x - 1, pos.y + p }, 1))
				v.push_back(Vec2Int(pos.x - 1, pos.y + p));
			if (HfsmData::canFullM({ pos.x + p, pos.y - 1 }, 1))
				v.push_back(Vec2Int(pos.x + p, pos.y - 1));
			if (HfsmData::canFullM({ pos.x + size, pos.y + p }, 1))
				v.push_back(Vec2Int(pos.x + size, pos.y + p));
			if (HfsmData::canFullM({ pos.x + p, pos.y + size }, 1))
				v.push_back(Vec2Int(pos.x + p, pos.y + size));
		}
		if (!v.size())
			return Vec2Int(0, 0);
		sort(v.begin(), v.end(),
			[=](Vec2Int a, Vec2Int b) { return HfsmData::dis(a, self) > HfsmData::dis(b, self); });
		return v.back();
	}
	Vec2Int finRoundM(Vec2Int pos, int size)
	{
		for (int p = 0; p < size; p++)
		{
			if (HfsmData::canFullM({ pos.x - 1, pos.y + p }, 1))
				return Vec2Int(pos.x - 1, pos.y + p);
			if (HfsmData::canFullM({ pos.x + p, pos.y - 1 }, 1))
				return Vec2Int(pos.x + p, pos.y + size);
			if (HfsmData::canFullM({ pos.x + size, pos.y + p }, 1))
				return Vec2Int(pos.x + size, pos.y + p);
			if (HfsmData::canFullM({ pos.x + p, pos.y + size }, 1))
				return Vec2Int(pos.x + p, pos.y + size);
			if (HfsmData::canFullM({ pos.x + p, pos.y - 1 }, 1))
				return Vec2Int(pos.x + p, pos.y + size);
		}
		return Vec2Int(0, 0);
	}
	int countPopulation()
	{
		int population = 0;
		for (size_t i = 0; i < playerView.entities.size(); i++)
		{
			const Entity& entity = playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)
				continue;
			if (!entity.active)
				continue;
			population -= playerView.entityProperties.at(entity.entityType).populationUse;
			population += playerView.entityProperties.at(entity.entityType).populationProvide;
		}
		return population;
	}
	int countFullPopulation()
	{
		int population = 0;
		for (size_t i = 0; i < playerView.entities.size(); i++)
		{
			const Entity& entity = playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)
				continue;
			if (!entity.active)
				continue;
			population += playerView.entityProperties.at(entity.entityType).populationProvide;
		}
		return population;
	}
	void init()
	{
		sort(playerView.entities.begin(), playerView.entities.end(),
			[](const Entity& a, const Entity& b) { return a.position.x + a.position.y < b.position.x + b.position.y; });
		for (size_t i = 0; i < playerView.players.size(); i++)
		{
			const Player& playerE = playerView.players[i];
			if (playerE.id == playerView.myId)
				player = playerE;
		}
		TURRET = HfsmData::playerView.entityProperties.at(EntityType::BUILDER_BASE);
		BUILDER_UNIT = HfsmData::playerView.entityProperties.at(EntityType::BUILDER_UNIT);
		HOUSE = HfsmData::playerView.entityProperties.at(EntityType::HOUSE);
		MELEE_BASE = HfsmData::playerView.entityProperties.at(EntityType::MELEE_BASE);
		MELEE_UNIT = HfsmData::playerView.entityProperties.at(EntityType::MELEE_UNIT);
		RANGED_BASE = HfsmData::playerView.entityProperties.at(EntityType::RANGED_BASE);
		RANGED_UNIT = HfsmData::playerView.entityProperties.at(EntityType::RANGED_UNIT);
		RESOURCE = HfsmData::playerView.entityProperties.at(EntityType::RESOURCE);
		TURRET = HfsmData::playerView.entityProperties.at(EntityType::TURRET);
		WALL = HfsmData::playerView.entityProperties.at(EntityType::WALL);
		viewById.clear();
		memset(vis, 0, sizeof(vis));
		memset(visM, 0, sizeof(visM));
		for (size_t i = 0; i < playerView.entities.size(); i++)
		{
			const Entity& entity = playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)
				continue;
			int sightRange = playerView.entityProperties.at(entity.entityType).sightRange;
			for (int x = std::max(0, entity.position.x - sightRange); x <= std::min(playerView.mapSize - 1, entity.position.x + sightRange); x++)
			{
				for (int y = std::max(0, entity.position.y - sightRange); y <= std::min(playerView.mapSize - 1, entity.position.y + sightRange); y++)
				{
					if (fog[x][y])
						continue;
					if (dis(entity.position, Vec2Int(x, y)) > sightRange)
						continue;
					fog[x][y] = 1;
				}
			}
		}
		for (size_t i = 0; i < playerView.entities.size(); i++)
		{
			const Entity& entity = playerView.entities[i];
			viewById[entity.id] = entity;
			makeFullM(entity.position, playerView.entityProperties.at(entity.entityType).size);
			if (playerView.entityProperties.at(entity.entityType).canMove)
				continue;
			makeFull(entity.position, playerView.entityProperties.at(entity.entityType).size);
		}
	}
} // namespace HfsmData
class HfsmNode
{
public:
	Entity selfInfo;
	virtual NodeType get_type()
	{
		return NodeType::HfsmNode;
	}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		return nullptr;
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = nullptr;
		std::shared_ptr<BuildAction> buildAction = nullptr;
		std::shared_ptr<RepairAction> repairAction = nullptr;
		std::shared_ptr<AttackAction> attackAction = nullptr;
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
	HfsmNode(Entity entity)
	{
		selfInfo = entity;
	}
	Vec2Int findPos()
	{
		int size = HfsmData::playerView.entityProperties.at(selfInfo.entityType).size;
		return HfsmData::finRoundM(selfInfo.position, size);
	}
};
std::shared_ptr<HfsmNode> init_node(Entity entity);
class EscaperNode : public HfsmNode
{
public:
	std::set<EntityType> noEscape;
	virtual NodeType get_type()
	{
		return NodeType::EscaperNode;
	}
	EscaperNode(Entity entity, std::set<EntityType> noV = {}) : HfsmNode(entity)
	{
		noEscape = noV;
	}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			const Entity& entity = HfsmData::playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId == HfsmData::playerView.myId)
				continue;
			if (noEscape.count(entity.entityType))
				continue;
			if (HfsmData::playerView.entityProperties.at(entity.entityType).attack == nullptr)
				continue;
			if (HfsmData::playerView.entityProperties.at(entity.entityType).attack->attackRange + 2 >=
				HfsmData::dis(selfInfo, entity))
				return nullptr;
		}
		return init_node(selfInfo);
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = nullptr;
		if (selfInfo.position.x > selfInfo.position.y)
			moveAction = std::shared_ptr<MoveAction>(new MoveAction(
				Vec2Int(0, selfInfo.position.y), true, true));
		else
			moveAction = std::shared_ptr<MoveAction>(new MoveAction(
				Vec2Int(selfInfo.position.x, 0), true, true));
		std::shared_ptr<BuildAction> buildAction = nullptr;
		std::shared_ptr<AttackAction> attackAction = nullptr;
		std::shared_ptr<RepairAction> repairAction = nullptr;
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};
class BuildNode : public HfsmNode
{
public:
	int timer;
	BuildNode(Entity entity, int sleepTime = 0) : HfsmNode(entity)
	{
		timer = sleepTime;
	}
};
class TurretRangerOnlyNode : public BuildNode
{
public:
	TurretRangerOnlyNode(Entity entity) : BuildNode(entity) {}
	virtual std::shared_ptr<HfsmNode> get_next_state();
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = nullptr;
		std::shared_ptr<BuildAction> buildAction = nullptr;
		std::shared_ptr<RepairAction> repairAction = nullptr;
		std::vector<EntityType> validAutoAttackTargets = { EntityType::RANGED_UNIT };
		std::shared_ptr<AttackAction> attackAction = std::shared_ptr<AttackAction>(new AttackAction(
			nullptr, std::shared_ptr<AutoAttack>(new AutoAttack(HfsmData::TURRET.sightRange, validAutoAttackTargets))));
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};
class TurretAllNode : public BuildNode
{
public:
	TurretAllNode(Entity entity) : BuildNode(entity) {}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		for (int i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			const Entity& entityE = HfsmData::playerView.entities[i];
			if (entityE.playerId == nullptr || *entityE.playerId != HfsmData::playerView.myId)
				continue;
			if (entityE.entityType != EntityType::RANGED_UNIT)
				continue;
			if (HfsmData::dis(selfInfo.position, entityE.position) <= HfsmData::TURRET.attack->attackRange)
			{
				return std::shared_ptr<HfsmNode>(new TurretAllNode(selfInfo));
			}
		}
		return nullptr;
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = nullptr;
		std::shared_ptr<BuildAction> buildAction = nullptr;
		std::vector<EntityType> validAutoAttackTargets;
		std::shared_ptr<AttackAction> attackAction = std::shared_ptr<AttackAction>(new AttackAction(
			nullptr, std::shared_ptr<AutoAttack>(new AutoAttack(HfsmData::TURRET.sightRange, validAutoAttackTargets))));
		std::shared_ptr<RepairAction> repairAction = nullptr;
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};
class BaseNode : public BuildNode
{
public:
	EntityType BaseType;
	BaseNode(Entity entity) : BuildNode(entity)
	{
		BaseType = HfsmData::playerView.entityProperties.at(selfInfo.entityType).build->options[0];
	}
};
class BuilderBaseNode : public BaseNode
{
public:
	BuilderBaseNode(Entity entity) : BaseNode(entity) {}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		return nullptr;
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = nullptr;
		std::shared_ptr<BuildAction> buildAction = std::shared_ptr<BuildAction>(new BuildAction(BaseType, findPos()));
		int cnt = 0;
		for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			const Entity& entity = HfsmData::playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)
				continue;
			if (entity.entityType == EntityType::BUILDER_UNIT)
				cnt++;
		}
		//if (cnt > HfsmData::countFullPopulation() * 0.8)
		//	buildAction = nullptr;
		std::shared_ptr<RepairAction> repairAction = nullptr;
		std::shared_ptr<AttackAction> attackAction = nullptr;
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};
class RangerBaseNode : public BaseNode
{
public:
	RangerBaseNode(Entity entity) : BaseNode(entity) {}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		return nullptr;
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = nullptr;
		std::shared_ptr<BuildAction> buildAction = nullptr;
		std::shared_ptr<RepairAction> repairAction = nullptr;
		std::shared_ptr<AttackAction> attackAction = nullptr;

		if (timer <= 20)
			buildAction = std::shared_ptr<BuildAction>(new BuildAction(BaseType, findPos()));
		timer--;
		if (timer <= 0)
			timer = 20;

		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};
class MinerNode : public HfsmNode
{
public:
	int mineType;
	std::set<EntityType> noEscape;
	MinerNode(Entity entity) : HfsmNode(entity)
	{
		mineType = HfsmData::minerCounter++;
		noEscape = { EntityType::BUILDER_UNIT };
	}
	virtual NodeType get_type()
	{
		return NodeType::MinerNode;
	}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			const Entity& entity = HfsmData::playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)
				continue;
			if (entity.entityType != EntityType::RANGED_UNIT)
				continue;
			if (HfsmData::dis(selfInfo, entity) <= 2)
				return nullptr;
		}
		for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			const Entity& entity = HfsmData::playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId == HfsmData::playerView.myId)
				continue;
			if (noEscape.count(entity.entityType))
				continue;
			if (HfsmData::playerView.entityProperties.at(entity.entityType).attack == nullptr)
				continue;
			if (HfsmData::playerView.entityProperties.at(entity.entityType).attack->attackRange + 4 >=
				HfsmData::dis(selfInfo, entity))
				return std::shared_ptr<HfsmNode>(new EscaperNode(entity, { EntityType::BUILDER_UNIT }));
		}
		return nullptr;
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = nullptr;
		if (mineType % 3 == 0)
		{
			moveAction = std::shared_ptr<MoveAction>(new MoveAction(
				Vec2Int(HfsmData::playerView.mapSize - 1, HfsmData::playerView.mapSize - 1), true, true));
		}
		if (mineType % 3 == 1)
		{
			moveAction = std::shared_ptr<MoveAction>(new MoveAction(
				Vec2Int(selfInfo.position.x, HfsmData::playerView.mapSize - 1), true, true));
		}
		if (mineType % 3 == 2)
		{
			moveAction = std::shared_ptr<MoveAction>(new MoveAction(
				Vec2Int(HfsmData::playerView.mapSize - 1, selfInfo.position.y), true, true));
		}
		std::shared_ptr<BuildAction> buildAction = nullptr;
		std::vector<EntityType> validAutoAttackTargets = { EntityType::RESOURCE };
		std::shared_ptr<AttackAction> attackAction = std::shared_ptr<AttackAction>(new AttackAction(
			nullptr, std::shared_ptr<AutoAttack>(new AutoAttack(1000, validAutoAttackTargets))));
		std::shared_ptr<RepairAction> repairAction = nullptr;
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};
class RepairerNode : public HfsmNode
{
public:
	int target;
	RepairerNode(Entity entity, int entityE) : HfsmNode(entity)
	{
		target = entityE;
	}
	virtual NodeType get_type()
	{
		return NodeType::Repairer;
	}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		if (!HfsmData::viewById.count(target))
			return std::shared_ptr<HfsmNode>(new MinerNode(selfInfo));
		int maxHealth = HfsmData::playerView.entityProperties.at(HfsmData::viewById[target].entityType).maxHealth;
		if (HfsmData::viewById[target].health == maxHealth)
			return std::shared_ptr<HfsmNode>(new MinerNode(selfInfo));
		return nullptr;
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = nullptr;
		if (HfsmData::dis(selfInfo, HfsmData::viewById[target]) > 1)
		{
			Vec2Int pos = HfsmData::finRoundM(HfsmData::viewById[target].position,
				HfsmData::playerView.entityProperties.at(HfsmData::viewById[target].entityType).size, selfInfo.position);
			moveAction = std::shared_ptr<MoveAction>(new MoveAction(pos, true, true));
		}
		std::shared_ptr<BuildAction> buildAction = nullptr;
		std::shared_ptr<AttackAction> attackAction = nullptr;
		std::shared_ptr<RepairAction> repairAction = std::shared_ptr<RepairAction>(new RepairAction(target));
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};
class BuilderNode : public HfsmNode
{
public:
	EntityType buildType;
	Vec2Int targetPos;
	int timer;
	BuilderNode(Entity entity, EntityType type, Vec2Int pos, int time = 10) : HfsmNode(entity)
	{
		buildType = type;
		targetPos = pos;
		timer = time;
	}
	virtual NodeType get_type()
	{
		return NodeType::BuilderNode;
	}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		timer--;
		if (timer <= 0 || !HfsmData::canFull(targetPos, HfsmData::playerView.entityProperties.at(buildType).size))
			return std::shared_ptr<HfsmNode>(new MinerNode(selfInfo));
		return nullptr;
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = std::shared_ptr<MoveAction>(new MoveAction(
			HfsmData::finRound(targetPos, HfsmData::playerView.entityProperties.at(buildType).size), true, true));
		std::shared_ptr<BuildAction> buildAction = std::shared_ptr<BuildAction>(new BuildAction(
			buildType, targetPos));
		std::shared_ptr<AttackAction> attackAction = nullptr;
		std::shared_ptr<RepairAction> repairAction = nullptr;
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};
class DaemonNode : public HfsmNode
{
public:
	std::set<EntityType> noEscape;
	DaemonNode(Entity entity) : HfsmNode(entity)
	{
		noEscape = { EntityType::RANGED_UNIT, EntityType::BUILDER_UNIT };
	}
	virtual NodeType get_type()
	{
		return NodeType::DaemonNode;
	}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			const Entity& entity = HfsmData::playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId == HfsmData::playerView.myId)
				continue;
			if (noEscape.count(entity.entityType))
				continue;
			if (HfsmData::playerView.entityProperties.at(entity.entityType).attack == nullptr)
				continue;
			if (HfsmData::playerView.entityProperties.at(entity.entityType).attack->attackRange + 1 >=
				HfsmData::dis(selfInfo, entity))
				return std::shared_ptr<HfsmNode>(new EscaperNode(entity, { EntityType::RANGED_UNIT, EntityType::BUILDER_UNIT }));
		}
		return nullptr;
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = nullptr;
		/*
				std::vector<Entity>enemy;
				for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
				{
					const Entity& entity = HfsmData::playerView.entities[i];
					if (entity.playerId == nullptr || *entity.playerId == HfsmData::playerView.myId)continue;
					if (entity.entityType != EntityType::BUILDER_UNIT&&
						HfsmData::playerView.entityProperties.at(entity.entityType).attack!=nullptr)continue;
					enemy.push_back(entity);
				}
				sort(enemy.begin(), enemy.end(),
					[=](Entity x, Entity y) {return HfsmData::dis(x, selfInfo) < HfsmData::dis(x, selfInfo); });
				if(enemy.size()) moveAction = std::shared_ptr<MoveAction>(new MoveAction(
					enemy[0].position, true, true));*/
		for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			const Entity& entity = HfsmData::playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId == HfsmData::playerView.myId)
				continue;
			if (entity.entityType != EntityType::RANGED_UNIT)
				continue;
			if (entity.health <= 0)
				continue;
			if (HfsmData::dis(entity, selfInfo) <= HfsmData::RANGED_UNIT.attack->attackRange)
				moveAction = nullptr;
		}
		std::shared_ptr<BuildAction> buildAction = nullptr;
		std::vector<EntityType> validAutoAttackTargets;
		std::shared_ptr<int> target = nullptr;
		std::shared_ptr<AutoAttack> autoAttack = std::shared_ptr<AutoAttack>(new AutoAttack(
			HfsmData::RANGED_UNIT.sightRange, validAutoAttackTargets));

		for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			Entity& entity = HfsmData::playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId == HfsmData::playerView.myId)
				continue;
			if (entity.entityType != EntityType::RANGED_UNIT)
				continue;
			if (entity.health > 0 && entity.health <= HfsmData::RANGED_UNIT.attack->damage && HfsmData::dis(entity, selfInfo) <= HfsmData::RANGED_UNIT.attack->attackRange)
			{
				target = std::shared_ptr<int>(new int(entity.id));
				entity.health -= HfsmData::RANGED_UNIT.attack->damage;
				break;
			}
		}
		if (target == nullptr)
		{
			for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
			{
				Entity& entity = HfsmData::playerView.entities[i];
				if (entity.playerId == nullptr || *entity.playerId == HfsmData::playerView.myId)
					continue;
				if (entity.entityType != EntityType::RANGED_UNIT)
					continue;
				if (entity.health > 0 && HfsmData::dis(entity, selfInfo) <= HfsmData::RANGED_UNIT.attack->attackRange)
				{
					target = std::shared_ptr<int>(new int(entity.id));
					entity.health -= HfsmData::RANGED_UNIT.attack->damage;
					break;
				}
			}
		}

		if (target != nullptr)
			autoAttack = nullptr;
		std::shared_ptr<AttackAction> attackAction = std::shared_ptr<AttackAction>(new AttackAction(
			target, autoAttack));
		std::shared_ptr<RepairAction> repairAction = nullptr;
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};
class GuardNode : public HfsmNode
{
public:
	std::set<EntityType> noEscape;
	int target;
	int timer;
	GuardNode(Entity entity, int enemy) : HfsmNode(entity)
	{
		timer = 5;
		target = enemy;
		noEscape = { EntityType::RANGED_UNIT, EntityType::TURRET, EntityType::BUILDER_UNIT };
	}
	virtual NodeType get_type()
	{
		return NodeType::GuardNode;
	}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			const Entity& entity = HfsmData::playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId == HfsmData::playerView.myId)
				continue;
			if (noEscape.count(entity.entityType))
				continue;
			if (HfsmData::playerView.entityProperties.at(entity.entityType).attack == nullptr)
				continue;
			if (HfsmData::playerView.entityProperties.at(entity.entityType).attack->attackRange + 1 >=
				HfsmData::dis(selfInfo, entity))
				return std::shared_ptr<HfsmNode>(new EscaperNode(entity,
					{ EntityType::RANGED_UNIT, EntityType::TURRET, EntityType::BUILDER_UNIT }));
		}
		if (!HfsmData::viewById.count(target))
		{
			return std::shared_ptr<HfsmNode>(new DaemonNode(selfInfo));
		}
		for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			const Entity& entity = HfsmData::playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId == HfsmData::playerView.myId)
				continue;
			if (HfsmData::playerView.entityProperties.at(entity.entityType).attack == nullptr)
				continue;
			if (HfsmData::playerView.entityProperties.at(entity.entityType).attack->attackRange >=
				HfsmData::dis(selfInfo, entity))
				return nullptr;
		}
		if (HfsmData::dis(selfInfo, HfsmData::viewById[target]) <= 2)
			timer--;
		if (timer <= 0)
			return std::shared_ptr<HfsmNode>(new DaemonNode(selfInfo));
		return nullptr;
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = std::shared_ptr<MoveAction>(new MoveAction(
			HfsmData::viewById[target].position, true, false));
		if (HfsmData::viewById[target].playerId != nullptr && *HfsmData::viewById[target].playerId == HfsmData::playerView.myId && HfsmData::hfsmStates[target]->get_type() == NodeType::MinerNode)
		{
			int minerType = std::dynamic_pointer_cast<MinerNode>(HfsmData::hfsmStates[target])->mineType % 3;
			if (minerType == 0)
			{
				moveAction->target.x++;
				moveAction->target.y++;
			}
			if (minerType == 1)
			{
				moveAction->target.y++;
			}
			if (minerType == 2)
			{
				moveAction->target.x++;
			}
		}
		for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			const Entity& entity = HfsmData::playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId == HfsmData::playerView.myId)
				continue;
			if (entity.entityType != EntityType::RANGED_UNIT)
				continue;
			if (entity.health <= 0)
				continue;
			if (HfsmData::dis(entity, selfInfo) <= HfsmData::RANGED_UNIT.attack->attackRange)
				moveAction = nullptr;
		}
		std::shared_ptr<BuildAction> buildAction = nullptr;
		std::vector<EntityType> validAutoAttackTargets;
		std::shared_ptr<int> target = nullptr;
		std::shared_ptr<AutoAttack> autoAttack = std::shared_ptr<AutoAttack>(new AutoAttack(
			HfsmData::RANGED_UNIT.sightRange, validAutoAttackTargets));

		for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			Entity& entity = HfsmData::playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId == HfsmData::playerView.myId)
				continue;
			if (entity.entityType != EntityType::RANGED_UNIT)
				continue;
			if (entity.health > 0 && entity.health <= HfsmData::RANGED_UNIT.attack->damage && HfsmData::dis(entity, selfInfo) <= HfsmData::RANGED_UNIT.attack->attackRange)
			{
				target = std::shared_ptr<int>(new int(entity.id));
				entity.health -= HfsmData::RANGED_UNIT.attack->damage;
				break;
			}
		}
		if (target == nullptr)
		{
			for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
			{
				Entity& entity = HfsmData::playerView.entities[i];
				if (entity.playerId == nullptr || *entity.playerId == HfsmData::playerView.myId)
					continue;
				if (entity.entityType != EntityType::RANGED_UNIT)
					continue;
				if (entity.health > 0 && HfsmData::dis(entity, selfInfo) <= HfsmData::RANGED_UNIT.attack->attackRange)
				{
					target = std::shared_ptr<int>(new int(entity.id));
					entity.health -= HfsmData::RANGED_UNIT.attack->damage;
					break;
				}
			}
		}
		if (target != nullptr)
			autoAttack = nullptr;
		std::shared_ptr<AttackAction> attackAction = std::shared_ptr<AttackAction>(new AttackAction(
			target, autoAttack));
		std::shared_ptr<RepairAction> repairAction = nullptr;
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};
class AttackNode : public HfsmNode
{
public:
	std::set<EntityType> noEscape;
	Vec2Int target;
	AttackNode(Entity entity, Vec2Int target) : HfsmNode(entity)
	{
		this->noEscape = { EntityType::RANGED_UNIT, EntityType::BUILDER_UNIT };
		this->target = target;
	}
	virtual NodeType get_type()
	{
		return NodeType::AttackNode;
	}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			const Entity& entity = HfsmData::playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId == HfsmData::playerView.myId)
				continue;
			if (noEscape.count(entity.entityType))
				continue;
			if (HfsmData::playerView.entityProperties.at(entity.entityType).attack == nullptr)
				continue;
			if (HfsmData::playerView.entityProperties.at(entity.entityType).attack->attackRange + 1 >=
				HfsmData::dis(selfInfo, entity))
				return std::shared_ptr<HfsmNode>(new EscaperNode(entity, { EntityType::RANGED_UNIT, EntityType::BUILDER_UNIT }));
		}
		return nullptr;
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = std::shared_ptr<MoveAction>(new MoveAction(
			target, true, true));
		for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			const Entity& entity = HfsmData::playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId == HfsmData::playerView.myId)
				continue;
			if (entity.entityType != EntityType::RANGED_UNIT)
				continue;
			if (entity.health <= 0)
				continue;
			if (HfsmData::dis(entity, selfInfo) <= HfsmData::RANGED_UNIT.attack->attackRange)
				moveAction = nullptr;
		}
		std::shared_ptr<BuildAction> buildAction = nullptr;
		std::vector<EntityType> validAutoAttackTargets;
		std::shared_ptr<int> target = nullptr;
		std::shared_ptr<AutoAttack> autoAttack = std::shared_ptr<AutoAttack>(new AutoAttack(
			HfsmData::RANGED_UNIT.sightRange, validAutoAttackTargets));

		for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			Entity& entity = HfsmData::playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId == HfsmData::playerView.myId)
				continue;
			if (entity.entityType != EntityType::RANGED_UNIT)
				continue;
			if (entity.health > 0 && entity.health <= HfsmData::RANGED_UNIT.attack->damage && HfsmData::dis(entity, selfInfo) <= HfsmData::RANGED_UNIT.attack->attackRange)
			{
				target = std::shared_ptr<int>(new int(entity.id));
				entity.health -= HfsmData::RANGED_UNIT.attack->damage;
				break;
			}
		}
		if (target == nullptr)
		{
			for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
			{
				Entity& entity = HfsmData::playerView.entities[i];
				if (entity.playerId == nullptr || *entity.playerId == HfsmData::playerView.myId)
					continue;
				if (entity.entityType != EntityType::RANGED_UNIT)
					continue;
				if (entity.health > 0 && HfsmData::dis(entity, selfInfo) <= HfsmData::RANGED_UNIT.attack->attackRange)
				{
					target = std::shared_ptr<int>(new int(entity.id));
					entity.health -= HfsmData::RANGED_UNIT.attack->damage;
					break;
				}
			}
		}

		if (target != nullptr)
			autoAttack = nullptr;
		std::shared_ptr<AttackAction> attackAction = std::shared_ptr<AttackAction>(new AttackAction(
			target, autoAttack));
		std::shared_ptr<RepairAction> repairAction = nullptr;
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};

std::shared_ptr<HfsmNode> init_node(Entity entity)
{
	if (entity.entityType == EntityType::HOUSE)
	{
		return std::shared_ptr<HfsmNode>(new BuildNode(entity));
	}
	if (entity.entityType == EntityType::TURRET)
	{
		return std::shared_ptr<HfsmNode>(new TurretAllNode(entity));
	}
	if (entity.entityType == EntityType::BUILDER_BASE)
	{
		return std::shared_ptr<HfsmNode>(new BuilderBaseNode(entity));
	}
	if (entity.entityType == EntityType::RANGED_BASE)
	{
		return std::shared_ptr<HfsmNode>(new RangerBaseNode(entity));
	}
	if (entity.entityType == EntityType::BUILDER_UNIT)
	{
		return std::shared_ptr<HfsmNode>(new MinerNode(entity));
	}
	if (entity.entityType == EntityType::RANGED_UNIT)
	{
		return std::shared_ptr<HfsmNode>(new DaemonNode(entity));
	}
	if (entity.entityType == EntityType::MELEE_UNIT)
	{
		return std::shared_ptr<HfsmNode>(new DaemonNode(entity));
	}
	return std::shared_ptr<HfsmNode>(new HfsmNode(entity));
}
std::vector<Entity> getNodes(NodeType nodeType)
{
	std::vector<Entity> miner;
	for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
	{
		const Entity& entity = HfsmData::playerView.entities[i];
		if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)
			continue;
		if (HfsmData::hfsmStates[entity.id]->get_type() != nodeType)
			continue;
		miner.push_back(entity);
	}
	return miner;
}
std::vector<Entity> getUnits(EntityType entityType)
{
	std::vector<Entity> units;
	for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
	{
		const Entity& entity = HfsmData::playerView.entities[i];
		if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)
			continue;
		if (HfsmData::hfsmStates[entity.id]->selfInfo.entityType != entityType)
			continue;
		units.push_back(entity);
	}
	return units;
}
void repairConvene()
{
	std::vector<Entity> unhealthy;
	for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
	{
		const Entity& entity = HfsmData::playerView.entities[i];
		if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)
			continue;
		if (HfsmData::playerView.entityProperties.at(entity.entityType).canMove)
			continue;
		if (entity.health == HfsmData::playerView.entityProperties.at(entity.entityType).maxHealth)
			continue;
		if (HfsmData::player.resource < 20 && entity.active)
			continue;
		unhealthy.push_back(entity);
	}
	std::vector<Entity> miner = getNodes(NodeType::MinerNode);
	sort(unhealthy.begin(), unhealthy.end(),
		[](Entity a, Entity b) { return a.position.x + a.position.y < b.position.x + b.position.y; });
	for (Entity entity : unhealthy)
	{
		int cnt = 4;
		for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			const Entity& entityE = HfsmData::playerView.entities[i];
			if (entityE.playerId == nullptr || *entityE.playerId != HfsmData::playerView.myId)
				continue;
			if (HfsmData::hfsmStates[entityE.id]->get_type() != NodeType::Repairer)
				continue;
			if (std::dynamic_pointer_cast<RepairerNode>(HfsmData::hfsmStates[entityE.id])->target == entity.id)
				cnt--;
		}
		sort(miner.begin(), miner.end(),
			[=](Entity a, Entity b) { return HfsmData::dis(entity, a) > HfsmData::dis(entity, b); });
		while (miner.size() && cnt-- > 0)
		{
			Entity entityE = miner.back();
			HfsmData::hfsmStates[entityE.id] = std::shared_ptr<HfsmNode>(new RepairerNode(entityE, entity.id));
			miner.pop_back();
		}
	}
}
void buildHouseConvene()
{
	if (HfsmData::player.resource < HfsmData::HOUSE.initialCost)
		return;
	int population = HfsmData::countPopulation();
	for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
	{
		const Entity& entity = HfsmData::playerView.entities[i];
		if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)
			continue;
		population += HfsmData::playerView.entityProperties.at(entity.entityType).populationProvide;
		population -= HfsmData::playerView.entityProperties.at(entity.entityType).populationUse;
	}
	for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
	{
		const Entity& entity = HfsmData::playerView.entities[i];
		if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)
			continue;
		if (HfsmData::hfsmStates[entity.id]->get_type() != NodeType::BuilderNode)
			continue;
		EntityType buildType = std::dynamic_pointer_cast<BuilderNode>(HfsmData::hfsmStates[entity.id])->buildType;
		population += HfsmData::playerView.entityProperties.at(buildType).populationProvide;
	}
	if (population >= 15 && HfsmData::playerView.currentTick < 200)
		return;
	if (population >= 5&&HfsmData::playerView.currentTick>=200)
		return;
	std::vector<Entity> miner = getNodes(NodeType::MinerNode);
	if (miner.size() == 0)
		return;
	for (int i = 0; i <= 23; i += 4)
	{
		for (int j = 0; j <= 23; j += 4)
		{
			//if (i < 5 && j < 5)continue;;
			if (HfsmData::canFullM(Vec2Int(i, j), HfsmData::HOUSE.size))
			{
				sort(miner.begin(), miner.end(),
					[=](Entity a, Entity b) { return HfsmData::dis(Vec2Int(i, j), a.position) > HfsmData::dis(Vec2Int(i, j), b.position); });
				Entity entityE = miner.back();
				HfsmData::hfsmStates[entityE.id] = std::shared_ptr<HfsmNode>(new BuilderNode(
					entityE, EntityType::HOUSE, Vec2Int(i, j)));
				miner.pop_back();
				return;
			}
		}
	}
}
void buildRangerBaseConvene()
{
	if (HfsmData::player.resource < HfsmData::RANGED_BASE.initialCost)
		return;
	for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
	{
		const Entity& entity = HfsmData::playerView.entities[i];
		if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)
			continue;
		if (HfsmData::hfsmStates[entity.id]->get_type() != NodeType::BuilderNode)
			continue;
		EntityType buildType = std::dynamic_pointer_cast<BuilderNode>(HfsmData::hfsmStates[entity.id])->buildType;
		if (buildType == EntityType::RANGED_BASE)
			return;
	}
	for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
	{
		const Entity& entity = HfsmData::playerView.entities[i];
		if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)
			continue;
		if (entity.entityType == EntityType::RANGED_BASE)
			return;
	}
	std::vector<Entity> miner = getNodes(NodeType::MinerNode);
	if (miner.size() == 0)
		return;
	for (int i = 0; i <= 23; i++)
	{
		for (int j = 0; j <= 23; j++)
		{
			if (HfsmData::canFullM(Vec2Int(i, j), HfsmData::RANGED_BASE.size))
			{
				sort(miner.begin(), miner.end(),
					[=](Entity a, Entity b) { return HfsmData::dis(Vec2Int(i, j), a.position) > HfsmData::dis(Vec2Int(i, j), b.position); });
				Entity entityE = miner.back();
				HfsmData::hfsmStates[entityE.id] = std::shared_ptr<HfsmNode>(new BuilderNode(
					entityE, EntityType::RANGED_BASE, Vec2Int(i, j)));
				miner.pop_back();
				return;
			}
		}
	}
}
void buildTurret1Convene()
{
	if (HfsmData::playerView.currentTick < 200 || HfsmData::player.resource < HfsmData::TURRET.initialCost)
		return;
	std::vector<Entity> miner = getNodes(NodeType::MinerNode);
	if (miner.size() == 0)
		return;
	for (int i = 2; i <= 24; i += 5)
	{
		for (int j = 24; j <= 24; j++)
		{
			if (HfsmData::canFullM(Vec2Int(i, j), HfsmData::TURRET.size))
			{
				sort(miner.begin(), miner.end(),
					[=](Entity a, Entity b) { return HfsmData::dis(Vec2Int(i, j), a.position) > HfsmData::dis(Vec2Int(i, j), b.position); });
				Entity entityE = miner.back();
				HfsmData::hfsmStates[entityE.id] = std::shared_ptr<HfsmNode>(new BuilderNode(
					entityE, EntityType::TURRET, Vec2Int(i, j)));
				miner.pop_back();
				return;
			}
		}
	}
	for (int i = 24; i <= 24; i++)
	{
		for (int j = 2; j <= 24; j += 5)
		{
			if (HfsmData::canFullM(Vec2Int(i, j), HfsmData::TURRET.size))
			{
				sort(miner.begin(), miner.end(),
					[=](Entity a, Entity b) { return HfsmData::dis(Vec2Int(i, j), a.position) > HfsmData::dis(Vec2Int(i, j), b.position); });
				Entity entityE = miner.back();
				HfsmData::hfsmStates[entityE.id] = std::shared_ptr<HfsmNode>(new BuilderNode(
					entityE, EntityType::TURRET, Vec2Int(i, j)));
				miner.pop_back();
				return;
			}
		}
	}
}
void buildTurret2Convene()
{
	if (HfsmData::playerView.currentTick < 250 || HfsmData::player.resource < HfsmData::TURRET.initialCost)
		return;
	std::vector<Entity> miner = getNodes(NodeType::MinerNode);
	if (miner.size() == 0)
		return;
	for (int i = 3; i <= 24; i += 5)
	{
		for (int j = 24; j <= 24; j++)
		{
			if (HfsmData::canFullM(Vec2Int(i, j), HfsmData::TURRET.size))
			{
				sort(miner.begin(), miner.end(),
					[=](Entity a, Entity b) { return HfsmData::dis(Vec2Int(i, j), a.position) > HfsmData::dis(Vec2Int(i, j), b.position); });
				Entity entityE = miner.back();
				HfsmData::hfsmStates[entityE.id] = std::shared_ptr<HfsmNode>(new BuilderNode(
					entityE, EntityType::TURRET, Vec2Int(i, j)));
				miner.pop_back();
				return;
			}
		}
	}
	for (int i = 24; i <= 24; i++)
	{
		for (int j = 3; j <= 24; j += 5)
		{
			if (HfsmData::canFullM(Vec2Int(i, j), HfsmData::TURRET.size))
			{
				sort(miner.begin(), miner.end(),
					[=](Entity a, Entity b) { return HfsmData::dis(Vec2Int(i, j), a.position) > HfsmData::dis(Vec2Int(i, j), b.position); });
				Entity entityE = miner.back();
				HfsmData::hfsmStates[entityE.id] = std::shared_ptr<HfsmNode>(new BuilderNode(
					entityE, EntityType::TURRET, Vec2Int(i, j)));
				miner.pop_back();
				return;
			}
		}
	}
}
void buildTurret3Convene()
{
	if (HfsmData::playerView.currentTick < 250 || HfsmData::player.resource < HfsmData::TURRET.initialCost)
		return;
	std::vector<Entity> miner = getNodes(NodeType::MinerNode);
	if (miner.size() == 0)
		return;
	for (int i = 1; i <= 26; i += 5)
	{
		for (int j = 26; j <= 26; j++)
		{
			if (HfsmData::canFullM(Vec2Int(i, j), HfsmData::TURRET.size))
			{
				sort(miner.begin(), miner.end(),
					[=](Entity a, Entity b) { return HfsmData::dis(Vec2Int(i, j), a.position) > HfsmData::dis(Vec2Int(i, j), b.position); });
				Entity entityE = miner.back();
				HfsmData::hfsmStates[entityE.id] = std::shared_ptr<HfsmNode>(new BuilderNode(
					entityE, EntityType::TURRET, Vec2Int(i, j)));
				miner.pop_back();
				return;
			}
		}
	}
	for (int i = 26; i <= 26; i++)
	{
		for (int j = 1; j <= 26; j += 5)
		{
			if (HfsmData::canFullM(Vec2Int(i, j), HfsmData::TURRET.size))
			{
				sort(miner.begin(), miner.end(),
					[=](Entity a, Entity b) { return HfsmData::dis(Vec2Int(i, j), a.position) > HfsmData::dis(Vec2Int(i, j), b.position); });
				Entity entityE = miner.back();
				HfsmData::hfsmStates[entityE.id] = std::shared_ptr<HfsmNode>(new BuilderNode(
					entityE, EntityType::TURRET, Vec2Int(i, j)));
				miner.pop_back();
				return;
			}
		}
	}
}
void buildTurret4Convene()
{
	if (HfsmData::playerView.currentTick < 350 || HfsmData::player.resource < HfsmData::TURRET.initialCost)
		return;
	std::vector<Entity> miner = getNodes(NodeType::MinerNode);
	if (miner.size() == 0)
		return;
	for (int i = 3; i <= 26; i += 5)
	{
		for (int j = 26; j <= 26; j++)
		{
			if (HfsmData::canFullM(Vec2Int(i, j), HfsmData::TURRET.size))
			{
				sort(miner.begin(), miner.end(),
					[=](Entity a, Entity b) { return HfsmData::dis(Vec2Int(i, j), a.position) > HfsmData::dis(Vec2Int(i, j), b.position); });
				Entity entityE = miner.back();
				HfsmData::hfsmStates[entityE.id] = std::shared_ptr<HfsmNode>(new BuilderNode(
					entityE, EntityType::TURRET, Vec2Int(i, j)));
				miner.pop_back();
				return;
			}
		}
	}
	for (int i = 26; i <= 26; i++)
	{
		for (int j = 3; j <= 26; j += 5)
		{
			if (HfsmData::canFullM(Vec2Int(i, j), HfsmData::TURRET.size))
			{
				sort(miner.begin(), miner.end(),
					[=](Entity a, Entity b) { return HfsmData::dis(Vec2Int(i, j), a.position) > HfsmData::dis(Vec2Int(i, j), b.position); });
				Entity entityE = miner.back();
				HfsmData::hfsmStates[entityE.id] = std::shared_ptr<HfsmNode>(new BuilderNode(
					entityE, EntityType::TURRET, Vec2Int(i, j)));
				miner.pop_back();
				return;
			}
		}
	}
}
class Group;
int groupCnt = 0;
std::vector<Group> groups;
std::set<int> routeTypes = { 0, 1, 2 };
class Group
{
public:
	int timer = 20;
	int routeType;
	std::vector<Entity> entitys;
	std::deque<Vec2Int> targets;
	Group(std::vector<Entity> entitys, std::deque<Vec2Int> targets, int routeType)
	{
		this->entitys = entitys;
		this->targets = targets;
		this->routeType = routeType;
	}
	void freshEntitys()
	{
		std::vector<Entity> attackers;
		for (int i = 0; i < entitys.size(); i++)
		{
			if (HfsmData::viewById.count(entitys[i].id))
				attackers.push_back(entitys[i]);
		}
		swap(attackers, entitys);
	}
	void dismiss()
	{
		for (Entity attacker : entitys)
		{
			HfsmData::hfsmStates[attacker.id] = std::shared_ptr<HfsmNode>(new DaemonNode(attacker));
		}
	}
	bool convene()
	{
		//if (timer-- <= 0)return 0;
		if (!routeTypes.count(routeType))
			return 0;
		if (!entitys.size())
			return 0;
		while (targets.size())
		{
			bool isClose = 1;
			Vec2Int target = targets.front();
			for (Entity attacker : entitys)
				if (HfsmData::dis(target, HfsmData::viewById[attacker.id].position) > entitys.size() - 1)
					isClose = 0;
			if (isClose)
			{
				timer = 20;
				targets.pop_front();
			}
			else
				break;
		}
		if (!targets.size())
		{
			routeTypes.erase(routeType);
			return 0;
		}
		for (Entity attacker : entitys)
		{
			if (HfsmData::hfsmStates[attacker.id]->get_type() == NodeType::EscaperNode)continue;
			HfsmData::hfsmStates[attacker.id] = std::shared_ptr<HfsmNode>(new AttackNode(
				attacker, targets.front()));
		}
		return 1;
	}
};
void daemonConvene(int limit)
{
	std::vector<Entity> entitys;
	std::vector<Entity> builders = getUnits(EntityType::BUILDER_UNIT);
	entitys.insert(entitys.end(), builders.begin(), builders.end());
	builders = getUnits(EntityType::HOUSE);
	entitys.insert(entitys.end(), builders.begin(), builders.end());
	builders = getUnits(EntityType::TURRET);
	entitys.insert(entitys.end(), builders.begin(), builders.end());
	builders = getUnits(EntityType::BUILDER_BASE);
	entitys.insert(entitys.end(), builders.begin(), builders.end());
	builders = getUnits(EntityType::RANGED_BASE);
	entitys.insert(entitys.end(), builders.begin(), builders.end());
	builders = getUnits(EntityType::MELEE_BASE);
	entitys.insert(entitys.end(), builders.begin(), builders.end());
	std::vector<Entity> unsafe;
	std::map<int, int> cnt;
	for (Entity builder : entitys)
	{
		int dis = 999;
		Entity enemy;
		for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			const Entity& entity = HfsmData::playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId == HfsmData::playerView.myId)
				continue;
			/*
						if (entity.entityType != EntityType::RANGED_UNIT &&
							entity.entityType != EntityType::MELEE_UNIT)
							continue;*/
			int range = 0;
			if (HfsmData::playerView.entityProperties.at(entity.entityType).attack != nullptr)
				range = HfsmData::playerView.entityProperties.at(entity.entityType).attack->attackRange;
			if (dis > HfsmData::dis(entity, builder) - range)
			{
				dis = HfsmData::dis(entity, builder) - range;
				enemy = entity;
			}
		}
		if (dis < limit)
		{
			unsafe.push_back(enemy);
			cnt[enemy.id] = 2;
			if (HfsmData::playerView.entityProperties.at(builder.entityType).canMove)
			{
				unsafe.push_back(builder);
				cnt[builder.id] = 0;
			}
			else
			{
				cnt[enemy.id] = 4;
			}
		}
	}
	sort(unsafe.begin(), unsafe.end(),
		[](Entity a, Entity b) { return a.position.x + a.position.y > b.position.x + b.position.y; });
	std::vector<Entity> daemons = getNodes(NodeType::DaemonNode);
	std::vector<Entity> guards = getNodes(NodeType::GuardNode);
	for (Entity guard : guards)
		cnt[std::dynamic_pointer_cast<GuardNode>(HfsmData::hfsmStates[guard.id])->target]--;
	for (Entity enemy : unsafe)
	{
		sort(daemons.begin(), daemons.end(),
			[=](Entity a, Entity b) { return HfsmData::dis(enemy, a) > HfsmData::dis(enemy, b); });
		while (daemons.size() && cnt[enemy.id]-- > 0)
		{
			Entity guard = daemons.back();
			HfsmData::hfsmStates[guard.id] = std::shared_ptr<HfsmNode>(new GuardNode(guard, enemy.id));
			daemons.pop_back();
		}
	}
}
void groupsConvene()
{
	std::vector<Entity> daemons = getNodes(NodeType::DaemonNode);
	while (routeTypes.size() && daemons.size() >= 2)
	{
		std::vector<Entity> entitys;
		std::deque<Vec2Int> targets;
		while (!routeTypes.count(groupCnt % 3) || (groupCnt % 3 == 2 && routeTypes.size() > 1))
			groupCnt++;
		int routeType = groupCnt % 3;
		for (int i = 0; i < 2; i++)
		{
			entitys.push_back(daemons.back());
			daemons.pop_back();
		}
		if (groupCnt % 3 == 0)
		{
			targets = {
				Vec2Int(50, 1),
				Vec2Int(75, 10),
			};
		}
		if (groupCnt % 3 == 1)
		{
			targets = {
				Vec2Int(1, 50),
				Vec2Int(10, 75),
			};
		}
		if (groupCnt % 3 == 2)
		{
			targets = {
				Vec2Int(30, 30),
				Vec2Int(40, 40),
				Vec2Int(50, 50),
				Vec2Int(60, 60),
				Vec2Int(70, 70),
			};
		}
		groups.push_back(Group(entitys, targets, routeType));
		++groupCnt;
	}
	std::vector<Group> groupsN;
	for (Group group : groups)
	{
		group.freshEntitys();
		if (group.convene())
			groupsN.push_back(group);
		else
			group.dismiss();
	}
	swap(groupsN, groups);
}
void convene()
{
	repairConvene();
	buildHouseConvene();
	buildRangerBaseConvene();
	buildTurret1Convene();
	//buildTurret2Convene();
	buildTurret3Convene();
	buildTurret4Convene();
	daemonConvene(15);
	groupsConvene();
}
Action MyStrategy::getAction(const PlayerView& playerView, DebugInterface* debugInterface)
{
	HfsmData::playerView = playerView;
	HfsmData::init();
	for (size_t i = 0; i < playerView.entities.size(); i++)
	{
		const Entity& entity = playerView.entities[i];
		if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)
			continue;
		if (!HfsmData::hfsmStates.count(entity.id))
			HfsmData::hfsmStates[entity.id] = init_node(entity);
		HfsmData::hfsmStates[entity.id]->selfInfo = entity;
		auto state = HfsmData::hfsmStates[entity.id]->get_next_state();
		if (state != nullptr)
			HfsmData::hfsmStates[entity.id] = state;
	}
	convene();

	Action result = Action(std::unordered_map<int, EntityAction>());
	for (size_t i = 0; i < playerView.entities.size(); i++)
	{
		const Entity& entity = playerView.entities[i];
		if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)
			continue;
		result.entityActions[entity.id] = HfsmData::hfsmStates[entity.id]->get_action();
	}
	return result;
}

void MyStrategy::debugUpdate(const PlayerView& playerView, DebugInterface& debugInterface)
{
	debugInterface.send(DebugCommand::Clear());
	debugInterface.getState();
	debugInterface.send(
		DebugCommand::Add(
			std::shared_ptr<DebugData::Log>(new DebugData::Log("population:" + std::to_string(HfsmData::countFullPopulation())))));
	debugInterface.send(
		DebugCommand::Add(
			std::shared_ptr<DebugData::Log>(new DebugData::Log("guard:" + std::to_string(getNodes(NodeType::GuardNode).size())))));
	debugInterface.send(
		DebugCommand::Add(
			std::shared_ptr<DebugData::Log>(new DebugData::Log("daemon:" + std::to_string(getNodes(NodeType::DaemonNode).size())))));
	debugInterface.send(
		DebugCommand::Add(
			std::shared_ptr<DebugData::Log>(new DebugData::Log("builder:" + std::to_string(getNodes(NodeType::BuilderNode).size())))));
	debugInterface.send(
		DebugCommand::Add(
			std::shared_ptr<DebugData::Log>(new DebugData::Log("minier:" + std::to_string(getNodes(NodeType::MinerNode).size())))));
	debugInterface.send(
		DebugCommand::Add(
			std::shared_ptr<DebugData::Log>(new DebugData::Log("groups:" + std::to_string(groups.size())))));
	debugInterface.send(
		DebugCommand::Add(
			std::shared_ptr<DebugData::Log>(new DebugData::Log("routeType:" + std::to_string(routeTypes.size())))));
}

inline std::shared_ptr<HfsmNode> TurretRangerOnlyNode::get_next_state()
{
	for (int i = 0; i < HfsmData::playerView.entities.size(); i++)
	{
		const Entity& entityE = HfsmData::playerView.entities[i];
		if (entityE.playerId == nullptr || *entityE.playerId != HfsmData::playerView.myId)
			continue;
		if (entityE.entityType != EntityType::RANGED_UNIT)
			continue;
		if (HfsmData::dis(selfInfo.position, entityE.position) > HfsmData::TURRET.attack->attackRange)
		{
			return nullptr;
		}
	}
	return std::shared_ptr<HfsmNode>(new TurretAllNode(selfInfo));
}