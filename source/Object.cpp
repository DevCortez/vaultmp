#include "Object.h"
#include "Container.h"
#include "PacketFactory.h"

RawParameter Object::param_Axis = RawParameter(vector<string>());

#ifdef VAULTMP_DEBUG
Debug* Object::debug;
#endif

#ifdef VAULTMP_DEBUG
void Object::SetDebugHandler(Debug* debug)
{
	Object::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to Object class", true);
}
#endif

Object::Object(unsigned int refID, unsigned int baseID) : Reference(refID, baseID)
{
	initialize();
}

Object::Object(const pDefault* packet) : Reference(0x00000000, 0x00000000)
{
	initialize();

	NetworkID id;
	unsigned int refID, baseID;
	bool changed;
	string name;
	double X, Y, Z, aX, aY, aZ;
	unsigned int cell;
	bool enabled;

	PacketFactory::Access<pTypes::ID_OBJECT_NEW>(packet, id, refID, baseID, changed, name, X, Y, Z, aX, aY, aZ, cell, enabled);

	GameFactory::SetChangeFlag(changed);

	this->SetNetworkID(id);
	this->SetReference(refID);
	this->SetBase(baseID);
	this->SetName(move(name));
	this->SetNetworkPos(Axis_X, X);
	this->SetNetworkPos(Axis_Y, Y);
	this->SetNetworkPos(Axis_Z, Z);
	this->SetAngle(Axis_X, aX);
	this->SetAngle(Axis_Y, aY);
	this->SetAngle(Axis_Z, aZ);
	this->SetNetworkCell(cell);
	this->SetEnabled(enabled);
}

Object::Object(pPacket&& packet) : Object(packet.get())
{

}

Object::~Object()
{

}

void Object::initialize()
{
	vector<unsigned char> data = API::RetrieveAllAxis();

	for (unsigned char _data : data)
	{
		// emplace
		object_Game_Pos.insert(make_pair(_data, Value<double>()));
		object_Network_Pos.insert(make_pair(_data, Value<double>()));
		object_Angle.insert(make_pair(_data, Value<double>()));
	}
}

inline
bool Object::IsValidCoordinate(double C)
{
	return (C != 2048.0 && C != 128.0 && C != 0.0);
}

inline
bool Object::IsValidAngle(unsigned char axis, double A)
{
	return (axis == Axis_Z ? (A >= 0.0 && A <= 360.0) : (A >= -90.0 && A <= 90.0));
}

const RawParameter& Object::Param_Axis()
{
	if (param_Axis.get().empty())
		param_Axis = API::RetrieveAllAxis_Reverse();

	return param_Axis;
}

string Object::GetName() const
{
	return object_Name.get();
}

double Object::GetGamePos(unsigned char axis) const
{
	return object_Game_Pos.at(axis).get();
}

double Object::GetNetworkPos(unsigned char axis) const
{
	return object_Network_Pos.at(axis).get();
}

double Object::GetAngle(unsigned char axis) const
{
	return object_Angle.at(axis).get();
}

bool Object::GetEnabled() const
{
	return state_Enabled.get();
}

unsigned int Object::GetGameCell() const
{
	return cell_Game.get();
}

unsigned int Object::GetNetworkCell() const
{
	return cell_Network.get();
}

Lockable* Object::SetName(const string& name)
{
	return SetObjectValue(this->object_Name, name);
}

Lockable* Object::SetGamePos(unsigned char axis, double pos)
{
	if (!IsValidCoordinate(pos))
		return nullptr;

	return SetObjectValue(this->object_Game_Pos.at(axis), pos);
}

Lockable* Object::SetNetworkPos(unsigned char axis, double pos)
{
	if (!IsValidCoordinate(pos))
		return nullptr;

	return SetObjectValue(this->object_Network_Pos.at(axis), pos);
}

Lockable* Object::SetAngle(unsigned char axis, double angle)
{
	if (!IsValidAngle(axis, angle))
		return nullptr;

	return SetObjectValue(this->object_Angle.at(axis), angle);
}

Lockable* Object::SetEnabled(bool state)
{
	return SetObjectValue(this->state_Enabled, state);
}

Lockable* Object::SetGameCell(unsigned int cell)
{
	if (!cell)
		return nullptr;

	return SetObjectValue(this->cell_Game, cell);
}

Lockable* Object::SetNetworkCell(unsigned int cell)
{
	if (!cell)
		return nullptr;

	return SetObjectValue(this->cell_Network, cell);
}

VaultVector Object::vvec() const
{
	return VaultVector(GetGamePos(Axis_X), GetGamePos(Axis_Y), GetGamePos(Axis_Z));
}

bool Object::IsNearPoint(double X, double Y, double Z, double R) const
{
	return this->vvec().IsNearPoint(VaultVector(X, Y, Z), R);
}

bool Object::IsCoordinateInRange(unsigned char axis, double pos, double R) const
{
	return (GetGamePos(axis) > (pos - R) && GetGamePos(axis) < (pos + R));
}

pair<double, double> Object::GetOffset(double N) const
{
	return this->vvec().GetOffset(GetAngle(Axis_Z), N);
}

bool Object::HasValidCoordinates() const
{
	for (const auto& pos : object_Network_Pos)
		if (!IsValidCoordinate(pos.second.get()))
			return false;

	return true;
}

pPacket Object::toPacket() const
{
	pPacket packet = PacketFactory::Create<pTypes::ID_OBJECT_NEW>(const_cast<Object*>(this)->GetNetworkID(), this->GetReference(), this->GetBase(), this->GetChanged(),
		this->GetName(), this->GetNetworkPos(Values::Axis_X), this->GetNetworkPos(Values::Axis_Y), this->GetNetworkPos(Values::Axis_Z),
		this->GetAngle(Values::Axis_X), this->GetAngle(Values::Axis_Y), this->GetAngle(Values::Axis_Z), this->GetNetworkCell(), this->GetEnabled());

	return packet;
}

vector<string> ObjectFunctor::operator()()
{
	vector<string> result;
	NetworkID id = get();

	if (id)
	{
		FactoryObject reference = GameFactory::GetObject(id);
		Reference* object = vaultcast<Object>(reference);

		if (object)
		{
			unsigned int flags = this->flags();

			if (flags & FLAG_REFERENCE)
				result.emplace_back(Utils::toString(object->GetReference()));
			else if (flags & FLAG_BASE)
				result.emplace_back(Utils::toString(object->GetBase()));
		}
	}
	else
	{
		vector<FactoryObject>::iterator it;
		vector<FactoryObject> references = GameFactory::GetObjectTypes(ID_OBJECT);
		unsigned int refID;

		for (it = references.begin(); it != references.end(); GameFactory::LeaveReference(*it), ++it)
			if ((refID = (*it)->GetReference()) && !filter(*it))
				result.emplace_back(Utils::toString(refID));
	}

	_next(result);

	return result;
}

bool ObjectFunctor::filter(FactoryObject& reference)
{
	Object* object = vaultcast<Object>(reference);
	unsigned int flags = this->flags();

	if (flags & FLAG_NOTSELF && object->GetReference() == PLAYER_REFERENCE)
		return true;

	else if (flags & FLAG_SELF && object->GetReference() != PLAYER_REFERENCE)
		return true;

	if (flags & FLAG_ENABLED && !object->GetEnabled())
		return true;

	else if (flags & FLAG_DISABLED && object->GetEnabled())
		return true;

	return false;
}
