#pragma once

#define DECLARE_COMPONENT_POOL(TYPE, SIZE) \


#define BEGIN_DECLARE_COMPONENT(TYPENAME) \
class TYPENAME : public IComponent { \
public: \
static char* GetComponentTypeName(){return #TYPENAME;}\
static UINT GetComponentType() {return s_typeID;} \
UINT GetTypeID()const override {return s_typeID;} \
private: static UINT s_typeID;

#define BEGIN_DECLARE_COMPONENT_DERIVE(TYPENAME, PARENTNAME) \
class TYPENAME : public PARENTNAME { \
public: \
static char* GetComponentTypeName(){return #TYPENAME;}\
static UINT GetComponentType() {return s_typeID;} \
UINT GetTypeID()const override {return s_typeID;} \
private: static UINT s_typeID;

#define END_DECLARE_COMPONENT };

#define DEFINE_COMPONENT_TYPE_ID(TYPENAME) \
UINT TYPENAME::s_typeID = ComponentPool::GetComponentTypeID()
