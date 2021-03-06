#ifndef VALUE_H
#define VALUE_H

#include "vaultmp.h"
#include "Lockable.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace std;

/**
 * \brief A container class which simply stores a variable of type T
 *
 * Derives from Lockable to lock / unlock the data member
 */

template <typename T>
class Value : public Lockable
{
	private:
		T value;

#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		Value& operator=(const Value&) = delete;

	public:
		Value() : value(T()) {};
		Value(const T& t) : value(t) {};
		Value(Value&&) = default;
		Value& operator=(Value&&) = default;
		virtual ~Value() {}

		/**
		 * \brief Sets the value
		 */
		bool set(const T& value)
		{
			if (this->IsLocked())
				return false;

			this->value = value;

			return true;
		}
		/**
		 * \brief Gets the value (a copy)
		 */
		T get() const { return value; }
		/**
		 * \brief Returns a reference to the value
		 */
		T& operator*() { return value; }
		/**
		 * \brief Returns a pointer to the value
		 */
		T* operator->() { return &value; }

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug)
		{
			Value::debug = debug;

			if (debug)
				debug->PrintFormat("Attached debug handler to Value<%s> class", true, typeid(T).name());
		}
#endif
};

#ifdef VAULTMP_DEBUG
template <typename T>
Debug* Value<T>::debug;
#endif

#endif
