#include "Weapon.h"

unordered_map<unsigned int, const Weapon*> Weapon::weapons;

Weapon::Weapon(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 7)
		throw VaultException("Malformed input database (weapons): %s", table.c_str());

	unsigned int dlc = static_cast<unsigned int>(sqlite3_column_int(stmt, 6));
	// if DLC enabled

	dlc <<= 24;

	baseID = static_cast<unsigned int>(sqlite3_column_int(stmt, 0));
	damage = sqlite3_column_double(stmt, 1);
	reload = sqlite3_column_double(stmt, 2);
	rate = sqlite3_column_double(stmt, 3);
	automatic = static_cast<bool>(sqlite3_column_int(stmt, 4));
	ammo = sqlite3_column_double(stmt, 5);

	if (ammo & 0xFF000000)
	{
		ammo &= 0x00FFFFFF;
		ammo |= dlc;
	}

	if (baseID & 0xFF000000)
	{
		baseID &= 0x00FFFFFF;
		baseID |= dlc;
	}
	else
		weapons.erase(baseID);

	weapons.emplace(baseID, this);
}

Weapon::~Weapon()
{
	weapons.erase(baseID);
}

const Weapon& Weapon::Lookup(unsigned int baseID)
{
	auto it = weapons.find(baseID);

	if (it != weapons.end())
		return *it->second;

	throw VaultException("No weapon with baseID %08X found", baseID);
}

unsigned int Weapon::GetBase() const
{
	return baseID;
}

double Weapon::GetDamage() const
{
	return damage;
}

double Weapon::GetReloadTime() const
{
	return reload;
}

double Weapon::GetFireRate() const
{
	return rate;
}

bool Weapon::IsAutomatic() const
{
	return automatic;
}

unsigned int Weapon::GetAmmo() const
{
	return ammo;
}
