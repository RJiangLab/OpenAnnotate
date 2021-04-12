#ifndef _IOBJECT_H_
#define _IOBJECT_H_

struct Object
{
	virtual const char * GetName() {
		return "Object";
	}
};

struct Interface : public Object
{
	virtual const char * GetName() {
		return "Interface";
	}
};

#endif
