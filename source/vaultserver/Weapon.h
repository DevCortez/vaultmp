#ifndef WEAPON_H
#define WEAPON_H

#include <unordered_map>

#include "vaultmp.h"
#include "vaultserver.h"
#include "Database.h"

#include "sqlite/sqlite3.h"

using namespace std;

/**
 * \brief Represents a game weapon
 */

class Weapon
{
	private:
		static unordered_map<unsigned int, const Weapon*> weapons;

		unsigned int baseID;
		double damage;
		double reload;
		double rate;
		bool automatic;
		unsigned int ammo;

		Weapon(const Weapon&) = delete;
		Weapon& operator=(const Weapon& p) = delete;

	public:
		static const Weapon& Lookup(unsigned int baseID);

		unsigned int GetBase() const;
		double GetDamage() const;
		double GetReloadTime() const;
		double GetFireRate() const;
		bool IsAutomatic() const;
		unsigned int GetAmmo() const;

		Weapon(const string& table, sqlite3_stmt* stmt);
		// must never be called. only defined because vector requires it
		Weapon(Weapon&&) { terminate(); }
		Weapon& operator=(Weapon&&) = delete;
		~Weapon();
};

#endif
