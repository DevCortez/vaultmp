#include "BaseContainer.h"

unordered_map<unsigned int, vector<const BaseContainer*>> BaseContainer::baseContainers;

BaseContainer::BaseContainer(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 5)
		throw VaultException("Malformed input database (base containers): %s", table.c_str());

	unsigned int dlc = static_cast<unsigned int>(sqlite3_column_int(stmt, 4));
	// if DLC enabled

	dlc <<= 24;

	baseID = static_cast<unsigned int>(sqlite3_column_int(stmt, 0));
	item = static_cast<unsigned int>(sqlite3_column_int(stmt, 1));
	count = static_cast<unsigned int>(sqlite3_column_int(stmt, 2));
	condition = sqlite3_column_double(stmt, 3) * 100.0;

	if (!condition)
		condition = 100.0;

	if (item & 0xFF000000)
	{
		item &= 0x00FFFFFF;
		item |= dlc;
	}

	// leveld items not implemented yet
	// STAT, MSTT: some "Test" containers contain illegal stuff
	if (!Record::Lookup(item).GetType().compare("LVLI") || !Record::Lookup(item).GetType().compare("STAT") || !Record::Lookup(item).GetType().compare("MSTT"))
		return;

	if (baseID & 0xFF000000)
	{
		baseID &= 0x00FFFFFF;
		baseID |= dlc;
	}
	else
		baseContainers[baseID].erase(remove_if(baseContainers[baseID].begin(), baseContainers[baseID].end(), [=](const BaseContainer* container) { return container->GetItem() == item; }), baseContainers[baseID].end());

	baseContainers[baseID].emplace_back(this);
}

BaseContainer::~BaseContainer()
{
	auto it = find(baseContainers[baseID].begin(), baseContainers[baseID].end(), this);

	if (it != baseContainers[baseID].end())
		baseContainers[baseID].erase(it);
}

const vector<const BaseContainer*>& BaseContainer::Lookup(unsigned int baseID)
{
	return baseContainers[baseID];
}

unsigned int BaseContainer::GetBase() const
{
	return baseID;
}

unsigned int BaseContainer::GetItem() const
{
	return item;
}

unsigned int BaseContainer::GetCount() const
{
	return count;
}

double BaseContainer::GetCondition() const
{
	return condition;
}
